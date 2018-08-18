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
#version 440 core

layout(std430, binding = 0) buffer Result{
  float out_picture[];
};
layout(std430, binding = 1) buffer In_p1{
  float in_p1[];
};
layout(local_size_x = 1000) in;

void main() {
  in_p1[gl_GlobalInvocationID.x] = 1.0f;
  out_picture[gl_GlobalInvocationID.x] = out_picture[gl_GlobalInvocationID.x] + in_p1[gl_GlobalInvocationID.x];

}
)";


std::unique_ptr<OpenGL::ShaderProgramSimple> g_compute_prog;

std::chrono::high_resolution_clock::time_point g_start_time;


struct SSBO
{
  unsigned int handle;
  int NUM_PIX;
  int WORK_GROUP_SIZE;
};

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
   
    // extensions
    //std::cout << glGetStringi( GL_EXTENSIONS, ... ) << std::endl;

    std::cout << std::endl;

    std::cout << "computer shader workgroup capabilities:" << std::endl;

    int work_grp_cnt[3];
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);
    std::cout << "    max global (total) work group size" << std::endl;
    std::cout << "        x: " << work_grp_cnt[0] << std::endl;
    std::cout << "        y: " << work_grp_cnt[1] << std::endl;
    std::cout << "        z: " << work_grp_cnt[2] << std::endl;
    
    int work_grp_size[3];
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);
    std::cout << "    max local (in one shader) work group sizes" << std::endl;
    std::cout << "        x: " << work_grp_size[0] << std::endl;
    std::cout << "        y: " << work_grp_size[1] << std::endl;
    std::cout << "        z: " << work_grp_size[2] << std::endl;

    int work_grp_inv;
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
    std::cout << "    max local work group invocations" << std::endl;
    std::cout << "        x: " << work_grp_inv << std::endl;
    
    std::cout << std::endl;

    g_compute_prog.reset( new OpenGL::ShaderProgramSimple(
  {
    { compute_sh, GL_COMPUTE_SHADER }
  } ) );
  GLuint g_avg_program = g_compute_prog->Prog();


    std::vector<SSBO> ssbo_container;
    SSBO ssbo_result;

    ssbo_result.NUM_PIX = 1920*1080*4;
    ssbo_result.WORK_GROUP_SIZE = 1000;
    std::vector<float> result_container(ssbo_result.NUM_PIX);

    glGenBuffers(1, &ssbo_result.handle);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_result.handle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, ssbo_result.NUM_PIX * sizeof(float), NULL, GL_DYNAMIC_DRAW);

    for(unsigned int i = 1; i < 2; i++){
      SSBO ssbo;
      glGenBuffers(1, &ssbo.handle);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, ssbo.handle);
      glBufferData(GL_SHADER_STORAGE_BUFFER, ssbo_result.NUM_PIX * sizeof(float), NULL, GL_DYNAMIC_DRAW);
      ssbo_container.push_back(ssbo);
    }

    // Updateing SSBO Contents
    // [https://stackoverflow.com/questions/51899747/updateing-ssbo-contents/51902049#51902049]

    bool run = true;
    while (run) {

      std::cout << "container:" << result_container[0] << std::endl;

      glUseProgram(g_avg_program);
      //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_result.handle);
      glDispatchCompute(ssbo_result.NUM_PIX / ssbo_result.WORK_GROUP_SIZE, 1, 1);
      glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
      //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

      glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_result.handle);
      GLfloat *ptr = (GLfloat *) glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
      memcpy(result_container.data(), ptr, ssbo_result.NUM_PIX * sizeof(float));
      glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

      glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_result.handle);
      ptr = (GLfloat *) glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
      memcpy(ptr, result_container.data(), ssbo_result.NUM_PIX * sizeof(float));
      glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


      
    }
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
