#include <stdafx.h>

// OpenGL
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// FreeGLUT
#include <GLFW/glfw3.h>

// STL
#include <vector>
#include <stdexcept>
#include <chrono>
#include <memory>
#include <cmath>

#define _USE_MATH_DEFINES
#include <math.h>

// Own
#include <OpenGL_Matrix_Camera.h>
#include <OpenGL_SimpleShaderProgram_temp.h>
#include <OpenGLError.h>
                          

// Controls model tracking
class CModelControl
{
public:

  using TM44         = glm::mat4;
  using TV3          = glm::vec3;
  using TPos         = std::array<int, 2>; 
  using TSize        = std::array<int, 2>;
  using TTime        = std::chrono::high_resolution_clock::time_point;
  using TAttenuation = std::array<float, 3>;

  const TM44 _ident_mat = glm::mat4( 1.0f );

  CModelControl( void ) { Init(); }
  virtual ~CModelControl() = default;

  TM44 OrbitMatrix( void )
  {
    return (_mouse_drag || (_auto_rotate && _auto_spin )) ? _current_orbit_mat * _orbit_mat : _orbit_mat;
  }

  TM44 AutoModelMatrix( void )
  {
    return _auto_rotate ? _current_model_mat * _model_mat : _model_mat;
  }

  CModelControl & VpSize( TSize vp_size ) { _vp_size = vp_size; return *this; }

  //! Set constant, linear and quadratic attenuation
  //! \f$ f(x) \; = \; \frac{ x }{ a[0] \,+\, a[1]*x \,+\, a[2]*x*x } \f$
  //! `attenuation = {1.0, 0.0, 0.0 }` is a linear uniform motion
  //! TODO function plotter "x/dot(vec3(1.0,x,x*x),vec3(1.0,0.2,0.2))"
  CModelControl & Attenuation( const TAttenuation &attenuation ) { _attenuation = attenuation; return *this; }  


  CModelControl & Init( void );
  CModelControl & Update( void );
  CModelControl & UpdatePosition( TPos pos );

  CModelControl & StartRotate( TPos pos )
  {
    _mouse_start = pos;
    return ChangeMotionMode( true, false, false );
  }

  CModelControl & FinishRotate( TPos pos )
  {
    UpdatePosition( pos );
    return ChangeMotionMode( false, true, true );
  }

  CModelControl & ToogleRotate( void )
  {
    return ChangeMotionMode( false, false, !_auto_rotate );
  }

private:

  TM44 RotateMat( float angle_rad, const TV3 &axis ) const 
  {
    return glm::rotate( glm::mat4( 1.0f ), angle_rad, axis );
  }

  TTime TimeNow( void ) const
  { 
    return std::chrono::high_resolution_clock::now();
  }

  double DeltaTime_s( TTime time_start, TTime time_end ) const
  {
    auto   delta_time = time_end - time_start;
    double time_ms    = (double)std::chrono::duration_cast<std::chrono::milliseconds>(delta_time).count() / 1000.0;
    return time_ms;
  }

  CModelControl & ChangeMotionMode( bool drag, bool spin, bool automatically );

  TM44         _orbit_mat{ _ident_mat };             // persistent orbit orientation matrix
  TM44         _current_orbit_mat{ _ident_mat };     // additional orbit orientation while dragging
  bool         _mouse_drag{ false };                 // dragging on or off
  TV3          _mouse_drag_axis{ 1.0f, 0.0f, 0.0f }; // current drag axis
  float        _mouse_drag_angle{ 0.0f };            // current drag distance
  double       _mouse_drag_time_s{ 0 };              // current drag time in seconds
  TPos         _mouse_start{ 0, 0 };                 // start of mouse dragging operation
  bool         _auto_rotate{ true };                 // auto rotate on or of
  bool         _auto_spin{ false };                  // auto spin
  TM44         _model_mat{ _ident_mat };             // persistent model matrix
  TM44         _current_model_mat{ _ident_mat };     // current model matrix
  TTime        _currentTime;                         // current time
  TTime        _rotateStartTime;                     // start time of rotation
  TTime        _dragStartTime;                       // start time of dragging
  TSize        _vp_size;                             // viewport size
  TAttenuation _attenuation{ 1.0f, 0.0, 0.0 };       // attenuation of the automatic spin
};


CModelControl & CModelControl::Init( void )
{
  _currentTime     = TimeNow();  
  _rotateStartTime = _currentTime;
  _dragStartTime   = _currentTime;
  return *this;
}


CModelControl & CModelControl::Update( void )
{
  _currentTime       = TimeNow();
  _current_model_mat = _ident_mat;

  double delta_time_s = DeltaTime_s( _rotateStartTime, _currentTime );

  if ( _mouse_drag ) 
  {
    _current_orbit_mat = RotateMat( _mouse_drag_angle, _mouse_drag_axis );
  }
  else if ( _auto_rotate && _auto_spin )
  {
    if ( _mouse_drag_time_s > 0.0 )
    {
      double angle = _mouse_drag_angle * delta_time_s / _mouse_drag_time_s;
      if ( fabs( _attenuation[0] ) > 0.001 )
        angle /= _attenuation[0] + _attenuation[1] * angle + _attenuation[2] * angle*angle;
      _current_orbit_mat = RotateMat( (float)angle, _mouse_drag_axis );
    }
  }
  else if ( _auto_rotate && _auto_spin == false )
  {
    double dummy;
    double auto_angle_x = std::modf( delta_time_s / 13.0, &dummy ) * 2.0 * M_PI;
    double auto_angle_y = std::modf( delta_time_s / 17.0, &dummy ) * 2.0 * M_PI;
    _current_model_mat = glm::rotate( _current_model_mat, (float)auto_angle_x, glm::vec3( 1.0f, 0.0f, 0.0f ) );
    _current_model_mat = glm::rotate( _current_model_mat, (float)auto_angle_y, glm::vec3( 0.0f, 1.0f, 0.0f ) );
  }

  return *this;
}


CModelControl & CModelControl::UpdatePosition( TPos pos )
{
  if ( _mouse_drag == false )
    return *this;

  double dx = (double)(pos[0] - _mouse_start[0]) / (double)_vp_size[0];
  double dy = (double)(pos[1] - _mouse_start[1]) / (double)_vp_size[1];

  double len = std::sqrt(dx*dx + dy*dy);
  if ( len <= 0.000001 )
    return *this;
  
  _mouse_drag_angle  = (float)(M_PI * len);
  _mouse_drag_axis   = glm::vec3((float)(dy/len), 0.0f, (float)(dx/len));
  _mouse_drag_time_s = DeltaTime_s( _dragStartTime, TimeNow() );

  return *this;
}


CModelControl & CModelControl::ChangeMotionMode( bool drag, bool spin, bool automatically )
{
  bool new_drag = drag;
  bool new_auto = new_drag == false && automatically;
  bool new_spin = new_auto && spin;
  bool change   = _mouse_drag != new_drag || _auto_rotate != new_auto || _auto_spin != new_spin; 
  if ( change == false )
    return *this;

  if ( new_drag && _mouse_drag == false )
  {
    _dragStartTime     = TimeNow();
    _mouse_drag_angle  = 0.0f;
    _mouse_drag_time_s = 0.0;
  }
  if ( new_auto && _auto_rotate == false )
  {
    _rotateStartTime = TimeNow();
  }

  _mouse_drag  = new_drag;
  _auto_rotate = new_auto;
  _auto_spin   = new_spin;

  _orbit_mat         = _current_orbit_mat * _orbit_mat;
  _current_orbit_mat = _ident_mat;
  _model_mat         = _current_model_mat * _model_mat;
  _current_model_mat =_ident_mat;

  return *this;
}


namespace Render
{

namespace GLSL
{

using vec2 = std::array<float, 4>;
using vec3 = std::array<float, 4>;
using vec4 = std::array<float, 4>;
using mat2 = std::array<vec2, 2>;
using mat3 = std::array<vec3, 3>;

struct mat4: std::array<vec4, 4>
{
  using std::array<vec4, 4>::operator =;
  mat4 & operator = (glm::mat4 &source)
  {
    float *data_ptr = glm::value_ptr( source );
    std::copy( data_ptr, data_ptr+16, (float*)(this) );
    return *this;
  }
};

}

}


struct TUB_MVP
{
  Render::GLSL::mat4 _projection;
  Render::GLSL::mat4 _view;
  Render::GLSL::mat4 _model;
};


namespace Rubiks
{

static const int c_no_of_cubes = 27;
using TM44Cubes = std::array<glm::mat4, c_no_of_cubes>;
using TMapCubes = std::array<int, c_no_of_cubes>;


struct T_RUBIKS_DATA
{
  std::array<Render::GLSL::mat4, c_no_of_cubes> _model;
};

enum class TAxis { x, y, z };
enum class TRow { low, mid, high };
enum class TDirection { left, right };

class CCube
{
public:

  CCube( void ) { Init(); }
  virtual ~CCube() = default;

  const T_RUBIKS_DATA * Data( void ) const { return &_data; }

  CCube & Init( void );
  CCube & UpdateM44Cubes( void );
  CCube & Rotate( TAxis axis, TRow row, TDirection dir );

private:

  T_RUBIKS_DATA _data;

  TMapCubes _cube_map;
  TM44Cubes _trans_scale;
  TM44Cubes _current_pos;
  TM44Cubes _animation;
};


CCube & CCube::Init( void )
{
  for ( int z=0; z<3; ++ z )
  {
    for ( int y=0; y<3; ++ y )
    {
      for ( int x=0; x<3; ++ x )
      {
        int i = z * 9 + y * 3 + x;
        _cube_map[i] = i;
        glm::mat4 part_scale = glm::scale( glm::mat4(1.0f), glm::vec3(1.0f/3.0f) );
        float offset = 1.1f * 2.0f / 3.0f;
        glm::vec3 trans_vec( (float)(x-1), (float)(y-1), (float)(z-1) );
        glm::mat4 part_trans = glm::translate( glm::mat4(1.0f), trans_vec * offset );
        _trans_scale[i] = part_trans * part_scale;
      }
    }
  }

  for ( int i=0; i<c_no_of_cubes; ++i )
  {
    _current_pos[i] = glm::mat4( 1.0f );
    _animation[i] = glm::mat4( 1.0f );
  }

  return UpdateM44Cubes();
}


CCube & CCube::UpdateM44Cubes( void )
{
  for ( int i = 0; i < c_no_of_cubes; ++i )
    _data._model[i] = _animation[i] * _current_pos[i] * _trans_scale[i];
  return *this;
}


CCube & CCube::Rotate( TAxis axis, TRow row, TDirection dir )
{
  static const std::unordered_map<TAxis, int> axis_map{ {TAxis::x, 0}, {TAxis::y, 1}, {TAxis::z, 2}};
  static const std::unordered_map<TRow, int> row_map{ {TRow::low, 0}, {TRow::mid, 1}, {TRow::high, 2}};
  std::array<glm::vec3, 3> axis_vec{ glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)};

  int axis_i = axis_map.find(axis)->second;
  int row_i  = row_map.find(row)->second;

  int r_x[2]{ 0,2 };
  int r_y[2]{ 0,2 };
  int r_z[2]{ 0,2 };

  switch ( axis )
  {
    case TAxis::x: r_x[0] = r_x[1] = row_i; break;
    case TAxis::y: r_y[0] = r_y[1] = row_i; break;
    case TAxis::z: r_z[0] = r_z[1] = row_i; break;
  }

  for ( int z = r_z[0]; z <= r_z[1]; ++ z )
  {
    for ( int y = r_y[0]; y <= r_y[1]; ++ y )
    {
      for ( int x = r_x[0]; x <= r_x[1]; ++ x )
      {
        int i = z * 9 + y * 3 + x;
        i = _cube_map[i];
        float angle = glm::radians( 90.0f ) * (dir == TDirection::left ? -1.0f : 1.0f);
        glm::mat4 rot_mat = glm::rotate(glm::mat4(1.0f), angle, axis_vec[axis_i] );
        _current_pos[i] = rot_mat * _current_pos[i];
      }
    }
  }

  static const std::array<std::array<int, 2>, 8> indices
  { 
    std::array<int, 2>{0, 0}, std::array<int, 2>{1, 0}, std::array<int, 2>{2, 0}, std::array<int, 2>{2, 1},
    std::array<int, 2>{2, 2}, std::array<int, 2>{1, 2}, std::array<int, 2>{0, 2}, std::array<int, 2>{0, 1} 
  };
  TMapCubes current_map = _cube_map;
  for ( int i = 0; i < 8; ++ i )
  {
    int j = ( dir == TDirection::left ? i + 6 : i + 2 ) % 8;
    
    std::array<int, 3> ao, an;
    ao[axis_i] = row_i;
    an[axis_i] = row_i;
    ao[(axis_i+1) % 3] = indices[i][0];
    an[(axis_i+1) % 3] = indices[j][0];
    ao[(axis_i+2) % 3] = indices[i][1];
    an[(axis_i+2) % 3] = indices[j][1];

    int io = ao[0] + ao[1] * 3 + ao[2] * 9;
    int in = an[0] + an[1] * 3 + an[2] * 9;

    _cube_map[in] = current_map[io];
  }

  // TODO $$$ index mapping and index rotation

  UpdateM44Cubes();
  return *this;
}


}


// [Switching Between windowed and full screen in OpenGL/GLFW 3.2](https://stackoverflow.com/questions/47402766/switching-between-windowed-and-full-screen-in-opengl-glfw-3-2/47462358#47462358)
class CWindow_Glfw
{
public:

    virtual ~CWindow_Glfw();

    void Init( int width, int height, int multisampling, bool doubleBuffer );
    void InitDebug( void );
    static void CallbackResize(GLFWwindow* window, int cx, int cy);
    static void CallbackMouseButton( GLFWwindow *window, int button, int action, int mode );
    static void CallbackCursorPos( GLFWwindow *window, double x, double y );
    static void CallbackKey( GLFWwindow *window, int key, int scancode, int action, int mods );
    void MainLoop( void );

    void Resize( int cx, int cy );
    void MouseButton( int button, int action, int mode );
    void CursorPos( double x, double y );
    void Key( int key, int scancode, int action, int mods );

    std::chrono::high_resolution_clock::time_point _start_time;
    std::chrono::high_resolution_clock::time_point _current_time;

    std::unique_ptr<CModelControl>         _model_control;
    std::unique_ptr<Render::IDebug>        _debug;
    std::unique_ptr<OpenGL::ShaderProgram> _prog;

    void InitScene( void );
    void Render( double time_ms );

private:

    std::array< int, 2 > _wndPos         {0, 0};
    std::array< int, 2 > _wndSize        {0, 0};
    std::array< int, 2 > _vpSize         {0, 0};
    bool                 _updateViewport = true;
    bool                 _doubleBuffer   = true;
    GLFWwindow *         _wnd            = nullptr;
    GLFWmonitor *        _monitor        = nullptr;

    GLuint                         _ubo_mvp = 0;
    TUB_MVP                        _ubo_mvp_data;
    GLuint                         _ubo_rubiks = 0;
    std::unique_ptr<Rubiks::CCube> _rubiks_cube;
};

int main(int argc, char** argv)
{
    if ( glfwInit() == GLFW_FALSE )
        throw std::runtime_error( "error initializing GLFW" );

    // create OpenGL window and make OpenGL context current (`glfwInit` has to be done before).
    CWindow_Glfw window;
    window.Init( 800, 600, 4, true );

    // OpenGL context needs to be current for `glewInit`
    glewExperimental = true;
    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing GLEW" );
    window.InitDebug();

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

    window.MainLoop();
    return 0;
}


CWindow_Glfw::~CWindow_Glfw()
{
  _debug.reset( nullptr );

  if ( _wnd != nullptr)
    glfwDestroyWindow( _wnd );
  glfwTerminate();
}


void CWindow_Glfw::CallbackResize(GLFWwindow* window, int cx, int cy)
{
    void *ptr = glfwGetWindowUserPointer( window );
    if ( CWindow_Glfw *wndPtr = static_cast<CWindow_Glfw*>( ptr ) )
        wndPtr->Resize( cx, cy );
}


void CWindow_Glfw::CallbackMouseButton( GLFWwindow *window, int button, int action, int mode )
{
    void *ptr = glfwGetWindowUserPointer( window );
    if ( CWindow_Glfw *wndPtr = static_cast<CWindow_Glfw*>( ptr ) )
        wndPtr->MouseButton( button, action, mode );
}


void CWindow_Glfw::CallbackCursorPos( GLFWwindow *window, double x, double y )
{
    void *ptr = glfwGetWindowUserPointer( window );
    if ( CWindow_Glfw *wndPtr = static_cast<CWindow_Glfw*>( ptr ) )
        wndPtr->CursorPos( x, y );
}


void CWindow_Glfw::CallbackKey( GLFWwindow *window, int key, int scancode, int action, int mods )
{
    void *ptr = glfwGetWindowUserPointer( window );
    if ( CWindow_Glfw *wndPtr = static_cast<CWindow_Glfw*>( ptr ) )
        wndPtr->Key( key, scancode, action, mods );
}


void CWindow_Glfw::InitDebug( void ) // has to be done after GLEW initialization!
{
 #if defined(_DEBUG)
    static bool synchromous = true;
    _debug = std::make_unique<OpenGL::CDebug>();
    _debug->Init( Render::TDebugLevel::error_only );
    _debug->Activate( synchromous );
#endif
}


void CWindow_Glfw::Init( int width, int height, int multisampling, bool doubleBuffer )
{
    _doubleBuffer = doubleBuffer;

    // [GLFW Window guide; Window creation hints](http://www.glfw.org/docs/latest/window_guide.html#window_hints_values)

    glfwWindowHint( GLFW_DEPTH_BITS, 24 );
    glfwWindowHint( GLFW_STENCIL_BITS, 8 ); 

    glfwWindowHint( GLFW_SAMPLES, multisampling );
    glfwWindowHint( GLFW_DOUBLEBUFFER, _doubleBuffer ? GLFW_TRUE : GLFW_FALSE );

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
    //glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
#if defined(_DEBUG)
    //glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
#endif

    //GLFW_CONTEXT_ROBUSTNESS 
    
    _wnd = glfwCreateWindow( width, height, "OGL window", nullptr, nullptr );
    if ( _wnd == nullptr )
    {
        glfwTerminate();
        throw std::runtime_error( "error initializing window" ); 
    }

    glfwMakeContextCurrent( _wnd );

    glfwSetWindowUserPointer( _wnd, this );
    glfwSetWindowSizeCallback( _wnd, CWindow_Glfw::CallbackResize );
    glfwSetMouseButtonCallback( _wnd, CWindow_Glfw::CallbackMouseButton );
    glfwSetCursorPosCallback( _wnd, CWindow_Glfw::CallbackCursorPos );
    glfwSetKeyCallback( _wnd, CWindow_Glfw::CallbackKey );

    _monitor =  glfwGetPrimaryMonitor();
    glfwGetWindowSize( _wnd, &_wndSize[0], &_wndSize[1] );
    glfwGetWindowPos( _wnd, &_wndPos[0], &_wndPos[1] );
    _updateViewport = true;
}


void CWindow_Glfw::Resize( int cx, int cy )
{
    _updateViewport = true;
}


void CWindow_Glfw::MouseButton( int button, int action, int mode )
{
  if ( _model_control == nullptr )
    return;

  double x, y;
  glfwGetCursorPos( _wnd, &x, &y );

  switch (button)
  {
    case GLFW_MOUSE_BUTTON_LEFT:
      if ( action == GLFW_PRESS )
        _model_control->StartRotate( { (int)x, (int)y } );
      else if ( action == GLFW_RELEASE )
        _model_control->FinishRotate( { (int)x, (int)y } );
      break;

    case GLFW_MOUSE_BUTTON_RIGHT:
      if ( action == GLFW_RELEASE )
        _model_control->ToogleRotate();
      break;
  }
}


void CWindow_Glfw::CursorPos( double x, double y )
{
  if ( _model_control == nullptr )
    return;
  _model_control->UpdatePosition( { (int)x, (int)y } );
}


void CWindow_Glfw::Key( int key, int scancode, int action, int mods )
{
  if ( _rubiks_cube == nullptr )
    return;

  if ( action != GLFW_PRESS )
    return;

  switch (key)
  {
    case GLFW_KEY_1:
    case GLFW_KEY_KP_1: _rubiks_cube->Rotate( Rubiks::TAxis::x, Rubiks::TRow::low, Rubiks::TDirection::left ); break;
    case GLFW_KEY_2:
    case GLFW_KEY_KP_2: _rubiks_cube->Rotate( Rubiks::TAxis::x, Rubiks::TRow::mid, Rubiks::TDirection::left ); break;
    case GLFW_KEY_3:
    case GLFW_KEY_KP_3: _rubiks_cube->Rotate( Rubiks::TAxis::x, Rubiks::TRow::high, Rubiks::TDirection::left ); break;

    case GLFW_KEY_4:
    case GLFW_KEY_KP_4: _rubiks_cube->Rotate( Rubiks::TAxis::y, Rubiks::TRow::low, Rubiks::TDirection::left ); break;
    case GLFW_KEY_5:
    case GLFW_KEY_KP_5: _rubiks_cube->Rotate( Rubiks::TAxis::y, Rubiks::TRow::mid, Rubiks::TDirection::left ); break;
    case GLFW_KEY_6:
    case GLFW_KEY_KP_6: _rubiks_cube->Rotate( Rubiks::TAxis::y, Rubiks::TRow::high, Rubiks::TDirection::left ); break;

    case GLFW_KEY_7:
    case GLFW_KEY_KP_7: _rubiks_cube->Rotate( Rubiks::TAxis::z, Rubiks::TRow::low, Rubiks::TDirection::left ); break;
    case GLFW_KEY_8:
    case GLFW_KEY_KP_8: _rubiks_cube->Rotate( Rubiks::TAxis::z, Rubiks::TRow::mid, Rubiks::TDirection::left ); break;
    case GLFW_KEY_9:
    case GLFW_KEY_KP_9: _rubiks_cube->Rotate( Rubiks::TAxis::z, Rubiks::TRow::high, Rubiks::TDirection::left ); break;
  }
}


void CWindow_Glfw::MainLoop ( void )
{
    InitScene();
   
    _start_time    = std::chrono::high_resolution_clock::now();
    _model_control = std::make_unique<CModelControl>();
    _rubiks_cube   = std::make_unique<Rubiks::CCube>();

    while (!glfwWindowShouldClose(_wnd))
    {
        if ( _updateViewport )
        {
            glfwGetFramebufferSize( _wnd, &_vpSize[0], &_vpSize[1] );
            glViewport( 0, 0, _vpSize[0], _vpSize[1] );
            _model_control->VpSize( _vpSize );
            _updateViewport = false;
        }

        static std::array<float, 3>attenuation{ 1.0f, 0.05f, 0.0f };
        //static std::array<float, 3>attenuation{ 1.0f, 0.1f, 0.1f };
        //static std::array<float, 3>attenuation{ 1.0f, 0.05f, 0.0001f };
        _model_control->Attenuation( attenuation );

        _current_time     = std::chrono::high_resolution_clock::now();
        auto   delta_time = _current_time - _start_time;
        double time_ms    = (double)std::chrono::duration_cast<std::chrono::milliseconds>(delta_time).count();
        
        _model_control->Update();

        Render( time_ms );

        if ( _doubleBuffer )
          glfwSwapBuffers( _wnd );
        else
          glFinish();
        
        glfwPollEvents();
    }
} 


std::string sh_vert = R"(
#version 460 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;

out vec3 vertPos;
out vec4 vertCol;

layout (std140, binding = 1) uniform UB_MVP
{ 
    mat4 u_projection;
    mat4 u_view;
    mat4 u_model;
};

layout (std140, binding = 2) uniform UB_RUBIKS
{ 
    mat4 u_rubiks_model[27];
};

void main()
{
    mat4 model_view = u_view * u_model * u_rubiks_model[gl_InstanceID];
    vec4 vertx_pos  = model_view * vec4(inPos, 1.0);

    vertCol     = inColor;
		vertPos     = vertx_pos.xyz;
		gl_Position = u_projection * vertx_pos;
}
)";

std::string sh_frag = R"(
#version 460 core


in vec3 vertPos;
in vec4 vertCol;

out vec4 fragColor;

void main()
{
    fragColor  = vertCol;
}
)";


void CWindow_Glfw::InitScene( void )
{
    _prog.reset( new OpenGL::ShaderProgram(
    {
      { sh_vert, GL_VERTEX_SHADER },
      { sh_frag, GL_FRAGMENT_SHADER }
    } ) );

    // to do mesh buffer creator form multi indices mesh 
    // use in-source wave font file?

    static const std::vector<float> varray
    { 
      // front
      -1.0f,  -1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f, 
       1.0f,  -1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,
       1.0f,  -1.0f,  1.0f,    1.0f, 0.0f, 0.0f, 1.0f,
      -1.0f,  -1.0f,  1.0f,    1.0f, 0.0f, 0.0f, 1.0f,

      // back
       1.0f,   1.0f, -1.0f,    0.0f, 1.0f, 0.0f, 1.0f, 
      -1.0f,   1.0f, -1.0f,    0.0f, 1.0f, 0.0f, 1.0f,
      -1.0f,   1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,
       1.0f,   1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,

       // left
      -1.0f,   1.0f, -1.0f,    0.0f, 0.0f, 1.0f, 1.0f, 
      -1.0f,  -1.0f, -1.0f,    0.0f, 0.0f, 1.0f, 1.0f,
      -1.0f,  -1.0f,  1.0f,    0.0f, 0.0f, 1.0f, 1.0f,
      -1.0f,   1.0f,  1.0f,    0.0f, 0.0f, 1.0f, 1.0f,

       // right
       1.0f,  -1.0f, -1.0f,    1.0f, 0.5f, 0.0f, 1.0f, 
       1.0f,   1.0f, -1.0f,    1.0f, 0.5f, 0.0f, 1.0f,
       1.0f,   1.0f,  1.0f,    1.0f, 0.5f, 0.0f, 1.0f,
       1.0f,  -1.0f,  1.0f,    1.0f, 0.5f, 0.0f, 1.0f,

       // bottom
      -1.0f,   1.0f, -1.0f,    1.0f, 1.0f, 0.0f, 1.0f, 
       1.0f,   1.0f, -1.0f,    1.0f, 1.0f, 0.0f, 1.0f,
       1.0f,  -1.0f, -1.0f,    1.0f, 1.0f, 0.0f, 1.0f,
      -1.0f,  -1.0f, -1.0f,    1.0f, 1.0f, 0.0f, 1.0f,

       // top
      -1.0f,  -1.0f,  1.0f,    1.0f, 0.0f, 1.0f, 1.0f, 
       1.0f,  -1.0f,  1.0f,    1.0f, 0.0f, 1.0f, 1.0f,
       1.0f,   1.0f,  1.0f,    1.0f, 0.0f, 1.0f, 1.0f,
      -1.0f,   1.0f,  1.0f,    1.0f, 0.0f, 1.0f, 1.0f
    };

    static const std::vector<unsigned int> indices
    {
       0,  1,  2,  0,  2,  3, // front
       4,  5,  6,  4,  6,  7, // back
       8,  9, 10,  8, 10, 11, // left
      12, 13, 14, 12, 14, 15, // right
      16, 17, 18, 16, 18, 19,  // bottom
      20, 21, 22, 20, 22, 23  // top
    };

    std::array<GLuint, 2> buffers;
    glGenBuffers( 2, buffers.data() );
    GLuint vbo = buffers[0];
    GLuint ibo = buffers[1];

    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(*indices.data()), indices.data(), GL_STATIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );

    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 7*sizeof(*varray.data()), 0 );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 7*sizeof(*varray.data()), (void*)(3*sizeof(*varray.data())) );
    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
 
    glGenBuffers( 1, &_ubo_mvp );
    glBindBuffer( GL_UNIFORM_BUFFER, _ubo_mvp );
    glBufferData( GL_UNIFORM_BUFFER, sizeof( TUB_MVP ), nullptr, GL_STATIC_DRAW );
    // glBufferStorage(GL_UNIFORM_BUFFER, GLsizeiptr size?, const GLvoid * data?, GLbitfield flags?);
    glBindBufferBase( GL_UNIFORM_BUFFER, 1, _ubo_mvp );
    glBindBuffer( GL_UNIFORM_BUFFER, 0 );

    glGenBuffers( 1, &_ubo_rubiks );
    glBindBuffer( GL_UNIFORM_BUFFER, _ubo_rubiks );
    glBufferData( GL_UNIFORM_BUFFER, sizeof( Rubiks::T_RUBIKS_DATA ), nullptr, GL_STATIC_DRAW );
    // glBufferStorage(GL_UNIFORM_BUFFER, GLsizeiptr size?, const GLvoid * data?, GLbitfield flags?);
    glBindBufferBase( GL_UNIFORM_BUFFER, 2, _ubo_rubiks );
    glBindBuffer( GL_UNIFORM_BUFFER, 0 );

    glEnable( GL_DEPTH_TEST );

    glEnable( GL_CULL_FACE );
    glFrontFace( GL_CCW );
    glCullFace( GL_BACK );
}


void CWindow_Glfw::Render( double time_ms )
{
    float     aspect    = (float)_vpSize[0] / (float)_vpSize[1];

    _ubo_mvp_data._projection = glm::perspective( glm::radians( 70.0f ), aspect, 0.01f, 100.0f );
    _ubo_mvp_data._view       = glm::lookAt( glm::vec3(0.0f, -4.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f) );
    _ubo_mvp_data._model      = _model_control != nullptr ? _model_control->OrbitMatrix() * _model_control->AutoModelMatrix() : glm::mat4( 1.0f );
   
    glBindBuffer( GL_UNIFORM_BUFFER, _ubo_mvp );
    glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof( TUB_MVP ), &_ubo_mvp_data );
    glBindBuffer( GL_UNIFORM_BUFFER, 0 );

    if ( _rubiks_cube != nullptr )
    {
      glBindBuffer( GL_UNIFORM_BUFFER, _ubo_rubiks );
      glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof( Rubiks::T_RUBIKS_DATA ), _rubiks_cube->Data() );
      glBindBuffer( GL_UNIFORM_BUFFER, 0 );
    }

    _prog->Use();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
      
    glDrawElementsInstanced( GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr, 27 );
}