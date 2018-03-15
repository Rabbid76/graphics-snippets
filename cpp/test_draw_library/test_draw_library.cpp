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
#include <GLFW/glfw3.h>

// stl
#include <vector>
#include <stdexcept>
#include <chrono>
#include <memory>
#include <cmath>

#define _USE_MATH_DEFINES
#include <math.h>

// Own
#include <OpenGL_Matrix_Camera.h>
#include <OpenGL_SimpleShaderProgram.h>
#include <OpenGLBasicDraw.h>


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

    std::chrono::high_resolution_clock::time_point _start_time;
    std::chrono::high_resolution_clock::time_point _current_time;

    std::unique_ptr<OpenGL::ShaderProgram> _prog;
    std::unique_ptr<Render::IDraw>         _draw;

    void InitScene( void );
    void Render( double time_ms );

public:

    virtual ~CWindow_Glfw();

    void Init( int width, int height, int multisampling, bool doubleBuffer );
    static void CallbackResize(GLFWwindow* window, int cx, int cy);
    void MainLoop( void );
};

int main(int argc, char** argv)
{
    if ( glfwInit() == GLFW_FALSE )
        throw std::runtime_error( "error initializing glfw" );

    // create OpenGL window and make OpenGL context current (`glfwInit` has to be done before).
    CWindow_Glfw window;
    window.Init( 800, 600, 4, true );

    // OpenGL context needs to be current for `glewInit`
    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    window.MainLoop();
    return 0;
}

CWindow_Glfw::~CWindow_Glfw()
{
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

void CWindow_Glfw::Init( int width, int height, int multisampling, bool doubleBuffer )
{
    _doubleBuffer = doubleBuffer;

    // [GLFW Window guide; Window creation hints](http://www.glfw.org/docs/latest/window_guide.html#window_hints_values)

    glfwWindowHint( GLFW_DEPTH_BITS, 24 );
    glfwWindowHint( GLFW_STENCIL_BITS, 8 ); 

    glfwWindowHint( GLFW_SAMPLES, multisampling );
    glfwWindowHint( GLFW_DOUBLEBUFFER, _doubleBuffer ? GLFW_TRUE : GLFW_FALSE );
    
    _wnd = glfwCreateWindow( width, height, "OGL window", nullptr, nullptr );
    if ( _wnd == nullptr )
    {
        glfwTerminate();
        throw std::runtime_error( "error initializing window" ); 
    }

    glfwMakeContextCurrent( _wnd );

    glfwSetWindowUserPointer( _wnd, this );
    glfwSetWindowSizeCallback( _wnd, CWindow_Glfw::CallbackResize );

    _monitor =  glfwGetPrimaryMonitor();
    glfwGetWindowSize( _wnd, &_wndSize[0], &_wndSize[1] );
    glfwGetWindowPos( _wnd, &_wndPos[0], &_wndPos[1] );
    _updateViewport = true;

    _draw = std::make_unique<OpenGL::CBasicDraw>();
}

void CWindow_Glfw::Resize( int cx, int cy )
{
    _updateViewport = true;
}

void CWindow_Glfw::MainLoop ( void )
{
    InitScene();

    _start_time = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(_wnd))
    {
        if ( _updateViewport )
        {
            glfwGetFramebufferSize( _wnd, &_vpSize[0], &_vpSize[1] );
            glViewport( 0, 0, _vpSize[0], _vpSize[1] );
            _updateViewport = false;
        }

        _current_time     = std::chrono::high_resolution_clock::now();
        auto   delta_time = _current_time - _start_time;
        double time_ms    = (double)std::chrono::duration_cast<std::chrono::milliseconds>(delta_time).count();
        
        Render( time_ms );

        if ( _doubleBuffer )
          glfwSwapBuffers( _wnd );
        else
          glFinish();
        
        glfwPollEvents();
    }

    _draw.reset( nullptr );
} 


std::string sh_vert = R"(
#version 400

layout (location = 0) in vec4 in_pos;
layout (location = 1) in vec4 in_col;

out TVertexData
{
    vec3 pos;
    vec4 col;
} out_data;


uniform mat4 u_proj;
uniform mat4 u_view;
uniform mat4 u_model;

void main()
{
    vec4 view_pos = u_view * u_model * in_pos; 
    out_data.col  = in_col;
    out_data.pos  = view_pos.xyz / view_pos.w;
    gl_Position   = u_proj * view_pos;
}
)";

std::string sh_frag = R"(
#version 400

in TVertexData
{
    vec3 pos;
    vec4 col;
} in_data;

out vec4 fragColor;

void main()
{
    fragColor = in_data.col;
}
)";

void CWindow_Glfw::InitScene( void )
{
    _prog.reset( new OpenGL::ShaderProgram(
    {
      { sh_vert, GL_VERTEX_SHADER },
      { sh_frag, GL_FRAGMENT_SHADER }
    } ) );

    _prog->Use();

    _prog->SetUniformM44( "u_proj", OpenGL::Identity() );
    _prog->SetUniformM44( "u_view", OpenGL::Identity() );
    _prog->SetUniformM44( "u_model", OpenGL::Identity() );
}

void CWindow_Glfw::Render( double time_ms )
{
  _prog->SetUniformM44( "u_proj", OpenGL::Camera::Orthopraphic( (float)_vpSize[0] / (float)_vpSize[1], { -1.0f, 1.0f } ) );

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
      
    _draw->DrawConvexPolygon(
    2, { -0.8f, -0.8f, 0.8f, -0.8f, 0.0f, 0.0f },
    { 1.0f, 0.0f, 0.0f, 1.0f } );

    _draw->DrawConvexPolygon(
    2, { -0.8f, 0.8f, 0.8f, 0.8f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f, 1.0f } );

    _draw->DrawConvexPolygon(
    2, { -0.8f, -0.8f, -0.8f, 0.8f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f, 1.0f } );

     _draw->DrawConvexPolygon(
    2, { 0.8f, -0.8f, 0.8f, 0.8f, 0.0f, 0.0f },
    { 1.0f, 1.0f, 0.0f, 1.0f } );
}