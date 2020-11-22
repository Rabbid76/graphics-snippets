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

// glfw
#include <GLFW/glfw3.h>

// stb 
#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
//#include <stb_image_write.h>

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

// project includes
#include <gl/gl_debug.h>
#include <gl/gl_shader.h>

// preprocessor definitions

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif


std::string sh_vert_nearest = R"(
#version 400

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTex;
layout (location = 2) in vec4 inColor;

out vec3 vertPos;
out vec2 vertTex;
out vec4 vertCol;

uniform mat4 project;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vec4 view_pos = view * model * vec4(inPos, 1.0);
 
    vertTex     = inTex;
    vertCol     = inColor;
		vertPos     = view_pos.xyz;
		gl_Position = project * vec4(view_pos.xyz, 1.0);
}
)";

std::string sh_frag_nearest = R"(
#version 420

in vec3 vertPos;
in vec2 vertTex;
in vec4 vertCol;

out vec4 fragColor;

layout (binding = 1) uniform sampler2D u_image;
layout (binding = 2) uniform sampler2D u_table;

void main()
{
    vec2 tiles    = vec2(8.0, 8.0);
    vec2 tileSize = vec2(64.0);

    vec4 imageColor = texture(u_image, vertTex);

    float index = imageColor.b * (tiles.x * tiles.y - 1.0);
    
    vec2 tileIndex;
    tileIndex.y = floor(index / tiles.x);
    tileIndex.x = floor(index - tileIndex.y * tiles.x);

    vec2 tileUV = mix(0.5/tileSize, (tileSize-0.5)/tileSize, imageColor.rg);

    vec2 tableUV = tileIndex / tiles + tileUV / tiles;
    
    vec3 lookUpColor = texture(u_table, tableUV).rgb;

    fragColor = vec4(lookUpColor, 1.0);
    fragColor = vertTex.x+vertTex.y >= 1.0 ? vec4(lookUpColor, 1.0) : imageColor;
}
)";


std::string sh_vert_linear = R"(
#version 400

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTex;
layout (location = 2) in vec4 inColor;

out vec3 vertPos;
out vec2 vertTex;
out vec4 vertCol;

uniform mat4 project;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vec4 view_pos = view * model * vec4(inPos, 1.0);
 
    vertTex     = inTex;
    vertCol     = inColor;
		vertPos     = view_pos.xyz;
		gl_Position = project * vec4(view_pos.xyz, 1.0);
}
)";

std::string sh_frag_linear = R"(
#version 420

in vec3 vertPos;
in vec2 vertTex;
in vec4 vertCol;

out vec4 fragColor;

layout (binding = 1) uniform sampler2D u_image;
layout (binding = 2) uniform sampler2D u_table;

void main()
{
    vec2 tiles    = vec2(8.0, 8.0);
    vec2 tileSize = vec2(64.0);

    vec4 imageColor = texture(u_image, vertTex);

    float index     = imageColor.b * (tiles.x * tiles.y - 1.0);
    float index_min = min(62.0, floor(index));
    float index_max = index_min + 1.0;
    
    vec2 tileIndex_min;
    tileIndex_min.y = floor(index_min / tiles.x);
    tileIndex_min.x = floor(index_min - tileIndex_min.y * tiles.x);
    vec2 tileIndex_max;
    tileIndex_max.y = floor(index_max / tiles.x);
    tileIndex_max.x = floor(index_max - tileIndex_max.y * tiles.x);

    vec2 tileUV = mix(0.5/tileSize, (tileSize-0.5)/tileSize, imageColor.rg);

    vec2 tableUV_1 = tileIndex_min / tiles + tileUV / tiles;
    vec2 tableUV_2 = tileIndex_max / tiles + tileUV / tiles;
    
    vec3 lookUpColor_1 = texture(u_table, tableUV_1).rgb;
    vec3 lookUpColor_2 = texture(u_table, tableUV_2).rgb;
    vec3 lookUpColor   = mix(lookUpColor_1, lookUpColor_2, index-index_min); 

    fragColor = vec4(lookUpColor, 1.0);
    fragColor = vertTex.x+vertTex.y >= 1.0 ? vec4(lookUpColor, 1.0) : imageColor;
}
)";

int main(void)
{
    std::string dir = "";// argc > 1 ? argv[1] : "";

    if ( glfwInit() == 0 )
        throw std::runtime_error( "error initializing glfw" );

    GLFWwindow *wnd = glfwCreateWindow( 800, 600, "OGL window", nullptr, nullptr );
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


    std::string image_path = dir + "./texture/parrot_image.jpg";
    image_path = "C:/source/rabbid76workbench/test/texture/parrot_image.jpg";
    int cx_i, cy_i, ch_i;
    stbi_uc *image = stbi_load( image_path.c_str(), &cx_i, &cy_i, &ch_i, 3 );
    if ( image == nullptr )
        throw std::runtime_error( "error loading texture" );

    int truth_texture_unit_image = 1;
    unsigned int image_tex_obj;
    glGenTextures(1, &image_tex_obj);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_2D, image_tex_obj);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, cx_i, cy_i, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    stbi_image_free( image );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glGenerateMipmap( GL_TEXTURE_2D );

    std::string lookup_path = dir + "./texture/lookup_table.png";
    lookup_path = "C:/source/rabbid76workbench/test/texture/lookup_table.png";
    int cx_t, cy_t, ch_t;
    stbi_uc *lookup_table = stbi_load( lookup_path.c_str(), &cx_t, &cy_t, &ch_t, 4 );
    if ( lookup_table == nullptr )
        throw std::runtime_error( "error loading texture" );

    int truth_texture_unit_table = 2;
    unsigned int lookup_tex_obj;
    glGenTextures(1, &lookup_tex_obj);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_2D, lookup_tex_obj);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, cx_t, cy_t, 0, GL_RGBA, GL_UNSIGNED_BYTE, lookup_table);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    stbi_image_free( lookup_table );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
          
    glBindTexture( GL_TEXTURE0, 0 );
    glBindTextureUnit( truth_texture_unit_image, image_tex_obj );
    glBindTextureUnit( truth_texture_unit_table, lookup_tex_obj );
    
    OpenGL::ShaderProgramSimple prog(
    {
      { sh_vert_linear, GL_VERTEX_SHADER },
      { sh_frag_linear, GL_FRAGMENT_SHADER }
    } );
    

    static const std::vector<float> varray
    { 
      -1.0f, -1.0f, 0.0f,    0.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f, 
       1.0f, -1.0f, 0.0f,    1.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,
       1.0f,  1.0f, 0.0f,    1.0f, 0.0f,    0.0f, 0.0f, 1.0f, 1.0f,
      -1.0f,  1.0f, 0.0f,    0.0f, 0.0f,    0.0f, 1.0f, 0.0f, 1.0f
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
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 9*sizeof(*varray.data()), 0 );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 9*sizeof(*varray.data()), (void*)(3*sizeof(*varray.data())) );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 2, 4, GL_FLOAT, GL_FALSE, 9*sizeof(*varray.data()), (void*)(5*sizeof(*varray.data())) );
    glEnableVertexAttribArray( 2 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    prog.Use();       

    while (!glfwWindowShouldClose(wnd))
    {
        static float angle = 1.0f;

        int vpSize[2];
        glfwGetFramebufferSize( wnd, &vpSize[0], &vpSize[1] );
        
        float ascpect = (float)vpSize[0] / (float)vpSize[1];
        float orthoX = ascpect > 1.0f ? ascpect : 1.0f;
        float orthoY = ascpect > 1.0f ? 1.0f : ascpect;

        glm::mat4 project( 1.0f );
        //project = glm::scale( glm::mat4(1.0f), glm::vec3( 0.5f, 0.5f, 1.0f ) );
        //glm::mat4 project = glm::ortho( -orthoX, orthoX, -orthoY, orthoY, -1.0f, 1.0f );
        
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 0.5f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 model( 1.0f );
        
        prog.SetUniformM44( "project", glm::value_ptr(project) );
        prog.SetUniformM44( "view", glm::value_ptr(view) );
        prog.SetUniformM44( "model", glm::value_ptr(model) );

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