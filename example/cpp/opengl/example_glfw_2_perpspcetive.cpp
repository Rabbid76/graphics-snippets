// how to warp texture in openGL? (Perspective correction?)
// https://stackoverflow.com/questions/60418001/how-to-warp-texture-in-opengl-perspective-correction

// STL
#include <iostream>
#include <string>
#include <stdexcept>
#include <chrono>
#include <filesystem>

// GLM [https://glm.g-truc.net/0.9.9/api/index.html]
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// OpenGL
#include <gl/gl_glew.h>

// GLFW [https://www.glfw.org/]
#include <GLFW/glfw3.h>


// STB 
#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
//#include <stb_image_write.h>

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

layout (location = 0) in vec4 inPos;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 inUV;

out vec4 vertCol;
out vec2 vertUV;

void main()
{  
    vertCol     = inColor;
    vertUV      = inUV;
	gl_Position = inPos * vec4(vec2(0.8), 1.0, 1.0);
}
)";

std::string sh_frag = R"(
#version 410 core

in vec4 vertCol;
in vec2 vertUV;

out vec4 fragColor;
//layout(binding = 0) uniform sampler2D u_texture;
uniform sampler2D u_texture;

void main()
{
    fragColor = texture(u_texture, vertUV.st);
}
)";


// main

int main(void)
{
    if ( glfwInit() == 0 )
        throw std::runtime_error( "error initializing glfw" );

    //glfwWindowHint(GLFW_REFRESH_RATE, 10);

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glewExperimental = true;
#endif
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, debug_level != OpenGL::CContext::TDebugLevel::off ? GLFW_TRUE : GLFW_FALSE);
    GLFWwindow *wnd = glfwCreateWindow( 400, 400, "GLFW OGL window", nullptr, nullptr );
    if ( wnd == nullptr )
    {
        glfwTerminate();
        throw std::runtime_error( "error initializing window" ); 
    }

    glfwMakeContextCurrent(wnd);

    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    OpenGL::CContext context;
    context.Init( debug_level );

    //glfwSwapInterval( 2 );

    std::cout << "Current path is " << std::filesystem::current_path() << '\n';
    //std::string path = "C:/source/rabbid76workbench/test/texture/boomerang.png";
    std::string path = "../../resource/texture/ObjectSheet.png";
    int cx, cy, ch;
    stbi_uc *img = stbi_load( path.c_str(), &cx, &cy, &ch, 4 );
    if ( img == nullptr )
        std::cout << "error loading texture" << std::endl;

    GLuint tex_obj;
    glGenTextures(1, &tex_obj);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_2D, tex_obj);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  //for BMP
    
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RED, GL_FLOAT, values);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, cx, cy, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
    stbi_image_free( img );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    //glGenerateMipmap( GL_TEXTURE_2D );
          
    //glBindTexture( GL_TEXTURE_2D, 0 );

    GLint program_obj = OpenGL::LinkProgram({
        OpenGL::CompileShader(GL_VERTEX_SHADER, sh_vert.c_str()), 
        OpenGL::CompileShader(GL_FRAGMENT_SHADER, sh_frag.c_str())});
    glUseProgram(program_obj);
    
    static const std::vector<float> varray
    { 
        1.0f,  1.0f,  0.5f, 2.0f,   1.0f, 0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        1.0f, -1.0f, -0.5f, 1.0f,   1.0f, 1.0f, 0.0f, 1.0f,  1.0f, 0.0f,
       -1.0f, -1.0f, -0.5f, 1.0f,   0.0f, 1.0f, 0.0f, 1.0f,  0.0f, 0.0f, 
       -1.0f,  1.0f,  0.5f, 2.0f,   0.0f, 0.0f, 1.0f, 1.0f,  0.0f, 1.0f
    };

    GLuint vbo;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    //glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );
    glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), nullptr, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, GL_ZERO, varray.size()*sizeof(*varray.data()), varray.data() );

    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 10*sizeof(*varray.data()), 0 );
    glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 10*sizeof(*varray.data()), (void*)(4*sizeof(*varray.data())) );
    glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 10*sizeof(*varray.data()), (void*)(8*sizeof(*varray.data())) );
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glEnableVertexAttribArray( 2 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
      
    while (!glfwWindowShouldClose(wnd))
    {
        static float angle = 1.0f;

        int vpSize[2];
        glfwGetFramebufferSize( wnd, &vpSize[0], &vpSize[1] );
        
        float aspect = (float)vpSize[0] / (float)vpSize[1];
        float orthoX = aspect > 1.0f ? aspect : 1.0f;
        float orthoY = aspect > 1.0f ? 1.0f : aspect;

        glViewport( 0, 0, vpSize[0], vpSize[1] );
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      
        glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );

        glfwSwapBuffers(wnd);
        glfwPollEvents();
    }

    glfwDestroyWindow( wnd );
    wnd = nullptr;
    glfwTerminate();

    return 0;
}
    
