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

#define MAIN_LOOP_EVENT 1

// prototypes

void display( void );

// shader

std::string sh_vert = R"(
#version 460 core

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
#version 460 core

in vec3 vertPos;
in vec4 vertCol;

out vec4 fragColor;

void main()
{
    fragColor = vertCol;
}
)";


// globale variables

std::chrono::high_resolution_clock::time_point start_time;

OpenGL::ShaderProgramSimple *g_prog;

int numOfIndices = 0;

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

    float z = -1.5f;

    
    static const std::vector<float> varray{
      -1.0f, 1.0f, 1.0f,  1.0f, 0.0f, 0.0f, 1.0f, 
      -1.0f,-1.0f, 1.0f,  1.0f, 1.0f, 0.0f, 1.0f,
       1.0f, 1.0f, 1.0f,  1.0f, 1.0f, 0.0f, 1.0f,
       1.0f,-1.0f, 1.0f,  0.0f, 0.0f, 1.0f, 1.0f,
      -1.0f, 1.0f,-1.0f,  1.0f, 0.0f, 0.0f, 1.0f, 
      -1.0f,-1.0f,-1.0f,  1.0f, 1.0f, 0.0f, 1.0f,
       1.0f, 1.0f,-1.0f,  1.0f, 1.0f, 0.0f, 1.0f,
       1.0f,-1.0f,-1.0f,  0.0f, 0.0f, 1.0f, 1.0f
    };
    
    /*
    std::vector<unsigned int> indices01 {
      0, 2, 3, 0, 3, 1,
      2, 6, 7, 2, 7, 3,
      6, 4, 5, 6, 5, 7,
      4, 0, 1, 4, 1, 5,
      0, 4, 6, 0, 6, 2,
      1, 5, 7, 1, 7, 3,
    };
    */
    
    std::vector<unsigned int> indices01 {
      0, 1, 2, 3, 6, 7, 4, 5,
      99, // 99 is the restart index
      7, 3, 5, 1, 4, 0, 6, 2
    };

    numOfIndices = (int)indices01.size();

    // create vertex buffer object

    GLuint vbo= 70;
    glGenBuffers( 1, &vbo );
    
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

    GLuint IBO;
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices01[0])*numOfIndices, indices01.data(), GL_STATIC_DRAW);

    glBindVertexArray( 0 );

    //  create and initialize a buffer object's data store

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    //glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );
    glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), nullptr, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, GL_ZERO, varray.size()*sizeof(*varray.data()), varray.data() );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    g_prog->Use();

    // use vertex array object for drawing

    glBindVertexArray( vao );

    start_time = std::chrono::high_resolution_clock::now();
 
    std::cout << "main loop" << std::endl;

#if MAIN_LOOP_EVENT == 1
    int i = 0;
    while ( i++ >= 0 )
    {
        glutMainLoopEvent(); 
        glutPostRedisplay();
    }
#else
    glutMainLoop(); 
#endif

    std::cout << "terminate" << std::endl;
    glutDestroyWindow(wnd);
    return 0;    
} 

void display( void )
{
    static float angle = 0.0f;

    int vpSize[2]
    {
        glutGet(GLUT_WINDOW_WIDTH),
        glutGet(GLUT_WINDOW_HEIGHT)
    };
    float ascpect = (float)vpSize[0] / (float)vpSize[1];
    float orthoX = ascpect > 1.0f ? ascpect : 1.0f;
    float orthoY = ascpect > 1.0f ? 1.0f : ascpect;

    //glm::mat4 project = glm::ortho( -orthoX, orthoX, -orthoY, orthoY, 0.01f, 4.0f );
    glm::mat4 project = glm::perspective(glm::radians(90.0f), ascpect, 0.01f, 20.0f);
        
    glm::mat4 view = glm::lookAt(
      glm::vec3(0.0f, 0.0f, 1.0f),
      glm::vec3(0.0f, 0.0f, 0.0f),
      glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 model( 1.0f );
    model = glm::translate( model, glm::vec3(0.0f, 0.0f, -5.0f) );
    model = glm::rotate( model, glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f) );
    model = glm::rotate( model, glm::radians(angle/2), glm::vec3(0.0f, 1.0f, 0.0f) );
    //model = glm::scale( model, glm::vec3(0.5f, 0.5f, 1.0f) );
    angle += 1.0f;
        
    g_prog->SetUniformM44( "project", glm::value_ptr(project) );
    g_prog->SetUniformM44( "view", glm::value_ptr(view) );
    g_prog->SetUniformM44( "model", glm::value_ptr(model) );

    glEnable( GL_DEPTH_TEST );
    glEnable( GL_PRIMITIVE_RESTART );
    glPrimitiveRestartIndex( 99 );

    glViewport( 0, 0, vpSize[0], vpSize[1] );
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    //glDrawElements( GL_TRIANGLES, numOfIndices, GL_UNSIGNED_INT, nullptr );
    glDrawElements( GL_TRIANGLE_STRIP, numOfIndices, GL_UNSIGNED_INT, nullptr );

    glutSwapBuffers();

#if MAIN_LOOP_EVENT != 1
    glutPostRedisplay();
#endif
} 