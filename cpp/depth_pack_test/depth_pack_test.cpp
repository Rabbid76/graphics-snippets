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
#include <OpenGLVertexBuffer.h>
#include <OpenGLFramebuffer.h>


// [Switching Between windowed and full screen in OpenGL/GLFW 3.2](https://stackoverflow.com/questions/47402766/switching-between-windowed-and-full-screen-in-opengl-glfw-3-2/47462358#47462358)
class CWindow_Glfw
{
public:

    using TDrawBufferPtr = std::unique_ptr<Render::IDrawBuffer>;
    using TDrawBuffers   = std::vector<TDrawBufferPtr>;
    using TProgramPtr    = std::unique_ptr<OpenGL::ShaderProgram>;
    using TPrograms      = std::vector<TProgramPtr>;
    using TProcessPtr    = std::unique_ptr<Render::IRenderProcess>;

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

    TDrawBuffers _drawBuffers;
    TPrograms    _prgrams;    
    TProcessPtr  _process;    

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
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
#if defined(_DEBUG)
    glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
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

    _monitor =  glfwGetPrimaryMonitor();
    glfwGetWindowSize( _wnd, &_wndSize[0], &_wndSize[1] );
    glfwGetWindowPos( _wnd, &_wndPos[0], &_wndPos[1] );
    _updateViewport = true;
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
} 


std::string sh_vert = R"(
#version 460 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;

out vec3 vertPos;
out vec4 vertCol;

void main()
{
    vertCol     = inColor;
		vertPos     = inPos;
		gl_Position = vec4(inPos, 1.0);
}
)";

std::string sh_frag = R"(
#version 460 core

in vec3 vertPos;
in vec4 vertCol;

out vec4 fragColor;

void main()
{
    fragColor = vertCol;
}
)";

std::string sh_screenspace_vert = R"(
#version 460 core

layout (location = 0) in vec3 inPos;

out vec3 vertPos;

void main()
{
		vertPos     = inPos;
		gl_Position = vec4(inPos, 1.0);
}
)";

std::string sh_screenspace_frag = R"(
#version 460 core

in vec3 vertPos;

out vec4 fragColor;

layout (binding = 1) uniform sampler2D u_samplerDepth;
layout (binding = 2) uniform sampler2D u_samplerColor;

void main()
{
    vec2 texCoord = vertPos.xy * 0.5 + 0.5;
    vec4 vertCol = texture(u_samplerColor, texCoord);
    fragColor = vertCol.ggga;
}
)";

size_t c_mesh_screenspace;

size_t c_prog_draw;
size_t c_prog_screenspace;

GLuint vao;

void CWindow_Glfw::InitScene( void )
{
    c_prog_draw = _prgrams.size();
    _prgrams.emplace_back( new OpenGL::ShaderProgram(
      {
        { sh_vert, GL_VERTEX_SHADER },
        { sh_frag, GL_FRAGMENT_SHADER }
      } )
    );

    c_prog_screenspace = _prgrams.size();
    _prgrams.emplace_back( new OpenGL::ShaderProgram(
      {
        { sh_screenspace_vert, GL_VERTEX_SHADER },
        { sh_screenspace_frag, GL_FRAGMENT_SHADER }
      } )
    );

    // screen space rectangle

    c_mesh_screenspace = _drawBuffers.size();
    _drawBuffers.emplace_back( new OpenGL::CDrawBuffer( Render::TDrawBufferUsage::static_draw, 1024 ) );

    static const std::vector<char> &vert_2d_descr = Render::IDrawBuffer::VADescription( Render::TVA::i0__b0_xy );

    std::vector<float> screen_sapce_pt{ -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f };
    std::vector<unsigned int> screen_sapce_inx{ 0, 1, 2, 0, 2, 3 };

    _drawBuffers[c_mesh_screenspace]->SpecifyVA( vert_2d_descr.size(), vert_2d_descr.data() );
    _drawBuffers[c_mesh_screenspace]->UpdateVB( 0, sizeof(float), screen_sapce_pt.size(), screen_sapce_pt.data() );
    _drawBuffers[c_mesh_screenspace]->UpdateIB( 0, sizeof(unsigned int), screen_sapce_inx.size(), screen_sapce_inx.data() );

    static const std::vector<float> varray
    { 
      -0.707f, -0.75f,    1.0f, 0.0f, 0.0f, 1.0f, 
       0.707f, -0.75f,    1.0f, 1.0f, 0.0f, 1.0f,
       0.0f,    0.75f,    0.0f, 0.0f, 1.0f, 1.0f
    };

    GLuint vbo;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );

    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 6*sizeof(*varray.data()), 0 );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 6*sizeof(*varray.data()), (void*)(2*sizeof(*varray.data())) );
    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );

    const size_t c_depth_ID  = 0;
    const size_t c_color_ID  = 1;
  
    Render::IRenderProcess::TBufferMap buffers;
    Render::IRenderProcess::TPassMap passes;

    // specify drawing buffers

    // depth buffer
    Render::TBuffer depth_buffer;
    depth_buffer._type   = Render::TBufferType::DEPTH;
    depth_buffer._format = Render::TBufferDataType::DEFAULT;
    buffers.emplace( c_depth_ID, depth_buffer );

    // color buffer
    Render::TBuffer color_buffer;
    color_buffer._type   = Render::TBufferType::COLOR4;
    color_buffer._format = Render::TBufferDataType::DEFAULT;
    buffers.emplace( c_color_ID, color_buffer );

    // specify drawing passes

    // draw pass
    Render::TPass draw_pass;
    draw_pass._depth = Render::TPassDepthTest::LESS;
    draw_pass._targets.emplace_back( c_depth_ID, Render::TPass::TTarget::depth ); // depth target
    draw_pass._targets.emplace_back( c_color_ID, 0 );                             // color target
    passes.emplace( 0, draw_pass );

    // color effect pass
    Render::TPass effect_pass;
    effect_pass._sources.emplace_back( c_depth_ID,  1 ); // depth buffer source
    effect_pass._sources.emplace_back( c_color_ID,  2 ); // color buffer source
    passes.emplace( 1, effect_pass );

     _process = std::make_unique<OpenGL::CRenderProcess>();
    bool complete = _process->SpecifyBuffers( buffers );
    if ( complete == false )
      std::cout << "ERROR : buffer specification" << std::endl;
    if ( complete )
    {
      complete = _process->SpecifyPasses( passes );
      if ( complete == false )
        std::cout << "ERROR : pass specification" << std::endl;
    }
    if ( complete )
    {
      complete = _process->Validate();
      if ( complete == false )
        std::cout << "ERROR : render process validation" << std::endl;
    }
}

void CWindow_Glfw::Render( double time_ms )
{
    if ( _process->IsValid() )
    {
      if ( _process->Create( { (size_t)_vpSize[0], (size_t)_vpSize[1] } ) == false )
        std::cout << "ERROR : creating render process" << std::endl;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    _process->Prepare( 0 );
    _prgrams[c_prog_draw]->Use();
    
    glBindVertexArray( vao );
    glDrawArrays( GL_TRIANGLES, 0, 3 );
    glBindVertexArray( 0 );

    _prgrams[c_prog_draw]->Release();
    _process->Release();

    _process->Prepare( 1 );
    _prgrams[c_prog_screenspace]->Use();

    auto & draw_scree_space = _drawBuffers[c_mesh_screenspace];
    draw_scree_space->DrawAllElements( Render::TPrimitive::triangles, true );
    draw_scree_space->Release();

    _prgrams[c_prog_screenspace]->Release();
    _process->Release();
}