#include <stdafx.h>

// OpenGL
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// freeglut
#include <GL/freeglut.h>

// stl
#include <vector>
#include <stdexcept>
#include <array>
#include <map>
#include <tuple>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>

#define _USE_MATH_DEFINES
#include <math.h>

// project includes
#include <gl/gl_debug.h>
#include <gl/gl_shader.h>

// preprocessor definitions

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

std::string sh_vert = R"(
#version 460

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;

out vec3 vertPos;
out vec4 vertCol;

uniform mat4 project;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vec4 view_pos = view * model * vec4(inPos, 1.0);
 
    vertCol     = inColor;
		vertPos     = view_pos.xyz;
		gl_Position = project * vec4(view_pos.xyz, 1.0);
}
)";

std::string sh_frag = R"(
#version 460

in vec3 vertPos;
in vec4 vertCol;

out vec4 fragColor;

void main()
{
    fragColor = vertCol;
}
)";


std::string sh_vert_2 = R"(
#version 460

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;

out vec3 vertPos;
out vec4 vertCol;

uniform mat4 project;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vec4 view_pos = view * model * vec4(inPos, 1.0);
 
    vertCol     = inColor;
		vertPos     = view_pos.xyz;
		gl_Position = project * vec4(view_pos.xyz, 1.0);
}
)";

std::string sh_frag_2 = R"(
#version 460

in vec3 vertPos;
in vec4 vertCol;

out uint fragColor;

void main()
{
    fragColor = uint((vertCol.x + vertCol.y + vertCol.z) * 100.0);
}
)";

std::string sh_v_q1 = R"(
#version 460

layout (location = 0) in vec2 inPos;

out vec2 vertPos;

void main()
{
    vertPos     = inPos.xy;
    gl_Position = vec4(inPos.xy, 0.0, 1.0);
}
)";

std::string sh_f_q1 = R"(
#version 330

in vec2 vertPos;

out vec4 fragColor;

uniform sampler2DMS u_texture;
//uniform sampler2D u_texture;

void main()
{
    vec2 uv = vertPos * 0.5 + 0.5;
    vec2 size = textureSize(u_texture);
    vec4 color = texelFetch(u_texture, ivec2(uv*size), 0);
    //vec4 color = texture(u_texture, uv);
    fragColor = color;
    fragColor.x = uv.x;
}
)";

std::string sh_v_q2 = R"(
#version 460

layout (location = 0) in vec2 inPos;

out vec2 vertPos;

void main()
{
    vertPos     = inPos.xy;
    gl_Position = vec4(inPos.xy, 0.0, 1.0);
}
)";

std::string sh_f_q2 = R"(
#version 330

in vec2 vertPos;

out vec4 fragColor;

//uniform sampler2D u_texture;
uniform usampler2D u_texture;

void main()
{
    vec2 uv = vertPos * 0.5 + 0.5;
    uint red = texture(u_texture, uv).x;
    fragColor = vec4(vec3( float(red) / 300.0), 1.0);
    fragColor.x = uv.x;
}
)";

std::chrono::high_resolution_clock::time_point start_time;


void display( void );

#define SAMPLES 4

#define WIDHT 800
#define HEIGHT 600

struct Framebuffer
{
  GLuint fbo = 0;
  GLuint fboColorTexture = 0;
  GLuint fboDepthStencilTexture = 0;
  GLenum textureType = 0;

  Framebuffer(bool multisampled, bool redOnly) {
    int internalFormat = redOnly ? GL_R32UI : GL_RGBA;
    int format = redOnly ? GL_RED_INTEGER : GL_RGBA;
    int dataType = redOnly ? GL_UNSIGNED_INT : GL_UNSIGNED_BYTE;
    int interpolation = redOnly ? GL_NEAREST : GL_LINEAR;

    textureType = multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
    // Prepare FBO textures
    glGenTextures(1, &fboColorTexture);
    glGenTextures(1, &fboDepthStencilTexture);
    if (multisampled) {
       glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fboColorTexture);
       glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, SAMPLES, internalFormat, WIDHT, HEIGHT, false);
       glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
       glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
       glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fboDepthStencilTexture);
       glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, SAMPLES, GL_DEPTH24_STENCIL8, WIDHT, HEIGHT, false);            
       glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
       /*
        textureType = GL_TEXTURE_2D;
        glBindTexture(GL_TEXTURE_2D, fboColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WIDHT, HEIGHT, 0, format, dataType, nullptr); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, fboDepthStencilTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, WIDHT, HEIGHT, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);        
        glBindTexture(GL_TEXTURE_2D, 0);
        */

    } else {
        glBindTexture(GL_TEXTURE_2D, fboColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, WIDHT, HEIGHT, 0, format, dataType, nullptr);            
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interpolation);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interpolation);
        glBindTexture(GL_TEXTURE_2D, fboDepthStencilTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, WIDHT, HEIGHT, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
    }        

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureType, fboColorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, textureType, fboDepthStencilTexture, 0);
    GLenum draw_buffers[]{ GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, draw_buffers);
    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error( "frambuffer inclomplete" );
    }
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  }
};

OpenGL::ShaderProgramSimple *g_prog_draw = nullptr;
OpenGL::ShaderProgramSimple *g_prog_draw_2 = nullptr;
OpenGL::ShaderProgramSimple *g_prog_screen1 = nullptr;
OpenGL::ShaderProgramSimple *g_prog_screen2 = nullptr;

GLuint vao = 0;
GLuint vao_q = 0;

Framebuffer *colorFbo = nullptr;
Framebuffer *counterFbo = nullptr;


int main(int argc, char** argv)
{
    // Initialise FreeGLUT
    glutInit(&argc, argv);

    // Setup OpenGL window properties
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

    // Create window
    glutInitWindowSize(800, 600);
    int wnd = glutCreateWindow("test");
    if ( wnd == 0 )
        throw std::runtime_error( "error initializing window" ); 

    // Register display callback function
    glutDisplayFunc(display);  

    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
    OpenGL::CContext context;
    context.Init( debug_level );

    g_prog_draw = new OpenGL::ShaderProgramSimple(
    {
      { sh_vert, GL_VERTEX_SHADER },
      { sh_frag, GL_FRAGMENT_SHADER }
    } );

    g_prog_draw_2 = new OpenGL::ShaderProgramSimple(
    {
      { sh_vert_2, GL_VERTEX_SHADER },
      { sh_frag_2, GL_FRAGMENT_SHADER }
    } );

    g_prog_screen1 = new OpenGL::ShaderProgramSimple(
    {
      { sh_v_q1, GL_VERTEX_SHADER },
      { sh_f_q1, GL_FRAGMENT_SHADER }
    } );

    g_prog_screen2 = new OpenGL::ShaderProgramSimple(
    {
      { sh_v_q2, GL_VERTEX_SHADER },
      { sh_f_q2, GL_FRAGMENT_SHADER }
    } );

    static const std::vector<float> vquad{ -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f };

    GLuint vbo_q;
    glGenBuffers( 1, &vbo_q );
    glBindBuffer( GL_ARRAY_BUFFER, vbo_q );
    glBufferData( GL_ARRAY_BUFFER, vquad.size()*sizeof(*vquad.data()), nullptr, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, GL_ZERO, vquad.size()*sizeof(*vquad.data()), vquad.data() );

    glGenVertexArrays( 1, &vao_q );
    glBindVertexArray( vao_q );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );


    static const std::vector<float> varray
    { 
      -0.707f, -0.75f,    1.0f, 0.0f, 0.0f, 1.0f, 
       0.707f, -0.75f,    1.0f, 1.0f, 0.0f, 1.0f,
       0.0f,    0.75f,    0.0f, 0.0f, 1.0f, 1.0f
    };

    GLuint vbo;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    //glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );
    glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), nullptr, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, GL_ZERO, varray.size()*sizeof(*varray.data()), varray.data() );

    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 6*sizeof(*varray.data()), 0 );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 6*sizeof(*varray.data()), (void*)(2*sizeof(*varray.data())) );
    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );

    colorFbo = new Framebuffer( true, false );
    counterFbo = new Framebuffer(false, true);

    start_time = std::chrono::high_resolution_clock::now();
    
    glutMainLoop(); 

    glutDestroyWindow(wnd);
    return 0;
}


void display( void )
{
    static float angle = 1.0f;

    int vpSize[2]
    {
        glutGet(GLUT_WINDOW_WIDTH),
        glutGet(GLUT_WINDOW_HEIGHT)
    };
    float ascpect = (float)vpSize[0] / (float)vpSize[1];
    float orthoX = ascpect > 1.0f ? ascpect : 1.0f;
    float orthoY = ascpect > 1.0f ? 1.0f : ascpect;

    glm::mat4 project = glm::ortho( -orthoX, orthoX, -orthoY, orthoY, -1.0f, 1.0f );
        
    static bool invert = false;
    glm::mat4 view = glm::lookAt(
      glm::vec3(0.0f, 0.0f, 0.5f * (invert ? -1.0f : 1.0f)),
      glm::vec3(0.0f, 0.0f, 0.0f),
      glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 model( 1.0f );
    model = glm::translate( model, glm::vec3(0.1f, 0.0f, 1.0f) );
    model = glm::rotate( model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f) );
    angle += 1.0f;

    glViewport( 0, 0, vpSize[0], vpSize[1] );

    //Framebuffer *fbo = colorFbo;
    //OpenGL::ShaderProgramSimple *prog_d = g_prog_draw;
    //OpenGL::ShaderProgramSimple *prog_q = g_prog_screen1;

    Framebuffer *fbo = counterFbo;
    OpenGL::ShaderProgramSimple *prog_d = g_prog_draw_2;
    OpenGL::ShaderProgramSimple *prog_q = g_prog_screen2;

    // draw geometry

    glBindFramebuffer( GL_FRAMEBUFFER, fbo->fbo );
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    prog_d->Use();
    prog_d->SetUniformM44( "project", glm::value_ptr(project) );
    prog_d->SetUniformM44( "view", glm::value_ptr(view) );
    prog_d->SetUniformM44( "model", glm::value_ptr(model) );
      
    glEnable(GL_MULTISAMPLE);
    glBindVertexArray( vao );
    glDrawArrays( GL_TRIANGLES, 0, 3 );
    glBindVertexArray( 0 );
    glDisable(GL_MULTISAMPLE);

    // draw screen space quad

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glActiveTexture( GL_TEXTURE0+1 );
    glBindTexture( fbo->textureType, fbo->fboColorTexture );
    
    prog_q->Use();
    prog_q->SetUniformI1( "u_texture", 1 );

    glBindVertexArray( vao_q );
    glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
    glBindVertexArray( 0 );

    glBindTexture( fbo->textureType, 0 );

    glutSwapBuffers();
    glutPostRedisplay();
}