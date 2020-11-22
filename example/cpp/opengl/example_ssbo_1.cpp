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
#version 460 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;

out vec3 vertPos;
out vec4 vertCol;

layout(std430, binding = 3) buffer TMatrix
{
  mat4 project;
  mat4 view;
  mat4 model;
} matrix;

void main()
{
    vec4 view_pos = matrix.view * matrix.model * vec4(inPos, 1.0);
 
    vertCol     = inColor;
		vertPos     = view_pos.xyz;
		gl_Position = matrix.project * vec4(view_pos.xyz, 1.0);
}
)";

std::string sh_frag = R"(
#version 460 core

in vec3 vertPos;
in vec4 vertCol;

out vec4 fragColor;

void main()
{
    fragColor = vertCol;
}
)";


struct TSSBOData
{
  glm::mat4 _project;
  glm::mat4 _view;
  glm::mat4 _model;
};

struct TSSBOResource
{
  GLint resInx;
  GLint binding;
  std::vector<GLint> offsets;
  std::vector<std::string> names;
};

int main(void)
{
    if ( glfwInit() == 0 )
        throw std::runtime_error( "error initializing glfw" );

    //glfwWindowHint(GLFW_REFRESH_RATE, 10);

    GLFWwindow *wnd = glfwCreateWindow( 800, 600, "OGL window", nullptr, nullptr );
    if ( wnd == nullptr )
    {
        glfwTerminate();
        throw std::runtime_error( "error initializing window" ); 
    }

    glfwMakeContextCurrent(wnd);

    //glfwSwapInterval( 2 );

    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
    OpenGL::CContext context;
    context.Init( debug_level );


    OpenGL::ShaderProgramSimple prog(
    {
      { sh_vert, GL_VERTEX_SHADER },
      { sh_frag, GL_FRAGMENT_SHADER }
    } );

    std::map<std::string, TSSBOResource> ssbo_map;

    GLuint prog_obj = prog.Prog();

    GLint no_of, ssbo_max_len, var_max_len;
    glGetProgramInterfaceiv(prog_obj, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &no_of);
    glGetProgramInterfaceiv(prog_obj, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &ssbo_max_len);
    glGetProgramInterfaceiv(prog_obj, GL_BUFFER_VARIABLE, GL_MAX_NAME_LENGTH, &var_max_len);

    //GLint max_num;
    //glGetProgramInterfaceiv(prog_obj, GL_SHADER_STORAGE_BLOCK, GL_MAX_NUM_ACTIVE_VARIABLES, &max_num );
      
	  std::vector< GLchar >name( ssbo_max_len );
	  for( int i_resource = 0; i_resource < no_of; i_resource++ )
	  {
      GLsizei strLength;
      glGetProgramResourceName( prog_obj, GL_SHADER_STORAGE_BLOCK, i_resource, ssbo_max_len, &strLength, name.data() );

      GLint resInx = glGetProgramResourceIndex(prog_obj, GL_SHADER_STORAGE_BLOCK, name.data()); 

      GLenum props[]{ GL_NUM_ACTIVE_VARIABLES, GL_BUFFER_BINDING };
      GLint values[]{ 0, 0 };
      glGetProgramResourceiv(prog_obj, GL_SHADER_STORAGE_BLOCK, resInx, 2, props, 2, nullptr, values);
      GLint num_var = values[0];
      GLint binding = values[1];

      std::vector<GLint> vars(num_var);
      GLenum prop = GL_ACTIVE_VARIABLES;
      glGetProgramResourceiv(prog_obj, GL_SHADER_STORAGE_BLOCK, resInx, 1, &prop, (GLsizei)vars.size(), nullptr, vars.data());

      std::vector<GLint> offsets(num_var);
      std::vector<std::string> var_names(num_var);
      for (GLint i = 0; i < num_var; i++) {
          GLenum prop = GL_OFFSET;
          glGetProgramResourceiv(prog_obj, GL_BUFFER_VARIABLE, vars[i], 1, &prop, (GLsizei)offsets.size(), nullptr, &offsets[i]);

          std::vector<GLchar>var_name(var_max_len);
          GLsizei strLength;
          glGetProgramResourceName(prog_obj, GL_BUFFER_VARIABLE, vars[i], var_max_len, &strLength, var_name.data());
          var_names[i] = var_name.data();
      }

      TSSBOResource res = { resInx, binding, offsets, var_names };
      ssbo_map[name.data()] = res;
	  }
    

    static const std::vector<float> varray
    { 
      -0.866f, -0.5f,    1.0f, 0.0f, 0.0f, 1.0f, 
       0.866f, -0.5f,    1.0f, 1.0f, 0.0f, 1.0f,
       0.0f,    1.0f,    0.0f, 0.0f, 1.0f, 1.0f
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

    GLuint ssbo;
    glGenBuffers( 1, &ssbo );
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, ssbo );
    GLsizei buffer_size = sizeof( TSSBOData );
    glBufferData( GL_SHADER_STORAGE_BUFFER, buffer_size, nullptr, GL_STATIC_DRAW );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 3, ssbo );
          
    prog.Use();       

    while (!glfwWindowShouldClose(wnd))
    {
        static float angle = 1.0f;

        int vpSize[2];
        glfwGetFramebufferSize( wnd, &vpSize[0], &vpSize[1] );
        
        float ascpect = (float)vpSize[0] / (float)vpSize[1];
        float orthoX = ascpect > 1.0f ? ascpect : 1.0f;
        float orthoY = ascpect > 1.0f ? 1.0f : ascpect;

        glm::mat4 project = glm::ortho( -orthoX, orthoX, -orthoY, orthoY, -1.0f, 1.0f );
        
        float orthScale = 1.0f;
        project = glm::scale(project, glm::vec3(orthScale, orthScale, 1.0f));
        
        static bool invert = false;
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 0.5f * (invert ? -1.0f : 1.0f)),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f) );

        glm::mat4 model( 1.0f );
        model = glm::translate( model, glm::vec3(0.0f, 0.0f, 0.0f) );
        model = glm::rotate( model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f) );
        angle += 1.0f;
        
        TSSBOData mvp;
        mvp._project = project;
        mvp._view    = view;
        mvp._model   = model;

        glBindBuffer( GL_SHADER_STORAGE_BUFFER, ssbo );
        glBufferSubData( GL_SHADER_STORAGE_BUFFER, 0, sizeof( TSSBOData ), &mvp );

        glViewport( 0, 0, vpSize[0], vpSize[1] );
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      
        glDrawArrays( GL_TRIANGLES, 0, 3 );

        glfwSwapBuffers(wnd);
        glfwPollEvents();
    }

    glfwDestroyWindow( wnd );
    wnd = nullptr;
    glfwTerminate();

    return 0;
}