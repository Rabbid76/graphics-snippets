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
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

// Own
#include <OpenGL_Matrix_Camera.h>
#include <OpenGL_SimpleShaderProgram.h>
#include <OpenGLError.h>


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
layout (binding = 2) uniform sampler2D u_texture_truth; 

void main()
{
    vec2 uv = vec2(in_data.uv.x, 1.0-in_data.uv.y);
    vec4 tex_color = texture(u_texture, uv);
    
    //vec4 tex_truth = texture(u_texture_truth, uv);
    //float delta = abs(tex_color.g - tex_truth.g);
    //fragColor = vec4(0.0, delta*10.0, 0.0, 1.0);
    //fragColor = vec4(tex_color.rg, 0.0, 1.0);
    
    fragColor = vec4(tex_color.rg, 0.0, 1.0);
    //fragColor = vec4( normalize( vec3(tex_color.ba, 1.0) ), 1.0);
    //fragColor = tex_color;
}
)";

// test compute shader
std::string sh_cone_compute = R"(
#version 460

layout(local_size_x = 1, local_size_y = 1) in;

//#define SOURCE_TEXTURE

#if defined( SOURCE_TEXTURE )

// writeonly cone map image
//layout(binding = 1) writeonly uniform image2D img_output;
layout(rgba8, binding = 1) writeonly uniform image2D cone_map_image;

// height map source texture
layout(binding = 2) uniform sampler2D u_height_map;

// read height from height map
float get_height(in ivec2 coord)
{
  return texelFetch(u_height_map, coord, 0).x;
}

#else

// read and write cone map image
layout(rgba8, binding = 1) uniform image2D cone_map_image;

// read height from image
float get_height(in ivec2 coord)
{
  return imageLoad(cone_map_image, coord).x;
}

#endif


const float max_cone_c = 1.0;

void main()
{  
  ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);  // get index in global work group i.e x,y position

  ivec2 map_dim  = imageSize(cone_map_image);
  int   cx       = map_dim.x;   
  int   cy       = map_dim.y;  
  int   x        = pixel_coords.x;   
  int   y        = pixel_coords.y;        
  float step_x   = 1.0 / float(cx);
  float step_y   = 1.0 / float(cy);
  float step     = max(step_x, step_y); 
  
  float h        = get_height(pixel_coords);  
  float c        = max_cone_c;
  float max_h    = 1.0 - h;
  float max_dist = min(max_cone_c * max_h, 1.0);

  for( float dist = step; dist <= max_dist && c > dist / max_h; dist += step )
  {
    int   d2       = int(round((dist*dist) / (step*step)));
    int   dy       = int(round(dist / step_y));
    float sample_h = 0;
    for( int dx = 0; sample_h < 1.0 && float(dx) / float(cx) <= dist; ++ dx )
    {
      if ( (dx*dx + dy*dy) < d2 && dy < cy-1 )
        dy ++;
      do
      {
        int sx_n = ((cx + x - dx) % cx);
        int sx_p = ((cx + x + dx) % cx);
        int sy_n = ((cy + y - dy) % cy);
        int sy_p = ((cy + y + dy) % cy);
            
        sample_h = max( sample_h, get_height(ivec2(sx_p, sy_p)) );
        sample_h = max( sample_h, get_height(ivec2(sx_p, sy_n)) );
        sample_h = max( sample_h, get_height(ivec2(sx_n, sy_p)) );
        sample_h = max( sample_h, get_height(ivec2(sx_n, sy_n)) );

        dy --;
      }
      while ( dy > 0 && (dx*dx + dy*dy) >= d2 );
    }
    if ( sample_h > h )
    {
      float d_h      = float(sample_h - h);
      float sample_c = dist / d_h; 
      c              = min(c, sample_c);
    }
  }
    
  vec4 cone_map = vec4(h, sqrt(c), 0.0, 0.0);
  
  imageStore(cone_map_image, pixel_coords, cone_map);
}
)";


std::unique_ptr<OpenGL::ShaderProgram> g_prog;
std::unique_ptr<OpenGL::ShaderProgram> g_cone_prog;

std::chrono::high_resolution_clock::time_point g_start_time;

bool valid_viewport = false;

void OnIdle( void );
void Resize( int, int );
void Display( void );

void ComputeShader( unsigned int target_tex_obj, int tex_format, unsigned int shader_obj, long cx, long cy, long ch, long bpl, const unsigned char *data_in, int log_level );
void CreateConeMap_1( std::vector<unsigned char> &data_out, long cx, long cy, long ch, long bpl, const unsigned char *data_in, int log_level );
void CreateConeMap_flat( std::vector<unsigned char> &data_out, long cx, long cy, long ch, long bpl, const unsigned char *data_in, int log_level );
void CreateConeMap_from_ConeStepMapping_pdf( std::vector<unsigned char> &data_out, long cx, long cy, long ch, long bpl, const unsigned char *data_in, int log_level );

int main(int argc, char** argv)
{
    std::string dir = argc > 1 ? argv[1] : "";

    // Initialise FreeGLUT
    glutInit(&argc, argv);

    // Setup OpenGL window properties
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_BORDERLESS | GLUT_CAPTIONLESS);

    // Create window
    glutInitWindowSize(512, 512);
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

    g_prog.reset( new OpenGL::ShaderProgram(
    {
      { sh_vert, GL_VERTEX_SHADER },
      { sh_frag, GL_FRAGMENT_SHADER }
    } ) );

    g_cone_prog.reset( new OpenGL::ShaderProgram(
    {
      { sh_cone_compute.c_str(), GL_COMPUTE_SHADER }
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

    std::string path = dir + "../resource/texture/example_1_heightmap.bmp";
    path = "C:/source/graphics-snippets/resource/texture/example_1_heightmap.bmp";
    int cx, cy, ch;
    stbi_uc *img = stbi_load( path.c_str(), &cx, &cy, &ch, 3 );
    if ( img != nullptr )
    {
        static bool ground_truth = false;
        if (ground_truth)
        {
          std::vector<unsigned char> cone_map_truth;
          CreateConeMap_from_ConeStepMapping_pdf ( cone_map_truth, cx, cy, 3, cx*3, img, 1 );

          int truth_texture_unit = 2;
          unsigned int tobj;
          glGenTextures(1, &tobj);
          glActiveTexture( GL_TEXTURE0 + truth_texture_unit );
          glBindTexture(GL_TEXTURE_2D, tobj);
          glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
          glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
          glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
          glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, cx, cy, 0, GL_RGBA, GL_UNSIGNED_BYTE, cone_map_truth.data());
        }

        int trace_level = 1;
#ifdef _DEBUG
        trace_level = 2;
#endif

        std::vector<unsigned char> cone_map;
        
        int con_algorithm = 0;
        bool use_compute_shader = con_algorithm == 0;

        for ( int i = 0; i < 100; ++ i)
        switch ( con_algorithm )
        {
          default: 
          case 0: break;
          case 1: CreateConeMap_1( cone_map, cx, cy, 3, cx * 3, img, trace_level ); break;
          case 2: CreateConeMap_flat( cone_map, cx, cy, 3, cx*3, img, trace_level ); break;
          case 3: CreateConeMap_from_ConeStepMapping_pdf ( cone_map, cx, cy, 3, cx*3, img, trace_level ); break;
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
        if ( use_compute_shader)
        {
          glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, cx, cy, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
          glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
        }
        else
        {
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, cx, cy, 0, GL_RGBA, GL_UNSIGNED_BYTE, cone_map.data());
        }

        switch ( con_algorithm )
        {
          case 0: ComputeShader( tobj, GL_RGBA8, g_cone_prog->Prog(), cx, cy, 3, cx * 3, img, trace_level ); break;
          default:
          case 1: break;
          case 2: break;
          case 3: break;
        }

        stbi_image_free( img );
    }

    g_prog->Use();

    g_start_time = std::chrono::high_resolution_clock::now();
    glutMainLoop();

    g_prog.reset( nullptr );

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

      g_prog->SetUniformF2( "u_vp_size", { (float)vpSize[0], (float)vpSize[1] } );
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


static int mesaure_count = 0;
static double time_sum = 0.0;


void ComputeShader(  
  unsigned int         target_tex_obj, //!< in:  target texture object 
  int                  tex_format,     //!< in:  internal texture format
  unsigned int         shader_obj,     //!< in:  compute shader program object 
  long                 cx,             //!< in:  width of the image 
  long                 cy,             //!< in:  height of the image
  long                 ch,             //!< in:  color channels (e.g. 3 for RGB, 4 for RGBA)
  long                 bpl,            //!< in:  bytes per scan line
  const unsigned char *data_in,        //!< in:  image bits
  int                  log_level )     //!< in:  true: process logging
{
  /*
  int height_map_texture_unit = 2;
  unsigned int height_map_obj;
  glGenTextures(1, &height_map_obj);
  glActiveTexture( GL_TEXTURE0 + height_map_texture_unit );
  glBindTexture(GL_TEXTURE_2D, height_map_obj);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, cx, cy, 0, ch == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data_in);
  glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
  */

  if ( log_level > 0 )
  {
    std::cout << "Create cone step map" << std::endl;
    std::cout << "    Image size: " << cx << " x " << cy << std::endl;

  }
  clock_t t_start = std::clock();

  //glBindImageTexture(1, target_tex_obj, 0, GL_FALSE, 0, GL_WRITE_ONLY, tex_format); 
  glBindImageTexture(1, target_tex_obj, 0, GL_FALSE, 0, GL_READ_WRITE, tex_format);
  OPENGL_CHECK_GL_ERROR

  glUseProgram( shader_obj );
  glDispatchCompute((GLuint)cx, (GLuint)cy, 1); 
  OPENGL_CHECK_GL_ERROR
    
  //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  glMemoryBarrier(GL_ALL_BARRIER_BITS); 
  OPENGL_CHECK_GL_ERROR

  clock_t t_end = std::clock();
  clock_t dt = t_end - t_start;
  if ( log_level )
  {
    std::cout << "Processed in " << (double)dt * 0.001 << " seconds" << std::endl;

    mesaure_count ++;
    time_sum += (double)dt;
    if ( mesaure_count > 1 )
      std::cout << "Average " << (double)time_sum * 0.001 / (double)mesaure_count << " seconds (" << time_sum << ")" << std::endl;
    std::cout << std::endl;
  }
}


void CreateConeMap_1( 
  std::vector<unsigned char> &data_out,   //!< out: conse step map
  long                        cx,         //!< in:  width of the image 
  long                        cy,         //!< in:  height of the image
  long                        ch,         //!< in:  color channels (e.g. 3 for RGB, 4 for RGBA)
  long                        bpl,        //!< in:  bytes per scan line
  const unsigned char        *data_in,    //!< in:  image bits
  int                         log_level ) //!< in:  true: process logging
{
  if ( log_level > 0 )
  {
    std::cout << "Create cone step map" << std::endl;
    std::cout << "    Image size: " << cx << " x " << cy << std::endl;

  }
  clock_t t_start = std::clock();

  const double max_cone_c = 1.0;

  data_out.reserve( cx*cy * 4 );
  for ( int y=0; y < cy; ++ y )
  {
    for ( int x=0; x < cx; ++ x )
    {
      unsigned char c = data_in[y*bpl + x*ch];

      // invert this (used to convert depth-map to height-map)
      if (false)
        c = 255 - c;
      
      unsigned char rgba[]{c, c, c, c};
      data_out.insert( data_out.end(), rgba, rgba + 4 );
    }
  }

  long width           = cx;
  long height          = cy;
  long chans           = 4;
  long ScanWidth       = width*chans;
  long TheSize         = ScanWidth * height;
  unsigned char  *Data = data_out.data();

  int   wProgress = width / 50;
  int   hProgress = height / 50;
  int   dProgress = width * height / 50;

  // pre-processing: compute derivatives
  if ( log_level > 1 )
    std::cout << "    calcualte derivatives for normal vectors (blue = dx, alpha = dy)" << std::endl << "        [";
  for (int y = 0; y < height; ++y)
  {
    // progress report: works great...if it's square!
    if ( log_level > 1 && (y % wProgress) == 0 )
      std::cout << ".";
    for (int x = 0; x < width; ++x)
    {
      int der;
      // Blue is the slope in x
      if (x == 0)
        der = (Data[y*ScanWidth + chans*(x+1)] - Data[y*ScanWidth + chans*(x)]) / 2;
      else if (x == width - 1)
        der = (Data[y*ScanWidth + chans*(x)] - Data[y*ScanWidth + chans*(x-1)]) / 2;
      else
        der = Data[y*ScanWidth + chans*(x+1)] - Data[y*ScanWidth + chans*(x-1)];
      Data[y*ScanWidth + chans*x + 2] = 127 + der / 2;
      // Alpha is the slope in y
      if (y == 0)
        der = (Data[(y+1)*ScanWidth + chans*x] - Data[(y)*ScanWidth + chans*x]) / 2;
      else if (y == height - 1)
        der = (Data[(y)*ScanWidth + chans*x] - Data[(y-1)*ScanWidth + chans*x]) / 2;
      else
        der = (Data[(y+1)*ScanWidth + chans*x] - Data[(y-1)*ScanWidth + chans*x]);
      // And the sign of Y will be reversed in OpenGL
      Data[y*ScanWidth + chans*x + 3] = 127 - der / 2;
    }
  }
  if ( log_level > 1 )
    std::cout << "]" << std::endl;

  if ( log_level > 1 )
      std::cout << "    scan " << height << " lines; red ... height, green ... cone equation (x = y * c) " << std::endl << "        [";
  

  float step_x = 1.0f / (float)cx;
  float step_y = 1.0f / (float)cy;
  float step   = std::max( step_x, step_y );
  for (int y = 0; y < height; ++y)
  {
    if ( log_level > 1 && (y % hProgress) == 0 )
      std::cout << ".";
    for (int x = 0; x < cx; ++x)
    {
      int   act_h        = Data[y*ScanWidth + chans * x];
      float c            = max_cone_c;
      float h            = (float)act_h / 255.0f;
      float max_h        = 1.0f - h;
      float max_dist     = std::min((float)max_cone_c * max_h, 1.0f);

      for( float dist = step; dist <= max_dist && c > dist / max_h; dist += step )
      {
        int   d2 = (int)(0.5f + (dist*dist) / (step*step));
        int   dy = (int)(0.5f + dist / step_y);
        int   sample_h = 0;
        for( int dx = 0; sample_h < 255 && (float)dx / (float)cx <= dist; ++ dx )
        {
          if ( (dx*dx + dy*dy) < d2 && dy < cy-1 )
            dy ++;
          do
          {
            int sx_n = ((cx + x - dx) % cx) * chans;
            int sx_p = ((cx + x + dx) % cx) * chans;
            int sy_n = ((cy + y - dy) % cy) * ScanWidth;
            int sy_p = ((cy + y + dy) % cy) * ScanWidth;
            
            sample_h = std::max( sample_h, (int)Data[sy_p + sx_p] );
            sample_h = std::max( sample_h, (int)Data[sy_n + sx_p] );
            sample_h = std::max( sample_h, (int)Data[sy_p + sx_n] );
            sample_h = std::max( sample_h, (int)Data[sy_n + sx_n] );

            dy --;
          }
          while ( dy > 0 && (dx*dx + dy*dy) >= d2 );
        }
        if ( sample_h > act_h )
        {
          float d_h =  (float)(sample_h-act_h) / 255.0f;
          float sample_c = dist / d_h; 
          c = std::min( c, sample_c );
        }
      }

      Data[y*ScanWidth + chans * x + 1] = (unsigned char)( sqrt(c)*255.0f );
    }
  }
  if ( log_level > 1 )
    std::cout << "]" << std::endl;

  clock_t t_end = std::clock();
  clock_t dt = t_end - t_start;
  if ( log_level )
  {
    std::cout << "Processed in " << (double)dt * 0.001 << " seconds" << std::endl;

    mesaure_count ++;
    time_sum += (double)dt;
    if ( mesaure_count > 1 )
      std::cout << "Average " << (double)time_sum * 0.001 / (double)mesaure_count << " seconds (" << time_sum << ")" << std::endl;
    std::cout << std::endl;
  }
}


void CreateConeMap_flat( 
  std::vector<unsigned char> &data_out,   //!< out: conse step map
  long                        cx,         //!< in:  width of the image 
  long                        cy,         //!< in:  height of the image
  long                        ch,         //!< in:  color channels (e.g. 3 for RGB, 4 for RGBA)
  long                        bpl,        //!< in:  bytes per scan line
  const unsigned char        *data_in,    //!< in:  image bits
  int                         log_level ) //!< in:  true: process logging
{
  if ( log_level > 0 )
  {
    std::cout << "Create cone step map" << std::endl;
    std::cout << "    Image size: " << cx << " x " << cy << std::endl;
  }
  clock_t t_start = std::clock();

  const double max_cone_c = 1.0;

  data_out.reserve( cx*cy * 4 );
  for ( int y=0; y < cy; ++ y )
  {
    for ( int x=0; x < cx; ++ x )
    {
      unsigned char c = data_in[y*bpl + x*ch];

      // invert this (used to convert depth-map to height-map)
      if (false)
        c = 255 - c;
      
      unsigned char rgba[]{c, c, c, c};
      data_out.insert( data_out.end(), rgba, rgba + 4 );
    }
  }

  long width           = cx;
  long height          = cy;
  long chans           = 4;
  long ScanWidth       = width*chans;
  long TheSize         = ScanWidth * height;
  unsigned char  *Data = data_out.data();

  int   wProgress = width / 50;
  int   hProgress = height / 50;
  int   dProgress = width * height / 50;

  // pre-processing: compute derivatives
  if ( log_level > 1 )
    std::cout << "    calcualte derivatives for normal vectors (blue = dx, alpha = dy)" << std::endl << "        [";
  for (int y = 0; y < height; ++y)
  {
    // progress report: works great...if it's square!
    if ( log_level > 1 && (y % wProgress) == 0 )
      std::cout << ".";
    for (int x = 0; x < width; ++x)
    {
      int der;
      // Blue is the slope in x
      if (x == 0)
        der = (Data[y*ScanWidth + chans*(x+1)] - Data[y*ScanWidth + chans*(x)]) / 2;
      else if (x == width - 1)
        der = (Data[y*ScanWidth + chans*(x)] - Data[y*ScanWidth + chans*(x-1)]) / 2;
      else
        der = Data[y*ScanWidth + chans*(x+1)] - Data[y*ScanWidth + chans*(x-1)];
      Data[y*ScanWidth + chans*x + 2] = 127 + der / 2;
      // Alpha is the slope in y
      if (y == 0)
        der = (Data[(y+1)*ScanWidth + chans*x] - Data[(y)*ScanWidth + chans*x]) / 2;
      else if (y == height - 1)
        der = (Data[(y)*ScanWidth + chans*x] - Data[(y-1)*ScanWidth + chans*x]) / 2;
      else
        der = (Data[(y+1)*ScanWidth + chans*x] - Data[(y-1)*ScanWidth + chans*x]);
      // And the sign of Y will be reversed in OpenGL
      Data[y*ScanWidth + chans*x + 3] = 127 - der / 2;
    }
  }
  if ( log_level > 1 )
    std::cout << "]" << std::endl;

  if ( log_level > 1 )
      std::cout << "    scan " << height << " lines; red ... height, green ... cone equation (x = y * c) " << std::endl << "        [";
  

  float step_x = 1.0f / (float)cx;
  float step_y = 1.0f / (float)cy;
  float step   = std::max( step_x, step_y );
  for (int y = 0; y < height; ++y)
  {
    if ( log_level > 1 && (y % hProgress) == 0 )
      std::cout << ".";
    for (int x = 0; x < cx; ++x)
    {
      int   act_h        = Data[y*ScanWidth + chans * x];
      float c            = max_cone_c;
      float h            = (float)act_h / 255.0f;
      float max_h        = 1.0f - h;
      float max_dist     = std::min((float)max_cone_c * max_h, 1.0f);

      for( float dist = step; dist <= max_dist && c > dist / max_h; dist += step )
      {
        int   d2 = (int)(0.5f + (dist*dist) / (step*step));
        int   dy = (int)(0.5f + dist / step_y);
        int   sample_h = 0;
        for( int dx = 0; sample_h < 255 && (float)dx / (float)cx <= dist; ++ dx )
        {
          if ( (dx*dx + dy*dy) < d2 && dy < cy-1 )
            dy ++;
          do
          {
            int sx_n = ((cx + x - dx) % cx) * chans;
            int sx_p = ((cx + x + dx) % cx) * chans;
            int sy_n = ((cy + y - dy) % cy) * ScanWidth;
            int sy_p = ((cy + y + dy) % cy) * ScanWidth;
            
            sample_h = std::max( sample_h, (int)Data[sy_p + sx_p] );
            sample_h = std::max( sample_h, (int)Data[sy_n + sx_p] );
            sample_h = std::max( sample_h, (int)Data[sy_p + sx_n] );
            sample_h = std::max( sample_h, (int)Data[sy_n + sx_n] );

            dy --;
          }
          while ( dy > 0 && (dx*dx + dy*dy) >= d2 );
        }
        if ( sample_h > act_h )
        {
          float d_h =  (float)(sample_h-act_h) / 255.0f;
          float sample_c = dist / d_h; 
          c = std::min( c, sample_c );
        }
      }

      Data[y*ScanWidth + chans * x + 1] = (unsigned char)( sqrt(c)*255.0f );
    }
  }
  if ( log_level > 1 )
    std::cout << "]" << std::endl;

  clock_t t_end = std::clock();
  clock_t dt = t_end - t_start;
  if ( log_level )
  {
    std::cout << "Processed in " << (double)dt * 0.001 << " seconds" << std::endl;

    mesaure_count ++;
    time_sum += (double)dt;
    if ( mesaure_count > 1 )
      std::cout << "Average " << (double)time_sum * 0.001 / (double)mesaure_count << " seconds (" << time_sum << ")" << std::endl;
    std::cout << std::endl;
  }
}

/*
basically, 99% of all pixels will fall in under 2.0
(most of the time, on the heightmaps I've tested)
the question:
Is reduced resolution worth missing
the speedup of the slow ones?
*/
const float max_ratio = 1.0;

// Do I want the textures to be computed as tileable?
bool x_seamless = true;
bool y_seamless = true;


// Algortihm from http://www.lonesock.net/files/ConeStepMapping.pdf
void CreateConeMap_from_ConeStepMapping_pdf( 
  std::vector<unsigned char> &data_out,   //!< out: conse step map
  long                        cx,         //!< in:  width of the image 
  long                        cy,         //!< in:  height of the image
  long                        ch,         //!< in:  color channels (e.g. 3 for RGB, 4 for RGBA)
  long                        bpl,        //!< in:  bytes per scan line
  const unsigned char        *data_in,    //!< in:  image bits
  int                         log_level ) //!< in:  true: process logging
{
  if ( log_level > 0 )
  {
    std::cout << "Create cone step map" << std::endl;
    std::cout << "    Image size: " << cx << " x " << cy << std::endl;

  }
  clock_t t_start = std::clock();

  float really_max = 1.0;
  
  data_out.reserve( cx*cy * 4 );
  for ( int y=0; y < cy; ++ y )
  {
    for ( int x=0; x < cx; ++ x )
    {
      unsigned char c = data_in[y*bpl + x*ch];

      // invert this (used to convert depth-map to height-map)
      if (false)
        c = 255 - c;
      
      unsigned char rgba[]{c, c, c, c};
      data_out.insert( data_out.end(), rgba, rgba + 4 );
    }
  }

  long width           = cx;
  long height          = cy;
  long chans           = 4;
  long ScanWidth       = width*chans;
  long TheSize         = ScanWidth * height;
  unsigned char  *Data = data_out.data();

  float iheight   = 1.0f / height;
  float iwidth    = 1.0f / width;
  int   wProgress = width / 50;
  int   hProgress = height / 50;
  
  // pre-processing: compute derivatives
  if ( log_level > 1 )
    std::cout << "    calcualte derivatives for normal vectors (blue = dx, alpha = dy)" << std::endl << "        [";
  for (int y = 0; y < height; ++y)
  {
    // progress report: works great...if it's square!
    if ( log_level > 1 && y % wProgress == 0)
      std::cout << ".";
    for (int x = 0; x < width; ++x)
    {
      int der;
      // Blue is the slope in x
      if (x == 0)
        der = (Data[y*ScanWidth + chans*(x+1)] - Data[y*ScanWidth + chans*(x)]) / 2;
      else if (x == width - 1)
        der = (Data[y*ScanWidth + chans*(x)] - Data[y*ScanWidth + chans*(x-1)]) / 2;
      else
        der = Data[y*ScanWidth + chans*(x+1)] - Data[y*ScanWidth + chans*(x-1)];
      Data[y*ScanWidth + chans*x + 2] = 127 + der / 2;
      // Alpha is the slope in y
      if (y == 0)
        der = (Data[(y+1)*ScanWidth + chans*x] - Data[(y)*ScanWidth + chans*x]) / 2;
      else if (y == height - 1)
        der = (Data[(y)*ScanWidth + chans*x] - Data[(y-1)*ScanWidth + chans*x]) / 2;
      else
        der = (Data[(y+1)*ScanWidth + chans*x] - Data[(y-1)*ScanWidth + chans*x]);
      // And the sign of Y will be reversed in OpenGL
      Data[y*ScanWidth + chans*x + 3] = 127 - der / 2;
    }
  }
  if ( log_level > 1 )
    std::cout << "]" << std::endl;
  // OK, do the processing
  if ( log_level > 1 )
      std::cout << "    scan " << height << " lines; red ... height, green ... cone equation (x = y * c) " << std::endl << "        [";
  for (int y = 0; y < height; ++y)
  {
    if (log_level > 1 && (y % hProgress) == 0)
      std::cout << ".";
    for (int x = 0; x < width; ++x)
    {
      float actual_ratio;
      int x1, x2, y1, y2;
      float r2, h2;
      // set up some initial values
      // (note I'm using ratio squared throughout,
      // and taking sqrt at the end...faster)
      float min_ratio2 = max_ratio * max_ratio;
      // information about this center point
      float ht   = Data[y*ScanWidth + chans*x] / 255.0f;
      float dhdx = +(Data[y*ScanWidth + chans*x + 2] / 255.0f - 0.5f) * width;
      float dhdy = -(Data[y*ScanWidth + chans*x + 3] / 255.0f - 0.5f) * height;
      // scan in outwardly expanding blocks
      // (so I can stop if I reach my minimum ratio)
      for (int rad = 1;
        (rad*rad <= 1.1*1.1*(1.0-ht)*(1.0-ht)*min_ratio2*width*height)
        && (rad <= 1.1*(1.0-ht)*width) && (rad <= 1.1*(1.0-ht)*height);
        ++rad)
      {
        // ok, for each of these lines...
        // West
        x1 = x - rad;
        while (x_seamless && (x1 < 0))
          x1 += width;
        if (x1 >= 0)
        {
          float delx = -rad*iwidth;
          // y limits
          // (+- 1 because I'll cover the corners in the X-run)
          y1 = y - rad + 1;
          if (y1 < 0)
            y1 = 0;
          y2 = y + rad - 1;
          if (y2 >= height)
            y2 = height - 1;
          // and check the line
          for (int dy = y1; dy <= y2; ++dy)
          {
            float dely = (dy-y)*iheight;
            r2 = delx*delx + dely*dely;
             //h2 = Data[dy*ScanWidth + chans*x1] / 255.0f - ht;
            h2 = (0.5f+Data[dy*ScanWidth + chans*x1]) / 255.0f - ht;
            if ((h2 > 0.0f) && (h2*h2 * min_ratio2> r2))
            {
              // this is the new (lowest) value
              min_ratio2 = r2 / (h2 * h2);
            }
          }
        }
        // East
        x2 = x + rad;
        while (x_seamless && (x2 >= width))
          x2 -= width;
        if (x2 < width)
        {
          float delx = rad*iwidth;
          // y limits
          // (+- 1 because I'll cover the corners in the X-run)
          y1 = y - rad + 1;
          if (y1 < 0)
            y1 = 0;
          y2 = y + rad - 1;
          if (y2 >= height)
            y2 = height - 1;
          // and check the line
          for (int dy = y1; dy <= y2; ++dy)
          {
            float dely = (dy-y)*iheight;
            r2 = delx*delx + dely*dely;
            //h2 = Data[dy*ScanWidth + chans*x2] / 255.0f - ht;
            h2 = (0.5f+Data[dy*ScanWidth + chans*x2]) / 255.0f - ht;
            if ((h2 > 0.0f) && (h2*h2 * min_ratio2> r2))
            {
              // this is the new (lowest) value
              min_ratio2 = r2 / (h2 * h2);
            }
          }
        }
        // North
        y1 = y - rad;
        while (y_seamless && (y1 < 0))
          y1 += height;
        if (y1 >= 0)
        {
          float dely = -rad*iheight;
          // y limits
          // (+- 1 because I'll cover the corners in the X-run)
          x1 = x - rad;
          if (x1 < 0)
            x1 = 0;
          x2 = x + rad;
          if (x2 >= width)
            x2 = width - 1;
          // and check the line
          for (int dx = x1; dx <= x2; ++dx)
          {
            float delx = (dx-x)*iwidth;
            r2 = delx*delx + dely*dely;
            //h2 = Data[y1*ScanWidth + chans*dx] / 255.0f - ht;
            h2 = (0.5f+Data[y1*ScanWidth + chans*dx]) / 255.0f - ht;
            if ((h2 > 0.0f) && (h2*h2 * min_ratio2> r2))
            {
              // this is the new (lowest) value
              min_ratio2 = r2 / (h2 * h2);
            }
          }
        }
        // South
        y2 = y + rad;
        while (y_seamless && (y2 >= height))
          y2 -= height;
        if (y2 < height)
        {
          float dely = rad*iheight;
          // y limits
          // (+- 1 because I'll cover the corners in the X-run)
          x1 = x - rad;
          if (x1 < 0)
            x1 = 0;
          x2 = x + rad;
          if (x2 >= width)
            x2 = width - 1;
          // and check the line
          for (int dx = x1; dx <= x2; ++dx)
          {
            float delx = (dx-x)*iwidth;
            r2 = delx*delx + dely*dely;
            //h2 = Data[y2*ScanWidth + chans*dx] / 255.0f - ht;
            h2 = (0.5f+Data[y2*ScanWidth + chans*dx]) / 255.0f - ht;
            if ((h2 > 0.0f) && (h2*h2 * min_ratio2> r2))
            {
              // this is the new (lowest) value
              min_ratio2 = r2 / (h2 * h2);
            }
          }
        }
        // done with the expanding loop
      }
      /********** CONE VERSION **********/
      // actually I have the ratio squared. Sqrt it
      actual_ratio = sqrt (min_ratio2);
      // here I need to scale to 1.0
      actual_ratio /= max_ratio;
      // most of the data is on the low end...sqrting again spreads it better
      // (plus multiply is a cheap operation in shaders!)
      actual_ratio = sqrt (actual_ratio);
      // Red stays height
      // Blue remains the slope in x
      // Alpha remains the slope in y
      // but Green becomes Step-Cone-Ratio
      Data[y*ScanWidth + chans*x + 1] = static_cast<unsigned char>(255.0 * actual_ratio + 0.5);
      // but make sure it is > 0.0, since I divide by it in the shader
      if (Data[y*ScanWidth + chans*x + 1] < 1)
        Data[y*ScanWidth + chans*x + 1] = 1;
    }
  }
  if ( log_level > 1 )
    std::cout << "]" << std::endl;

  clock_t t_end = std::clock();
  clock_t dt = t_end - t_start;
  if ( log_level )
    std::cout << "Processed in " << (double)dt * 0.001 << " seconds" << std::endl << std::endl;
}