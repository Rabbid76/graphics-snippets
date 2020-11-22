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
    std::vector<glm::mat4> m(100, glm::mat4(1.0f));


    if ( glfwInit() == 0 )
        throw std::runtime_error( "error initializing glfw" );

    GLFWwindow *wnd = glfwCreateWindow( 800, 600, "OGL window", nullptr, nullptr );
    if ( wnd == nullptr )
    {
        glfwTerminate();
        throw std::runtime_error( "error initializing window" ); 
    }

    glfwMakeContextCurrent(wnd);

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
    

    static const std::vector<float> varray
    { 
      -0.5f, -0.288f,    1.0f, 0.0f, 0.0f, 1.0f, 
       0.5f, -0.288f,    1.0f, 1.0f, 0.0f, 1.0f,
       0.0f,  0.577f,    0.0f, 0.0f, 1.0f, 1.0f
    };

    GLuint vbo;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
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
          

    GLenum error0 = glGetError();
    GLuint b0 = 111, b1 = 222, b2 = 333;
    glGetVertexAttribIuiv( 0, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &b0 );
    GLenum error1 = glGetError();
    glGetVertexAttribIuiv( 1, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &b1 );
    GLenum error2 = glGetError();
    glGetVertexAttribIuiv( 2, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &b2 );
    GLenum error3 = glGetError();

    prog.Use();       

    // OpenGL Move Polygons across screen
    // [https://stackoverflow.com/questions/53090559/opengl-move-polygons-across-screen]

    std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(wnd))
    {
        std::chrono::high_resolution_clock::time_point current_time = std::chrono::high_resolution_clock::now();
        auto   delta_time = current_time - start_time;
        double time_s     = (double)std::chrono::duration_cast<std::chrono::milliseconds>(delta_time).count() / 1000.0;

        float rot_per_s = 1.0f/5.0f; // 1 full rotations in 5 second
        float angle_deg = (float)(360.0 * time_s * rot_per_s);

        double    motiontime_s = 5.0f; // from start to end and back in 5 seconds
        glm::vec3 start_pt1    = glm::vec3( -1.0f, 0.0f, 0.0f );
        glm::vec3 end_pt1      = glm::vec3(  0.0f, 0.0f, 0.0f );

        double times;
        double pos_rel   = modf(time_s / motiontime_s, &times);
        float  w         = pos_rel < 0.5 ? (float)pos_rel * 2.0f : (float)(1.0 - pos_rel) * 2.0f;
        glm::vec3 trans1 = start_pt1 * (w-1.0f) + end_pt1*w;

        glm::vec3 start_pt2 = glm::vec3(  1.0f, 0.0f, 0.0f );
        glm::vec3 end_pt2   = glm::vec3(  0.0f, 0.0f, 0.0f );
        glm::vec3 trans2    = start_pt2 * (w-1.0f) + end_pt2*w;
        
        glm::mat4 model1( 1.0f );
        model1 = glm::translate( model1, trans1 );
        model1 = glm::rotate( model1, glm::radians(angle_deg), glm::vec3(0.0f, 0.0f, 1.0f) );
        model1 = glm::scale( model1, glm::vec3(0.5f) );

        glm::mat4 model2( 1.0f );
        model2 = glm::translate( model2, trans2 );
        model2 = glm::rotate( model2, glm::radians(-angle_deg), glm::vec3(0.0f, 0.0f, 1.0f) );
        model2 = glm::scale( model2, glm::vec3(0.5f) );


        int vpSize[2];
        glfwGetFramebufferSize( wnd, &vpSize[0], &vpSize[1] );
        
        float ascpect = (float)vpSize[0] / (float)vpSize[1];
        float orthoX = ascpect > 1.0f ? ascpect : 1.0f;
        float orthoY = ascpect > 1.0f ? 1.0f : ascpect;

        glm::mat4 project = glm::ortho( -orthoX, orthoX, -orthoY, orthoY, -1.0f, 1.0f );
        
        glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, -0.5f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));
        
        prog.SetUniformM44( "project", glm::value_ptr(project) );
        prog.SetUniformM44( "view", glm::value_ptr(view) );
        

        glViewport( 0, 0, vpSize[0], vpSize[1] );
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      
        prog.SetUniformM44( "model", glm::value_ptr(model1) );
        glDrawArrays( GL_TRIANGLES, 0, 3 );

        prog.SetUniformM44( "model", glm::value_ptr(model2) );
        glDrawArrays( GL_TRIANGLES, 0, 3 );

        glfwSwapBuffers(wnd);
        glfwPollEvents();
    }

    glfwDestroyWindow( wnd );
    wnd = nullptr;
    glfwTerminate();

    return 0;
}