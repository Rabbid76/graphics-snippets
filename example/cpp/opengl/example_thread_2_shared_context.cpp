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

// freeglut
//#include <GL/freeglut.h>

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
#include <chrono>
#include <atomic>

// [How can I detect g++ and MinGW in C++ preprocessor?](https://stackoverflow.com/questions/17493759/how-can-i-detect-g-and-mingw-in-c-preprocessor)
//#if defined(__GNUC__) && defined(__MINGW32__)
//[Update WINVER and _WIN32_WINNT](https://docs.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt?view=vs-2019)
//#define _WIN32_WINNT 0x0A00
// [std::thread error (thread not member of std)](https://stackoverflow.com/questions/2519607/stdthread-error-thread-not-member-of-std)
// [meganz/mingw-std-threads](https://github.com/meganz/mingw-std-threads)
//#include<mingw/mingw.thread.h>
//#include<mingw/mingw.mutex.h>
//#include<mingw/mingw.condition_variable.h>
//#else
#include <thread> 
#include <mutex>
#include <condition_variable>
//#endif

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

std::atomic<bool> finished = false;
GLFWwindow *wnd = nullptr;
bool wnd2created = false;
std::mutex mtx;
std::condition_variable cv;

void wnd2func( void )
{
    GLFWwindow *wnd2 = glfwCreateWindow( 800, 600, "OGL window", nullptr, wnd );
    std::cout << "create 2" << std::endl;
    
    {
      std::unique_lock<std::mutex> lck( mtx );
      wnd2created = true;
      cv.notify_one();
    }

    if (wnd2 == nullptr)
        return;
    
    glfwMakeContextCurrent(wnd2);
    std::cout << "current 2" << std::endl;

    static const std::vector<float> vquad
    { 
      -0.5f, -0.5f,    1.0f, 0.0f, 0.0f, 1.0f, 
       0.5f, -0.5f,    1.0f, 1.0f, 0.0f, 1.0f,
       0.5f,  0.5f,    0.0f, 1.0f, 0.0f, 1.0f,
      -0.5f,  0.5f,    0.0f, 0.0f, 1.0f, 1.0f
    };
    static const std::vector<unsigned int> iquad{ 0, 1, 2, 0, 2, 3 };

    std::vector<float>        varray = vquad;
    std::vector<unsigned int> iarray = iquad;

    GLuint vbo;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );
    
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    glVertexPointer( 2, GL_FLOAT, 6*sizeof(float), 0 );
    glEnableClientState( GL_VERTEX_ARRAY );
    glColorPointer( 4, GL_FLOAT, 6*sizeof(float), (void*)(2*sizeof(float)) ); 
    glEnableClientState( GL_COLOR_ARRAY );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    GLuint ibo;
    glGenBuffers( 1, &ibo );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, iarray.size()*sizeof(*iarray.data()), iarray.data(), GL_STATIC_DRAW );

    while (!finished && !glfwWindowShouldClose(wnd2))
    {
        int vpSize[2];
        glfwGetFramebufferSize( wnd2, &vpSize[0], &vpSize[1] );
        glViewport( 0, 0, vpSize[0], vpSize[1] );
        glClearColor(0.5f, 0.2f, 0.2f, 0.0f);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glm::mat4 aspect( 1.0f );
        aspect = glm::scale( aspect, glm::vec3((float)vpSize[1] / (float)vpSize[0], 1.0f, 0.0f) );

        glm::mat4 project( 1.0f );
        project = aspect * project;

        glm::mat4 model( 1.0f );

        glMatrixMode( GL_PROJECTION );
        glLoadMatrixf( glm::value_ptr(project) );

        glMatrixMode( GL_MODELVIEW );
        glLoadMatrixf( glm::value_ptr(model) );

        glDrawElements( GL_TRIANGLES, (GLsizei)iarray.size(), GL_UNSIGNED_INT, nullptr );

        glfwPollEvents();
        glfwSwapBuffers(wnd2);
    }
}


int main(void)
{
    if ( glfwInit() != GLFW_TRUE )
        throw std::runtime_error( "error initializing glfw" );

    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    wnd = glfwCreateWindow( 800, 600, "OGL window", nullptr, nullptr );
    if ( wnd == nullptr )
    {
        glfwTerminate();
        throw std::runtime_error( "error initializing window" ); 
    }
    std::cout << "create 1" << std::endl;


    glfwMakeContextCurrent(wnd);
    
    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
    OpenGL::CContext context;
    context.Init( debug_level );
    
    glfwMakeContextCurrent(0);


    std::thread wnd2thread(wnd2func);
    {
      std::unique_lock<std::mutex> lck( mtx );
      cv.wait(lck, []() -> bool { return wnd2created; });
    }

    glfwMakeContextCurrent(wnd);
    std::cout << "current 1" << std::endl;

    //std::this_thread::sleep_for(std::chrono::milliseconds(200));

    static const std::vector<float> varray
    //static const float varray[] =
    { 
      -0.866f, -0.5f,    1.0f, 0.0f, 0.0f, 1.0f, 
       0.866f, -0.5f,    1.0f, 1.0f, 0.0f, 1.0f,
       0.0f,    1.0f,    0.0f, 0.0f, 1.0f, 1.0f
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


        float angle = 0.25f * 360.0f * (float)time_ms / 1000.0f;

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

        glfwSwapBuffers(wnd);
        glfwPollEvents();
    }


    finished = true;
    wnd2thread.join();

    glfwDestroyWindow( wnd );
    wnd = nullptr;
    glfwTerminate();

    return 0;
}