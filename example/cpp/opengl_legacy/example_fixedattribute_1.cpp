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
#include <iostream>
#include <chrono>

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>

// project includes
#include <gl/gl_debug.h>

// preprocessor definitions

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

int main(void)
{
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

    bool start = true;
    std::chrono::high_resolution_clock::time_point prev_time;
    std::chrono::high_resolution_clock::time_point start_time;
    double time_sum = 0.0;
    int    samples = 0;
    while (!glfwWindowShouldClose(wnd))
    {
        std::chrono::high_resolution_clock::time_point current_time = std::chrono::high_resolution_clock::now();
        if ( start )
          start_time = prev_time = current_time;

        auto   delta    = current_time - prev_time;
        auto   time     = current_time - start_time;
        double delta_ms = (double)std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();
        double time_ms  = (double)std::chrono::duration_cast<std::chrono::milliseconds>(time).count();
        start = false;
        prev_time = current_time;

        if ( start == false )
        {
          samples ++;
          time_sum += delta_ms;
          if ( samples == 100 )
          {
            double average_time = time_sum / samples;
            //std::cout << "fps: " << 1000.0 / average_time << std::endl;
            samples = 0;
            time_sum = 0.0;
          }
        }

        float angle = 0.25f * 360.0 * time_ms / 1000.0f;

        glm::mat4 aspect( 1.0f );
        aspect = glm::scale( aspect, 0.8f * glm::vec3(600.0f / 800.0f, 1.0f, 0.0f) );

        glm::mat4 project( 1.0f );
        //project = aspect * project;

        glm::mat4 model( 1.0f );
        //model = glm::translate( model, glm::vec3(0.1f, 0.0f, 1.0f) );
        //model = glm::rotate( model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f) );
        //model = aspect * model;

        glMatrixMode( GL_PROJECTION );
        glLoadMatrixf( glm::value_ptr(project) );

        glMatrixMode( GL_MODELVIEW );
        glLoadMatrixf( glm::value_ptr(model) );

        int vpSize[2];
        glfwGetFramebufferSize( wnd, &vpSize[0], &vpSize[1] );
        glViewport( 0, 0, vpSize[0], vpSize[1] );
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      
        float varray[]{ -0.707f, -0.75f, 0.707f, -0.75f, 0.0f, 0.75f };

        //glVertexPointer( 2, GL_FLOAT, 0, varray );
        //glEnableClientState( GL_VERTEX_ARRAY );
        //glDrawArrays( GL_TRIANGLES, 0, 3 );

        glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, varray );
        glEnableVertexAttribArray( 0 );
        glDrawArrays( GL_TRIANGLES, 0, 3 );

        glfwSwapBuffers(wnd);
        glfwPollEvents();
    }

    glfwDestroyWindow( wnd );
    wnd = nullptr;
    glfwTerminate();

    return 0;
}