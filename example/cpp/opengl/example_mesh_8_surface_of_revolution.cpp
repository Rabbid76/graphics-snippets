// How to compute vertex normals for a triangle mesh in OpenGl?
// https://stackoverflow.com/questions/61241414/how-to-compute-vertex-normals-for-a-triangle-mesh-in-opengl

// Surface of revolution
// https://en.wikipedia.org/wiki/Surface_of_revolution

#include <stdafx.h>

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

out vec3 fpos;
out vec3 Normal;

layout (location = 0) uniform mat4 project;
layout (location = 1) uniform mat4 view;
layout (location = 2) uniform mat4 model;

void main()
{  
    mat4 modelView = view * model;
    mat3 normalMat = mat3(inverse(transpose(modelView)));
    vec4 view_pos  = modelView * vec4(inPos, 1.0);

	  fpos        = view_pos.xyz;
    Normal     = normalMat * inNormal;
	  gl_Position = project * vec4(view_pos.xyz, 1.0);
}
)";

std::string sh_frag = R"(
#version 410

// Define INPUTS from fragment shader
//uniform mat4 view_mat;
in vec3 Normal;
in vec3 fpos;

// These come from the VAO for texture coordinates.
in vec2 texture_coords;

// And from the uniform outputs for the textures setup in main.cpp.
uniform sampler2D texture00;
uniform sampler2D texture01;

out vec4 fragment_color; //RGBA color

const vec3 lightPos = vec3(0.0,0.0,5.0);
const vec3 diffColor = vec3(1.0,0.5,0.0);
const vec3 specColor = vec3(1.0,1.0,1.0);

void main () {
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(-fpos);
    if (dot(normal, viewDir) < 0.0)
        normal *= -1;
  
    vec3 lightDir = normalize(lightPos - fpos);
    float lamb = max(dot(lightDir, normal), 0.0);
    float spec = 0.0;

    if (lamb > 0.0) {
        vec3 refDir = reflect(-lightDir, normal);

        float specAngle = max(dot(refDir, viewDir), 0.0);
        spec = pow(specAngle, 4.0);
    }

    fragment_color = vec4(lamb * diffColor + spec * specColor, 1.0);
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
GLuint vbos[]{ 0, 0 };
GLsizei no_of_elems = 0;
GLuint program_obj = 0; 

void init(void)
{
    GLuint program_obj = OpenGL::CreateProgram(sh_vert, sh_frag);
    glUseProgram(program_obj);

    GLfloat vp[49 * 49 * 18];    // array of vertex points

    int _i = 50;
    int _j = 50;
    float vertices[50][50][3];
    for (int i = 0; i < _i; i++) {
        float fT = (float) i / (_i - 1);
        float fY = fT;
        float fZ = sqrt(fT);
        for (int j = 0; j < _j; j++) {
            float fS = 2 * M_PI * (float) j / (_j - 1);
            vertices[i][j][0] = fZ * cos(fS);
            vertices[i][j][1] = fY - 0.5; // offset by 0.5 to make center of mass the center
            vertices[i][j][2] = fZ * sin(fS);
        }
    }
    int curr = 0;
    for (int i = 0; i < _i - 1; i++) {
        for (int j = 0; j < _j - 1; j++) {
            vp[curr++] = vertices[i][j][0];
            vp[curr++] = vertices[i][j][1];
            vp[curr++] = vertices[i][j][2];
            vp[curr++] = vertices[i+1][j][0];
            vp[curr++] = vertices[i+1][j][1];
            vp[curr++] = vertices[i+1][j][2];
            vp[curr++] = vertices[i][j+1][0];
            vp[curr++] = vertices[i][j+1][1];
            vp[curr++] = vertices[i][j+1][2];
            vp[curr++] = vertices[i+1][j][0];
            vp[curr++] = vertices[i+1][j][1];
            vp[curr++] = vertices[i+1][j][2];
            vp[curr++] = vertices[i+1][j+1][0];
            vp[curr++] = vertices[i+1][j+1][1];
            vp[curr++] = vertices[i+1][j+1][2];
            vp[curr++] = vertices[i][j+1][0];
            vp[curr++] = vertices[i][j+1][1];
            vp[curr++] = vertices[i][j+1][2];
        }
    }

    GLfloat normals[49 * 49 * 18];
    curr = 0;
    for (int i = 0; i < 49 * 49 * 18; i += 9){
        float Ux = vp[i+3] - vp[i];
        float Uy = vp[i+4] - vp[i+1];
        float Uz = vp[i+5] - vp[i+2];
        float Vx = vp[i+6] - vp[i];
        float Vy = vp[i+7] - vp[i+1];
        float Vz = vp[i+8] - vp[i+2];

        float nx = Uy * Vz - Uz * Vy;
        float ny = Uz * Vx - Ux * Vz;
        float nz = Ux * Vy - Uy * Vx;

        for (int j = 0; j < 3; ++j) {
            normals[curr++] = nx;
            normals[curr++] = ny;
            normals[curr++] = nz;
        }
    }

    glGenBuffers( 2, vbos );
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    glBindBuffer( GL_ARRAY_BUFFER, vbos[0] );
    glBufferData( GL_ARRAY_BUFFER, 49 * 49 * 18 * sizeof(GLfloat), vp, GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );
    
    glBindBuffer( GL_ARRAY_BUFFER, vbos[1] );
    glBufferData( GL_ARRAY_BUFFER, 49 * 49 * 18 * sizeof(GLfloat), normals, GL_STATIC_DRAW );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, nullptr );
    
    glEnableVertexAttribArray( 0 );
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
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
    glDrawArrays( GL_TRIANGLES, 0, 49 * 49 * 18 / 3 );
}