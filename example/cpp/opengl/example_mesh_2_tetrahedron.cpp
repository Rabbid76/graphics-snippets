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
#version 400

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
#version 400

in vec3 vertPos;
in vec4 vertCol;

out vec4 fragColor;

void main()
{
    fragColor = vertCol;
}
)";


int main(void)
{
    if ( glfwInit() == 0 )
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
    

    const float s_8_9 = ( float )std::sqrt(0.8/0.9);
    const float s_2_9 = ( float )std::sqrt(0.2/0.9);
    const float s_2_3 = ( float )std::sqrt(0.2/0.3);

    static const std::vector<float> varray
    { 
       0.0f,   0.0f,   1.0f,       1.0f, 0.0f, 0.0f, 1.0f, 
       s_8_9,  0.0f,  -1.0f/3.0f,  1.0f, 1.0f, 0.0f, 1.0f,
      -s_2_9,  s_2_3, -1.0f/3.0f,  0.0f, 0.0f, 1.0f, 1.0f,
      -s_2_9, -s_2_3, -1.0f/3.0f,  0.0f, 1.0f, 0.0f, 1.0f,
    };
    static const std::vector<unsigned int> iarray{ 0, 1, 2,  0, 2, 3,  0, 3, 1,  1, 2, 3 };

    // create vertex and index buffer object

    GLuint vbo;
    glGenBuffers( 1, &vbo );

    GLuint ibo;
    glGenBuffers( 1, &ibo );
    
    // create vertex array object

    GLuint vao;
    glGenVertexArrays( 1, &vao );

    // specify vertex arrays
    
    glBindVertexArray( vao );
    
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 7*sizeof(*varray.data()), 0 );
    glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 7*sizeof(*varray.data()), (void*)(3*sizeof(*varray.data())) );    
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, iarray.size()*sizeof(*iarray.data()), iarray.data(), GL_STATIC_DRAW );

    //  create and initialize a buffer object's data store

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    //glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );
    glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), nullptr, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, GL_ZERO, varray.size()*sizeof(*varray.data()), varray.data() );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    prog.Use();       

    glEnable( GL_DEPTH_TEST );

    while (!glfwWindowShouldClose(wnd))
    {
        static float angle = 1.0f;

        int vpSize[2];
        glfwGetFramebufferSize( wnd, &vpSize[0], &vpSize[1] );
        
        float ascpect = (float)vpSize[0] / (float)vpSize[1];
        float orthoX = ascpect > 1.0f ? ascpect : 1.0f;
        float orthoY = ascpect > 1.0f ? 1.0f : ascpect;

         glm::mat4 project = glm::perspective( glm::radians(90.0f), ascpect, 0.1f, 20.0f );
        
         glm::vec3 world_up( 0.0f, 0.0f, 1.0f );
         glm::vec3 world_north( 0.0f, 1.0f, 0.0f );

         float height = 2.0f;
         glm::vec3 camPosition  = world_up * height;
         glm::vec3 camTraget    = glm::vec3(0.0f, 0.0f, 0.0f);
         glm::vec3 camUp        = world_north;

         glm::mat4 view = glm::lookAt( camPosition, camTraget, camUp );

        glm::mat4 model( 1.0f );
        model = glm::rotate( model, glm::radians(angle), glm::vec3(0.0f, 0.5f, 1.0f) );
        angle += 1.0f;
        
        prog.SetUniformM44( "project", glm::value_ptr(project) );
        prog.SetUniformM44( "view", glm::value_ptr(view) );
        prog.SetUniformM44( "model", glm::value_ptr(model) );

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