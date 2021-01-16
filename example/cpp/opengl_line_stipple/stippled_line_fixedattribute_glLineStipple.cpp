// Drawing rectangle with dashed boundary in openGL android
// [https://stackoverflow.com/questions/60188402/drawing-rectangle-with-dashed-boundary-in-opengl-android]
//
// OpenGL ES - Dashed Lines
// [https://stackoverflow.com/questions/37975618/opengl-es-dashed-lines]

#include <stdafx.h>

// OpenGL
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

// GLM [https://glm.g-truc.net/0.9.9/api/index.html]
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// GLFW [https://www.glfw.org/]
#include <GLFW/glfw3.h>


// STL
#include <iostream>
#include <string>
#include <stdexcept>
#include <chrono>
#include <vector>

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>

// preprocessor definitions

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

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


void init_opengl_debug() {
    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    if ( glDebugMessageCallback != nullptr && glDebugMessageControl != nullptr )
    {
        glDebugMessageCallback( &DebugCallback, nullptr );
        glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE );
        //glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
        //glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, NULL, GL_TRUE);
        glEnable( GL_DEBUG_OUTPUT );
        glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
    }
}

// main

int main(void)
{
    if ( glfwInit() == 0 )
        throw std::runtime_error( "error initializing glfw" );

    //glfwWindowHint(GLFW_REFRESH_RATE, 10);

    GLFWwindow *window = glfwCreateWindow( 800, 600, "GLFW OGL window", nullptr, nullptr );
    if ( window == nullptr )
    {
        glfwTerminate();
        throw std::runtime_error( "error initializing window" ); 
    }

    glfwMakeContextCurrent(window);

    init_opengl_debug();

    std::vector<glm::vec3> attribarray1{
        glm::vec3(-1.0f, -1.0f, 0.0f),
        glm::vec3( 1.0f, -1.0f, 0.0f),
        glm::vec3( 1.0f,  1.0f, 0.0f),
        glm::vec3(-1.0f,  1.0f, 0.0f)
    };

    std::vector<glm::vec3> attribarray2;
    for (size_t u=0; u <= 360; u += 8)
    {
        double a = u*M_PI/180.0;
        double c = cos(a), s = sin(a);
        attribarray2.emplace_back(glm::vec3((float)c, (float)s, 0.0f));
    }

    GLushort pattern = 0x18ff;
    GLfloat  factor  = 2.0f;
    glLineStipple(2.0, pattern);
    glEnable(GL_LINE_STIPPLE);
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glm::mat4(project);
    int vpSize[2]{0, 0};
    while (!glfwWindowShouldClose(window))
    {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        if (w != vpSize[0] ||  h != vpSize[1])
        {
            vpSize[0] = w; vpSize[1] = h;
            glViewport(0, 0, vpSize[0], vpSize[1]);
            float aspect = (float)w/(float)h;
            project = glm::ortho(-aspect, aspect, -1.0f, 1.0f, -10.0f, 10.0f);
        }

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(glm::value_ptr(project));
        glMatrixMode(GL_MODELVIEW);
             
        glClear(GL_COLOR_BUFFER_BIT);

        glEnableClientState(GL_VERTEX_ARRAY);
        
        glPushMatrix();
        glTranslatef( -0.6f, 0.0f, 0.0f );
        glScalef( 0.5f, 0.5f, 0.5f );
        glVertexPointer(3, GL_FLOAT, 0, attribarray1.data());
        glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)attribarray1.size());
        glPopMatrix();

        glPushMatrix();
        glTranslatef( 0.6f, 0.0f, 0.0f );
        glScalef( 0.5f, 0.5f, 0.5f );
        glVertexPointer(3, GL_FLOAT, 0, attribarray2.data());
        glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)attribarray2.size());
        glPopMatrix();

        glDisableClientState(GL_VERTEX_ARRAY);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    return 0;
}
    
