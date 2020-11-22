// Drawing Filled, Concave Polygons Using the Stencil Buffer (OpenGL Programming)
// http://what-when-how.com/opengl-programming-guide/drawing-filled-concave-polygons-using-the-stencil-buffer-opengl-programming/

// Drawing Filled, Concave Polygons Using the Stencil Buffer
// http://glprogramming.com/red/chapter14.html#name13

// Getting errors when trying to draw complex polygons with triangles in OpenGL
// https://stackoverflow.com/questions/59807065/getting-errors-when-trying-to-draw-complex-polygons-with-triangles-in-opengl/59808813#59808813

// OpenGL catmull rom spline loop coloring
// https://stackoverflow.com/questions/58465687/opengl-catmull-rom-spline-loop-coloring


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


// project includes
#include <gl/gl_debug.h>
#include <gl/gl_shader.h>

// preprocessor definitions

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

// forward declarations

void display( void );


// shader

std::string sh_vert = R"(
#version 400

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec4 inColor;

out vec4 vertCol;

void main()
{  
    vertCol     = inColor;
	  gl_Position = vec4(inPos.xy, 0.0, 1.0);
}
)";

std::string sh_frag = R"(
#version 400

in  vec4 vertCol;
out vec4 fragColor;

void main()
{
    fragColor = vertCol;
}
)";


// globale variables

std::chrono::high_resolution_clock::time_point start_time;

OpenGL::ShaderProgramSimple *g_prog;

GLuint vbo;
GLuint vao;

static const size_t elem_per_vertex = 6;

static std::vector<float> varray_1
{ 
   -0.707f, -0.75f,    1.0f, 0.0f, 0.0f, 1.0f,
    0.0f,   -0.20f,    1.0f, 0.5f, 0.0f, 1.0f,
    0.707f, -0.75f,    1.0f, 1.0f, 0.0f, 1.0f,
    0.1f,    0.0f,     0.5f, 0.5f, 0.5f, 1.0f,
    0.0f,    0.75f,    0.0f, 0.0f, 1.0f, 1.0f,
   -0.1f,    0.0f,     0.5f, 0.0f, 0.5f, 1.0f,
};

static const std::vector<float> varray_2
{ 
    -0.5f,    0.5f,    1.0f, 1.0f, 0.0f, 1.0f,
     0.0f,    0.5f,    0.5f, 0.5f, 0.5f, 1.0f,
     0.0f,    0.0f,    0.0f, 0.0f, 1.0f, 1.0f,
     0.5f,    0.0f,    0.5f, 0.0f, 0.5f, 1.0f,
     0.5f,   -0.5f,    1.0f, 0.0f, 0.0f, 1.0f,
    -0.5f,   -0.5f,    1.0f, 0.5f, 0.0f, 1.0f,
};

static std::vector<float> varray = varray_2;

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

    g_prog = new OpenGL::ShaderProgramSimple(
    {
      { sh_vert, GL_VERTEX_SHADER },
      { sh_frag, GL_FRAGMENT_SHADER }
    } );                                            

    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );

    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 6*sizeof(*varray.data()), 0 );
    glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 6*sizeof(*varray.data()), (void*)(2*sizeof(*varray.data())) );
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    g_prog->Use();

    // use vertex array object for drawing

    glBindVertexArray( vao );

    start_time = std::chrono::high_resolution_clock::now();
 
    std::cout << "main loop" << std::endl;
    glutMainLoop(); 

    std::cout << "terminate" << std::endl;
    glutDestroyWindow(wnd);
    return 0;    
} 

void display( void )
{
    for (int i = 0; i < elem_per_vertex; ++i)
        varray.push_back( varray[i] );
    varray.erase( varray.begin(), varray.begin() + elem_per_vertex );

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferSubData( GL_ARRAY_BUFFER, 0, varray.size()*sizeof(*varray.data()), varray.data() );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    int vpSize[2]
    {
        glutGet(GLUT_WINDOW_WIDTH),
        glutGet(GLUT_WINDOW_HEIGHT)
    };
    glViewport( 0, 0, vpSize[0], vpSize[1] );
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
      
    glDisable(GL_DEPTH_TEST);

    glClear( GL_STENCIL_BUFFER_BIT );
    glEnable( GL_STENCIL_TEST );
    glColorMask( GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE );
    glStencilOp( GL_KEEP, GL_KEEP, GL_INVERT );
    glStencilFunc( GL_ALWAYS, 0x1, 0x1 );

    glDrawArrays( GL_TRIANGLE_FAN, 0, (GLsizei)(varray.size()/elem_per_vertex) );

    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
    glStencilFunc( GL_EQUAL, 0x1, 0x1 );

    glDrawArrays( GL_TRIANGLE_FAN, 0, (GLsizei)(varray.size()/elem_per_vertex) );
    
    glDisable( GL_STENCIL_TEST );

    glutSwapBuffers();
    glutPostRedisplay();
} 