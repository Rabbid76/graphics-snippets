#include <stdafx.h>

// OpenGL
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

// glad
//#include <glad/glad.h>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// glfw
#include <GLFW/glfw3.h>

// stb 
#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
//#include <stb_image_write.h>

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
#include <cmath>
#include <math.h>

// project includes
#include <gl/gl_debug.h>
#include <gl/gl_shader.h>

// preprocessor definitions

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif


// GLSL Geometry shader to replace glLineWidth
// https://stackoverflow.com/questions/54686818/glsl-geometry-shader-to-replace-gllinewidth


std::string sh_vert = R"(
#version 330

in vec4 a_position;
uniform mat4  u_modelviewprojection_matrix;

void main()
{
    gl_Position = u_modelviewprojection_matrix * a_position;
}
)";

/*
std::string sh_geom = R"(
#version 330

layout (lines) in;                              // now we can access 2 vertices
layout (triangle_strip, max_vertices = 4) out;  // always (for now) producing 2 triangles (so 4 vertices)

uniform vec2  u_viewportSize;
uniform float u_thickness = 4;

void main()
{
    vec3 ndc_1 = gl_in[0].gl_Position.xyz / gl_in[0].gl_Position.w;
    vec3 ndc_2 = gl_in[1].gl_Position.xyz / gl_in[1].gl_Position.w;

    vec2 dir    = normalize((ndc_2.xy - ndc_1.xy) * u_viewportSize);
    vec2 normal = vec2(-dir.y, dir.x);

    vec3 offset = vec3(normal * u_thickness * 0.5 / u_viewportSize * 2.0, 0.0);

    gl_Position = vec4((ndc_1 + offset) * gl_in[0].gl_Position.w, gl_in[0].gl_Position.w);
    EmitVertex(); 
    gl_Position = vec4((ndc_1 - offset) * gl_in[0].gl_Position.w, gl_in[0].gl_Position.w);
    EmitVertex();
    gl_Position = vec4((ndc_2 + offset) * gl_in[1].gl_Position.w, gl_in[0].gl_Position.w);
    EmitVertex();
    gl_Position = vec4((ndc_2 - offset) * gl_in[1].gl_Position.w, gl_in[0].gl_Position.w);
    EmitVertex();

    EndPrimitive();
}
)";
*/

std::string sh_geom = R"(
#version 330

layout (lines) in;                              // now we can access 2 vertices
layout (triangle_strip, max_vertices = 4) out;  // always (for now) producing 2 triangles (so 4 vertices)

uniform vec2  u_viewportSize;
uniform float u_thickness = 4;

void main()
{
    vec4 p1 = gl_in[0].gl_Position;
    vec4 p2 = gl_in[1].gl_Position;

    vec2 dir    = normalize((p2.xy - p1.xy) * u_viewportSize);
    vec2 offset = vec2(-dir.y, dir.x) * u_thickness / u_viewportSize;

    gl_Position = p1 + vec4(offset.xy * p1.w, 0.0, 0.0);
    EmitVertex();
    gl_Position = p1 - vec4(offset.xy * p1.w, 0.0, 0.0);
    EmitVertex();
    gl_Position = p2 + vec4(offset.xy * p2.w, 0.0, 0.0);
    EmitVertex();
    gl_Position = p2 - vec4(offset.xy * p2.w, 0.0, 0.0);
    EmitVertex();

    EndPrimitive();
}
)";

std::string sh_frag = R"(
#version 330

out vec4 fragColor;
uniform vec4 u_color = vec4(1, 0, 1, 1);

void main()
{
    fragColor = u_color;
}
)";


int main(void)
{
    if ( glfwInit() != GLFW_TRUE )
        throw std::runtime_error( "error initializing glfw" );

    //glfwWindowHint(GLFW_REFRESH_RATE, 10);

    GLFWwindow *wnd = glfwCreateWindow( 800, 600, "OGL window", nullptr, nullptr );
    if ( wnd == nullptr )
    {
        glfwTerminate();
        throw std::runtime_error( "error initializing window" ); 
    }

    glfwMakeContextCurrent(wnd);

    //glfwSwapInterval( 2 );

    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
    OpenGL::CContext context;
    context.Init( debug_level );

    
    OpenGL::ShaderProgramSimple prog(
    {
      { sh_vert, GL_VERTEX_SHADER },
      { sh_geom, GL_GEOMETRY_SHADER },
      { sh_frag, GL_FRAGMENT_SHADER }
    } );
    

    static const std::vector<float> vtriangle
    { 
      -0.866f, -0.5f,    1.0f, 0.0f, 0.0f, 1.0f, 
       0.866f, -0.5f,    1.0f, 1.0f, 0.0f, 1.0f,
       0.0f,    1.0f,    0.0f, 0.0f, 1.0f, 1.0f
    };
    static const std::vector<unsigned int> itriangle{ 0, 1, 2 };

    static const std::vector<float> vquad
    { 
      -0.5f, -0.5f,    1.0f, 0.0f, 0.0f, 1.0f, 
       0.5f, -0.5f,    1.0f, 1.0f, 0.0f, 1.0f,
       0.5f,  0.5f,    0.0f, 1.0f, 0.0f, 1.0f,
      -0.5f,  0.5f,    0.0f, 0.0f, 1.0f, 1.0f
    };
    static const std::vector<unsigned int> iquad{ 0, 1, 2, 3 };

    //std::vector<float>        varray = vtriangle;
    //std::vector<unsigned int> iarray = itriangle;
    std::vector<float>        varray = vquad;
    std::vector<unsigned int> iarray = iquad;

    GLuint ibo;
    glGenBuffers( 1, &ibo );

    GLuint vbo;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    GLenum error = glGetError();

    //glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );
    glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), nullptr, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, GL_ZERO, varray.size()*sizeof(*varray.data()), varray.data() );

    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 6*sizeof(*varray.data()), 0 );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 6*sizeof(*varray.data()), (void*)(2*sizeof(*varray.data())) );
    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, iarray.size()*sizeof(*iarray.data()), iarray.data(), GL_STATIC_DRAW );
      

    GLenum error0 = glGetError();
    GLuint b0 = 111, b1 = 222, b2 = 333;
    glGetVertexAttribIuiv( 0, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &b0 );
    GLenum error1 = glGetError();
    glGetVertexAttribIuiv( 1, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &b1 );
    GLenum error2 = glGetError();
    glGetVertexAttribIuiv( 2, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &b2 );
    GLenum error3 = glGetError();

    prog.Use();       

    GLint loc_mvp = glGetUniformLocation( prog.Prog(), "u_modelviewprojection_matrix" );
    GLint loc_vps = glGetUniformLocation( prog.Prog(), "u_viewportSize" );
    GLint loc_col = glGetUniformLocation( prog.Prog(), "u_color" );
    GLint loc_thi = glGetUniformLocation( prog.Prog(), "u_thickness" );


    while (!glfwWindowShouldClose(wnd))
    {
        static float angle = 1.0f;

        int vpSize[2];
        glfwGetFramebufferSize( wnd, &vpSize[0], &vpSize[1] );
        
        float ascpect = (float)vpSize[0] / (float)vpSize[1];
        float orthoX = ascpect > 1.0f ? ascpect : 1.0f;
        float orthoY = ascpect > 1.0f ? 1.0f : 1.0 / ascpect;

        glm::mat4 project = glm::ortho( -orthoX, orthoX, -orthoY, orthoY, -1.0f, 1.0f );
        
        float orthScale = ascpect > 1.0f ? 600.0 / vpSize[1] : 600.0 / vpSize[0];
        project = glm::scale(project, glm::vec3(orthScale, orthScale, 1.0f));
        
        project = glm::perspective( glm::radians( 120.0f ), ascpect, 0.1f, 10.0f );

        static bool invert = false;
        glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 0.8f * (invert ? -1.0f : 1.0f)),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 model( 1.0f );
        model = glm::translate( model, glm::vec3(0.1f, 0.0f, 0.2f) );
        model = glm::rotate( model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f) );
        angle += 1.0f;
        
        glUniformMatrix4fv( loc_mvp, 1, GL_FALSE, glm::value_ptr(project * view * model) );
        glUniform2f( loc_vps, (float)vpSize[0], (float)vpSize[1] );
        glUniform4f( loc_col, 1.0f, 1.0f, 1.0f, 1.0f );
        glUniform1f( loc_thi, 10.0f );

        glViewport( 0, 0, vpSize[0], vpSize[1] );
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      
        glDrawElements( GL_LINE_LOOP, (GLsizei)iarray.size(), GL_UNSIGNED_INT, nullptr );

        glfwSwapBuffers(wnd);
        glfwPollEvents();
    }

    glfwDestroyWindow( wnd );
    wnd = nullptr;
    glfwTerminate();

    return 0;
}