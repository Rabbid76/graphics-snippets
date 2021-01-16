// Dashed line in OpenGL3?
// [https://stackoverflow.com/questions/52928678/dashed-line-in-opengl3/54543267#54543267]


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

std::string vertShader = R"(
#version 330

layout (location = 0) in vec3 inPos;

flat out vec3 startPos;
out vec3 vertPos;

uniform mat4 u_mvp;

void main()
{
	vec4 pos    = u_mvp * vec4(inPos, 1.0);
    gl_Position = pos;
    vertPos     = pos.xyz / pos.w;
    startPos    = vertPos;
}
)";

std::string fragShader = R"(
#version 330

flat in vec3 startPos;
in vec3 vertPos;

out vec4 fragColor;

uniform vec2  u_resolution;
//uniform uint  u_pattern;
//uniform float u_factor;

void main()
{
    vec2  dir  = (vertPos.xy-startPos.xy) * u_resolution/2.0;
    float dist = length(dir);

    //uint bit = uint(round(dist / u_factor)) & 15U;
    //if ((u_pattern & (1U<<bit)) == 0U)
    //    discard; 
    fragColor = vec4(1.0);
}
)";


// main

GLuint CreateVAO(std::vector<glm::vec3> &varray)
{
    GLuint bo[2], vao;
    glGenBuffers(2, bo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0); 
    glBindBuffer(GL_ARRAY_BUFFER, bo[0] );
    glBufferData(GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 

    return vao;
}

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


    GLuint program    = OpenGL::CreateProgram(vertShader, fragShader);
    GLint loc_mvp     = glGetUniformLocation(program, "u_mvp");
    GLint loc_res     = glGetUniformLocation(program, "u_resolution");
    GLint loc_pattern = glGetUniformLocation(program, "u_pattern");
    GLint loc_factor  = glGetUniformLocation(program, "u_factor");

    glUseProgram(program);

    GLushort pattern = 0x18ff;
    GLfloat  factor  = 2.0f;
    //glUniform1ui(loc_pattern, pattern);
    //glUniform1f(loc_factor, factor);
    glLineStipple(2.0, pattern);
    glEnable(GL_LINE_STIPPLE);

    glm::vec3 p0(-1.0f, -1.0f, 0.0f);
    glm::vec3 p1(1.0f, -1.0f, 0.0f);
    glm::vec3 p2(1.0f, 1.0f, 0.0f);
    glm::vec3 p3(-1.0f, 1.0f, 0.0f);   
    std::vector<glm::vec3> varray1{ p0, p1, p1, p2, p2, p3, p3, p0 };
    GLuint vao1 = CreateVAO(varray1);

    std::vector<glm::vec3> varray2;
    for (size_t u=0; u <= 360; u += 8)
    {
        double a = u*M_PI/180.0;
        double c = cos(a), s = sin(a);
        varray2.emplace_back(glm::vec3((float)c, (float)s, 0.0f));
    }
    GLuint vao2 = CreateVAO(varray2);
    
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
            glUniform2f(loc_res, (float)w, (float)h);
        }
             
        glClear(GL_COLOR_BUFFER_BIT);

        glm::mat4 modelview1( 1.0f );
        modelview1 = glm::translate(modelview1, glm::vec3(-0.6f, 0.0f, 0.0f) );
        modelview1 = glm::scale(modelview1, glm::vec3(0.5f, 0.5f, 1.0f) );
        glm::mat4 mvp1 = project * modelview1;
        
        glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp1));
        glBindVertexArray(vao1);
        glDrawArrays(GL_LINES, 0, (GLsizei)varray1.size());

        glm::mat4 modelview2( 1.0f );
        modelview2 = glm::translate(modelview2, glm::vec3(0.6f, 0.0f, 0.0f) );
        modelview2 = glm::scale(modelview2, glm::vec3(0.5f, 0.5f, 1.0f) );
        glm::mat4 mvp2 = project * modelview2;
        
        glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp2));
        glBindVertexArray(vao2);
        glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)varray2.size());
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    return 0;
}
    
