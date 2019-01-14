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

    std::unique_ptr<OpenGL::ShaderProgramSimple> _prog_2;
    std::unique_ptr<OpenGL::ShaderProgramSimple> _prog_4;

    void InitScene( void );
    void Render( double time_ms );

public:

    virtual ~CWindow_Glfw();

    void Init( int width, int height, int multisampling, bool doubleBuffer, bool debugcontext );
    static void CallbackResize(GLFWwindow* window, int cx, int cy);
    void MainLoop( void );
};


class COpenGLContext
{
public:

    enum class TDebugLevel
    {
      off,
      all,
      error_only
    };

    void Init( TDebugLevel debug_level );

    static void DebugCallback( unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message, const void* userParam );
    void DebugCallback( unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message );

private:

    TDebugLevel _debug_level = TDebugLevel::off;
};


COpenGLContext::TDebugLevel debug_level = COpenGLContext::TDebugLevel::all;
int mulit_samples = 8;

int main(int argc, char** argv)
{
    if ( glfwInit() == GLFW_FALSE )
        throw std::runtime_error( "error initializing glfw" );

    // create OpenGL window and make OpenGL context current (`glfwInit` has to be done before).
    CWindow_Glfw window;
    window.Init( 800, 600, mulit_samples, true, debug_level != COpenGLContext::TDebugLevel::off );

    // OpenGL context needs to be current for `glewInit`
    glewExperimental = true;
    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    COpenGLContext context;
    context.Init( debug_level );

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

void CWindow_Glfw::Init( int width, int height, int multisampling, bool doubleBuffer, bool debugcontext )
{
    _doubleBuffer = doubleBuffer;

    // [GLFW Window guide; Window creation hints](http://www.glfw.org/docs/latest/window_guide.html#window_hints_values)

    glfwWindowHint( GLFW_DEPTH_BITS, 24 );
    glfwWindowHint( GLFW_STENCIL_BITS, 8 ); 

    glfwWindowHint( GLFW_SAMPLES, multisampling );
    glfwWindowHint( GLFW_DOUBLEBUFFER, _doubleBuffer ? GLFW_TRUE : GLFW_FALSE );

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 5 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    //glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );

    glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, debugcontext ? GLFW_TRUE : GLFW_FALSE );

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


std::string sh_vert_2 = R"(
#version 110

varying vec4 vertCol;

uniform mat4 u_modelview; 

void main()
{
    vertCol     = gl_Color;
		gl_Position = u_modelview * gl_Vertex;
}
)";

std::string sh_vert_4 = R"(
#version 400

layout (location = 0) in vec3 inPos;
layout (location = 3) in vec4 inColor;

out vec4 vertCol;

uniform mat4 u_modelview; 

void main()
{
    vertCol     = inColor;
		gl_Position = u_modelview * vec4(inPos, 1.0);
}
)";

std::string sh_frag = R"(
#version 400

in vec4 vertCol;

out vec4 fragColor;

void main()
{
    fragColor = vertCol;
}
)";

void CWindow_Glfw::InitScene( void )
{
    _prog_2.reset( new OpenGL::ShaderProgramSimple(
    {
      { sh_vert_2, GL_VERTEX_SHADER },
      { sh_frag, GL_FRAGMENT_SHADER }
    } ) ); 

    _prog_4.reset( new OpenGL::ShaderProgramSimple(
    {
      { sh_vert_4, GL_VERTEX_SHADER },
      { sh_frag, GL_FRAGMENT_SHADER }
    } ) ); 
}

void SetModelView( GLint location, glm::mat4 &m )
{
  if ( location < 0 )
  {
    glMatrixMode( GL_MODELVIEW );
    glLoadMatrixf( glm::value_ptr( m ) );
  }
  else
  {
    glUniformMatrix4fv( location, 1, GL_FALSE, glm::value_ptr(m) );
  }
}

void CWindow_Glfw::Render( double time_ms )
{
    // Using vertex attribute index 0, instead of Fixed-Function attribute GL_VERTEX_ARRAY
    // https://stackoverflow.com/questions/51944318/using-vertex-attribute-index-0-instead-of-fixed-function-attribute-gl-vertex-ar
    //
    // What are the Attribute locations for fixed function pipeline in OpenGL 4.0++ core profile?
    // https://stackoverflow.com/questions/20573235/what-are-the-attribute-locations-for-fixed-function-pipeline-in-opengl-4-0-cor
    //
    // ARB_vertex_program Table X.1
    // https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_vertex_program.txt
    //
    // Release Notes for NVIDIA OpenGL Shading Language Support; November 9, 2006; - pp. 7-8
    // http://developer.download.nvidia.com/opengl/glsl/glsl_release_notes.pdf


    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glMatrixMode( GL_MODELVIEW );

    glm::mat4 scale = glm::scale( glm::mat4( 1.0f ), glm::vec3( 0.25f, 0.25f, 0.25f ) );

    static const float varray[]
    { 
      // x        y         red   green blue  alpha
        -0.707f, -0.75f,    1.0f, 0.0f, 0.0f, 1.0f, 
         0.707f, -0.75f,    1.0f, 1.0f, 0.0f, 1.0f,
         0.0f,    0.75f,    0.0f, 0.0f, 1.0f, 1.0f
    };

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    
    GLfloat y[]{ 0.5f, 0.0f, -0.5f };
    GLuint prog[]{ 0, _prog_2->Prog(), _prog_4->Prog() };
    for ( int i = 0; i < 3; ++ i )
    {
        glUseProgram( prog[i] );
        GLint modelview_loc = prog[i] == 0 ? -1 : glGetUniformLocation( prog[i], "u_modelview" );

        // glBegin/glEnd sequence
        if ( i==0 || i==1 || i==2 )
        {
            SetModelView( modelview_loc, glm::translate(glm::mat4(1.0f),glm::vec3(-0.5f, y[i], 0.0f)) * scale );
            glBegin( GL_TRIANGLES );
            for ( int j=0; j < 3; ++j )
            {
              glVertex2fv( varray + j*6 );
              glColor4fv( varray + j*6 + 2 );
            }
            glEnd();
        }

        // fixed function attributes
        if ( i==0 || i==1 || i==2 )
        {
            SetModelView( modelview_loc, glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, y[i], 0.0f)) * scale );
            glVertexPointer( 2, GL_FLOAT, 6*sizeof(*varray), varray );
            glColorPointer( 4, GL_FLOAT, 6*sizeof(*varray), varray+2 );
            glEnableClientState( GL_VERTEX_ARRAY );
            glEnableClientState( GL_COLOR_ARRAY );
            glDrawArrays( GL_TRIANGLES, 0, 3 );
            glDisableClientState( GL_VERTEX_ARRAY );
            glDisableClientState( GL_COLOR_ARRAY );
        }

        // generic vertex attribute data
        if ( i==0 || i==1 || i == 2 )
        {
            SetModelView( modelview_loc, glm::translate(glm::mat4(1.0f),glm::vec3(0.5f, y[i], 0.0f)) * scale );
            glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 6*sizeof(*varray), varray );
            glVertexAttribPointer( 3, 4, GL_FLOAT, GL_FALSE, 6*sizeof(*varray), varray+2 );
            glEnableVertexAttribArray( 0 );
            glEnableVertexAttribArray( 3 );
            glDrawArrays( GL_TRIANGLES, 0, 3 );
            glDisableVertexAttribArray( 0 );
            glDisableVertexAttribArray( 3 );
        }
    }
}


void COpenGLContext::Init( COpenGLContext::TDebugLevel debug_level )
{
    _debug_level = debug_level;

    bool enable_debug = _debug_level != TDebugLevel::off;

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

    glDebugMessageCallback( &COpenGLContext::DebugCallback, this );

    if ( enable_debug )
    {
        // See also [How to use glDebugMessageControl](https://stackoverflow.com/questions/51962968/how-to-use-gldebugmessagecontrol/51963554#51963554)
        switch(_debug_level)
        {
        default:
            glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE );
            break;

        case TDebugLevel::error_only:
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
            glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, NULL, GL_TRUE);
            break;
        }
    
        glEnable( GL_DEBUG_OUTPUT );
        glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );

        std::string debug_message = "Starting debug messaging service";
        glDebugMessageInsert( GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, (GLsizei)debug_message.size(), debug_message.c_str());
    }
}


void COpenGLContext::DebugCallback( 
    unsigned int  source,     //!< I - 
    unsigned int  type,       //!< I - 
    unsigned int  id,         //!< I - 
    unsigned int  severity,   //!< I - 
    int           length,     //!< I - length of debug message
    const char   *message,    //!< I - debug message
    const void   *userParam ) //!< I - user parameter
{
    if ( userParam == nullptr )
        return;
    COpenGLContext *context_ptr = static_cast<COpenGLContext*>(const_cast<void*>(userParam));
    context_ptr->DebugCallback(source, type, id, severity, length, message);
}


void COpenGLContext::DebugCallback( 
    unsigned int  source,   //!< I - 
    unsigned int  type,     //!< I - 
    unsigned int  id,       //!< I - 
    unsigned int  severity, //!< I - 
    int           length,   //!< I - length of debug message
    const char   *message ) //!< I - debug message
{
    static const std::vector<GLenum> error_ids
    {
        GL_INVALID_ENUM,
        GL_INVALID_VALUE,
        GL_INVALID_OPERATION,
        GL_STACK_OVERFLOW,
        GL_STACK_UNDERFLOW,
        GL_OUT_OF_MEMORY
    };

    auto error_it = std::find( error_ids.begin(), error_ids.end(), id );
    if ( error_it != error_ids.end() )
    {
        std::cout << "error: " << message << std::endl << std::endl;
    }
    else
    {
        std::cout << message << std::endl << std::endl;
    }
}