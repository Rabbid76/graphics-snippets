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

    enum TScene
    {
      e_default,
      e_viewport_coordsys,
      e_model
    };

    std::array< int, 2 > _wndPos         {0, 0};
    std::array< int, 2 > _wndSize        {0, 0};
    std::array< int, 2 > _vpSize         {0, 0};
    bool                 _updateViewport = true;
    bool                 _doubleBuffer   = true;
    GLFWwindow *         _wnd            = nullptr;
    GLFWmonitor *        _monitor        = nullptr;

    static constexpr const Render::TColor Color_white( void )        { return { 1.0f, 1.0f, 1.0f, 1.0f }; }
    static constexpr const Render::TColor Color_black( void )        { return { 0.0f, 0.0f, 0.0f, 1.0f }; }
    static constexpr const Render::TColor Color_red( void )          { return { 1.0f, 0.0f, 0.0f, 1.0f }; }
    static constexpr const Render::TColor Color_green( void )        { return { 0.0f, 1.0f, 0.0f, 1.0f }; }
    static constexpr const Render::TColor Color_blue( void )         { return { 0.0f, 0.0f, 1.0f, 1.0f }; }
    static constexpr const Render::TColor Color_yellow( void )       { return { 1.0f, 1.0f, 0.0f, 1.0f }; }
    static constexpr const Render::TColor Color_magenta( void )      { return { 1.0f, 0.0f, 1.0f, 1.0f }; }
    static constexpr const Render::TColor Color_cyan( void )         { return { 0.0f, 1.0f, 1.0f, 1.0f }; }
    static constexpr const Render::TColor Color_orange( void )       { return { 1.0f, 0.5f, 0.0f, 1.0f }; }
    static constexpr const Render::TColor Color_red_2( void )        { return { 0.8f, 0.0f, 0.0f, 1.0f }; }
    static constexpr const Render::TColor Color_green_2( void )      { return { 0.0f, 0.8f, 0.0f, 1.0f }; }
    static constexpr const Render::TColor Color_blue_2( void )       { return { 0.0f, 0.0f, 0.8f, 1.0f }; }
    static constexpr const Render::TColor Color_ink( void )          { return { 0.1f, 0.3f, 0.8f, 1.0f }; }
    static constexpr const Render::TColor Color_paper_nature( void ) { return { 0.96f, 0.96f, 0.93f, 1.0f }; }
    static constexpr const Render::TColor Color_paper_line( void )   { return { 0.8f, 0.9f, 0.9f, 1.0f }; }
   
    void Resize( int cx, int cy );

    std::chrono::high_resolution_clock::time_point _start_time;
    std::chrono::high_resolution_clock::time_point _current_time;

    std::unique_ptr<Render::IDraw> _draw;
    float  _aspect = 1.0;
    float  _scale_x = 1.0;
    float  _scale_y = 1.0;
    TScene _scene   = e_model;

    Render::TPoint2 BL( void ) const { return{ -_scale_x, -_scale_y}; }
    Render::TPoint2 TL( void ) const { return{  _scale_x, -_scale_y}; }
    Render::TPoint2 BR( void ) const { return{ -_scale_x,  _scale_y}; }
    Render::TPoint2 TR( void ) const { return{  _scale_x,  _scale_y}; }

    void InitScene( void );
    void Render( double time_ms );

    void Lined( const Render::TPoint2 &bl, const Render::TPoint2 &tr, float dist=0.08f, float z=0.0f, float thickness=1.0f, const Render::TColor color=Color_paper_line() );
    void Checkered( const Render::TPoint2 &bl, const Render::TPoint2 &tr, Render::TVec2 dist={ 0.05f, 0.05f }, float z=0.0f, float thickness=1.0f, const Render::TColor color=Color_paper_line() );

    void TestScene( double time_ms );
    void ViewportCoordsys( double time_ms );
    void Model( double time_ms );

public:

    virtual ~CWindow_Glfw();

    void Init( int width, int height, bool doubleBuffer );
    static void CallbackResize(GLFWwindow* window, int cx, int cy);
    void MainLoop( void );
};

int main(int argc, char** argv)
{
    if ( glfwInit() == GLFW_FALSE )
        throw std::runtime_error( "error initializing glfw" );

    // create OpenGL window and make OpenGL context current (`glfwInit` has to be done before).
    static int cx = 800;
    static int cy = 600;
    CWindow_Glfw window;
    window.Init( cx, cy, true );

    // OpenGL context needs to be current for `glewInit`
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

void CWindow_Glfw::Init( int width, int height, bool doubleBuffer )
{
    static bool         c_core    = true;
    static float        c_scale   = 1.0f;
    static bool         c_fxaa    = false;
    static unsigned int c_samples = 4;

    _doubleBuffer = doubleBuffer;

    // [GLFW Window guide; Window creation hints](http://www.glfw.org/docs/latest/window_guide.html#window_hints_values)

    if ( c_core )
    {
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
        glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
    }

#if defined(_DEBUG)
    glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
#endif

    //GLFW_CONTEXT_ROBUSTNESS

    glfwWindowHint( GLFW_DEPTH_BITS, 24 );
    glfwWindowHint( GLFW_STENCIL_BITS, 8 ); 

    glfwWindowHint( GLFW_SAMPLES, 1 );
    glfwWindowHint( GLFW_DOUBLEBUFFER, _doubleBuffer ? GLFW_TRUE : GLFW_FALSE );

    glfwWindowHint( GLFW_DEPTH_BITS, 24 );
    glfwWindowHint( GLFW_STENCIL_BITS, 8 ); 
    
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

    _draw = std::make_unique<OpenGL::CBasicDraw>( c_core, c_samples, c_scale, c_fxaa );
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
    //if ( _scene == )
    //_draw->BackgroundColor( Color_white() );
    _draw->BackgroundColor( Color_paper_nature() );
    _draw->Init();
}


void CWindow_Glfw::Render( double time_ms )
{
  _aspect = (float)_vpSize[0] / (float)_vpSize[1];
  _scale_x = _aspect < 1.0f ? 1.0f : _aspect;
  _scale_y = _aspect < 1.0f ? 1.0f/_aspect : 1.0f;

  _draw->ViewportSize( { (size_t)_vpSize[0], (size_t)_vpSize[1] } );
  _draw->Projection( OpenGL::Camera::Orthopraphic( _scale_x, _scale_y, { -10.0f, 10.0f } )  );
  _draw->View( OpenGL::Identity() );
  _draw->Model( OpenGL::Identity() );

  _draw->Begin();

  switch (_scene)
  {

    default:
    case e_default:           TestScene( time_ms ); break;
    case e_viewport_coordsys: ViewportCoordsys( time_ms ); break;
    case e_model:             Model( time_ms ); break;
  }

  _draw->Finish();
}


void CWindow_Glfw::Lined( const Render::TPoint2 &bl, const Render::TPoint2 &tr, float dist, float z, float thickness, const Render::TColor color )
{
  _draw->DrawLines2D( bl, tr, dist, z, { 0.8f, 0.9f, 0.9f, 1.0f }, thickness );
}

void CWindow_Glfw::Checkered( const Render::TPoint2 &bl, const Render::TPoint2 &tr, Render::TVec2 dist, float z, float thickness, const Render::TColor color )
{
  // { 0.3f, 0.8f, 0.8f, 1.0f }
  _draw->DrawGrid2D( bl, tr, dist, z, { 0.8f, 0.9f, 0.9f, 1.0f }, thickness );
}


void CWindow_Glfw::TestScene( double time_ms )
{
    // TODO $$$ SSOA (3 frequences)
    // TODO $$$ text + greek letters: distance fields!
    // TODO $$$ post effects (cell (toon), sketch, gamma, hdr) - book of shaders
    // TOOO $$$ meshs
    // TODO $$$ input polyline
    // TODO $$$ draw arcs, curves (nurbs, spline) by tessellation shader
    // TODO $$$ glLineWidth( > 1.0 ) is deprecated in core profile
    // TODO $$$ orbit controll

    // TODO view matrix from pitch, yaw (and roll) or quaternation
    
    _draw->ActivateBackground();

    Lined( BL(), { 0.0f, _scale_y } );
    Checkered( { 0.0f, -_scale_y }, TR() );

    _draw->ActivateOpaque();

    static float  text_height  = 0.126f;
    static float  text_scale_y = 1.0f;
    const char *text = "Hello, a long text with a lot of different letters.";
    _draw->DrawText2D( OpenGL::CBasicDraw::font_sans, text, text_height, text_scale_y, { _scale_x * -0.96f, 0.3f, -0.01f }, Color_orange() );
    const char *greek_text = "DEFGHIJKLMNOPQRSTUVW";
    _draw->DrawText2D( OpenGL::CBasicDraw::font_symbol, greek_text, 0.2f, 1.0f, { _scale_x * -0.96f, 0.0f, -0.01f }, Color_cyan() );
    const char *letter_text = "Hallo letter";
    _draw->DrawText2D( OpenGL::CBasicDraw::font_pcifico, letter_text, 0.2f, 1.0f, { _scale_x * -0.96f, -0.3f, -0.01f }, Color_ink() );
    
    _draw->DrawRectangle2D( {-0.8f, -0.8f}, {0.8f, 0.8f}, 0.0f, Color_magenta(), 5 );

    _draw->DrawArrow( 
      3, {-_scale_x * 0.95f, _scale_y * 0.95f, 0.5f, _scale_x * 0.95f, -_scale_y * 0.95f, 0.5f },
      Color_red(), 3, { 0.05f, 0.03f }, true, true );


    _draw->ActivateTransparent();

    _draw->DrawConvexPolygon( 2, { -0.8f, -0.8f,  0.8f, -0.8f,  0.0f,  0.8f }, { 1.0f, 0.0f, 0.0f, 0.5f } );
    _draw->DrawConvexPolygon( 2, { -0.8f,  0.8f,  0.8f,  0.8f,  0.0f, -0.8f }, { 0.0f, 1.0f, 0.0f, 0.5f } );
    _draw->DrawConvexPolygon( 2, { -0.8f, -0.8f, -0.8f,  0.8f,  0.8f,  0.0f }, { 0.0f, 0.0f, 1.0f, 0.5f } );
    _draw->DrawConvexPolygon( 2, {  0.8f, -0.8f,  0.8f,  0.8f, -0.8f,  0.0f }, { 1.0f, 1.0f, 0.0f, 0.5f } );
}


void CWindow_Glfw::ViewportCoordsys( double time_ms )
{
  Render::TPoint2 vp_bl{ -1.0f*16.0f/9.0f, -1.0f };
  Render::TPoint2 vp_tr{  1.0f*16.0f/9.0f,  1.0f };
  Render::TPoint2 vp_br{ vp_tr[0], vp_bl[1] };
  Render::TPoint2 vp_tl{ vp_bl[0], vp_tr[1] };
  Render::TColor  vp_col{ 0.3f, 0.3f, 0.8f, 1.0f };
  Render::TColor  vp_col_t{ vp_col[0], vp_col[1], vp_col[2], 0.5f };
  Render::TVec2   arr_size{ 0.2f, 0.12f };
  Render::TVec2   arr_los_size{ arr_size[0]*2.0f, arr_size[1]*2.0f };
  float           axis_len = 2.0f;
  static float    axis_text_height  = 0.126f;
  static float    axis_text_scale_y = 0.9f;

  _draw->ActivateBackground();
  //Checkered( BL(), TR() );

  OpenGL::Camera camera;
  camera._vp    = { (int)_vpSize[0], (int)_vpSize[1] };
  camera._near  = 0.5f;
  camera._far   = 10.0f;
  camera._pos   = { 3.0f, 1.0f, 3.0f };
  camera._up    = { 0.0f, 1.0f, 0.0f };
  camera._fov_y = 80.0f;
  _draw->Projection( camera.Perspective() );
  _draw->View( camera.LookAt() );

  _draw->ActivateOpaque();

  // TODO $$$ draw matrix coordinat system
  _draw->DrawArrow( 3, {0.0f, 0.0f, 0.0f, axis_len, 0.0f, 0.0f }, Color_red_2(), 3, arr_size, false, true );
  _draw->DrawArrow( 3, {0.0f, 0.0f, 0.0f, 0.0f, axis_len, 0.0f }, Color_green_2(), 3, arr_size, false, true );
  _draw->DrawArrow( 3, {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, axis_len }, Color_blue_2(), 3, arr_size, false, true );

  _draw->DrawRectangle2D( vp_bl, vp_tr, 0.0f, vp_col, 3 );
  _draw->DrawArrow( 3, { 0.0f, 0.0f, 2.5f, 0.0f, 0.0f, -6.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, 1, arr_los_size, false, true );

  _draw->ActivateTransparent();

  _draw->DrawConvexPolygon( { vp_bl, vp_br, vp_tr, vp_tl }, vp_col_t );

  _draw->ActivateOpaque();
  _draw->ClearDepth();
  
  _draw->DrawText2DProjected( OpenGL::CBasicDraw::font_sans, "X", 7, axis_text_height, axis_text_scale_y, { axis_len, 0.0f, 0.0f }, Color_red_2() );
  _draw->DrawText2DProjected( OpenGL::CBasicDraw::font_sans, "Y", 1, axis_text_height, axis_text_scale_y, { 0.0f, axis_len, 0.0f }, Color_green_2() );
  _draw->DrawText2DProjected( OpenGL::CBasicDraw::font_sans, "Z", 3, axis_text_height, axis_text_scale_y, { 0.0f, 0.0f, axis_len }, Color_blue_2() );

  const char *los_text = "Line of sight (-Z)";
  _draw->DrawText2DProjected( OpenGL::CBasicDraw::font_sans, los_text, 1, 0.09f, 0.7f, { 0.0f, 0.0f, -6.0f }, Color_black() );

  const char *title_text = "Viewport / view coordinate system";
  _draw->DrawText2DProjected( OpenGL::CBasicDraw::font_sans, title_text, 2, 0.09f, 0.7f, { 0.0f, 2.5f, 0.0f }, Color_black() );
}


void CWindow_Glfw::Model( double time_ms )
{
  _draw->ActivateBackground();
  Checkered( BL(), TR() );

  _draw->ActivateOpaque();

  _draw->DrawPolyline( 2, 
    { -0.1f, -0.2f, 0.1f, -0.2f, 0.1f, 0.0f, 0.2f, 0.0f, 0.0f, 0.2f, -0.2f, 0.0f, -0.1f, 0.0f, -0.1f, -0.2f },
    Color_ink(), 3.0f );
}