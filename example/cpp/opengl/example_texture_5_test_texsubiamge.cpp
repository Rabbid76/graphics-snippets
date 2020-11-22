// Is it possible to write to OpenGL texture 4 different planes
// https://stackoverflow.com/questions/61329383/is-it-possible-to-write-to-opengl-texture-4-different-planes

// how to update a certain channel of a texture
// https://stackoverflow.com/questions/62339996/how-to-update-a-certain-channel-of-a-texture

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


// FreeGLUT [http://freeglut.sourceforge.net/]
#include <gl/freeglut.h>


// STB 
#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include <stb_image.h>
//#include <stb_image_write.h>

// project includes
#include <gl/gl_debug.h>
#include <gl/gl_shader.h>

// preprocessor definitions

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

// forward declarations

void display( void );


// shader

std::string sh_vert = R"(
#version 460

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_uv;
layout (location = 2) in vec4 a_color;

out vec3 v_pos;
out vec3 v_uv;
out vec4 v_color;

uniform mat4 project;
uniform mat4 view;
uniform mat4 model;

void main()
{  
    vec4 view_pos = view * model * vec4(a_pos, 1.0);

    v_color     = a_color;
    v_uv        = a_uv;
	v_pos       = view_pos.xyz;
	gl_Position = project * vec4(view_pos.xyz, 1.0);
}
)";

std::string sh_frag = R"(
#version 460

in vec3 v_pos;
in vec3 v_uv;
in vec4 v_color;

out vec4 frag_color;

layout(binding = 0) uniform sampler2D u_texture;

void main()
{
    frag_color = texture(u_texture, v_uv.st);
}
)";


// globale variables

std::chrono::high_resolution_clock::time_point start_time;

OpenGL::ShaderProgramSimple *g_prog;

GLuint tex_obj;


// main

int main(int argc, char** argv)
{
    std::cout << "start" << std::endl;

    glutInit(&argc, argv);

    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

    glutInitWindowSize(800, 600);
    int wnd = glutCreateWindow("FreeGLUT OpenGL window");

    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
    OpenGL::CContext context;
    context.Init( debug_level );

    glutDisplayFunc(display);  


    glGenTextures(1, &tex_obj);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_2D, tex_obj);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  //for BMP
    
    unsigned char values[] {255, 0, 0, 255,  0, 255, 0, 255,  0, 0, 255, 255,  255, 255, 0, 255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, values);

    unsigned char values_1[] {255, 255, 255, 255 };
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RED, GL_UNSIGNED_BYTE, values_1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_GREEN, GL_UNSIGNED_BYTE, values_1);
    

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    //glGenerateMipmap( GL_TEXTURE_2D );
          
    //glBindTexture( GL_TEXTURE_2D, 0 );

    g_prog = new OpenGL::ShaderProgramSimple(
    {
      { sh_vert, GL_VERTEX_SHADER },
      { sh_frag, GL_FRAGMENT_SHADER }
    } );                                            

    static const std::vector<float> varray
    { 
    // x      y        u     v        r     g     b     a
      -1.0f, -1.0f,    0.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f, 
       1.0f, -1.0f,    1.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,
       1.0f,  1.0f,    1.0f, 0.0f,    0.0f, 1.0f, 0.0f, 1.0f,
      -1.0f,  1.0f,    0.0f, 0.0f,    0.0f, 0.0f, 1.0f, 1.0f
    };

    // create vertex buffer object

    GLuint vbo;
    glGenBuffers( 1, &vbo );
    
    // create vertex array object

    GLuint vao;
    glGenVertexArrays( 1, &vao );

    // specify vertex arrays
    
    glBindVertexArray( vao );
    
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glEnableVertexAttribArray( 2 );

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 8*sizeof(*varray.data()), 0 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 8*sizeof(*varray.data()), (void*)(2*sizeof(*varray.data())) );    
    glVertexAttribPointer( 2, 4, GL_FLOAT, GL_FALSE, 8*sizeof(*varray.data()), (void*)(4*sizeof(*varray.data())) );    
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    glBindVertexArray( 0 );

    //  create and initialize a buffer object's data store

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    //glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );
    glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), nullptr, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, GL_ZERO, varray.size()*sizeof(*varray.data()), varray.data() );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    g_prog->Use();

    // use vertex array object for drawing

    glBindVertexArray( vao );

    start_time = std::chrono::high_resolution_clock::now();
 
    std::cout << "main loop" << std::endl;
    glutMainLoop(); 

    std::cout << "terminate" << std::endl;
    glutDestroyWindow(wnd);
    return 0;    
} 

void display( void )
{
    static float angle = 0.0f;

    int vpSize[2]
    {
        glutGet(GLUT_WINDOW_WIDTH),
        glutGet(GLUT_WINDOW_HEIGHT)
    };
    float aspect = (float)vpSize[0] / (float)vpSize[1];
    float orthoX = aspect > 1.0f ? aspect : 1.0f;
    float orthoY = aspect > 1.0f ? 1.0f : aspect;

    glm::mat4 project = glm::ortho( -orthoX, orthoX, -orthoY, orthoY, -1.0f, 1.0f );
    glm::mat4 view = glm::lookAt( glm::vec3(0.0f, 0.0f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f) );

    glm::mat4 model( 1.0f );
    //model = glm::translate( model, glm::vec3(0.1f, 0.0f, 1.0f) );
    //model = glm::rotate( model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f) );
    model = glm::scale( model, glm::vec3(0.5f, 0.5f, 1.0f) );
    angle += 1.0f;
        
    g_prog->SetUniformM44( "project", glm::value_ptr(project) );
    g_prog->SetUniformM44( "view", glm::value_ptr(view) );
    g_prog->SetUniformM44( "model", glm::value_ptr(model) );

    glViewport( 0, 0, vpSize[0], vpSize[1] );
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      
    glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );

    glutSwapBuffers();
    glutPostRedisplay();
} 
