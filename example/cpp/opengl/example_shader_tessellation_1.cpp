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


std::string sh_vert = R"(
#version 410 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;

out vec3 vertPos;
//out vec4 vertCol;

void main()
{
    vertPos = inPos;
    //vertCol = inColor;
}
)";

std::string sh_tesc = R"(
#version 410 core

layout(vertices = 4) out;

in  vec3 vertPos[];
out vec3 tescPos[];

void main(void)
{
    gl_TessLevelOuter[0] = 2.0;
    gl_TessLevelOuter[1] = 4.0;
    gl_TessLevelOuter[2] = 6.0;
    gl_TessLevelOuter[3] = 8.0;

    gl_TessLevelInner[0] = 8.0;
    gl_TessLevelInner[1] = 8.0;

    tescPos[gl_InvocationID] = vertPos[gl_InvocationID];
}
)";

std::string sh_tese = R"(
#version 410 core

layout(quads, equal_spacing, ccw) in;

in  vec3  tescPos[];
out float patchHue;

uniform mat4 project;
uniform mat4 view;
uniform mat4 model;
 
uniform float scale;

vec3 interpolate(in vec3 v0, in vec3 v1, in vec3 v2, in vec3 v3)
{
    vec3 a = mix(v0, v1, gl_TessCoord.x);
    vec3 b = mix(v3, v2, gl_TessCoord.x);
    return mix(a, b, gl_TessCoord.y);
}

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{   
    patchHue      = rand( gl_TessCoord.xy );

    vec3 pos      = interpolate(tescPos[0], tescPos[1], tescPos[2], tescPos[3]);

    vec3 dx       = tescPos[1]-tescPos[0];
    vec3 dy       = tescPos[3]-tescPos[0];
    vec3 nv       = normalize(cross(dx,dy));
    
    vec3 disp_pos = pos + nv * patchHue * scale; 

    gl_Position = project * view * model * vec4(disp_pos, 1.0);
}
)";

std::string sh_geom = R"(
#version 410 core

layout(triangles, invocations = 1) in;
layout(triangle_strip, max_vertices = 4) out;

in float patchHue[];
flat out vec4 color;

vec3 HUEtoRGB(in float H)
{
    float R = abs(H * 6.0 - 3.0) - 1.0;
    float G = 2.0 - abs(H * 6.0 - 2.0);
    float B = 2.0 - abs(H * 6.0 - 4.0);
    return clamp( vec3(R,G,B), 0.0, 1.0 );
}

void main(void)
{
    float hue = (patchHue[0] + patchHue[1] + patchHue[2]) / 3.0;
    //color = vec4( HUEtoRGB( hue ), 1.0 );
    color = vec4( vec3( hue ), 1.0 );
    
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();   
    
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();
    
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();

    EndPrimitive();
}
)";

std::string sh_frag = R"(
#version 410 core

flat in vec4 color;

out vec4 fragColor;

void main()
{
    fragColor = color;
}
)";


int main(void)
{
    if ( glfwInit() == 0 )
        throw std::runtime_error( "error initializing glfw" );

    GLFWwindow *wnd = glfwCreateWindow( 400, 400, "OGL window", nullptr, nullptr );
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


    OpenGL::ShaderProgramSimple prog(
    {
      { sh_vert, GL_VERTEX_SHADER },
      { sh_tesc, GL_TESS_CONTROL_SHADER },
      { sh_tese, GL_TESS_EVALUATION_SHADER },
      { sh_geom, GL_GEOMETRY_SHADER },
      { sh_frag, GL_FRAGMENT_SHADER }
    } );


    static const std::vector<float> varray
    { 
      -0.75f, -0.75f,    1.0f, 0.0f, 0.0f, 1.0f, 
       0.75f, -0.75f,    1.0f, 1.0f, 0.0f, 1.0f,
       0.75f,  0.75f,    0.0f, 0.0f, 1.0f, 1.0f,
      -0.75f,  0.75f,    0.0f, 1.0f, 0.0f, 1.0f
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
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 6*sizeof(*varray.data()), 0 );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 6*sizeof(*varray.data()), (void*)(2*sizeof(*varray.data())) );
    glEnableVertexAttribArray( 1 );
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

        glm::mat4 project = glm::ortho( -orthoX, orthoX, -orthoY, orthoY, -10.0f, 10.0f );
        
        glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 1.6f, 0.5f),
        glm::vec3(0.0f, 0.6f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 model( 1.0f );
        //model = glm::translate( model, glm::vec3(0.1f, 0.0f, 1.0f) );
        //model = glm::rotate( model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f) );
        angle += 1.0f;
        
        prog.SetUniformM44( "project", glm::value_ptr(project) );
        prog.SetUniformM44( "view", glm::value_ptr(view) );
        prog.SetUniformM44( "model", glm::value_ptr(model) );
        prog.SetUniformF1( "scale", -1.0 );

        glViewport( 0, 0, vpSize[0], vpSize[1] );
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      
        //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        glPatchParameteri( GL_PATCH_VERTICES, 4 );
        glDrawArrays( GL_PATCHES, 0, 4 );

        glfwSwapBuffers(wnd);
        glfwPollEvents();
    }

    glfwDestroyWindow( wnd );
    wnd = nullptr;
    glfwTerminate();

    return 0;
}