// Dashed line in OpenGL3?
// [https://stackoverflow.com/questions/52928678/dashed-line-in-opengl3/54543267#54543267]


// STL
#include <iostream>
#include <string>
#include <stdexcept>
#include <chrono>

#define _USE_MATH_DEFINES
#include <math.h>

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



// shader

std::string vertShader = R"(
#version 330

layout (location = 0) in vec3 inPos;
layout (location = 1) in float inDist;

out float dist;

uniform mat4 u_mvp;

void main()
{
    dist        = inDist;
	gl_Position = u_mvp * vec4(inPos, 1.0);
}
)";

std::string fragShader = R"(
#version 330

in float dist;

out vec4 fragColor;

uniform vec2  u_resolution;
uniform float u_dashSize;
uniform float u_gapSize;

void main()
{
    if (fract(dist / (u_dashSize + u_gapSize)) > u_dashSize/(u_dashSize + u_gapSize))
        discard; 
    fragColor = vec4(1.0);
}
)";


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

    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
    OpenGL::CContext context;
    context.Init( debug_level );


    GLuint program = OpenGL::CreateProgram(vertShader, fragShader);
    GLint loc_mvp  = glGetUniformLocation(program, "u_mvp");
    GLint loc_res  = glGetUniformLocation(program, "u_resolution");
    GLint loc_dash = glGetUniformLocation(program, "u_dashSize");
    GLint loc_gap  = glGetUniformLocation(program, "u_gapSize");
        
    glUseProgram(program);
    glUniform1f(loc_dash, 10.0f);
    glUniform1f(loc_gap, 10.0f);

    std::vector<glm::vec3> varray;
    for (size_t u=0; u <= 360; ++u)
    {
        double a = u*M_PI/180.0;
        double c = cos(a), s = sin(a);
        varray.emplace_back(glm::vec3((float)c, (float)s, 0.0f));
    }
    std::vector<float> darray(varray.size(), 0.0f);

    GLuint bo[2], vao;
    glGenBuffers(2, bo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0); 
    glEnableVertexAttribArray(1); 
    glBindBuffer(GL_ARRAY_BUFFER, bo[0] );
    glBufferData(GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 
    glBindBuffer(GL_ARRAY_BUFFER, bo[1] );
    glBufferData(GL_ARRAY_BUFFER, darray.size()*sizeof(*darray.data()), darray.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0); 
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 project, wndmat;
    int vpSize[2]{0, 0};
    while (!glfwWindowShouldClose(window))
    {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        if (w != vpSize[0] ||  h != vpSize[1])
        {
            vpSize[0] = w; vpSize[1] = h;
            glViewport(0, 0, vpSize[0], vpSize[1]);
            project = glm::perspective(glm::radians(90.0f), (float)w/(float)h, 0.1f, 10.0f);
            glUniform2f(loc_res, (float)w, (float)h);
            wndmat = glm::scale(glm::mat4(1.0f), glm::vec3((float)w/2.0f, (float)h/2.0f, 1.0f));
            wndmat = glm::translate(wndmat, glm::vec3(1.0f, 1.0f, 0.0f));
        }
             
        static float angle = 1.0f;
        glm::mat4 modelview( 1.0f );
        modelview = glm::translate(modelview, glm::vec3(0.0f, 0.0f, -2.0f) );
        modelview = glm::rotate(modelview, glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f));
        modelview = glm::rotate(modelview, glm::radians(angle*0.5f), glm::vec3(0.0f, 1.0f, 0.0f));
        angle += 0.5f;
        glm::mat4 mvp = project * modelview;

        glm::vec2 vpPt(0.0f, 0.0f);
        float dist = 0.0f;
        for (size_t i=0; i < varray.size(); ++i)
        {
            darray[i] = dist;
            glm::vec4 clip = mvp * glm::vec4(varray[i], 1.0f);
            glm::vec4 ndc  = clip / clip.w;
            glm::vec4 vpC  = wndmat * ndc;
            float len = i==0 ? 0.0f :  glm::length(vpPt - glm::vec2(vpC));
            vpPt = glm::vec2(vpC);
            dist += len;
        }
        glBufferSubData(GL_ARRAY_BUFFER, 0, darray.size()*sizeof(*darray.data()), darray.data());
        
        glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)varray.size());
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    return 0;
}
    
