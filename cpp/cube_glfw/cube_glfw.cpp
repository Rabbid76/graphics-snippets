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

  using TM44  = glm::mat4;
  using TV3   = glm::vec3;
  using TPos  = std::array<int, 2>; 
  using TSize = std::array<int, 2>;
  using TTime = std::chrono::high_resolution_clock::time_point;

  const TM44 _ident_mat = glm::mat4( 1.0f );

  TM44 OrbitMatrix( void )
  {
    return (_mouse_drag || (_auto_rotate && _auto_spin )) ? _current_orbit_mat * _orbit_mat : _orbit_mat;
  }

  TM44 AutoModelMatrix( void )
  {
    return _auto_rotate ? _current_model_mat * _model_mat : _model_mat;
  }

  CModelControl & VpSize( TSize vp_size ) { _vp_size = vp_size; return *this; }

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

  TM44   _orbit_mat{ _ident_mat };             // persistent orbit orientation matrix
  TM44   _current_orbit_mat{ _ident_mat };     // additional orbit orientation while dragging
  bool   _mouse_drag{ false };                 // dragging on or off
  TV3    _mouse_drag_axis{ 1.0f, 0.0f, 0.0f }; // current drag axis
  float  _mouse_drag_angle{ 0.0f };            // current drag distance
  double _mouse_drag_time_s{ 0 };              // current drag time in seconds
  TPos   _mouse_start{ 0, 0 };                 // start of mouse dragging operation
  bool   _auto_rotate{ true };                 // auto rotate on or of
  bool   _auto_spin{ false };                  // auto spin
  TM44   _model_mat{ _ident_mat };             // persistent model matrix
  TM44   _current_model_mat{ _ident_mat };     // current model matrix
  TTime  _currentTime;                         // current time
  TTime  _rotateStartTime;                     // start time of rotation
  TTime  _dragStartTime;                       // start time of dragging
  TSize  _vp_size;                             // viewport size
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


// [Switching Between windowed and full screen in OpenGL/GLFW 3.2](https://stackoverflow.com/questions/47402766/switching-between-windowed-and-full-screen-in-opengl-glfw-3-2/47462358#47462358)
class CWindow_Glfw
{
private:

    std::array< int, 2 > _wndPos         {0, 0};
    std::array< int, 2 > _wndSize        {0, 0};
    std::array< int, 2 > _vpSize         {0, 0};
    bool                 _updateViewport = true;
    bool                 _doubleBuffer   = true;
    GLFWwindow *         _wnd            = nullptr;
    GLFWmonitor *        _monitor        = nullptr;

    void Resize( int cx, int cy );
    void MouseButton( int button, int action, int mode );
    void CursorPos( double x, double y );

    std::chrono::high_resolution_clock::time_point _start_time;
    std::chrono::high_resolution_clock::time_point _current_time;

    std::unique_ptr<CModelControl>         _model_control;
    std::unique_ptr<Render::IDebug>        _debug;
    std::unique_ptr<OpenGL::ShaderProgram> _prog;

    void InitScene( void );
    void Render( double time_ms );

public:

    virtual ~CWindow_Glfw();

    void Init( int width, int height, int multisampling, bool doubleBuffer );
    void InitDebug( void );
    static void CallbackResize(GLFWwindow* window, int cx, int cy);
    static void CallbackMouseButton( GLFWwindow *window, int button, int action, int mode );
    static void CallbackCursorPos( GLFWwindow *window, double x, double y );
    void MainLoop( void );
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
  _model_control.reset( nullptr );
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


void CWindow_Glfw::MainLoop ( void )
{
    InitScene();

    _start_time = std::chrono::high_resolution_clock::now();
    _model_control = std::make_unique<CModelControl>();
    _model_control->Init();

    while (!glfwWindowShouldClose(_wnd))
    {
        if ( _updateViewport )
        {
            glfwGetFramebufferSize( _wnd, &_vpSize[0], &_vpSize[1] );
            glViewport( 0, 0, _vpSize[0], _vpSize[1] );
            _model_control->VpSize( _vpSize );
            _updateViewport = false;
        }

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
#version 400 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;

out vec3 vertPos;
out vec4 vertCol;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

void main()
{
    mat4 model_view = u_view * u_model;
    vec4 vertx_pos  = model_view * vec4(inPos, 1.0);

    vertCol     = inColor;
		vertPos     = vertx_pos.xyz;
		gl_Position = u_projection * vertx_pos;
}
)";

std::string sh_frag = R"(
#version 400 core

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

    glEnable( GL_DEPTH_TEST );

    glEnable( GL_CULL_FACE );
    glFrontFace( GL_CCW );
    glCullFace( GL_BACK );
}


void CWindow_Glfw::Render( double time_ms )
{
    float     aspect    = (float)_vpSize[0] / (float)_vpSize[1];
    glm::mat4 projecion = glm::perspective( glm::radians( 70.0f ), aspect, 0.01f, 100.0f );
    glm::mat4 view      = glm::lookAt( glm::vec3(0.0f, -4.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f) );

    glm::mat4 model( 1.0f ); 
    if ( _model_control )
      model = _model_control->OrbitMatrix() * _model_control->AutoModelMatrix();

    _prog->Use();
    _prog->SetUniformM44( "u_projection", glm::value_ptr(projecion) );
    _prog->SetUniformM44( "u_view",       glm::value_ptr(view) );
    _prog->SetUniformM44( "u_model",       glm::value_ptr(model) );

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
      
    glDrawElements( GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr );
}