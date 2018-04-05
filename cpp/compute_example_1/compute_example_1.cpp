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


// vertex shader
std::string sh_vert = R"(
#version 460

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTex;
layout (location = 1) in vec4 inColor;

out TVertexData
{
    vec3 pos;
    vec4 col;
    vec2 uv;
} out_data;

void main()
{
    out_data.pos = inPos;
    out_data.col = inColor;
    out_data.uv  = inTex;
		gl_Position  = vec4(inPos, 1.0);
}
)";


// fragment shader
std::string sh_frag = R"(
#version 460

in TVertexData
{
    vec3 pos;
    vec4 col;
    vec2 uv;
} in_data;

out vec4 fragColor;

layout (binding = 1) uniform sampler2D u_texture; 

void main()
{
    vec4 tex_color = texture(u_texture, in_data.uv.st);
    fragColor = vec4(tex_color.rgb, 1.0);
}
)";


// test compute shader
std::string sh_test_compute = R"(
#version 460

layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D img_output;

void main() {
  
   // get index in global work group i.e x,y position
  ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
  
  float u = float(pixel_coords.x) / 512.0;
  float v = float(pixel_coords.y) / 512.0; 
  vec4 pixel = vec4(u, v, (1.0-u)*(1.0-v), 1.0);
  
  // output to a specific pixel in the image
  imageStore(img_output, pixel_coords, pixel);
}
)";


// compute shader
std::string sh_compute_1 = R"(
#version 460

layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D img_output;

void main() {
  
  // get index in global work group i.e x,y position
  ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
  
  // http://antongerdelan.net/opengl/compute.html

  vec4 pixel = vec4(0.5, 0.3, 0.0, 1.0); 

  float max_x  = 5.0;
  float max_y  = 5.0;
  ivec2 dims   = imageSize(img_output); // fetch image dimensions
  float x      = (float(pixel_coords.x * 2 - dims.x) / dims.x);
  float y      = (float(pixel_coords.y * 2 - dims.y) / dims.y);
  vec3  ray_o  = vec3(x * max_x, y * max_y, 0.0);
  vec3  ray_d  = vec3(0.0, 0.0, -1.0); // ortho

  vec3  sphere_c = vec3(0.0, 0.0, -10.0);
  float sphere_r = 1.0;

  vec3  omc   = ray_o - sphere_c;
  float b     = dot(ray_d, omc);
  float c     = dot(omc, omc) - sphere_r * sphere_r;
  float bsqmc = b * b - c;
  // hit one or both sides
  if (bsqmc >= 0.0) {
    pixel = vec4(0.4, 0.4, 1.0, 1.0);
  }

  // output to a specific pixel in the image
  imageStore(img_output, pixel_coords, pixel);
}
)";

// compute shader
std::string sh_compute_2 = R"(
#version 460

layout (local_size_x = 16, local_size_y = 16) in;
layout(rgba32f, binding = 0) uniform image2D destTex;

//uniform float roll;

void main() {
  const float roll = 0.0;

  // http://wili.cc/blog/opengl-cs.html
  ivec2 storePos   = ivec2(gl_GlobalInvocationID.xy);
  float localCoef  = length(vec2(ivec2(gl_LocalInvocationID.xy)-8)/8.0);
  float globalCoef = sin(float(gl_WorkGroupID.x+gl_WorkGroupID.y)*0.1 + roll)*0.5;
  imageStore(destTex, storePos, vec4(1.0-globalCoef*localCoef, 0.0, 0.0, 0.0));
}
)";


std::unique_ptr<OpenGL::ShaderProgram> g_compute_prog;
std::unique_ptr<OpenGL::ShaderProgram> g_draw_prog;

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

    g_draw_prog.reset( new OpenGL::ShaderProgram(
    {
      { sh_vert, GL_VERTEX_SHADER },
      { sh_frag, GL_FRAGMENT_SHADER }
    } ) );

    static int compute_id = 2;
    std::string sh_compute;
    switch (compute_id)
    {
      default:
      case 0: sh_compute = sh_test_compute; break;
      case 1: sh_compute = sh_compute_1; break;
      case 2: sh_compute = sh_compute_2; break;
    }
    g_compute_prog.reset( new OpenGL::ShaderProgram(
    {
      { sh_compute.c_str(), GL_COMPUTE_SHADER }
    } ) );

    static const std::vector<float> varray
    { 
      -1.0f, -1.0f,    0.0f, 0.0f,    0.0f, 0.0f, 1.0f, 1.0f, 
       1.0f, -1.0f,    1.0f, 0.0f,    1.0f, 0.0f, 0.0f, 1.0f,
      -1.0f,  1.0f,    0.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f,
       1.0f,  1.0f,    1.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,
    };

    GLuint vbo;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );

    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 8*sizeof(*varray.data()), 0 );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 8*sizeof(*varray.data()), (void*)(2*sizeof(*varray.data())) );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 2, 4, GL_FLOAT, GL_FALSE, 8*sizeof(*varray.data()), (void*)(4*sizeof(*varray.data())) );
    glEnableVertexAttribArray( 2 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    // intialze the image with a red diagonal coss, for debug resons
    int cx = wnd_w;
    int cy = wnd_h;
    std::vector<unsigned char> test_img(cx * cy * 4);
    for ( int y = 0; y < cy; ++ y )
    {
      for ( int x = 0; x < cx; ++ x )
      {
        float u = (float)x / (float)(cx - 1);
        float v = (float)y / (float)(cy - 1);

        int i = (y*cx + x) * 4;
        test_img[i+0] = fabs(u-v) < 0.01f || fabs(u-(1.0f-v)) < 0.01f ? 255 : 0;
        test_img[i+1] = 0;
        test_img[i+2] = 0;
        test_img[i+3] = 255;
      }
    }

    int texture_unit = 1;
    unsigned int tobj;
    glGenTextures(1, &tobj);
    glActiveTexture( GL_TEXTURE0 + texture_unit );
    glBindTexture(GL_TEXTURE_2D, tobj);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    //glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ); RGBA is always aliged 4
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, cx, cy, 0, GL_RGBA, GL_UNSIGNED_BYTE, test_img.data());
    //glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, cx, cy, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindImageTexture(0, tobj, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F); 
    OPENGL_CHECK_GL_ERROR

    // launch compute shaders!
    g_compute_prog->Use();
    glDispatchCompute((GLuint)cx, (GLuint)cy, 1); 
    OPENGL_CHECK_GL_ERROR
    
    // make sure writing to image has finished before read
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); // alternative: GL_ALL_BARRIER_BITS
    OPENGL_CHECK_GL_ERROR
    
    g_draw_prog->Use();

    g_start_time = std::chrono::high_resolution_clock::now();
    glutMainLoop();

    g_draw_prog.reset( nullptr );

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

      g_draw_prog->SetUniformF2( "u_vp_size", { (float)vpSize[0], (float)vpSize[1] } );
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
      
    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

    static bool read_pixel = false;
    if ( read_pixel )
    {
      std::vector<unsigned char> buffer(4 * vpSize[0] * vpSize[1]);
      glReadPixels( 0, 0, vpSize[0], vpSize[1], GL_RGBA, GL_UNSIGNED_BYTE, buffer.data() );
      const char *filename = "d:\\temp\temp01.bmp";
      if ( filename != nullptr )
      {
        // TODO $$$
      }
    }

    glutSwapBuffers();
}
