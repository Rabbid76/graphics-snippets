// How to implement flat shading in OpenGL without duplicate vertices?
// https://stackoverflow.com/questions/60022613/how-to-implement-flat-shading-in-opengl-without-duplicate-vertices/60025119#60025119

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


// GLFW [https://www.glfw.org/]
#include <GLFW/glfw3.h>


// project includes
#include <gl/gl_debug.h>
#include <gl/gl_shader.h>

// preprocessor definitions

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

// shader

std::string sh_vert = R"(
#version 460

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;

out vec3 vPos;
flat out vec3 vNormal;

layout (location = 0) uniform mat4 project;
layout (location = 1) uniform mat4 view;
layout (location = 2) uniform mat4 model;

void main()
{  
    mat4 modelView = view * model;
    mat3 normalMat = mat3(inverse(transpose(modelView)));
    vec4 view_pos  = modelView * vec4(inPos, 1.0);

	  vPos        = view_pos.xyz;
    vNormal     = normalMat * inNormal;
	  gl_Position = project * vec4(view_pos.xyz, 1.0);
}
)";

std::string sh_frag = R"(
#version 460

in vec3 vPos;
flat in vec3 vNormal;

out vec4 fragColor;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 L = vec3(0.0, 0.0, 1.0);
    float NdotL = max(0.0, dot(N, L));
    float light = NdotL;

    fragColor = vec4(vec3(light), 1.0);
}
)";


// function prototypes

void init(void);
void draw(int vpSize[]);

// main

int main(void)
{
    if ( glfwInit() == 0 )
        throw std::runtime_error( "error initializing glfw" );

    //glfwWindowHint(GLFW_REFRESH_RATE, 10);

    GLFWwindow *wnd = glfwCreateWindow( 800, 600, "GLFW OGL window", nullptr, nullptr );
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

    //glfwSwapInterval( 2 );

    init();

    while (!glfwWindowShouldClose(wnd))
    {
        int vpSize[2];
        glfwGetFramebufferSize( wnd, &vpSize[0], &vpSize[1] );

        draw(vpSize);

        glfwSwapBuffers(wnd);
        glfwPollEvents();
    }

    glfwDestroyWindow( wnd );
    wnd = nullptr;
    glfwTerminate();

    return 0;
}

GLuint vao = 0;
GLuint vbo = 0;
GLuint ibo = 0;
GLsizei no_of_elems = 0;
GLuint program_obj = 0; 

void init(void)
{
    GLuint program_obj = OpenGL::CreateProgram(sh_vert, sh_frag);
    glUseProgram(program_obj);

/*
  front      back

  1     3    7     5
   +---+      +---+
   |   |      |   |
   +---+      +---+
  0     2    6     4
*/

    static const std::vector<float> varray
    { 
    //  x   y   z   nx, ny, nz
       -1, -1, -1,   0, -1,  0,  // 0, nv front
       -1, -1,  1,   0,  0,  1,  // 1, nv top
        1, -1, -1,   0,  0,  0,  // 2
        1, -1,  1,   1,  0,  0,  // 3, nv right
        1,  1, -1,   0,  1,  0,  // 4, nv back
        1,  1,  1,   0,  0,  0,  // 5
       -1,  1, -1,   0,  0, -1,  // 6, nv bottom
       -1,  1,  1,  -1,  0,  0,  // 7, nv left 
    };

    glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
    static const std::vector<unsigned int> iarray
    { 
        0, 2, 3,   0, 3, 1, // front
        4, 6, 7,   4, 7, 5, // back
        3, 2, 4,   3, 4, 5, // right
        7, 6, 0,   7, 0, 1, // left
        6, 4, 2,   6, 2, 0, // bottom 
        1, 3, 5,   1, 5, 7  // top
    };

    /*
    glProvokingVertex(GL_LAST_VERTEX_CONVENTION);
    static const std::vector<unsigned int> iarray
    { 
        2, 3, 0,  3, 1, 0, // front
        6, 7, 4,  7, 5, 4, // back
        2, 4, 3,  4, 5, 3, // right
        6, 0, 7,  0, 1, 7, // left
        4, 2, 6,  2, 0, 6, // bottom 
        3, 5, 1,  5, 7, 1  // top
    };
    */
    
    no_of_elems = (GLsizei)iarray.size();

    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );
    
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    glGenBuffers( 1, &ibo );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, varray.size()*sizeof(*iarray.data()), iarray.data(), GL_STATIC_DRAW );

    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(*varray.data()), 0 );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(*varray.data()), (void*)(3*sizeof(*varray.data())) );
    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    glEnable(GL_DEPTH_TEST);
}

void draw(int vpSize[])
{
    static float angle = 1.0f;

    float aspect = (float)vpSize[0] / (float)vpSize[1];
    glm::mat4 project = glm::perspective(glm::radians(90.0f), aspect, 0.1f, 10.0f);
    
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 model( 1.0f );
    model = glm::rotate( model, glm::radians(angle), glm::vec3(1.0f, 0.5f, 0.0f) );
    angle += 1.0f;
    
    glUniformMatrix4fv( 0, 1, GL_FALSE, glm::value_ptr(project) );
    glUniformMatrix4fv( 1, 1, GL_FALSE, glm::value_ptr(view) );
    glUniformMatrix4fv( 2, 1, GL_FALSE, glm::value_ptr(model) );

    glViewport( 0, 0, vpSize[0], vpSize[1] );
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
    glDrawElements( GL_TRIANGLES, no_of_elems, GL_UNSIGNED_INT, nullptr );
}