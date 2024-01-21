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


// SFML [https://www.sfml-dev.org/]
//#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
//#include <SFML/Window.hpp>


// project includes
#include <gl/gl_debug.h>
#include <gl/opengl_shader.h>

// preprocessor definitions

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

// shader

std::string sh_vert = R"(
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

std::string sh_frag = R"(
#version 410 core

in vec3 vertPos;
in vec4 vertCol;

out vec4 fragColor;

void main()
{
    fragColor = vertCol;
}
)";


// main

int main(void)
{
    sf::VideoMode videoMode(sf::Vector2u(640, 480));

    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 4;
#ifdef __APPLE__
    settings.majorVersion = 4;
    settings.minorVersion = 1;
    settings.attributeFlags = sf::ContextSettings::Core | sf::ContextSettings::Debug;
        //| sf::ContextSettings::attributeFlags::;
#else
    settings.majorVersion = 4;
    settings.minorVersion = 6;
#endif

    sf::RenderWindow mainWindow;
    mainWindow.create(videoMode, sf::String("Test"), sf::Style::Default, sf::State::Windowed, settings);
    //mainWindow.setPosition(sf::Vector2i(0, 0));

    if (!mainWindow.isOpen())
        throw std::runtime_error("error initializing window");

    if (glewInit() != GLEW_OK)
        throw std::runtime_error("error initializing glew");

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
    OpenGL::CContext context;
    context.Init(debug_level);

    std::cout << "shader" << std::endl;
    GLint program_obj = OpenGL::LinkProgram({
        OpenGL::CompileShader(GL_VERTEX_SHADER, sh_vert.c_str()), 
        OpenGL::CompileShader(GL_FRAGMENT_SHADER, sh_frag.c_str())});
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

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    //glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );
    glBufferData(GL_ARRAY_BUFFER, varray.size() * sizeof(*varray.data()), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, GL_ZERO, varray.size() * sizeof(*varray.data()), varray.data());

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(*varray.data()), 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(*varray.data()), (void*)(2 * sizeof(*varray.data())));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    GLenum error0 = glGetError();
    GLuint b0 = 111, b1 = 222, b2 = 333;
    glGetVertexAttribIuiv(0, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &b0);
    GLenum error1 = glGetError();
    glGetVertexAttribIuiv(1, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &b1);
    GLenum error2 = glGetError();
    glGetVertexAttribIuiv(2, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &b2);
    GLenum error3 = glGetError();

    auto start_time = std::chrono::high_resolution_clock::now();
    auto prev_time = start_time;
    
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    sf::Time elapsedTime;
    bool running = true;
    while (running)
    {
        std::chrono::high_resolution_clock::time_point current_time = std::chrono::high_resolution_clock::now();
        auto frame_time = current_time - prev_time;
        prev_time = current_time;
        auto all_time = current_time - start_time;
        double delta_s = (double)std::chrono::duration_cast<std::chrono::milliseconds>(frame_time).count() / 1000.0;
        double time_s = (double)std::chrono::duration_cast<std::chrono::milliseconds>(all_time).count() / 1000.0;
        std::string title = "FPS: " + std::to_string(1 / delta_s);
        sf::String sfmlString(title.c_str());
        mainWindow.setTitle(sfmlString);

        sf::Vector2i mousePosition = sf::Mouse::getPosition(mainWindow);
        sf::Event sfmlEvent;
        while (mainWindow.pollEvent(sfmlEvent))
        {
            if (sfmlEvent.type == sf::Event::Closed)
                running = false;
            if (sfmlEvent.type == sf::Event::KeyPressed)
            {
                if (sfmlEvent.key.code == sf::Keyboard::Key::Escape)
                    running = false;
            }
        }

        float angle = (float)(time_s * 90.0);

        sf::Vector2u vpSize = mainWindow.getSize();

        float aspect = (float)vpSize.x / (float)vpSize.y;
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

        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(0.1f, 0.0f, 1.0f));
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));

        glUniformMatrix4fv(project_loc, 1, GL_FALSE, glm::value_ptr(project));
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));

        glViewport(0, 0, vpSize.x, vpSize.y);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        mainWindow.display();
    }

    mainWindow.close();
    return 0;
}
