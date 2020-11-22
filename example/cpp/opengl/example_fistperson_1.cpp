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

// FreeGLUT [http://freeglut.sourceforge.net/]
#include <gl/freeglut.h>

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
    vec4 view_pos =  view * model * vec4(inPos, 1.0);
 
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


std::chrono::high_resolution_clock::time_point start_time;

OpenGL::ShaderProgramSimple *g_prog;

#define MESH_QUAD
//#define MESH_THEDRAHEDRON

#if defined(MESH_QUAD)
static const std::vector<float> varray
{ 
    -1.0f, -1.0f,  0.0f,    1.0f, 0.0f, 0.0f, 1.0f, 
     1.0f, -1.0f,  0.0f,    1.0f, 1.0f, 0.0f, 1.0f,
     1.0f,  1.0f,  0.0f,    0.0f, 0.0f, 1.0f, 1.0f,
    -1.0f,  1.0f,  0.0f,    0.0f, 1.0f, 0.0f, 1.0f 
};
const std::vector<unsigned int> iarray
{
  0, 1, 2, 0, 2, 3 
};
#endif

#if defined(MESH_THEDRAHEDRON)
const float sin120 = 0.8660254f;
static const std::vector<float> varray
{ 
    0.0f,             0.0f,           1.0f,    1.0f, 0.0f, 0.0f, 1.0f, 
    0.0f,            -sin120,        -0.5f,    1.0f, 1.0f, 0.0f, 1.0f,
    sin120 * sin120,  0.5f * sin120, -0.5f,    0.0f, 0.0f, 1.0f, 1.0f,
  -sin120 * sin120,  0.5f * sin120, -0.5f,    0.0f, 1.0f, 0.0f, 1.0f 
};
const std::vector<unsigned int> iarray
{
  0, 1, 2, 0, 2, 3, 0, 3, 1, 1, 3, 2 
};
#endif


void display( void );
void keyboard( unsigned char key, int x, int y );
void special( int key, int x, int y );


glm::mat4 view_matrix(1.0f);
float pitch_angle = 0, roll_angle = 0;

int main(int argc, char** argv)
{
    // Initialise FreeGLUT
    glutInit(&argc, argv);

    // Setup OpenGL window properties
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

    // Create window
    glutInitWindowSize(800, 600);
    int wnd = glutCreateWindow("test");
    if ( wnd == 0 )
        throw std::runtime_error( "error initializing window" ); 

    // Register display callback function
    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
    glutSpecialFunc( special );

    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
    OpenGL::CContext context;
    context.Init( debug_level );

    g_prog = new OpenGL::ShaderProgramSimple(
    {
      { sh_vert, GL_VERTEX_SHADER },
      { sh_frag, GL_FRAGMENT_SHADER }
    } );


    GLuint vbo;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    //glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );
    glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), nullptr, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, GL_ZERO, varray.size()*sizeof(*varray.data()), varray.data() );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 7*sizeof(*varray.data()), 0 );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 7*sizeof(*varray.data()), (void*)(3*sizeof(*varray.data())) );
    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    
    GLuint ibo;
    glGenBuffers( 1, &ibo );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    //glBufferData( GL_ELEMENT_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, iarray.size()*sizeof(*iarray.data()), nullptr, GL_STATIC_DRAW );
    glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, GL_ZERO, iarray.size()*sizeof(*iarray.data()), iarray.data() );

    glBindVertexArray( 0 );

    // ...

    glBindVertexArray( vao );

    g_prog->Use();

    start_time = std::chrono::high_resolution_clock::now();
    
    view_matrix = glm::lookAt(glm::vec3(0.0f, -3.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    glutMainLoop(); 

    glutDestroyWindow(wnd);
    return 0;
}


void display( void )
{
    int vpSize[2]
    {
        glutGet(GLUT_WINDOW_WIDTH),
        glutGet(GLUT_WINDOW_HEIGHT)
    };
    glViewport( 0, 0, vpSize[0], vpSize[1] );
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    float aspect = (float)vpSize[0]/(float)vpSize[1];
    glm::mat4 project = glm::perspective( (float)M_PI/3.0f, aspect, 0.01f, 1000.0f );
        
    glm::mat4 view =
      glm::rotate(glm::mat4(1.0), roll_angle, glm::vec3(0, 0, 1)) *
      glm::rotate(glm::mat4(1.0), pitch_angle, glm::vec3(1, 0, 0)) *
      view_matrix;

    glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0));
           
    g_prog->SetUniformM44( "project", glm::value_ptr(project) );
    g_prog->SetUniformM44( "view", glm::value_ptr(view) );
    g_prog->SetUniformM44( "model", glm::value_ptr(model) );

    glEnable( GL_DEPTH_TEST );
      
    glDrawElements( GL_TRIANGLES, (GLsizei)iarray.size(), GL_UNSIGNED_INT, nullptr );

    glutSwapBuffers();
    glutPostRedisplay();
}



void keyboard( unsigned char key, int x, int y )
{
    float forward = 0.0f;
    float right = 0.0f;
    switch ( key )
    {
      case 'w': forward =  1.0f; break;
      case 's': forward = -1.0f; break;
      case 'a': right   =  1.0f;  break;
      case 'd': right   = -1.0f; break;
    }

    view_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(right*0.1f, 0.0f, forward*0.1f)) * view_matrix;
}

void special( int key, int x, int y )
{
  float yaw = 0.0f, pitch = 0.0f, roll = 0.0f;
  switch ( key )
  {
    case GLUT_KEY_RIGHT: yaw   =  1.0f; break;
    case GLUT_KEY_LEFT:  yaw   = -1.0f; break;
    case GLUT_KEY_UP:    pitch = -1.0f; break;
    case GLUT_KEY_DOWN:  pitch =  1.0f; break;
    case GLUT_KEY_HOME:  roll  = -1.0f; break;
    case GLUT_KEY_END:   roll  =  1.0f; break;
  }

  static float scale = 0.01f;

  float yaw_angle = yaw * scale; 
  view_matrix = glm::rotate(glm::mat4(1.0f), yaw_angle, glm::vec3(0.0f, 1.0f, 0.0f)) * view_matrix;
  
  pitch_angle += pitch * scale;
  roll_angle  += roll * scale;
}