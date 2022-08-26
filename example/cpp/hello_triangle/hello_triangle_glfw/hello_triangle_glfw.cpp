// GLM [https://glm.g-truc.net/0.9.9/api/index.html]
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// OpenGL
// GLEW [http://glew.sourceforge.net/]
#include <GL/glew.h>
#ifdef __APPLE__
//#include <OpenGL/gl.h>
//#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

// GLFW [https://www.glfw.org/]
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "../gl_debug.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const char* sh_vert = R"(
#version 410 core

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

const char* sh_frag = R"(
#version 410 core

in vec3 vertPos;
in vec4 vertCol;

out vec4 fragColor;

void main()
{
    fragColor = vertCol;
}
)";

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void saveScreenshotToFile(const char* filename, int width, int height);
GLuint CompileShader(GLenum ahader_type, const char* shader_code);
GLuint LinkProgram(std::initializer_list<GLuint> shader_objets);

bool debug_context = true;
bool screenshot = false;

int main()
{
    if ( glfwInit() == 0 )
        throw std::runtime_error( "error initializing glfw" );

    //glfwWindowHint(GLFW_REFRESH_RATE, 10);

#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glewExperimental = true;
#endif
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, debug_context ? GLFW_TRUE : GLFW_FALSE);
    GLFWwindow *wnd = glfwCreateWindow(800, 600, "GLFW OGL window", nullptr, nullptr);
    if ( wnd == nullptr )
    {
        glfwTerminate();
        throw std::runtime_error( "error initializing window" );
    }
    glfwSetKeyCallback(wnd, keyCallback);
    glfwMakeContextCurrent(wnd);
    if (glewInit() != GLEW_OK)
        throw std::runtime_error( "error initializing glew" );

    OpenGL::CContext::TDebugLevel debug_level = debug_context ? OpenGL::CContext::TDebugLevel::all : OpenGL::CContext::TDebugLevel::off;
    OpenGL::CContext context;
    context.init(debug_level);

    //glfwSwapInterval( 2 );

    std::cout << "shader" << std::endl;
    GLuint program_obj = LinkProgram({
        CompileShader(GL_VERTEX_SHADER, sh_vert),
        CompileShader(GL_FRAGMENT_SHADER, sh_frag)});
    GLint project_loc = glGetUniformLocation(program_obj, "project");
    GLint view_loc = glGetUniformLocation(program_obj, "view");
    GLint model_loc = glGetUniformLocation(program_obj, "model");
    glUseProgram(program_obj);

    static const std::vector<float> varray
            {
                    -0.866f, -0.5f,    1.0f, 0.0f, 0.0f, 1.0f,
                    0.866f, -0.5f,    1.0f, 1.0f, 0.0f, 1.0f,
                    0.0f,    1.0f,    0.0f, 0.0f, 1.0f, 1.0f
            };

    std::cout << "buffer" << std::endl;
    GLuint vbo;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    //glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );
    glBufferData( GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(varray.size()*sizeof(*varray.data())), nullptr, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, GL_ZERO, static_cast<GLsizeiptr>(varray.size()*sizeof(*varray.data())), varray.data() );

    std::cout << "vertex specification" << std::endl;
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6*sizeof(*varray.data()), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6*sizeof(*varray.data()), (void*)(2*sizeof(*varray.data())));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    std::cout << "application loop" << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();
    auto prev_time = start_time;
    while (!glfwWindowShouldClose(wnd))
    {
        std::chrono::high_resolution_clock::time_point current_time = std::chrono::high_resolution_clock::now();
        auto frame_time = current_time - prev_time;
        prev_time = current_time;
        auto all_time = current_time - start_time;
        double delta_s = (double)std::chrono::duration_cast<std::chrono::milliseconds>(frame_time).count() / 1000.0;
        double time_s = (double)std::chrono::duration_cast<std::chrono::milliseconds>(all_time).count() / 1000.0;
        std::string title = "FPS: " + std::to_string(1/ delta_s);
        glfwSetWindowTitle(wnd, title.c_str());

        auto angle = (float)(time_s * 90.0);

        int vpSize[2];
        glfwGetFramebufferSize(wnd, &vpSize[0], &vpSize[1]);

        float aspect = (float)vpSize[0] / (float)vpSize[1];
        float orthoX = aspect > 1.0f ? aspect : 1.0f;
        float orthoY = aspect > 1.0f ? 1.0f : aspect;

        glm::mat4 project = glm::ortho(-orthoX, orthoX, -orthoY, orthoY, -1.0f, 1.0f);

        float orthScale = 1.0f;
        project = glm::scale(project, glm::vec3(orthScale, orthScale, 1.0f));

        static bool invert = false;
        glm::mat4 view = glm::lookAt(
                glm::vec3(0.0f, 0.0f, 0.5f * (invert ? -1.0f : 1.0f)),
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 model( 1.0f );
        model = glm::translate(model, glm::vec3(0.1f, 0.0f, 1.0f));
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));

        glUniformMatrix4fv(project_loc, 1, GL_FALSE, glm::value_ptr(project));
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));

        glViewport(0, 0, vpSize[0], vpSize[1]);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(wnd);
        glfwPollEvents();

        if (screenshot) {
            screenshot = false;
            saveScreenshotToFile("screenshot.tga", vpSize[0], vpSize[1]);
        }
    }

    glfwDestroyWindow( wnd );
    glfwTerminate();

    return 0;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
        screenshot = true;
}

void saveScreenshotToFile(const char* filename, int width, int height) {
    short tgaHead[] = { 0, 2, 0, 0, 0, 0, (short)width, (short)height, 24 };
    std::vector<char> buffer(width*height*3);
    glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, buffer.data());
    FILE* out = fopen(filename, "w");
    std::fstream outFile;
    outFile.open(filename, std::ios::app | std::ios::binary);
    outFile.write(reinterpret_cast<char*>(&tgaHead), sizeof(tgaHead));
    outFile.write(buffer.data(), buffer.size());
    outFile.close();
}

bool CompileStatus(GLuint shader)
{
    GLint status = GL_TRUE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint logLen;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        std::vector< char >log(logLen);
        GLsizei written;
        glGetShaderInfoLog(shader, logLen, &written, log.data());
        std::cout << "compile error:" << std::endl << log.data() << std::endl;
    }
    return status != GL_FALSE;
}

bool LinkStatus(GLuint program)
{
    GLint status = GL_TRUE;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint logLen;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
        std::vector< char >log(logLen);
        GLsizei written;
        glGetProgramInfoLog(program, logLen, &written, log.data());
        std::cout << "link error:" << std::endl << log.data() << std::endl;
    }
    return status != GL_FALSE;
}

GLuint CompileShader(GLenum ahader_type, const char* shader_code)
{
    auto shader_obj = glCreateShader(ahader_type);
    glShaderSource(shader_obj, 1, &shader_code, nullptr);
    glCompileShader(shader_obj);
    CompileStatus(shader_obj);
    return shader_obj;
}

GLuint LinkProgram(std::initializer_list<GLuint> shader_objets)
{
    auto program_obj = glCreateProgram();
    for (auto shader_object: shader_objets)
    {
        glAttachShader(program_obj, shader_object);
    }
    glLinkProgram(program_obj);
    LinkStatus(program_obj);
    return program_obj;
}
