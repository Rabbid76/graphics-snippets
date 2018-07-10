#include <stdafx.h>

// OpenGL
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// freeglut
#include <GL/freeglut.h>

// stl
#include <array>
#include <vector>
#include <map>
#include <tuple>
#include <algorithm>
#include <stdexcept>
#include <chrono>
#include <memory>
#include <cmath>
#include <ctime>

#define _USE_MATH_DEFINES
#include <math.h>

// stb 
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Own
#include <OpenGL_Matrix_Camera.h>
#include <OpenGL_SimpleShaderProgram.h>
#include <OpenGLError.h>



// test compute shader
const char *compute_sh = R"(
#version 310 es

layout(local_size_x = 128) in;
layout(std430) buffer;
layout(binding = 0) writeonly buffer Output
{
    uint elements[];
} output_data;

layout(binding = 1) readonly buffer Input0
{
    uint elements[];
} input_data0;

void main()
{
    uint ident = gl_GlobalInvocationID.x;
    output_data.elements[ident] = input_data0.elements[ident] * input_data0.elements[ident];
}
)";


std::unique_ptr<OpenGL::ShaderProgram> g_compute_prog;

std::chrono::high_resolution_clock::time_point g_start_time;

bool valid_viewport = false;

void OnIdle( void );
void Resize( int, int );
void Display( void );

void CreateConeMap( std::vector<unsigned char> &data_out, long cx, long cy, long ch, long bpl, const unsigned char *data_in, int log_level );
void CreateConeMap_from_ConeStepMapping_pdf( std::vector<unsigned char> &data_out, long cx, long cy, long ch, long bpl, const unsigned char *data_in, int log_level );

int main(int argc, char** argv)
{
    std::string dir = argc > 1 ? argv[1] : "";

    static int wnd_w = 512;
    static int wnd_h = 512;

    // Initialise FreeGLUT
    glutInit(&argc, argv);

    // Setup OpenGL window properties
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    
    unsigned int display_mode = GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE;
    //display_mode = display_mode | GLUT_BORDERLESS | GLUT_CAPTIONLESS; 
    glutInitDisplayMode(display_mode);
   
    // Create window
    glutInitWindowSize(wnd_w, wnd_h);
    int wnd = glutCreateWindow("test");
    if ( wnd == 0 )
        throw std::runtime_error( "error initializing window" ); 

    // Register display callback function
    glutDisplayFunc( Display ); 
    glutReshapeFunc( Resize );
    glutIdleFunc( OnIdle );

    glewExperimental = true;
    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    std::cout << glGetString( GL_VENDOR ) << std::endl;
    std::cout << glGetString( GL_RENDERER ) << std::endl;
    std::cout << glGetString( GL_VERSION ) << std::endl;
    std::cout << glGetString( GL_SHADING_LANGUAGE_VERSION ) << std::endl;

    GLint major = 0, minor = 0, contex_mask = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &contex_mask);
    std::cout << "context: " << major << "." << minor << " ";
    if ( contex_mask & GL_CONTEXT_CORE_PROFILE_BIT  )
      std::cout << "core";
    else if ( contex_mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT )
      std::cout << "compatibility";
    if ( contex_mask & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT  )
      std::cout << ", forward compatibility";
    if ( contex_mask & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT  )
      std::cout << ", robust access";
    if ( contex_mask & GL_CONTEXT_FLAG_DEBUG_BIT  )
      std::cout << ", debug";
    std::cout << std::endl;


    g_compute_prog.reset( new OpenGL::ShaderProgram(
    {
      { compute_sh, GL_COMPUTE_SHADER }
    } ) );


    char hello[100] = "hello";

    GLuint data_buffer;
    GLuint output_buffer;

    uint32_t data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    glGenBuffers(1, &data_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, data_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * 10, (void*)data, GL_STREAM_DRAW);
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, data_buffer);

    glGenBuffers(0, &output_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, output_buffer);
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, output_buffer);

    static bool map_buffer = false;
    
    if ( map_buffer )
      glBufferStorage( GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * 10, nullptr, GL_MAP_READ_BIT );
    else
      glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * 10, nullptr, GL_DYNAMIC_READ);
    
    //GLuint program = glCreateProgram();
    //GLuint shader = LoadShader(COMPUTE_SHADER);

    //glAttachShader(program, shader);
    //glLinkProgram(program);

    GLuint program = g_compute_prog->Prog();

    glUseProgram(program);
    glDispatchCompute(10,1,1);

    //glMemoryBarrier(GL_ALL_BARRIER_BITS);


    GLuint info = 0;
    if ( map_buffer )
    {
      GLuint *ptr = (GLuint *) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint) * 10, GL_MAP_READ_BIT );
      glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
      info = ptr[ 1 ];
      glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );
    }
    else
    {
      //std::vector<GLuint> buffer( 10 );
      //glGetBufferSubData( GL_SHADER_STORAGE_BUFFER, sizeof( GLuint ) * 10, 0, buffer.data() );
      //info = buffer[ 1 ];

      GLuint *ptr = (GLuint *) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint) * 10, GL_MAP_READ_BIT );
      info = ptr[ 1 ];
      glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );
    }
    sprintf(hello, "%d ", info);

    
    std::cout << hello << std::endl;
    

    g_start_time = std::chrono::high_resolution_clock::now();
    //glutMainLoop();


    glutDestroyWindow(wnd);
    return 0;
}

void OnIdle( void )
{
  glutPostRedisplay();
  // ....
}

void Resize( int cx, int cy )
{
  valid_viewport = false;
  // ....
}


void Display( void )
{
    std::chrono::high_resolution_clock::time_point current_time = std::chrono::high_resolution_clock::now();
    auto   delta_time = current_time - g_start_time;
    double time_ms    = (double)std::chrono::duration_cast<std::chrono::milliseconds>(delta_time).count();

    static int vpSize[2]{ 0 };
    if ( valid_viewport == false )
    {
      vpSize[0] = glutGet( GLUT_WINDOW_WIDTH );
      vpSize[1] = glutGet( GLUT_WINDOW_HEIGHT );

      glViewport( 0, 0, vpSize[0], vpSize[1] );
      valid_viewport = true;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
      
    // TODO $$$

    glutSwapBuffers();
}
