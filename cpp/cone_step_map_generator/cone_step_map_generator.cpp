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

void main()
{
    vec4 tex_color = texture(u_texture, in_data.uv);
    fragColor = vec4(tex_color.rg, 0.0, 1.0);
    //fragColor = tex_color;
}
)";


std::unique_ptr<OpenGL::ShaderProgram> g_prog;

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
    int cx, cy, ch;
    stbi_uc *img = stbi_load( path.c_str(), &cx, &cy, &ch, 3 );
    if ( img != nullptr )
    {
        std::vector<unsigned char> cone_map;
        
        CreateConeMap( cone_map, cx, cy, 3, cx*3, img, 1 );
        //CreateConeMap_from_ConeStepMapping_pdf ( cone_map, cx, cy, 3, cx*3, img, 1 );

        stbi_image_free( img );

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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, cx, cy, 0, GL_RGBA, GL_UNSIGNED_BYTE, cone_map.data());
        //glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
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


void CreateConeMap( 
  std::vector<unsigned char> &data_out,   //!< out: conse step map
  long                        cx,         //!< in:  width of the image 
  long                        cy,         //!< in:  height of the image
  long                        ch,         //!< in:  color channels (e.g. 3 for RGB, 4 for RGBA)
  long                        bpl,        //!< in:  bytes per scan line
  const unsigned char        *data_in,    //!< in:  image bits
  int                         log_level ) //!< in:  true: process logging
{
  // TODO $$$ seamless?
  const double max_cone_c = 1.0;

  clock_t t_start = std::clock();

  if ( log_level > 0 )
  {
    std::cout << "Create cone step map" << std::endl;
    std::cout << "    Image size: " << cx << " x " << cy << std::endl;

  }

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
  int   dProgress = width * height / 50;

  if ( log_level > 0 )
    std::cout << "    create distance map" << std::endl << "        [";
  std::map<float, std::vector<std::array<int,2>>> dist_map;
  const int cone_div = 2;
  //const int cone_div = 10;
  for ( int y = -cy/cone_div; y < cy/cone_div; ++ y )
  {
    for ( int x = -cx/cone_div; x < cx/cone_div; ++ x )
    {
      if ( x == 0 && y == 0 )
        continue;
      int i = ( y*cx + x )-1;
      if ( log_level > 0 && (i % dProgress) == 0 )
        std::cout << ".";
      double fx = (double)x / (double)cx;
      double fy = (double)y / (double)cy;
      double dist = fx*fx + fy*fy;
      if ( dist < max_cone_c ) // TODO $$$ max 
        dist_map[(float)dist].emplace_back( std::array<int,2>{x, y} );
    }
  }
  if ( log_level > 0 )
    std::cout << "]" << std::endl;
  
  // pre-processing: compute derivatives
  if ( log_level > 0 )
    std::cout << "    calcualte derivatives for normal vectors (blue = dx, alpha = dy)" << std::endl << "        [";
  for (int y = 0; y < height; ++y)
  {
    // progress report: works great...if it's square!
    if ( log_level > 0 && (y % wProgress) == 0 )
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
  if ( log_level > 0 )
    std::cout << "]" << std::endl;

  if ( log_level > 0 )
      std::cout << "    scan " << height << " lines; red ... height, green ... cone equation (x = y * c) " << std::endl << "        [";
  for (int y = 0; y < height; ++y)
  {
    if ( log_level > 0 && (y % hProgress) == 0 )
      std::cout << ".";
    for (int x = 0; x < width; ++x)
    {
      float c        = max_cone_c;
      float h        = (float)Data[y*ScanWidth + chans * x] / 255.0f;
      float max_h    = 1.0f - h;
      float max_dist = (float)max_cone_c * max_h;
      for ( auto &d_info : dist_map )
      {
        float dist2 = d_info.first;
        if ( dist2 > max_dist*max_h )
          break;
        float min_c = d_info.first / (max_h*max_h);
        if ( min_c >= c )
          break;

        for ( auto &sample : d_info.second )
        {
          int sx = (cx + x + sample[0]) % cx;
          int sy = (cy + y + sample[1]) % cy;
          float sample_h = (float)Data[sy*ScanWidth + chans * sx] / 255.0f;
          if ( sample_h <= h )
            continue;
          float d_h = sample_h - h;
          float sample_c = dist2 / (d_h*d_h);
          c = std::min( c, sample_c );
        }
      }
      Data[y*ScanWidth + chans * x + 1] = (unsigned char)( sqrt(c)*255.0f );
    }
  }
  if ( log_level > 0 )
    std::cout << "]" << std::endl;

  clock_t t_end = std::clock();
  clock_t dt = t_end - t_start;
  if ( log_level )
    std::cout << "Processed in " << (double)dt * 0.001 << " seconds" << std::endl << std::endl;
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
  clock_t t_start = std::clock();

  if ( log_level > 0 )
  {
    std::cout << "Create cone step map" << std::endl;
    std::cout << "    Image size: " << cx << " x " << cy << std::endl;

  }

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
  if ( log_level > 0 )
    std::cout << "    calcualte derivatives for normal vectors (blue = dx, alpha = dy)" << std::endl << "        [";
  for (int y = 0; y < height; ++y)
  {
    // progress report: works great...if it's square!
    if ( y % wProgress == 0 && log_level > 0 )
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
  if ( log_level > 0 )
    std::cout << "]" << std::endl;
  // OK, do the processing
  if ( log_level > 0 )
      std::cout << "    scan " << height << " lines; red ... height, green ... cone equation (x = y * c) " << std::endl << "        [";
  for (int y = 0; y < height; ++y)
  {
    if ((y % hProgress) == 0 && log_level > 0)
      std::cout << ".";
    for (int x = 0; x < width; ++x)
    {
      float min_ratio2, actual_ratio;
      int x1, x2, y1, y2;
      float ht, dhdx, dhdy, r2, h2;
      // set up some initial values
      // (note I'm using ratio squared throughout,
      // and taking sqrt at the end...faster)
      min_ratio2 = max_ratio * max_ratio;
      // information about this center point
      ht = Data[y*ScanWidth + chans*x] / 255.0f;
      dhdx = +(Data[y*ScanWidth + chans*x + 2] / 255.0f - 0.5f) * width;
      dhdy = -(Data[y*ScanWidth + chans*x + 3] / 255.0f - 0.5f) * height;
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
            h2 = Data[dy*ScanWidth + chans*x1] / 255.0f - ht;
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
            h2 = Data[dy*ScanWidth + chans*x2] / 255.0f - ht;
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
            h2 = Data[y1*ScanWidth + chans*dx] / 255.0f - ht;
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
            h2 = Data[y2*ScanWidth + chans*dx] / 255.0f - ht;
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
  if ( log_level > 0 )
    std::cout << "]" << std::endl;

  clock_t t_end = std::clock();
  clock_t dt = t_end - t_start;
  if ( log_level )
    std::cout << "Processed in " << (double)dt * 0.001 << " seconds" << std::endl << std::endl;
}