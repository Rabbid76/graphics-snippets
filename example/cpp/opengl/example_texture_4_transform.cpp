#include <stdafx.h>

struct Camera;
void TestOutMat( Camera & );

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <tuple>
#include <array>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <math.h>

// project includes
#include <gl/gl_debug.h>
#include <gl/gl_shader.h>

// preprocessor definitions

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif


std::string sh_vert = R"(
#version 400

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTex;
layout (location = 2) in vec3 inCol;

out vec3 vertCol;
out vec2 vertTex;

uniform mat4 u_projectionMat44;
uniform mat4 u_viewMat44;
uniform mat4 u_modelMat44;
uniform mat4 u_textureMat44;

void main()
{
    vertCol       = inCol;
    vec4 texCoord = u_textureMat44 * vec4( inTex, 0.0, 1.0 );
    vertTex       = texCoord.st;
    vec4 modolPos = u_modelMat44 * vec4( inPos, 1.0 );
    vec4 viewPos  = u_viewMat44 * modolPos;
    gl_Position   = u_projectionMat44 * viewPos;
}
)";

std::string sh_frag = R"(
#version 400

in vec3 vertCol;
in vec2 vertTex;

out vec4 fragColor;

uniform sampler2D u_texture;

void main()
{
    vec4 texCol = texture( u_texture, vertTex.st );
    fragColor = vec4( texCol.xyz, 1.0 );
}
)";

const float cPI = 3.141593f;
float ToRad( float deg ) { return deg * cPI / 180.0f; }
float ToDeg( float rad ) { return rad * 180.0f / cPI; }


template< typename T_VEC >
TVec3 Cross( T_VEC a, T_VEC b ) { return { a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0] }; }

template< typename T_A, typename T_B >
float Dot( T_A a, T_B b ) { return a[0]*b[0] + a[1]*b[1] + a[2]*b[2]; }

template< typename T_VEC >
void Normalize( T_VEC & v )
{
  float len = sqrt( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] );
  v[0] /= len; v[1] /= len; v[2] /= len;
}

TMat44 Identity( void )
{
  return TMat44( { TVec4{ 1.0f, 0.0f, 0.0f, 0.0f },
                   TVec4{ 0.0f, 1.0f, 0.0f, 0.0f },
                   TVec4{ 0.0f, 0.0f, 1.0f, 0.0f },
                   TVec4{ 0.0f, 0.0f, 0.0f, 1.0f } } );
}

TMat44 Multiply( const TMat44 & matA, const TMat44 & matB )
{
  TMat44 matC;
  for ( int i0 = 0; i0 < 4; ++ i0 )
    for ( int i1 = 0; i1 < 4; ++ i1 )
      matC[i0][i1] = matB[i0][0] * matA[0][i1] + matB[i0][1] * matA[1][i1] + matB[i0][2] * matA[2][i1] + matB[i0][3] * matA[3][i1];
  return matC;
}

glm::mat4 ToGLM( const TMat44 &m )
{
  return glm::mat4(
    m[0][0], m[0][1], m[0][2], m[0][3],
    m[1][0], m[1][1], m[1][2], m[1][3],
    m[2][0], m[2][1], m[2][2], m[2][3],
    m[3][0], m[3][1], m[3][2], m[3][3]
  );
}

struct Camera
{
  TVec3 pos    {0.0, 0.0, 5.0};
  TVec3 target {0.0, 0.0, 0.0};
  TVec3 up     {0.0, 1.0, .0};
  float fov_y  {85.0};
  TSize vp     {800, 600};
  float near   {0.5};
  float far    {100.0};

  TMat44 Perspective( void );
  TMat44 LookAt( void );
};

TMat44 Camera::Perspective( void )
{
  float fn = far + near, f_n = far - near;
  float r = (float)vp[0] / vp[1], t = 1.0f / tan( ToRad( fov_y ) / 2.0f );
  return TMat44{ TVec4{ t / r, 0.0f, 0.0f, 0.0f }, TVec4{ 0.0f, t, 0.0f, 0.0f }, TVec4{ 0.0f, 0.0f, -fn / f_n, -1.0 }, TVec4{ 0.0f, 0.0f, -2.0f*far*near / f_n, 0.0f } };
}

TMat44 Camera::LookAt( void )
{ 
  TVec3 mz = { pos[0] - target[0], pos[1] - target[1], pos[2] - target[2] };
  Normalize( mz );
  TVec3 my = { up[0], up[1], up[2] };
  TVec3 mx = Cross( my, mz );
  Normalize( mx );
  my = Cross( mz, mx );
  
  TMat44 v{
    TVec4{ mx[0], my[0], mz[0], 0.0f },
    TVec4{ mx[1], my[1], mz[1], 0.0f },
    TVec4{ mx[2], my[2], mz[2], 0.0f },
    TVec4{ Dot(mx, pos), Dot(my, pos), -Dot(mz, pos), 1.0f }
  };
  
  return v;
}


struct Mat44 : public TMat44
{
  Mat44( void ) : TMat44( Identity() ) {}
  Mat44( const Mat44 & ) = default;
  Mat44( const TMat44 & mat ) : TMat44( mat ) {}

  using pointer = float *;
  operator pointer(void) { return data()->data(); }

  Mat44 & Translate( const TVec3 & trans )
  {
    for ( int i = 0; i < 4; ++ i )
      (*this)[3][i] = (*this)[0][i] * trans[0] + (*this)[1][i] * trans[1] + (*this)[2][i] * trans[2] + (*this)[3][i];
    return *this;
  }
  
  Mat44 & Scale( const TVec3 & s )
  {
    for ( int i0 = 0; i0 < 3; ++ i0 )
      for ( int i1 = 0; i1 < 4; ++ i1 ) ( *this )[i0][i1] *= s[i0];
    return *this;
  }     

  Mat44 & RotateX( float angRad ) { return RotateHlp( angRad, 1, 2 ); }
  Mat44 & RotateY( float angRad ) { return RotateHlp( angRad, 2, 0 ); }
  Mat44 & RotateZ( float angRad ) { return RotateHlp( angRad, 0, 1 ); }
  Mat44 & RotateView( std::array< float, 3 > angRad )
  {
    RotateX( angRad[0] );
    RotateY( angRad[1] );
    RotateZ( angRad[2] );
    return *this;
  }

private:

  Mat44 & RotateHlp( float angRad, int a0, int a1)
  {
    TMat44 &matA = *this;
    TMat44 matB = matA;
    float sinAng = sin( angRad ), cosAng = cos( angRad );
    for ( int i = 0; i < 4; ++ i )
    {
      matB[a0][i] = matA[a0][i] * cosAng + matA[a1][i] * sinAng;
      matB[a1][i] = matA[a0][i] * -sinAng + matA[a1][i] * cosAng;
    }
    matA = matB;
    return *this;
  }
};


// vertex array object
class VAObject
{
private:

  GLuint                _obj;
  std::vector< GLuint > _indices;

public:
  
  using TBuffer = std::tuple< GLint, std::vector< float > >;
  VAObject( const std::vector< TBuffer > & dataArrays, const std::vector< GLuint > & indices  )
    : _indices( indices )
  {
    GLuint indexBuffer;
    glGenBuffers( 1, &indexBuffer );
    

    glGenVertexArrays( 1, &_obj );
    std::vector<GLuint> buffers( dataArrays.size() );
    glGenBuffers( (GLsizei)buffers.size(), buffers.data() );
    glBindVertexArray( _obj );
    for ( size_t i_buffer = 0; i_buffer < buffers.size(); ++ i_buffer )
    {
      GLint elemSize = std::get<0>( dataArrays[i_buffer] );
      std::vector< float > buffer = std::get<1>( dataArrays[i_buffer] );
      glBindBuffer( GL_ARRAY_BUFFER, buffers[i_buffer] );
      glBufferData( GL_ARRAY_BUFFER, buffer.size() * sizeof( float ), buffer.data(), GL_STATIC_DRAW );
      glEnableVertexAttribArray( (GLuint)i_buffer );
      glVertexAttribPointer( (GLuint)i_buffer, elemSize, GL_FLOAT, GL_FALSE, 0, nullptr );
    }
    
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexBuffer );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof( GLuint ), _indices.data(), GL_STATIC_DRAW );
    glBindVertexArray( 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
  }

  virtual ~VAObject()
  {
    glDeleteVertexArrays( 1, &_obj );
  }
  
  void Draw( void ) const
  {  
    glBindVertexArray( _obj );
    //glDrawArrays( GL_TRIANGLES, 0, (GLsizei)_indices.size() );
    glDrawElements( GL_TRIANGLES, (GLsizei)_indices.size(), GL_UNSIGNED_INT, NULL );
    //glDrawElements( GL_TRIANGLES, (GLsizei)_indices.size(), GL_UNSIGNED_INT, _indices.data() );
  }
};

class Window
{
private:

  double               _startTime = 0.0;
  std::array< int, 2 > _size;
  GLFWwindow *         _wnd = nullptr;

public:

  Window( std::array< int, 2 > size )
    : _size( size )
  {
    if ( glfwInit() == 0 )
      throw std::runtime_error( "error initializing glfw" ); 
    _wnd = glfwCreateWindow( _size[0], _size[1], "OGL window", nullptr, nullptr );
    if ( _wnd == nullptr )
    {
      glfwTerminate();
      throw std::runtime_error( "error initializing window1" ); 
    }
    glfwMakeContextCurrent( _wnd );
    glfwSetCursorPosCallback( _wnd, Window::Callback );
  }
  
  virtual ~Window()
  {
    if ( _wnd == nullptr )
      return;
    glfwDestroyWindow( _wnd );
    glfwTerminate();
  }

  static void Callback(GLFWwindow* window, double xpos, double ypos)
  {
    //std::cout << xpos << ", " << ypos << std::endl;
    
  }
    
  void Run( const OpenGL::ShaderProgramSimple &prog, const VAObject &vao )
  {
    if ( _wnd == nullptr )
      throw std::runtime_error( "error creating glfw window" );

    _startTime = glfwGetTime();
    while ( glfwWindowShouldClose( _wnd ) == 0 )
    {
      double currentTime = glfwGetTime();
      glfwGetFramebufferSize( _wnd, &_size[0], &_size[1] );
      
      // set up camera
      Camera camera;
      camera.vp = _size;
      //camera.pos = EllipticalPosition( 15.0f, 12.0f, CalcAng( currentTime, 10.0f ) );

      static bool testPrint = false;
      if ( testPrint )
        TestOutMat( camera );
      
      // set up attributes and shader program
      glEnable( GL_DEPTH_TEST );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      prog.Use();
      prog.SetUniformM44( "u_projectionMat44", camera.Perspective() );
      prog.SetUniformM44( "u_viewMat44", camera.LookAt() );

      Mat44 texMat;
      texMat.Translate( { -0.5f, -0.5f, 0.0f } );
      texMat.RotateZ( CalcAng( currentTime, 20.0f ) );
      texMat.Translate( { 0.5f, 0.5f, 0.0f } );
      prog.SetUniformM44( "u_textureMat44", texMat );
      prog.SetUniformI1( "u_texture", 0 );
        
      // draw object
      Mat44 modelMat;
      modelMat.Scale( { 3.0f, 3.0f, 3.0f } );
      //modelMat.Translate( { 0.0f, 0.0, CalcMove( currentTime, 6.0f, {-1.0f, 1.0} ) } );
      //modelMat.RotateX( CalcAng( currentTime, 13.0f ) );
      //modelMat.RotateY( CalcAng( currentTime, 17.0f ) );
      prog.SetUniformM44( "u_modelMat44", modelMat );

      vao.Draw();

      glfwSwapBuffers( _wnd );
      glfwPollEvents();
    }

    glfwDestroyWindow( _wnd );
    _wnd = nullptr;
    glfwTerminate();
  }

  static float Fract( float val ) { return val - trunc(val); }
  float CalcAng( double currentTime, float intervall )
  { 
    return Fract( (float)( currentTime - _startTime ) / intervall ) * 2.0f * cPI;
  }
  float CalcMove( double currentTime, float intervall, std::array< float, 2 > range )
  {
    float pos = Fract( (float)( currentTime - _startTime ) / intervall ) * 2.0f;
    pos = pos < 1.0 ? pos : 2.0f - pos;
    return range[0] + (range[1] - range[0]) * pos;
  }
  TVec3 EllipticalPosition( float a, float b, float angRag )
  {
    float a_b = a * a - b * b;
    float ea = a_b > 0.0f ? sqrt( a_b ) : 0.0f;
    float eb = a_b < 0.0f ? sqrt( -a_b ) : 0.0f;
    return { a * sin( angRag ) - ea, b * cos( angRag ) - eb, 0.0f };
  }
};

int main(void)
{ 
    int width = 100, height = 100;
    std::vector<unsigned char> image( width * height * 4 );
    for ( int y = 0; y < height; ++ y )
    {
      for ( int x = 0; x < width; ++ x )
      {
        int i = (y * width + x ) * 4;
        float fx = (float)x / width;
        float fy = (float)y / height;

        image[i + 0] = (unsigned char)((1.0f-fx)*255.0f);
        image[i + 1] = (unsigned char)((1.0f-fy)*255.0f);
        image[i + 2] = (unsigned char)(fx*fy*255.0f);
        image[i + 3] = 255;
      }
    }


  try
  {
    // create window
    Window *wnd = new Window( { 800, 600 } );
    
    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
    OpenGL::CContext context;
    context.Init( debug_level );

    glActiveTexture( GL_TEXTURE0 );
    GLuint texture;
    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    // glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // Removed from GL 3.1 and above 

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data() );

    // define tetrahedron vertex array opject
    const float sin120 = 0.8660254f;
    VAObject * vaObj = new VAObject(
    { { 3, { -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f } },
      { 2, { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f } },
      { 3, { 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f } }
    },
    { 0, 1, 2, 0, 2, 3 } );
    //VAObject * vaObj2 = new VAObject(
    //{ { 3, { 0.0f, 0.0f, 1.0f, 0.0f, -sin120, -0.5f, sin120 * sin120, 0.5f * sin120, -0.5f, -sin120 * sin120, 0.5f * sin120, -0.5f } },
    //  { 3, { 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f } }
    //},
    //{ 0, 1, 2, 0, 2, 3, 0, 3, 1, 1, 3, 2 } );

    // load, compile and link shader
    OpenGL::ShaderProgramSimple * prog = new OpenGL::ShaderProgramSimple(
    { { sh_vert, GL_VERTEX_SHADER },
      { sh_frag, GL_FRAGMENT_SHADER }
    } );
  
    // start main loop
    wnd->Run( *prog, *vaObj );

    delete vaObj;
    delete prog;
    delete wnd;
  }
  catch( const std::runtime_error& re )
  {
    std::cerr << "runtime error: " << re.what() << std::endl;
  }
  catch( const std::exception& ex )
  {
    std::cerr << "exception: " << ex.what() << std::endl;
  }
  catch(...)
  {
    std::cerr << "exception exception" << std::endl;
  }
   
  return 0;
}


#include <stdio.h>
#include <iomanip>

void PrintMat( const TMat44 & m )
{
  char str[100];
  char str2[100];
  for ( auto & v : m )
  {
    for ( auto s : v )
    {
      std::cout << std::fixed << std::setprecision(4) << s;
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void TestOutMat( Camera &camera )
{
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluPerspective( camera.fov_y, (float)camera.vp[0] / camera.vp[1], camera.near, camera.far );
  TMat44 prjMatOGL;
  glGetFloatv( GL_PROJECTION_MATRIX, prjMatOGL.data()->data() );

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  gluLookAt( camera.pos[0], camera.pos[1], camera.pos[2], camera.target[0], camera.target[1], camera.target[2], camera.up[0], camera.up[1], camera.up[2] );
  TMat44 viewMatOGL;
  glGetFloatv( GL_MODELVIEW_MATRIX, viewMatOGL.data()->data() );

  TMat44 prjMat;
  glm::mat4 glmPrjMat = glm::perspective( ToRad( camera.fov_y ), (float)camera.vp[0] / (float)camera.vp[1], camera.near, camera.far );
  memcpy( prjMat.data()->data(), &(glmPrjMat[0][0]), 16 * sizeof( float ) );

  TMat44 viewMat;
  glm::mat4 glmViewMat = glm::lookAt( glm::vec3( camera.pos[0], camera.pos[1], camera.pos[2] ), glm::vec3( camera.target[0], camera.target[1], camera.target[2] ), glm::vec3( camera.up[0], camera.up[1], camera.up[2] ) );
  memcpy( viewMat.data()->data(), &(glmViewMat[0][0]), 16 * sizeof( float ) );

  TMat44 cameraPrjMat = camera.Perspective();
  TMat44 cameraViewMat = camera.LookAt();

  for ( int iy = 0; iy < 4; ++ iy )
  {
     for ( int ix = 0; ix < 4; ++ ix )
     {
       float testPrj = cameraPrjMat[ix][iy] - prjMat[ix][iy];
       float testView = cameraViewMat[ix][iy] - viewMat[ix][iy];
       assert( fabs( testPrj ) < 0.001f && fabs( testView ) < 0.001f );
     }
  }

 

  std::cout << std::endl;

  PrintMat( cameraPrjMat );
  PrintMat( prjMat );
  PrintMat( camera.LookAt() );
  PrintMat( viewMat );

   glm::mat4 trans(1.0);
   trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
   glm::vec4 vec(1.0, 0.0, 0.0,1.0);
   vec = trans*vec;
   std::cout << vec.x << "," << vec.y << "," << vec.z << std::endl;

   /*
    glm::mat4 test[4];
  test[0] = ToGLM( camera.Perspective() );
  test[1] = ToGLM( prjMat );
  test[2] = ToGLM( camera.LookAt() );
  test[3] = ToGLM( viewMat );

  GLfloat *ptr0 = glm::value_ptr( test[0] );
  GLfloat *ptr1 = glm::value_ptr( test[1] );
  GLfloat *ptr2 = glm::value_ptr( test[2] );
  GLfloat *ptr3 = glm::value_ptr( test[3] );

  size_t glmMat4Size = sizeof(glm::mat4);
  */
  /*
  col_type value[4];

  union
	{
	    struct { T x, y, z, w;};
			struct { T r, g, b, a; };
			struct { T s, t, p, q; };
  }
  */
}