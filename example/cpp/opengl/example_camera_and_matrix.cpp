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
layout (location = 1) in vec3 inCol;

out vec3 vertCol;

uniform mat4 u_projectionMat44;
uniform mat4 u_viewMat44;
uniform mat4 u_modelMat44;

void main()
{
    vertCol       = inCol;
    vec4 modolPos = u_modelMat44 * vec4( inPos, 1.0 );
    vec4 viewPos  = u_viewMat44 * modolPos;
    gl_Position   = u_projectionMat44 * viewPos;
		gl_PointSize  = 5.0;
}
)";

std::string sh_frag = R"(
#version 400

in vec3 vertCol;

out vec4 fragColor;

void main()
{
    fragColor = vec4( vertCol, 1.0 );
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
  TVec3 pos    {-7.0, -5.0, 10.};
  TVec3 target {0.0, 0.0, 0.0};
  TVec3 up     {0.0, 0.0, 1.0};
  float fov_y  {85.0};
  TSize vp     {800, 600};
  float near_  {0.5};
  float far_   {100.0};

  TMat44 Perspective( void );
  TMat44 LookAt( void );
};

TMat44 Camera::Perspective( void )
{
  float fn = far_ + near_, f_n = far_ - near_;
  float r = (float)vp[0] / vp[1], t = 1.0f / tan( ToRad( fov_y ) / 2.0f );
  return TMat44{ TVec4{ t / r, 0.0f, 0.0f, 0.0f }, TVec4{ 0.0f, t, 0.0f, 0.0f }, TVec4{ 0.0f, 0.0f, -fn / f_n, -1.0 }, TVec4{ 0.0f, 0.0f, -2.0f*far_*near_ / f_n, 0.0f } };
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
    glEnable( GL_PROGRAM_POINT_SIZE );
    glBindVertexArray( _obj );
    //glDrawArrays( GL_TRIANGLES, 0, (GLsizei)_indices.size() );
    
    //glDrawElements( GL_POINTS, (GLsizei)_indices.size(), GL_UNSIGNED_INT, NULL );
    //glDrawElements( GL_LINE_STRIP, (GLsizei)_indices.size(), GL_UNSIGNED_INT, NULL );
    
    //glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
    
    glDrawElements( GL_TRIANGLES, (GLsizei)_indices.size(), GL_UNSIGNED_INT, NULL );
    //glDrawElements( GL_POINTS, (GLsizei)_indices.size(), GL_UNSIGNED_INT, NULL );
    
    //glDrawElements( GL_TRIANGLES, (GLsizei)_indices.size(), GL_UNSIGNED_INT, _indices.data() );
  }
};

class Window
{
private:

  Camera _camera;
  double               _startTime = 0.0;
  std::array<double, 2> _cursorPos;

  std::array< int, 2 > _wndPos         {0, 0};
  std::array< int, 2 > _wndSize        {0, 0};
  std::array< int, 2 > _vpSize         {0, 0};
  bool                 _updateViewport = true;
  GLFWwindow *         _wnd            = nullptr;
  GLFWmonitor *        _monitor        = nullptr;

public:

  void Init( int width, int height )
  {
    _wnd = glfwCreateWindow( width, height, "OGL window", nullptr, nullptr );
    if ( _wnd == nullptr )
    {
      glfwTerminate();
      throw std::runtime_error( "error initializing window" ); 
    }

    glfwMakeContextCurrent( _wnd );

    glfwSetWindowUserPointer( _wnd, this );
    glfwSetWindowSizeCallback( _wnd, &Window::CallbackResize );

    _monitor = glfwGetPrimaryMonitor();
    glfwGetWindowSize( _wnd, &_wndSize[0], &_wndSize[1] );
    glfwGetWindowPos( _wnd, &_wndPos[0], &_wndPos[1] );
    _updateViewport = true;
  }

  static void CallbackResize(GLFWwindow* window, int cx, int cy)
  {
    void *ptr = glfwGetWindowUserPointer( window );
    if ( Window *wndPtr = static_cast<Window*>( ptr ) )
      wndPtr->Resize( cx, cy );
  }

  void Resize( int cx, int cy )
  {
    _updateViewport = true;
  }

  bool IsFullscreen( void )
  {
    return glfwGetWindowMonitor( _wnd ) != nullptr;
  }

  void SetFullScreen( bool fullscreen )
  {
    if ( IsFullscreen() == fullscreen )
      return;

    if ( fullscreen )
    {
      // backup windwo position and window size
      glfwGetWindowPos( _wnd, &_wndPos[0], &_wndPos[1] );
      glfwGetWindowSize( _wnd, &_wndSize[0], &_wndSize[1] );
      
      // get reolution of monitor
      const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

      // swithc to full screen
      glfwSetWindowMonitor( _wnd, _monitor, 0, 0, mode->width, mode->height, 0 );
    }
    else
    {
      // restor last window
      glfwSetWindowMonitor( _wnd, nullptr,  _wndPos[0], _wndPos[1], _wndSize[0], _wndSize[1], 0 );
    }

    _updateViewport = true;
  }

  void MainLoop ( void )
  {
    while (!glfwWindowShouldClose(_wnd))
    {
      if ( _updateViewport )
      {
        glfwGetFramebufferSize( _wnd, &_vpSize[0], &_vpSize[1] );
        glViewport( 0, 0, _vpSize[0], _vpSize[1] );
        _updateViewport = false;
      }

      Render();

      glfwSwapBuffers(_wnd);
      glfwPollEvents();
    }

    glfwDestroyWindow( _wnd );
    _wnd = nullptr;
    glfwTerminate();
  }


  Window( std::array< int, 2 > size )
    : _vpSize( size )
  {
    if ( glfwInit() == 0 )
      throw std::runtime_error( "error initializing glfw" ); 
    
     //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
     //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
     //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    
    Init( size[0], size[1] );

    glfwSetCursorPosCallback( _wnd, Window::CallbackPos );

    std::string ver = (const char*)glGetString( GL_VERSION );
    std::string ven = (const char*)glGetString( GL_VENDOR );
    std::cout << ver.data() << std::endl;
    std::cout << ven.data() << std::endl;

    glfwGetCursorPos( _wnd, &_cursorPos[0], &_cursorPos[1]);
    glfwSetMouseButtonCallback(_wnd, &Window::mouse_button_callback);

    _startTime = glfwGetTime();
  }

  

  std::array<float, 2> CursorDelta( void )
  {
    std::array<double, 2> newCursorPos;
    glfwGetCursorPos( _wnd, &newCursorPos[0], &newCursorPos[1] );

    std::array<float, 2> delta{ (float)newCursorPos[0] - (float)_cursorPos[0], (float)newCursorPos[1] - (float)_cursorPos[1] };
    _cursorPos = newCursorPos;
    return delta;
  }
  
  virtual ~Window()
  {
    if ( _wnd == nullptr )
      return;
    glfwDestroyWindow( _wnd );
    glfwTerminate();
  }

  static void CallbackPos(GLFWwindow* window, double xpos, double ypos)
  {
    //std::cout << xpos << ", " << ypos << std::endl;
    
  }

  static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
  {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
      void *ptr = glfwGetWindowUserPointer( window );
      if ( Window *wndPtr = static_cast<Window*>( ptr ) )
        wndPtr->SetFullScreen( false );
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
      void *ptr = glfwGetWindowUserPointer( window );
      if ( Window *wndPtr = static_cast<Window*>( ptr ) )
        wndPtr->SetFullScreen( true );
    }
  }


  const OpenGL::ShaderProgramSimple *prog = nullptr;
  const VAObject *vao = nullptr;
  void Set( const OpenGL::ShaderProgramSimple *p, const VAObject *v )
  {
    prog = p;
    vao = v;
  }

    
  glm::mat4 testTrans;
  void Render( void )
  {
    if ( _wnd == nullptr )
      throw std::runtime_error( "error creating glfw window" );

    
    std::array<float, 2> rotateAng{ 0.0f, 0.0f };
   
       _camera.vp = _vpSize;
      Mat44 viewMat = _camera.LookAt();

      /*
      testTrans = glm::rotate(testTrans, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
      for (int ix = 0; ix < 4; ++ ix)
      {
        for (int iy = 0; iy < 4; ++ iy)
          printf( "test %d: %f ", ix, testTrans[ix][iy] );
        printf( "\n" );
      }
      */

      float x = 0.0f;
      float y = 0.0f;
      float z = 0.0f;
      if ( glfwGetKey(_wnd, GLFW_KEY_W) == GLFW_PRESS )
      {
        y = 0.1f;
      }
      if ( glfwGetKey(_wnd, GLFW_KEY_S) == GLFW_PRESS ) {
        y = -0.1f;
      }
      if ( glfwGetKey(_wnd, GLFW_KEY_D) == GLFW_PRESS )
      {
        x = 0.1f;
      }
      if ( glfwGetKey(_wnd, GLFW_KEY_A) == GLFW_PRESS ) {
        x = -0.1f;
      }
      if ( glfwGetKey(_wnd, GLFW_KEY_Q) == GLFW_PRESS )
      {
        z = -0.1f;
      }
      if ( glfwGetKey(_wnd, GLFW_KEY_E) == GLFW_PRESS ) {
        z = 0.1f;
      }

      std::array<float, 2> delta = CursorDelta();
      delta[0] *= 0.2f * 3.1415296f / 180.0f;
      delta[1] *= -0.2f * 3.1415296f / 180.0f ;

      static bool autoCamera = true;
      static bool autoMove = true;

      double currentTime = glfwGetTime();
      glfwGetFramebufferSize( _wnd, &_vpSize[0], &_vpSize[1] );

      
      // set up camera
      //Camera camera;
      //camera.vp = _vpSize;
      Mat44 viewMatTest = _camera.LookAt();
      if ( autoCamera )
      {
        _camera.pos = EllipticalPosition( 15.0f, 12.0f, CalcAng( currentTime, 10.0f ) );
      }

      static bool move_and_rotate = false;
      if ( move_and_rotate )
      {
        //Mat44 viewMat = camera.LookAt();
        Mat44 moveMat = Identity();
        moveMat = moveMat.Translate( { -x, -z, y } );
      
        Mat44 rotMat = Identity();
        rotMat = rotMat.RotateX( delta[1] );
        rotMat = rotMat.RotateY( delta[0] );
      
        viewMat = Multiply( moveMat, viewMat );
        viewMat = Multiply( rotMat, viewMat );
      }

      static bool testPrint = true;
      if ( testPrint )
        TestOutMat( _camera );

      //camera.pos = { viewMat[3][0], viewMat[3][1], viewMat[3][2] };
      //camera.target = { camera.pos[0] - viewMat[1][0], camera.pos[1] - viewMat[1][1], camera.pos[2] - viewMat[1][2] };
      //camera.up = { viewMat[2][0], viewMat[2][1], viewMat[2][2] };
      
      // set up attributes and shader program
      glEnable( GL_DEPTH_TEST );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      prog->Use();
        
      // draw object
      Mat44 modelMat;
      modelMat.Scale( { 3.0f, 3.0f, 3.0f } );
      if ( autoMove )
      {
        modelMat.Translate( { 0.0f, 0.0, CalcMove( currentTime, 6.0f, {-1.0f, 1.0} ) } );
        modelMat.RotateX( CalcAng( currentTime, 13.0f ) );
        modelMat.RotateY( CalcAng( currentTime, 17.0f ) );
      }

      GLint loc_t = glGetUniformLocation( prog->Prog(),"u_mat" );
      GLint loc_t0 = glGetUniformLocation( prog->Prog(),"u_mat[0]" );
      GLint loc_t1 = glGetUniformLocation( prog->Prog(),"u_mat[1]" );
      GLint loc_t2 = glGetUniformLocation( prog->Prog(),"u_mat[2]" );
      
      glm::mat4 shader_mat[3];
      shader_mat[0] = ToGLM( _camera.Perspective() );
      shader_mat[1] = ToGLM( viewMat );
      shader_mat[2] = ToGLM( modelMat );
      GLfloat *ptr0 = glm::value_ptr( shader_mat[0] );
      
      // set all array elements at once
      //glUniformMatrix4fv( loc_t, 3, GL_FALSE, ptr0 );
      
      // set array elements separated
      //glUniformMatrix4fv( loc_t0, 1, GL_FALSE, glm::value_ptr( shader_mat[0] ) );
      //glUniformMatrix4fv( loc_t1, 1, GL_FALSE, glm::value_ptr( shader_mat[1] ) );
      //glUniformMatrix4fv( loc_t2, 1, GL_FALSE, glm::value_ptr( shader_mat[2] ) );

      // set mvp matrices
      prog->SetUniformM44( "u_projectionMat44", _camera.Perspective() );
      prog->SetUniformM44( "u_viewMat44",       viewMat );
      prog->SetUniformM44( "u_modelMat44",      modelMat );
  
      vao->Draw();
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
  try
  {
    // create window
    Window *wnd = new Window( { 800, 600 } );
    
    // initialize glut
    if ( glewInit() != GLEW_OK )
      throw std::runtime_error( "error initializing glew" );

    // define tetrahedron vertex array opject
    const float sin120 = 0.8660254f;
    VAObject * vaObj = new VAObject(
    { { 3, { 0.0f, 0.0f, 1.0f, 0.0f, -sin120, -0.5f, sin120 * sin120, 0.5f * sin120, -0.5f, -sin120 * sin120, 0.5f * sin120, -0.5f } },
      { 3, { 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f } }
    },
    { 0, 1, 2, 0, 2, 3, 0, 3, 1, 1, 3, 2 } );

    
    // load, compile and link shader
    OpenGL::ShaderProgramSimple * prog = new OpenGL::ShaderProgramSimple(
    { { sh_vert, GL_VERTEX_SHADER },
      { sh_frag, GL_FRAGMENT_SHADER }
    } );
    
    // start main loop
    wnd->Set( prog, vaObj );
    wnd->MainLoop();

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

void print_vec(glm::vec4 pnt) {
    std::cout << "(" << pnt.x << ", " << pnt.y << ", " << pnt.z <<  ", " << pnt.w << ")" << "\n";
}


    class Vertex   
    {   
    public:    
        Vertex(float xx, float yy, float zz) : x(xx), y(yy), z(zz) { }    
        virtual ~Vertex() {}    

        static void* StartOffset(void)
        {
            Vertex *dummyPtr = nullptr;
            return &dummyPtr->x;
        }
    
        float x;   
        float y;   
        float z;    
    };



void TestOutMat( Camera &camera )
{
  void * startOffs = Vertex::StartOffset();

  glm::mat4 m(
    0.707f, 0.707f, 0.0f, 0.0f,
    -0.707f, 0.707f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    4.0f, 5.0f, 6.0f, 1.0f );

  glm::vec3 v( 1.0f, 2.0f, 3.0f );

  glm::mat4 result1 = glm::translate( m, v );

  glm::mat4 t(
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    v.x, v.y, v.z, 1.0f );

  glm::mat4 result2 = m * t;

  glm::vec4 v1 = t * glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
  
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluPerspective( camera.fov_y, (float)camera.vp[0] / camera.vp[1], camera.near_, camera.far_ );
  TMat44 prjMatOGL;
  glGetFloatv( GL_PROJECTION_MATRIX, prjMatOGL.data()->data() );

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  gluLookAt( camera.pos[0], camera.pos[1], camera.pos[2], camera.target[0], camera.target[1], camera.target[2], camera.up[0], camera.up[1], camera.up[2] );
  TMat44 viewMatOGL;
  glGetFloatv( GL_MODELVIEW_MATRIX, viewMatOGL.data()->data() );

  TMat44 prjMat;
  glm::mat4 glmPrjMat = glm::perspective( ToRad( camera.fov_y ), (float)camera.vp[0] / (float)camera.vp[1], camera.near_, camera.far_ );
  memcpy( prjMat.data()->data(), &(glmPrjMat[0][0]), 16 * sizeof( float ) );

  TMat44 invPrjMat;
  glm::mat4 glmInvPrjMat = glm::inverse( glmPrjMat );
  memcpy( invPrjMat.data()->data(), &(glmInvPrjMat[0][0]), 16 * sizeof( float ) );

  glm::vec4 ndcNear( 0.0f, 0.0f, -1.0f, 1.0f );
  glm::vec4 ndcFar( 0.0f, 0.0f, 1.0f, 1.0f );

  glm::vec4 ptNear = glmInvPrjMat * ndcNear;
  glm::vec4 ptFar  = glmInvPrjMat * ndcFar;

  float near_T0 = - ptNear[2] / ptNear[3];
  float far_T0 = - ptFar[2] / ptFar[3];

  TMat44 tpPrjMat;
  glm::mat4 glmTpPrjMat = glm::transpose( glmPrjMat );
  memcpy( tpPrjMat.data()->data(), &(glmTpPrjMat[0][0]), 16 * sizeof( float ) );

  glm::vec4 ptNear_T1 = glmTpPrjMat * ndcNear;
  glm::vec4 ptFar_T1  = glmTpPrjMat * ndcFar;

  float near_T1 = - ptNear_T1[2] / ptNear_T1[3];
  float far_T1 = - ptFar_T1[2] / ptFar_T1[3];

  glm::vec4 ptNear_T2 = ndcNear * glmPrjMat;
  glm::vec4 ptFar_T2  = ndcFar * glmPrjMat;

  float near_T2 = - ptNear_T2[2] / ptNear_T2[3];
  float far_T2 = - ptFar_T2[2] / ptFar_T2[3];

  float near_T3 = glmPrjMat[3][2] / (glmPrjMat[2][2] - 1.0f);
  float far_T3 = glmPrjMat[3][2] / (glmPrjMat[2][2] + 1.0f);

  TMat44 viewMat;
  glm::mat4 glmViewMat = glm::lookAt( glm::vec3( camera.pos[0], camera.pos[1], camera.pos[2] ), glm::vec3( camera.target[0], camera.target[1], camera.target[2] ), glm::vec3( camera.up[0], camera.up[1], camera.up[2] ) );
  memcpy( viewMat.data()->data(), glm::value_ptr( glmViewMat ), 16 * sizeof( float ) );

  glm::mat4 glmPModelViewMat = glmPrjMat * glmViewMat;

  TMat44 cameraPrjMat = camera.Perspective();
  TMat44 cameraViewMat = camera.LookAt();


  glm::vec4 tev41( 1.0, 1.0, 1.0, 0.0 );
  glm::vec4 tev42( 1.0, 1.0, 1.0, 0.0 );

  glm::vec4 rtvec41 = glmViewMat * tev41;
  glm::vec4 rtvec40 = glmViewMat * glm::vec4( 0.0, 0.0, 0.0, 0.0 );
  glm::vec4 rtvecdir = rtvec41 - rtvec40;
  
  glm::vec4 rtvec42 = glmViewMat * tev42;

  glm::mat4 transMat(
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    4.0f, 5.0f, 6.0f, 1.0f );

  float ang = 1.0f;
  glm::mat4 rotMat(
    cos(ang), 0.0f, sin(ang), 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    -sin(ang), 0.0f, cos(ang), 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f );

   glm::mat4 modelMat = transMat * rotMat;

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



   //PrintMat( viewMat );

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

   glm::mat4x4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 0.0f),   // eye at origin
            glm::vec3(0.0f, 0.0f, 1.0f),   // looking towards +Z
            glm::vec3(0.0f, 1.0f, 0.0f));  // up is +Y

    glm::mat4x4 projection = glm::perspective(
            glm::radians(90.0f),  // field of view
            1.0f,            // aspect ratio
            1.0f,            // near clipping plane
            2.0f);           // far clipping plane

    // transformation matrix for world coordinates
    glm::mat4x4 vp = projection * view; 

    // points (0,0,1), (0,0,1.5) and (0,0,2) in homogeneous coordinates
    print_vec(vp * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
    print_vec(vp * glm::vec4(0.0f, 0.0f, 1.5f, 1.0f));
    print_vec(vp * glm::vec4(0.0f, 0.0f, 2.0f, 1.0f));
}