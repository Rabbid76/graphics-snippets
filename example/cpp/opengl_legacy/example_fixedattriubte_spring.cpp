// How to draw a perfect 3D Spring using Cylinders
// https://stackoverflow.com/questions/70553735/how-to-draw-a-perfect-3d-spring-using-cylinders/70554906#70554906

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

// function prototypes

void reshape(int w, int h);
void display(void);
void createSpring(
    GLfloat rounds, GLfloat height, GLfloat thickness, GLfloat radius,
    std::vector<GLfloat>& vertices, std::vector<GLuint>& indices);
void drawSpring(
    const std::vector<GLfloat>& vertices, const std::vector<GLuint>& indices);

// main

std::vector<GLfloat> springVertices;
std::vector<GLuint> springIndices;

int main(int argc, char** argv)
{
    std::cout << "start" << std::endl;

    glutInit(&argc, argv);

    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

    glutInitWindowSize(800, 600);
    int wnd = glutCreateWindow("FreeGLUT OpenGL window");

    if (glewInit() != GLEW_OK)
        throw std::runtime_error("error initializing glew");

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
    OpenGL::CContext context;
    context.Init(debug_level);

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);

    createSpring(1.5f, 0.25f, 0.03f, 0.1f, springVertices, springIndices);
    glEnable(GL_DEPTH_TEST);
   
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, -1, 0.125, 0, 0.0, 0.125, 0, 0, 1);

    glutMainLoop();

    std::cout << "terminate" << std::endl;
    glutDestroyWindow(wnd);
    return 0;
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(20.0f, (float)w / h, 0.1f, 1000);
    glMatrixMode(GL_MODELVIEW);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawSpring(springVertices, springIndices);
    glutPostRedisplay();
    glutSwapBuffers();
}

void createSpring(
    GLfloat rounds, GLfloat height, GLfloat thickness, GLfloat radius,
    std::vector<GLfloat>& vertices, std::vector<GLuint>& indices)
{
    const int slices = 32;
    const int step = 5;
    for (int i = -slices; i <= rounds * 360 + step; i += step)
    {
        for (int j = 0; j < slices; j++)
        {
            GLfloat t = (GLfloat)i / 360 + (GLfloat)j / slices * step / 360;
            t = std::max(0.0f, std::min(rounds, t));
            GLfloat a1 = t * (float)M_PI * 2.0f;
            GLfloat a2 = (GLfloat)j / slices * (float)M_PI * 2.0f;
            GLfloat d = radius + thickness * cos(a2);
            vertices.push_back(d * cos(a1));
            vertices.push_back(d * sin(a1));
            vertices.push_back(thickness * sin(a2) + height * t / rounds);
        }
    }
    for (GLuint i = 0; i < (GLuint)vertices.size() / 3 - slices; ++i)
    {
        indices.push_back(i);
        indices.push_back(i + slices);
    }
}

void drawSpring(
    const std::vector<GLfloat>& vertices, const std::vector<GLuint>& indices)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // just to see the mesh (delete later)

    glColor4f(1, 1, 1, 1);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices.data());
    glDrawElements(GL_TRIANGLE_STRIP, (GLsizei)indices.size(), GL_UNSIGNED_INT, indices.data());
    glDisableClientState(GL_VERTEX_ARRAY);
}