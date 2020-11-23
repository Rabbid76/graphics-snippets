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

// freeglut
#include <GL/freeglut.h>

// stb 
#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
//#include <stb_image_write.h>

// stl
#include <vector>
#include <stdexcept>
#include <chrono>
#include <iostream>
#include <array>

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>

// project includes
#include <gl/gl_helper.h>


GLUquadricObj *obj= nullptr;
float angle = 0.0f;

void reshape(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0f, (float)w/h, 0.1f, 10.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 2, 0, 0, 0, 0, 0, 0, 1);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glPushMatrix();
    glTranslatef(0, 0, 0);
    glRotatef(angle, 1, 0, 0);

    glPolygonOffset( 1.0, 1.0 );
    glEnable( GL_POLYGON_OFFSET_FILL );

    gluQuadricDrawStyle(obj, GLU_FILL);
    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    gluSphere(obj, 1.0f, 20, 10);

    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    gluQuadricDrawStyle(obj, GLU_SILHOUETTE);
    gluSphere(obj, 1.0f, 20, 10);

    glDisable( GL_POLYGON_OFFSET_FILL );
    glPopMatrix();

    glutSwapBuffers();
    glutPostRedisplay();

    angle ++;
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(500, 500);
    int wnd = glutCreateWindow("test");
    if ( wnd == 0 )
        throw std::runtime_error( "error initializing window" ); 

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);  

    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );
    
    obj = gluNewQuadric();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glutMainLoop(); 

    gluDeleteQuadric(obj);

    glutDestroyWindow(wnd);
    return 0;
}

