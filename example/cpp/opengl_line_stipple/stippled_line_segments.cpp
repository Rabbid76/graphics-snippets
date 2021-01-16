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
uniform float u_dashSize;
uniform float u_gapSize;
uniform uint u_pattern;

void main()
{
    vec2  dir  = (vertPos.xy-startPos.xy) * u_resolution/2.0;
    float dist = length(dir);

    uint bit = uint(round(dist)) & 31U;
    if ((u_pattern & (1U<<bit)) == 0U)
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


    GLuint program    = OpenGL::CreateProgram(vertShader, fragShader);
    GLint loc_mvp     = glGetUniformLocation(program, "u_mvp");
    GLint loc_res     = glGetUniformLocation(program, "u_resolution");
    GLint loc_dash    = glGetUniformLocation(program, "u_dashSize");
    GLint loc_gap     = glGetUniformLocation(program, "u_gapSize");
    GLint loc_pattern = glGetUniformLocation(program, "u_pattern");

    glUseProgram(program);
    glUniform1f(loc_dash, 10.0f);
    glUniform1f(loc_gap, 10.0f);

    GLuint pattern = 0x060fffff;
    glUniform1ui(loc_pattern, pattern);

    std::vector<float> varray{
        -1, -1, -1,   1, -1, -1,   1, 1, -1,   -1, 1, -1,
        -1, -1,  1,   1, -1,  1,   1, 1,  1,   -1, 1,  1
    };
    std::vector<unsigned int> iarray{
        0, 1, 1, 2, 2, 3, 3, 0, 
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7
    };

    GLuint bo[2], vao;
    glGenBuffers(2, bo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0); 
    glBindBuffer(GL_ARRAY_BUFFER, bo[0] );
    glBufferData(GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bo[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, iarray.size()*sizeof(*iarray.data()), iarray.data(), GL_STATIC_DRAW);
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 project;
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
        }
             
        static float angle = 1.0f;
        glm::mat4 modelview( 1.0f );
        modelview = glm::translate(modelview, glm::vec3(0.0f, 0.0f, -3.0f) );
        modelview = glm::rotate(modelview, glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f));
        modelview = glm::rotate(modelview, glm::radians(angle*0.5f), glm::vec3(0.0f, 1.0f, 0.0f));
        angle += 0.5f;
        glm::mat4 mvp = project * modelview;
        
        glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawElements(GL_LINES, (GLsizei)iarray.size(), GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    return 0;
}
    
