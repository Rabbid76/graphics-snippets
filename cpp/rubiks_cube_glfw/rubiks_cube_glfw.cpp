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
#include <limits>
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
#include <Render_GLSL.h>
#include <Render_GLM.h>
#include <ModelControl.h>                          
#include <RubiksControl.h>



struct TUB_MVP
{
  Render::GLSL::mat4 _projection;
  Render::GLSL::mat4 _view;
  Render::GLSL::mat4 _model;
};


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
    void UpdateRenderData( void );

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
    case GLFW_KEY_1:    _rubiks_cube->Change( { Rubiks::TAxis::x, Rubiks::TRow::low, Rubiks::TDirection::left } ); break;
    case GLFW_KEY_KP_1: _rubiks_cube->Change( { Rubiks::TAxis::x, Rubiks::TRow::low, Rubiks::TDirection::right } ); break;
    case GLFW_KEY_2:    _rubiks_cube->Change( { Rubiks::TAxis::x, Rubiks::TRow::mid, Rubiks::TDirection::left } ); break;
    case GLFW_KEY_KP_2: _rubiks_cube->Change( { Rubiks::TAxis::x, Rubiks::TRow::mid, Rubiks::TDirection::right } ); break;
    case GLFW_KEY_3:    _rubiks_cube->Change( { Rubiks::TAxis::x, Rubiks::TRow::high, Rubiks::TDirection::left } ); break;
    case GLFW_KEY_KP_3: _rubiks_cube->Change( { Rubiks::TAxis::x, Rubiks::TRow::high, Rubiks::TDirection::right } ); break;

    case GLFW_KEY_4:    _rubiks_cube->Change( { Rubiks::TAxis::y, Rubiks::TRow::low, Rubiks::TDirection::left } ); break;
    case GLFW_KEY_KP_4: _rubiks_cube->Change( { Rubiks::TAxis::y, Rubiks::TRow::low, Rubiks::TDirection::right } ); break;
    case GLFW_KEY_5:    _rubiks_cube->Change( { Rubiks::TAxis::y, Rubiks::TRow::mid, Rubiks::TDirection::left } ); break;
    case GLFW_KEY_KP_5: _rubiks_cube->Change( { Rubiks::TAxis::y, Rubiks::TRow::mid, Rubiks::TDirection::right } ); break;
    case GLFW_KEY_6:    _rubiks_cube->Change( { Rubiks::TAxis::y, Rubiks::TRow::high, Rubiks::TDirection::left } ); break;
    case GLFW_KEY_KP_6: _rubiks_cube->Change( { Rubiks::TAxis::y, Rubiks::TRow::high, Rubiks::TDirection::right } ); break;

    case GLFW_KEY_7:    _rubiks_cube->Change( { Rubiks::TAxis::z, Rubiks::TRow::low, Rubiks::TDirection::left } ); break;
    case GLFW_KEY_KP_7: _rubiks_cube->Change( { Rubiks::TAxis::z, Rubiks::TRow::low, Rubiks::TDirection::right } ); break;
    case GLFW_KEY_8:    _rubiks_cube->Change( { Rubiks::TAxis::z, Rubiks::TRow::mid, Rubiks::TDirection::left } ); break;
    case GLFW_KEY_KP_8: _rubiks_cube->Change( { Rubiks::TAxis::z, Rubiks::TRow::mid, Rubiks::TDirection::right } ); break;
    case GLFW_KEY_9:    _rubiks_cube->Change( { Rubiks::TAxis::z, Rubiks::TRow::high, Rubiks::TDirection::left } ); break;
    case GLFW_KEY_KP_9: _rubiks_cube->Change( { Rubiks::TAxis::z, Rubiks::TRow::high, Rubiks::TDirection::right } ); break;
  }
}


void CWindow_Glfw::MainLoop ( void )
{
    InitScene();
   
    _start_time    = std::chrono::high_resolution_clock::now();
    _model_control = std::make_unique<CModelControl>();

    static float offset = 2.0f * 1.1f;
    static float scale = 1.0f / 3.0f;
    _rubiks_cube = std::make_unique<Rubiks::CCube>( offset, scale );

    while (!glfwWindowShouldClose(_wnd))
    {
        if ( _updateViewport )
        {
            glfwGetFramebufferSize( _wnd, &_vpSize[0], &_vpSize[1] );
            glViewport( 0, 0, _vpSize[0], _vpSize[1] );
            _model_control->VpSize( _vpSize );
            _updateViewport = false;
        }

        if ( _model_control != nullptr )
        {
          static std::array<float, 3>attenuation{ 1.0f, 0.05f, 0.0f };
          //static std::array<float, 3>attenuation{ 1.0f, 0.1f, 0.1f };
          //static std::array<float, 3>attenuation{ 1.0f, 0.05f, 0.0001f };
          _model_control->Attenuation( attenuation );
        }

        if ( _rubiks_cube != nullptr )
        {
          static double time_s = 1.0;
          _rubiks_cube->AnimationTime( time_s );
        }

        _current_time     = std::chrono::high_resolution_clock::now();
        auto   delta_time = _current_time - _start_time;
        double time_ms    = (double)std::chrono::duration_cast<std::chrono::milliseconds>(delta_time).count();
        
        if ( _model_control != nullptr )
          _model_control->Update();
        if ( _rubiks_cube != nullptr )
          _rubiks_cube->Update();

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
layout (location = 1) in vec4 inTex;

out vec3  vertPos;
out vec4  vertTex;
out float highlight;

layout (std140, binding = 1) uniform UB_MVP
{ 
    mat4 u_projection;
    mat4 u_view;
    mat4 u_model;
};

layout (std140, binding = 2) uniform UB_RUBIKS
{ 
    mat4 u_rubiks_model[27];
    int  u_cube_hit;
    int  u_side_hit;
};

void main()
{
    vec4 tex     = inTex;
    int  cube_i  = gl_InstanceID;
    int  color_i = int(tex.z + 0.5); 
    int  x_i     = cube_i % 3;
    int  y_i     = (cube_i % 9) / 3;
    int  z_i     = cube_i / 9;

    if ( color_i == 1 )
        tex.z = x_i == 0 ? tex.z : 0.0;
    else if ( color_i == 2 )
        tex.z = x_i == 2 ? tex.z : 0.0;
    else if ( color_i == 3 )
        tex.z = y_i == 0 ? tex.z : 0.0;
    else if ( color_i == 4 )
        tex.z = y_i == 2 ? tex.z : 0.0;
    else if ( color_i == 5 )
        tex.z = z_i == 0 ? tex.z : 0.0;
    else if ( color_i == 6 )
        tex.z = z_i == 2 ? tex.z : 0.0;

    mat4 model_view = u_view * u_model * u_rubiks_model[cube_i];
    vec4 vertx_pos  = model_view * vec4(inPos, 1.0);

    vertPos     = vertx_pos.xyz;
    vertTex     = tex;
    //highlight   = tex.z > 0.5 && cube_i == u_cube_hit ? 1.0 : 0.0;	
    //highlight   = tex.z > 0.5 && color_i == u_side_hit ? 1.0 : 0.0;
    highlight   = tex.z > 0.5 && cube_i == u_cube_hit && color_i == u_side_hit ? 1.0 : 0.0;		

		gl_Position = u_projection * vertx_pos;
}
)";

std::string sh_frag = R"(
#version 460 core

in vec3  vertPos;
in vec4  vertTex;
in float highlight;

out vec4 fragColor;

vec4 color_table[7] = vec4[7](
    vec4(0.5, 0.5, 0.5, 1.0),
    vec4(1.0, 0.0, 0.0, 1.0),
    vec4(0.0, 1.0, 0.0, 1.0),
    vec4(0.0, 0.0, 1.0, 1.0),
    vec4(1.0, 0.5, 0.0, 1.0),
    vec4(1.0, 1.0, 0.0, 1.0),
    vec4(1.0, 0.0, 1.0, 1.0)
);

void main()
{
    int color_i = int(vertTex.z + 0.5);

    vec4 color = color_table[color_i]; 
    color.rgb *= max(0.5, highlight);

    fragColor  = color;
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
      // left
      -1.0f,   1.0f, -1.0f,    0.0f, 0.0f, 1.0f, 0.0f, 
      -1.0f,  -1.0f, -1.0f,    0.0f, 1.0f, 1.0f, 0.0f,
      -1.0f,  -1.0f,  1.0f,    1.0f, 1.0f, 1.0f, 0.0f,
      -1.0f,   1.0f,  1.0f,    1.0f, 0.0f, 1.0f, 0.0f,

       // right
       1.0f,  -1.0f, -1.0f,    0.0f, 0.0f, 2.0f, 0.0f, 
       1.0f,   1.0f, -1.0f,    0.0f, 1.0f, 2.0f, 0.0f,
       1.0f,   1.0f,  1.0f,    1.0f, 1.0f, 2.0f, 0.0f,
       1.0f,  -1.0f,  1.0f,    1.0f, 0.0f, 2.0f, 0.0f,

       // front
      -1.0f,  -1.0f, -1.0f,    0.0f, 0.0f, 3.0f, 0.0f, 
       1.0f,  -1.0f, -1.0f,    0.0f, 1.0f, 3.0f, 0.0f,
       1.0f,  -1.0f,  1.0f,    1.0f, 1.0f, 3.0f, 0.0f,
      -1.0f,  -1.0f,  1.0f,    1.0f, 0.0f, 3.0f, 0.0f,

      // back
       1.0f,   1.0f, -1.0f,    0.0f, 0.0f, 4.0f, 0.0f, 
      -1.0f,   1.0f, -1.0f,    0.0f, 1.0f, 4.0f, 0.0f,
      -1.0f,   1.0f,  1.0f,    1.0f, 1.0f, 4.0f, 0.0f,
       1.0f,   1.0f,  1.0f,    1.0f, 0.0f, 4.0f, 0.0f,

       // bottom
      -1.0f,   1.0f, -1.0f,    0.0f, 0.0f, 5.0f, 0.0f, 
       1.0f,   1.0f, -1.0f,    0.0f, 1.0f, 5.0f, 0.0f,
       1.0f,  -1.0f, -1.0f,    1.0f, 1.0f, 5.0f, 0.0f,
      -1.0f,  -1.0f, -1.0f,    1.0f, 0.0f, 5.0f, 0.0f,

       // top
      -1.0f,  -1.0f,  1.0f,    0.0f, 0.0f, 6.0f, 0.0f, 
       1.0f,  -1.0f,  1.0f,    0.0f, 1.0f, 6.0f, 0.0f,
       1.0f,   1.0f,  1.0f,    1.0f, 1.0f, 6.0f, 0.0f,
      -1.0f,   1.0f,  1.0f,    1.0f, 0.0f, 6.0f, 0.0f
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
    UpdateRenderData();
   
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


void CWindow_Glfw::UpdateRenderData( void )
{
    float aspect = (float)_vpSize[0] / (float)_vpSize[1];

    glm::mat4 projection = glm::perspective( glm::radians( 70.0f ), aspect, 0.01f, 100.0f );
    glm::mat4 view       = glm::lookAt( glm::vec3(0.0f, -4.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f) );
    glm::mat4 model      = _model_control != nullptr ? _model_control->OrbitMatrix() * _model_control->AutoModelMatrix() : glm::mat4( 1.0f );

    Render::GLM::CMat4(_ubo_mvp_data._projection) = projection;
    Render::GLM::CMat4(_ubo_mvp_data._view)       = view;
    Render::GLM::CMat4(_ubo_mvp_data._model)      = model;

    glm::mat4 modelview          = view * model;
    glm::mat4 inverse_modelview  = glm::inverse( modelview );
    glm::mat4 inverse_projection = glm::inverse( projection );

    if ( _rubiks_cube == nullptr )
      return;
    float cube_offset = _rubiks_cube->Offset();
    float cube_scale  = _rubiks_cube->Scale();
    
    // intersect ray with the side of the cube
    //
    // Is it possible get which surface of cube will be click in OpenGL?
    // [https://stackoverflow.com/questions/45893277/is-it-possble-get-which-surface-of-cube-will-be-click-in-opengl/45946943#45946943]
    //
    // How to recover view space position given view space depth value and ndc xy
    // [https://stackoverflow.com/questions/11277501/how-to-recover-view-space-position-given-view-space-depth-value-and-ndc-xy/46118945#46118945]


    // calculate the NDC position of the cursor on the far plane and the camera position

    double w = (double)_vpSize[0];
    double h = (double)_vpSize[1];
    double x, y;
    glfwGetCursorPos( _wnd, &x, &y );
    double ndc_x = 2.0 * x/w - 1.0;
    double ndc_y = 1.0 - 2.0 * y/h;
    glm::vec4 ndc_cursor_far( ndc_x, ndc_y, 1.0, 1.0 ); // z = 1.0 -> far plane

    int isect_i_side = -1;
    int isect_i_cube = -1;
    if ( fabs( ndc_x ) < 1.0f && fabs( ndc_y ) )
    {
        // TODO $$$ add to "RubikxCOntrol.h"

        // calculate a ray from the eye position along the line of sight through the cursor position

        glm::vec4 view_cursor = inverse_projection * ndc_cursor_far;

        glm::vec4 view_r0 = glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
        glm::vec4 view_r1 = glm::vec4( glm::vec3( view_cursor ) / view_cursor.w, 1.0f );

        glm::vec4 model_r0 = inverse_modelview * view_r0;
        glm::vec4 model_r1 = inverse_modelview * view_r1; 

        glm::vec3 r0_ray = glm::vec3( model_r0 );
        glm::vec3 d_ray  = glm::normalize( glm::vec3( model_r1 ) - r0_ray );


        // define the cube corner points and its faces

        static const std::array<glm::vec3, 8> cube_pts 
        {
          glm::vec3(-1.0f, -1.0f, -1.0f), // 0 : left  front bottom
          glm::vec3( 1.0f, -1.0f, -1.0f), // 1 : right front bottom
          glm::vec3(-1.0f,  1.0f, -1.0f), // 2 : left  back  bottom
          glm::vec3( 1.0f,  1.0f, -1.0f), // 3 : right back  bottom
          glm::vec3(-1.0f, -1.0f,  1.0f), // 4 : left  front top
          glm::vec3( 1.0f, -1.0f,  1.0f), // 5 : right front top
          glm::vec3(-1.0f,  1.0f,  1.0f), // 6 : left  back  top
          glm::vec3( 1.0f,  1.0f,  1.0f), // 7 : right back  top
        };
        static const std::array<std::array<int, 4>, 6> cube_faces
        {
          std::array<int, 4>{ 2, 0, 4, 6 }, // 0 : left
          std::array<int, 4>{ 1, 3, 7, 5 }, // 1 : right
          std::array<int, 4>{ 0, 1, 5, 4 }, // 2 : front
          std::array<int, 4>{ 2, 3, 7, 6 }, // 3 : back
          std::array<int, 4>{ 0, 1, 3, 2 }, // 4 : bottom
          std::array<int, 4>{ 4, 5, 7, 6 }  // 5 : top
        };

        // find the nearest intersection of a side of the cube and the ray 

        float cube_sidelen_2 = (cube_offset + 1.0f) * cube_scale; // half side length of the entire cube

        int       isect_side = -1;
        float     isect_dist = std::numeric_limits<float>::max();
        glm::vec3 isect_pt;
        for ( int i = 0; i < 6; ++ i )
        {
          // calculate the normal vector of the cube side
          const glm::vec3 &pa = cube_pts[cube_faces[i][0]];
          const glm::vec3 &pb = cube_pts[cube_faces[i][1]];
          const glm::vec3 &pc = cube_pts[cube_faces[i][3]];
          glm::vec3 n_plane  = glm::normalize( glm::cross( pb - pa, pc - pa ) );
          glm::vec3 p0_plane = pa * cube_sidelen_2;

          // calculate the distance to the intersection with the plane defined by the side of the cube
          float dist = glm::dot( pa - r0_ray, n_plane ) / glm::dot( d_ray, n_plane );
          if ( fabs(dist) > isect_dist )
            continue;

          // calculate the intersection point with the plane
          glm::vec3 x_pt = r0_ray + d_ray * dist;

          // check if the intersection is on the side of the cube
          bool on_side = fabs( x_pt.x ) < cube_sidelen_2 + 0.001 &&
                         fabs( x_pt.y ) < cube_sidelen_2 + 0.001 &&
                         fabs( x_pt.z ) < cube_sidelen_2 + 0.001;
          if ( on_side == false )
            continue;

          isect_side = i;
          isect_dist = fabs(dist);
          isect_pt   = x_pt;
        }

        // get intersected sub cube
        if ( isect_side >= 0 )
        {
          std::array<float, 3 > coords{ isect_pt.x, isect_pt.y, isect_pt.z };
          std::vector<int> i_coord;
          bool hit_is_on = true;
          for ( float coord : coords )
          {
            int i = -1;
            if ( fabs( coord ) <= 1.0f * cube_scale )
              i = 1;
            else if ( coord <= -(cube_offset - 1.0f) * cube_scale)
              i = 0;
             else if ( coord >= (cube_offset - 1.0f) * cube_scale)
              i = 2;
            i_coord.push_back( i );
            hit_is_on = hit_is_on && i >= 0;
          }
          if ( hit_is_on )
          {
            int i = 9 * i_coord[2] + 3 * i_coord[1] + i_coord[0];
            isect_i_cube = _rubiks_cube->CubeIndex(i);
          }
        }

        // get the side on the intersected sub cube
        if ( isect_side >= 0 && isect_i_cube >= 0 )
        {
          const glm::mat4 &cube_mat4 = _rubiks_cube->CubePosM44()[isect_i_cube];
          int axis_i = isect_side / 2;
          float sign = isect_side % 2 ? 1.0f : -1.0f;
          glm::vec3 test_vec = glm::vec3( cube_mat4[0][axis_i], cube_mat4[1][axis_i], cube_mat4[2][axis_i] ) * sign;

          if ( test_vec.x < -0.5f )
            isect_i_side = 0;
          else if ( test_vec.x > 0.5f )
            isect_i_side = 1;
          else if ( test_vec.y < -0.5f )
            isect_i_side = 2;
          else if ( test_vec.y > 0.5f )
            isect_i_side = 3;
          else if ( test_vec.z < -0.5f )
            isect_i_side = 4;
          else if ( test_vec.z > 0.5f )
            isect_i_side = 5;
        }
    }
    
    // set the hit data
    _rubiks_cube->Data()->_cube_hit = isect_i_cube;
    _rubiks_cube->Data()->_side_hit = isect_i_side + 1;
}