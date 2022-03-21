// STL
#include <iostream>
#include <string>
#include <stdexcept>
#include <chrono>


// GLM [https://glm.g-truc.net/0.9.9/api/index.html]
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// OpenGL
#include <gl/gl_glew.h>


// FreeGLUT [http://freeglut.sourceforge.net/]
#include <gl/freeglut.h>


// STB 
#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
//#include <stb_image_write.h>

// project includes
#include <gl/gl_debug.h>

// preprocessor definitions

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

// https://replit.com/@Rabbid76/freeglut-opengl-1#main.cpp

// function prototypes

void display( void );
void cube( void );

// main

int main(int argc, char** argv)
{
    std::cout << "start" << std::endl;

    glutInit(&argc, argv);

    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

    glutInitWindowSize(800, 600);
    int wnd = glutCreateWindow("FreeGLUT OpenGL window");

    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
    OpenGL::CContext context;
    context.Init( debug_level );

    glutDisplayFunc(display);  

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective(90.0, 800.0/600.0, 0.1, 100.0);

    glViewport( 0, 0, 800, 600 );
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  

    glEnable( GL_DEPTH_TEST );

    glutMainLoop();

    std::cout << "terminate" << std::endl;
    glutDestroyWindow(wnd);
    return 0;
}

float angle = 0.0f;

void display( void )
{
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    gluLookAt(0, 5.0f, 0, 0, 0, 0, 0, 0, 1);

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
      
    glPushMatrix();
    glTranslatef(-2.0f, 0, 0);
    glRotatef(angle, 1, 1, 0);
    cube();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(2.0f, 0, 0);
    glRotatef(angle*2.0f, 1, 0, 1);
    cube();
    glPopMatrix();

    glutSwapBuffers();
    glutPostRedisplay();

    angle += 1.0f;
}

void cube( void )
{
    static float verticies[] = {
         1, -1, -1,
         1,  1, -1,
        -1,  1, -1,
        -1, -1, -1,
         1, -1,  1,
         1,  1,  1,
        -1, -1,  1,
        -1,  1,  1,
    };

    static int surfaces[] = {0,1,2,3, 3,2,7,6, 6,7,5,4, 4,5,1,0, 1,5,7,2, 4,0,3,6 };
    static float colors[] = { 1,0,0, 0,1,0, 0,0,1, 1,1,0, 1,0,1, 1,0.5,0 };

    glBegin(GL_QUADS);
    for (int side = 0; side < 6; ++side)
    {
        glColor3fv(colors + side*3);
        for (int corner = 0; corner < 4; corner++)
        {
            int vi = surfaces[side*4+corner];
            glVertex3fv(verticies + vi*3);
        }
    }
    glEnd();
}