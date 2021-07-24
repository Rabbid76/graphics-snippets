#ifndef __EMSCRIPTEN__
#define USE_GLEW 0
#endif

#if USE_GLEW
#include "GL/glew.h"
#endif

#include <SDL/SDL.h>

#if !USE_GLEW
#include "SDL/SDL_opengl.h"
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <assert.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>

std::string vert_shader = R"(
#version 100
//precision mediump float;

attribute vec3 a_pos;

void main()
{
    gl_Position = vec4(a_pos, 1.0);
}
)";

std::string frag_shader = R"(
precision mediump float;

uniform float u_time;

vec3 HUEtoRGB(in float H)
{
    float R = abs(H * 6.0 - 3.0) - 1.0;
    float G = 2.0 - abs(H * 6.0 - 2.0);
    float B = 2.0 - abs(H * 6.0 - 4.0);
    return clamp( vec3(R,G,B), 0.0, 1.0 );
}

void main()
{
    float hue = fract(u_time);
    gl_FragColor = vec4(HUEtoRGB(hue), 1.0);
}
)";

bool CompileStatus(GLuint shader)
{
    GLint status = GL_TRUE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint logLen;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char>log( logLen );
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
        std::vector<char>log(logLen);
        GLsizei written;
        glGetProgramInfoLog( program, logLen, &written, log.data() );
        std::cout << "link error:" << std::endl << log.data() << std::endl;
    }
    return status != GL_FALSE;
}

GLuint shader_program_object = 0;
GLuint triangle_vbo;
GLint pos_attribute_index;
GLint time_uniform_location;
std::chrono::high_resolution_clock::time_point start_time;

void gl_init()
{
    auto vert_sh_obj = glCreateShader(GL_VERTEX_SHADER);
    const char *vert_shader_ptr = vert_shader.c_str();
    glShaderSource(vert_sh_obj, 1, &vert_shader_ptr, nullptr);
    glCompileShader(vert_sh_obj);
    CompileStatus(vert_sh_obj);

    auto frag_sh_obj = glCreateShader(GL_FRAGMENT_SHADER);
    const char *frag_shader_ptr = frag_shader.c_str();
    glShaderSource(frag_sh_obj, 1, &frag_shader_ptr, nullptr);
    glCompileShader(frag_sh_obj);
    CompileStatus(frag_sh_obj);

    shader_program_object = glCreateProgram();
    glAttachShader(shader_program_object, vert_sh_obj);
    glAttachShader(shader_program_object, frag_sh_obj);
    glLinkProgram(shader_program_object);
    LinkStatus(shader_program_object);

    glDeleteShader(vert_sh_obj);
    glDeleteShader(frag_sh_obj);

    pos_attribute_index = glGetAttribLocation(shader_program_object, "a_pos");
    std::cout << "`a_pos` attribute index: " << pos_attribute_index << std::endl;
    time_uniform_location = glGetUniformLocation(shader_program_object, "u_time");
    std::cout << "`u_time` uniform location: " << time_uniform_location << std::endl;

    static const std::vector<float> varray{ -0.866f, -0.75f, 0.0f, 0.866f, -0.75f, 0.0f, 0.0f, 0.75f, 0.0f };
    glGenBuffers(1, &triangle_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
    glBufferData(GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );

    start_time = std::chrono::high_resolution_clock::now();
}

bool sdl_events()
{
    bool end = false;
    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event))
    {
        switch (sdl_event.type)
        {
            case SDL_QUIT:
                end = false;
                break;

            case SDL_KEYDOWN:
                end = sdl_event.key.keysym.sym == SDLK_ESCAPE;
                break;
        }
    }
    return end;
}

void gl_draw()
{
    std::chrono::high_resolution_clock::time_point current_time = std::chrono::high_resolution_clock::now();
    auto delta_time = current_time - start_time;
    double time_s = (double)std::chrono::duration_cast<std::chrono::milliseconds>(delta_time).count() / 1000;

    sdl_events();

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program_object);
    glUniform1f(time_uniform_location, static_cast<float>(time_s));

    glEnableVertexAttribArray(pos_attribute_index);
    glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
    glVertexAttribPointer(pos_attribute_index, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

EM_BOOL gl_animation_frame(double time, void* userData) 
{
    gl_draw();
    return EM_TRUE;
}

int main(int argc, char *argv[])
{
    std::cout << "initialize view ..." << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cout << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    int width = 800;
    int height = 450;
#ifdef __EMSCRIPTEN__
    SDL_Surface *screen = SDL_SetVideoMode( width, height, 16, SDL_OPENGL );
    if ( !screen ) 
    {
        std::cout << "Unable to set video mode: " << SDL_GetError() << std::endl;
        return 1;
    }
#else
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Window *window = SDL_CreateWindow("OpenGL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);
    if (!window) 
    {
        std::cout << "Couldn't create window: " << SDL_GetError() << std::endl;
        return 0;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) 
    {
        std::cout << "Couldn't create context: " << SDL_GetError() << std::endl;
        return 0;
    }
#endif

    std::cout << "initialize OpenGL/WebGL ..." << std::endl;
    gl_init();
     
    std::cout << "run application ..." << std::endl; 
#ifdef __EMSCRIPTEN__
    //emscripten_request_animation_frame_loop(gl_animation_frame, 0);
    emscripten_set_main_loop(gl_draw, 0, false);
#else
    while (!sdl_events())
    {
        gl_draw();
        SDL_GL_SwapBuffers();
    }
    std::cout << "quit..." << std::endl; 
    SDL_Quit();
#endif
    
    return 0;
}
