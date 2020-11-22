#include <stdafx.h>

// OpenGL
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// glfw
#include <GLFW/glfw3.h>

// stl
#include <vector>
#include <stdexcept>
#include <iostream>
#include <chrono>

#define _USE_MATH_DEFINES
#include <math.h>

// preprocessor definitions

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

int main(void)
{
    if ( glfwInit() == 0 )
        throw std::runtime_error( "error initializing glfw" );

    //glfwWindowHint( GLFW_DOUBLEBUFFER, GL_FALSE );

    GLFWwindow *wnd = glfwCreateWindow( 800, 600, "OGL window", nullptr, nullptr );
    if ( wnd == nullptr )
    {
        glfwTerminate();
        throw std::runtime_error( "error initializing window" ); 
    }

    glfwMakeContextCurrent(wnd);
    
    
    // [`glfwSwapBuffers`](https://www.glfw.org/docs/latest/group__context.html#ga6d4e0cdf151b5e579bd67f13202994ed) no swap interval
    glfwSwapInterval( 0 );


    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    static const std::vector<float> varray
    //static const float varray[] =
    { 
      -0.707f, -0.75f,    1.0f, 0.0f, 0.0f, 1.0f, 
       0.707f, -0.75f,    1.0f, 1.0f, 0.0f, 1.0f,
       0.0f,    0.75f,    0.0f, 0.0f, 1.0f, 1.0f
    };

    GLuint vbo;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );
    //glBufferData( GL_ARRAY_BUFFER, sizeof(varray), varray, GL_STATIC_DRAW );

    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    //glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 6*sizeof(*varray.data()), 0 );
    //glEnableVertexAttribArray( 0 );
    //glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 6*sizeof(*varray.data()), (void*)(2*sizeof(*varray.data())) );
    //glEnableVertexAttribArray( 0 );
    glVertexPointer( 2, GL_FLOAT, 6*sizeof(float), 0 );
    glEnableClientState( GL_VERTEX_ARRAY );
    glColorPointer( 4, GL_FLOAT, 6*sizeof(float), (void*)(2*sizeof(float)) ); 
    glEnableClientState( GL_COLOR_ARRAY );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

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
            std::cout << "fps: " << 1000.0 / average_time << std::endl;
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
        model = glm::translate( model, glm::vec3(0.1f, 0.0f, 1.0f) );
        model = glm::rotate( model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f) );
        model = aspect * model;

        glMatrixMode( GL_PROJECTION );
        glLoadMatrixf( glm::value_ptr(project) );

        glMatrixMode( GL_MODELVIEW );
        glLoadMatrixf( glm::value_ptr(model) );

        int vpSize[2];
        glfwGetFramebufferSize( wnd, &vpSize[0], &vpSize[1] );
        glViewport( 0, 0, vpSize[0], vpSize[1] );
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      
        glDrawArrays( GL_TRIANGLES, 0, 3 );

        //glFlush();

      
        glfwSwapBuffers(wnd);
        glfwPollEvents();
    }

    glfwDestroyWindow( wnd );
    wnd = nullptr;
    glfwTerminate();

    return 0;
}