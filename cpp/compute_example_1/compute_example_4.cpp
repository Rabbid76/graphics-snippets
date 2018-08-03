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
const char compute_sh[] = R"(
    #version 310 es

    layout(local_size_x = 128) in;
    layout(std430) buffer;

    layout(binding = 0) buffer Output {
        uint elements[];
    } output_data;

    layout(binding = 1) buffer Input0 {
        uint elements[];
    } input_data0;

    layout(binding = 2) readonly buffer Input1 {
        uint elements[];
    } input_data1;

    void main()
    {
        uint ident = gl_GlobalInvocationID.x;
        output_data.elements[ident] = input_data0.elements[ident] * input_data1.elements[ident];
    }
    )";




std::unique_ptr<OpenGL::ShaderProgramSimple> g_compute_prog;

std::chrono::high_resolution_clock::time_point g_start_time;

bool valid_viewport = false;

void OnIdle( void );
void Resize( int, int );
void Display( void );

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

    GLint max_ssbo_size = 0;
    glGetIntegerv( GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &max_ssbo_size );
    std::cout << "MAX_SHADER_STORAGE_BLOCK_SIZE: " << max_ssbo_size  << std::endl;
   


  GLuint input_buffer1 = 0;
  GLuint input_buffer2 = 0;
  GLuint data_buffer = 0;

  std::vector<uint32_t> data1;
  std::vector<uint32_t> data2;
  std::vector<uint32_t> data3;
  size_t no = 100;
  for ( size_t i = 0; i < no; ++ i)
  {
    data1.push_back( i );
    data2.push_back( i );
    data3.push_back( i );
  }

  glGenBuffers(1, &input_buffer1);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, input_buffer1);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, input_buffer1);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * data2.size(), data2.data(),GL_STREAM_COPY);

  glGenBuffers(1, &input_buffer2);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, input_buffer2);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, input_buffer2);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * data3.size(), data3.data(),GL_STREAM_COPY);

  glGenBuffers(1, &data_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, data_buffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * data1.size(), (void*)data1.data(),GL_DYNAMIC_READ);
  //glBufferStorage( GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * data1.size(), nullptr, GL_MAP_READ_BIT );

  //GLuint program = glCreateProgram();
  //GLuint shader = LoadShader(COMPUTE_SHADER);
  //glAttachShader(program, shader);

  //glLinkProgram(program);
  g_compute_prog.reset( new OpenGL::ShaderProgramSimple(
  {
    { compute_sh, GL_COMPUTE_SHADER }
  } ) );
  GLuint program = g_compute_prog->Prog();
  glUseProgram(program);

  glDispatchCompute(no, 1, 1);

  // Write from compute shader to persistently mapped SSBO fails
  // [https://stackoverflow.com/questions/51182056/write-from-compute-shader-to-persistently-mapped-ssbo-fails]
  //GLsync sync = glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 );
  //GLenum sync_result = glClientWaitSync( sync, 0, 1000000000 );

  std::vector<GLuint> ready_data( data1.size() );
  
  //GLuint *ptr = (GLuint*)glMapBuffer( GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY );
  GLuint* ptr = (GLuint*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint) * data1.size(), GL_MAP_READ_BIT );
  
  //glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
  //glMemoryBarrier( GL_ALL_BARRIER_BITS );
  
  std::copy(ptr, ptr + data1.size(), ready_data.begin());
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    
  GLenum error = glGetError();
  if ( error != GL_NO_ERROR )
    std::cout << "error: " << std::hex << error << "h" << std::endl;

  for ( auto d : ready_data )
    std::cout << d << " ";
  std::cout << std::endl;
    

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
