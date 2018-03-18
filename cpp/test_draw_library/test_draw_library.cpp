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

    std::unique_ptr<Render::IDraw> _draw;

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


void CWindow_Glfw::InitScene( void )
{
    _draw->BackgroundColor( { 0.95f, 0.95f, 0.92f, 1.0f } );
    _draw->Init();
}

void CWindow_Glfw::Render( double time_ms )
{
    // TODO $$$ adaptie transparency
    // TODO $$$ SSOA (3 frequences)
    // TODO $$$ text + greek letters (distance fields)
    // TODO $$$ post effects (cell (toon), sketch, gamma, hdr) - book of shaders
    // TOOO $$$ meshs
    // TODO $$$ input polyline
    // TODO $$$ draw arcs, curves (nurbs, spline) by tessellation shader
    // TODO $$$ orbit controll

    float aspect = (float)_vpSize[0] / (float)_vpSize[1];
    float scale_x = aspect < 1.0f ? 1.0f : aspect;
    float scale_y = aspect < 1.0f ? 1.0f/aspect : 1.0f;
   
    _draw->ViewportSize( { (size_t)_vpSize[0], (size_t)_vpSize[1] } );
    _draw->Projection( OpenGL::Camera::Orthopraphic( scale_x, scale_y, { -1.0f, 1.0f } )  );
    _draw->View( OpenGL::Identity() );
    _draw->Model( OpenGL::Identity() );

    _draw->Begin();
    
    _draw->ActivateBackground();

    _draw->DrawLines2D( { -scale_x, -scale_y }, { 0.0f, scale_y }, 0.08f, 0.0f, { 0.1f, 0.8f, 0.8f, 1.0f }, 1.0f );
    _draw->DrawGrid2D( { 0.0f, -scale_y }, { scale_x, scale_y }, { 0.05f, 0.05f }, 0.0f, { 0.1f, 0.8f, 0.8f, 1.0f }, 1.0f );

    _draw->ActivateOpaque();

    const char   *text = "Hello, a long text with a lot of different letters.";
    static float  text_height  = 0.126f;
    static float  text_scale_y = 1.0f;
    
    std::array<float, 4> text_rect{ 0.0f };
    _draw->CalculateTextSize( 0, text, text_height, text_rect[2], text_rect[1], text_rect[3] );
    _draw->DrawText( OpenGL::CBasicDraw::font_sans, text, text_height, text_scale_y, { scale_x * -0.96f, 0.3f, -0.01f }, { 1.0f, 0.5f, 0.0f, 1.0f } );
    
    const char *greek_text = "DEFGHIJKLMNOPQRSTUVW";
    _draw->DrawText( OpenGL::CBasicDraw::font_symbol, greek_text, 0.2f, 1.0f, { scale_x * -0.96f, 0.0f, -0.01f }, { 0.0f, 1.0f, 1.0f, 1.0f } );
    
    const char *letter_text = "Hallo letter";
    _draw->DrawText( OpenGL::CBasicDraw::font_pcifico, letter_text, 0.2f, 1.0f, { scale_x * -0.96f, -0.3f, -0.01f }, { 0.0f, 0.1f, 0.1f, 1.0f } );
    
    _draw->DrawRectangle2D( {-0.8f, -0.8f}, {0.8f, 0.8f}, 0.0f, { 1.0f, 0.0f, 1.0f, 1.0f }, 5 );

    _draw->ActivateTransparent();

    _draw->DrawConvexPolygon( 2, { -0.8f, -0.8f,  0.8f, -0.8f,  0.0f,  0.8f }, { 1.0f, 0.0f, 0.0f, 0.5f } );
    _draw->DrawConvexPolygon( 2, { -0.8f,  0.8f,  0.8f,  0.8f,  0.0f, -0.8f }, { 0.0f, 1.0f, 0.0f, 0.5f } );
    _draw->DrawConvexPolygon( 2, { -0.8f, -0.8f, -0.8f,  0.8f,  0.8f,  0.0f }, { 0.0f, 0.0f, 1.0f, 0.5f } );
    _draw->DrawConvexPolygon( 2, {  0.8f, -0.8f,  0.8f,  0.8f, -0.8f,  0.0f }, { 1.0f, 1.0f, 0.0f, 0.5f } );
    
    _draw->Finish();
}