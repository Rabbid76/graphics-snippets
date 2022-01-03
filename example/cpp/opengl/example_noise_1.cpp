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

#include <stdlib.h>

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


std::string sh_vert = R"(
#version 460 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;

//out vec3 vertPos;
//out vec4 vertCol;

uniform mat4 project;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vec4 view_pos = view * model * vec4(inPos, 1.0);
 
    //vertCol     = inColor;
		//vertPos     = view_pos.xyz;
		gl_Position = project * vec4(view_pos.xyz, 1.0);
}
)";

std::string sh_frag = R"(
#version 460 core

out vec4 FragColor;

uniform float gradient[256];

float fade(float t)
{
    return t*t*t*(t*(t*6.0-15.0)+10.0);
}

/*vec2 grad(vec2 p){
    vec2 v = vec2(gradient[int(p.x)&255],gradient[int(p.y)&255]);
    return normalize(v.xy*2.0 - vec2(1.0));
}
*/

vec2 grad(vec2 p){

    ivec2 i00 = ivec2(int(p.x)&255, int(p.y)&255); 
    vec2  f   = floor(p); 
    
    float vx = mix(gradient[i00.x], gradient[i00.x+1], f.x);
    float vy = mix(gradient[i00.y], gradient[i00.y+1], f.y);

    float a = (vx + vy) * 3.141529;
    return vec2(cos(a), sin(a));
    //return normalize(vec2(vx, vy)*2.0 - vec2(1.0)); 
}

/*
float rand(vec2 co){
    return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

vec2 grad(vec2 p){
    float a = rand(p) * 2.0 * 3.1415926;
    return vec2(cos(a), sin(a)); 
}
*/

float noise(vec2 p){
    vec2 p0 = floor(p);
    vec2 p1 = p0 + vec2(1.0,0.0);
    vec2 p2 = p0 + vec2(0.0,1.0);
    vec2 p3 = p0 + vec2(1.0,1.0);

    vec2 g0 = grad(p0);
    vec2 g1 = grad(p1);
    vec2 g2 = grad(p2);
    vec2 g3 = grad(p3);

    float t0 = p.x - p0.x;
    float fade_t0 = fade(t0);
    float t1 = p.y - p0.y;
    float fade_t1 = fade(t1);

    float p0p1 = (1.0-fade_t0)*dot(g0,(p-p0)) + fade_t0*dot(g1,(p-p1));
    float p2p3 = (1.0-fade_t0)*dot(g2,(p-p2)) + fade_t0*dot(g3,(p-p3));

    return ((1.0-fade_t1)*p0p1 + fade_t1*p2p3);
}


void main()
{
    float n = noise(vec2(gl_FragCoord.x,gl_FragCoord.y)/64.0)*1.0 +
              noise(vec2(gl_FragCoord.x,gl_FragCoord.y)/32.0) * 0.5 +
              noise(vec2(gl_FragCoord.x,gl_FragCoord.y)/16.0) * 0.25 +
              noise(vec2(gl_FragCoord.x,gl_FragCoord.y)/8.0) * 0.125;

    FragColor = vec4(vec3(n*0.5+0.5),1.0);
}
)";


void GLAPIENTRY DebugCallback( 
    unsigned int  source,     //!< I - 
    unsigned int  type,       //!< I - 
    unsigned int  id,         //!< I - 
    unsigned int  severity,   //!< I - 
    int           length,     //!< I - length of debug message
    const char   *message,    //!< I - debug message
    const void   *userParam ) //!< I - user parameter
{
   std::cout << message << std::endl;
}


int main(void)
{
    if ( glfwInit() != GLFW_TRUE )
        throw std::runtime_error( "error initializing glfw" );

    //glfwWindowHint(GLFW_REFRESH_RATE, 10);

    GLFWwindow *wnd = glfwCreateWindow( 256, 256, "OGL window", nullptr, nullptr );
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
      -1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f, 
       1.0f, -1.0f,    1.0f, 1.0f, 0.0f, 1.0f,
       1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,
      -1.0f,  1.0f,    0.0f, 0.0f, 1.0f, 1.0f
    };
    static const std::vector<unsigned int> iquad{ 0, 1, 2, 0, 2, 3 };

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
      
    prog.Use();       

    std::vector<float> grad( 256 );
    for (int i = 0; i < 256; ++i)
      grad[i] = (float)rand() / (float)RAND_MAX;

    while (!glfwWindowShouldClose(wnd))
    {
        static float angle = 1.0f;

        int vpSize[2];
        glfwGetFramebufferSize( wnd, &vpSize[0], &vpSize[1] );
        
        float ascpect = (float)vpSize[0] / (float)vpSize[1];
        float orthoX = ascpect > 1.0f ? ascpect : 1.0f;
        float orthoY = ascpect > 1.0f ? 1.0f : 1.0f / ascpect;

        glm::mat4 project = glm::ortho( -orthoX, orthoX, -orthoY, orthoY, -1.0f, 1.0f );
        
        float orthScale = ascpect > 1.0f ? 600.0f / vpSize[1] : 600.0f / vpSize[0];
        project = glm::scale(project, glm::vec3(orthScale, orthScale, 1.0f));
        project = glm::mat4( 1.0f );

        glm::mat4 view( 1.0f );
        glm::mat4 model( 1.0f );
        
        prog.SetUniformM44( "project", glm::value_ptr(project) );
        prog.SetUniformM44( "view", glm::value_ptr(view) );
        prog.SetUniformM44( "model", glm::value_ptr(model) );

        GLint grad_loc = glGetUniformLocation( prog.Prog(), "gradient" );
        glUniform1fv( grad_loc, 256, grad.data() );

        for (int i = 0; i < 256; ++i)
        {
          std::string name = std::string( "gradient[" ) + std::to_string( i ) + "]";
          GLint grad_loc = glGetUniformLocation( prog.Prog(), name.c_str() );
          glUniform1f( grad_loc, grad[i] );
        }

        glViewport( 0, 0, vpSize[0], vpSize[1] );
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      
        glDrawElements( GL_TRIANGLES, (GLsizei)iarray.size(), GL_UNSIGNED_INT, nullptr );

        glfwSwapBuffers(wnd);
        glfwPollEvents();
    }

    glfwDestroyWindow( wnd );
    wnd = nullptr;
    glfwTerminate();

    return 0;
}