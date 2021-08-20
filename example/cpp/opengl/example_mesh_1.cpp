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

std::string sh_vert = R"(
#version 460 core

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
#version 460 core

in vec3 vertPos;
in vec4 vertCol;

out vec4 fragColor;

void main()
{
    fragColor = vertCol;
}
)";


static float ang_x = 0.0f;
static float ang_y = 0.1f;

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
      { sh_frag, GL_FRAGMENT_SHADER }
    } );
    

    const float sin120 = 0.8660254f;
    static const std::vector<float> varray
    { 
       0.0f,             0.0f,           1.0f,    1.0f, 0.0f, 0.0f, 1.0f, 
       0.0f,            -sin120,        -0.5f,    1.0f, 1.0f, 0.0f, 1.0f,
       sin120 * sin120,  0.5f * sin120, -0.5f,    0.0f, 0.0f, 1.0f, 1.0f,
      -sin120 * sin120,  0.5f * sin120, -0.5f,    0.0f, 1.0f, 0.0f, 1.0f 
    };
    const std::vector<unsigned int> iarray
    {
      0, 1, 2, 0, 2, 3, 0, 3, 1, 1, 3, 2 
    };

    GLuint vertex_binding_index = 0;
    GLuint color_binding_index = 1;

    GLuint vbo;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );

    GLuint ibo;
    glGenBuffers( 1, &ibo );

    //glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );
    glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), nullptr, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, GL_ZERO, varray.size()*sizeof(*varray.data()), varray.data() );

    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    GLuint vertex_attrib_inx = 0;
    GLuint color_attrib_inx  = 1;
    
    glBindVertexBuffer( vertex_binding_index, vbo, 0, 7*sizeof(*varray.data()) );
    glVertexAttribFormat( vertex_attrib_inx, 3, GL_FLOAT, GL_FALSE, 0 );

    glBindVertexBuffer( color_binding_index, vbo, 3*sizeof(*varray.data()), 7*sizeof(*varray.data()) );
    glVertexAttribFormat( color_attrib_inx, 4, GL_FLOAT, GL_FALSE, 0 );
    
    glVertexAttribBinding( vertex_attrib_inx, vertex_binding_index );
    glVertexAttribBinding( color_attrib_inx,  color_binding_index );

    glEnableVertexAttribArray( vertex_attrib_inx );
    glEnableVertexAttribArray( color_attrib_inx );
    
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    //glBufferData( GL_ELEMENT_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, iarray.size()*sizeof(*iarray.data()), nullptr, GL_STATIC_DRAW );
    glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, GL_ZERO, iarray.size()*sizeof(*iarray.data()), iarray.data() );

    glBindVertexArray( 0 );

    // ...

    glBindVertexArray( vao );

    prog.Use();       

    while (!glfwWindowShouldClose(wnd))
    {
        static float angle = 1.0f;

        int vpSize[2];
        glfwGetFramebufferSize( wnd, &vpSize[0], &vpSize[1] );
        
        float ascpect = (float)vpSize[0] / (float)vpSize[1];
        float orthoX = ascpect > 1.0f ? ascpect : 1.0f;
        float orthoY = ascpect > 1.0f ? 1.0f : 1.0 / ascpect;

        float aspect = (float)vpSize[0]/(float)vpSize[1];
        float near_dist = 0.01f;
        float far_dist = 1000.0f;
        glm::mat4 project = glm::perspective( (float)M_PI/3.0f, aspect, near_dist, far_dist );
        
        static bool invert = false;
        glm::mat4 view( 1.0f );
        
        view = glm::lookAt(
          glm::vec3(-1.0f, -3.0f, 0.0f),
          glm::vec3(0.0f, 0.0f,  0.0f),
          glm::vec3(0.0f, 0.0f, 1.0f));

        //glm::vec3 pos( 0.0f, 0.0f, (near_dist + far) / 2.0f );
        glm::vec3 pos( 0.0f, 0.0f, near_dist * 0.99f + far_dist * 0.01f );
        //pos = glm::vec3( 0.0f, 0.0f, 3.0f );
        //view = glm::translate( glm::mat4( 1.0f ), -pos );

        glm::mat4 model( 1.0f );
        model = glm::rotate( model, ang_x, glm::vec3( 1.0f, 0.0f, 0.0f ) );
        model = glm::rotate( model, ang_y, glm::vec3( 0.0f, 0.1f, 0.0f ) );
        ang_x += 0.013f;
        ang_y += 0.017f;
        
        prog.SetUniformM44( "project", glm::value_ptr(project) );
        prog.SetUniformM44( "view", glm::value_ptr(view) );
        prog.SetUniformM44( "model", glm::value_ptr(model) );

        glViewport( 0, 0, vpSize[0], vpSize[1] );
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      
        glEnable( GL_DEPTH_TEST );

        glDrawElements( GL_TRIANGLES, (GLsizei)iarray.size(), GL_UNSIGNED_INT, nullptr );

        glfwSwapBuffers(wnd);
        glfwPollEvents();
    }

    glfwDestroyWindow( wnd );
    wnd = nullptr;
    glfwTerminate();

    return 0;
}