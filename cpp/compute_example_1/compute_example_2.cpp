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
const char *csSrc_1[] = {
    
  "#version 440",
  
  R"(
  layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

  layout (binding = 0, rgba32f) uniform image2D destTex;
  layout (binding = 0) uniform sampler2D sourceTex;
 
  void main() {
           
      ivec2 pos = ivec2(gl_GlobalInvocationID.xy); 

      vec4 texel =  texelFetch(sourceTex, pos, 0);

      imageStore(destTex, pos, texel);
  })"
};

const char *csSrc_2[] = {
    
  "#version 440",
  
  R"(
  layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

  layout (binding = 0, rgba32f) uniform image2D destTex;
  layout (binding = 1, rgba32f) uniform image2D sourceTex;
  
  void main() {
           
      ivec2 pos = ivec2(gl_GlobalInvocationID.xy);  

      vec4 texel = imageLoad(sourceTex, pos);

      texel = vec4(1.0) - texel;
      imageStore(destTex, pos, texel);
  })"
};


std::unique_ptr<OpenGL::ShaderProgramSimple> g_compute_prog;
std::unique_ptr<OpenGL::ShaderProgramSimple> g_draw_prog;

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

    std::string path = dir + "../resource/texture/example_1_heightmap.bmp";
    //path = "C:/source/graphics-snippets/resource/texture/example_1_heightmap.bmp";
    int icx, icy, ich;
    stbi_uc *img = stbi_load( path.c_str(), &icx, &icy, &ich, 3 );

    unsigned int src_tobj;
    glGenTextures(1, &src_tobj);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_2D, src_tobj);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, icx, icy );
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, icx, icy, GL_RGB, GL_UNSIGNED_BYTE, img);
    glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );

    glBindTexture( GL_TEXTURE0, 0 );
   
    
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

    g_draw_prog.reset( new OpenGL::ShaderProgramSimple(
    {
      { sh_vert, GL_VERTEX_SHADER },
      { sh_frag, GL_FRAGMENT_SHADER }
    } ) );


    GLuint cshader = glCreateShader( GL_COMPUTE_SHADER );
    //glShaderSource(cshader, 2, csSrc_1, NULL);
    glShaderSource(cshader, 2, csSrc_2, NULL);
    glCompileShader(cshader);
    GLint compile_status;
    glGetShaderiv( cshader, GL_COMPILE_STATUS, &compile_status );
    if ( compile_status == GL_FALSE )
    {
        GLint maxLen;
	      glGetShaderiv( cshader, GL_INFO_LOG_LENGTH, &maxLen );
        std::vector< char >log( maxLen );
		    GLsizei len;
		    glGetShaderInfoLog( cshader, maxLen, &len, log.data() );
		    std::cout << "compile error:" << std::endl << log.data() << std::endl;
        throw std::runtime_error( "compile error" );
    }  

    GLuint cshaderprogram = glCreateProgram();
    glAttachShader(cshaderprogram, cshader);
    glLinkProgram(cshaderprogram);
    GLint link_status;
    glGetProgramiv( cshaderprogram, GL_LINK_STATUS, &link_status );
    if ( link_status == GL_FALSE )
    {
        GLint maxLen;
	      glGetProgramiv( cshaderprogram, GL_INFO_LOG_LENGTH, &maxLen );
        std::vector< char >log( maxLen );
		    GLsizei len;
		    glGetProgramInfoLog( cshaderprogram, maxLen, &len, log.data() );
		    std::cout  << "link error:" << std::endl << log.data() << std::endl;
        throw std::runtime_error( "link error" );
    }

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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, cx, cy);
    //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, cx, cy, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); // crash
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, cx, cy, 0, GL_RGBA, GL_FLOAT, NULL);
    OPENGL_CHECK_GL_ERROR

    //glBindImageTexture(0, tobj, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(0, tobj, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F); 
    OPENGL_CHECK_GL_ERROR

    //glBindTextureUnit( 0, src_tobj );
    //glBindImageTexture(1, src_tobj, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(1, src_tobj, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

      // launch compute shaders!
      glUseProgram( cshaderprogram );
    //g_compute_prog->Use();
    glDispatchCompute((GLuint)cx, (GLuint)cy, 1); 
    OPENGL_CHECK_GL_ERROR
    
    // make sure writing to image has finished before read
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); // alternative: GL_ALL_BARRIER_BITS
    //glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    //glMemoryBarrier(GL_ALL_BARRIER_BITS);
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
