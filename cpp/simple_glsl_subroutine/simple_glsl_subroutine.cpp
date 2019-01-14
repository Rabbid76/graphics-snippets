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


// [Why glGetSubroutineIndex returns the same value for different function?](https://stackoverflow.com/questions/52899233/why-glgetsubroutineindex-returns-the-same-value-for-different-function/52899723#52899723)


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

    std::unique_ptr<OpenGL::ShaderProgramSimple> _prog;

    void InitScene( void );
    void IntrospectProgram( void );
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
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
    //glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );

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
    IntrospectProgram();

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

subroutine vec4 TColor();

layout(index = 1) subroutine (TColor) 
vec4 Color() 
{
    return vertCol;
}

layout(index = 2) subroutine (TColor) 
vec4 Grayscale() 
{ 
    float gray = dot(vertCol.rgb, vec3(0.2126, 0.7152, 0.0722)); 
    return vec4(vec3(gray), vertCol.a);
} 

layout(location = 1) subroutine uniform TColor subColor;

void main()
{
    fragColor = subColor();
}
)";

void CWindow_Glfw::InitScene( void )
{
    _prog.reset( new OpenGL::ShaderProgramSimple(
    {
      { sh_vert, GL_VERTEX_SHADER },
      { sh_frag, GL_FRAGMENT_SHADER }
    } ) );

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

    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 6*sizeof(*varray.data()), 0 );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 6*sizeof(*varray.data()), (void*)(2*sizeof(*varray.data())) );
    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

GLint nSubroutineUniforms = 0;
GLint nLocationCount = 0;
GLint maxSubroutineUniformLen = 0;
GLint sub_loc = 0;
GLuint sub_i0 = 0;
GLuint sub_i1 = 0;

void CWindow_Glfw::IntrospectProgram( void )
{
    GLuint program = _prog->Prog();

	  glGetProgramStageiv( program, GL_FRAGMENT_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORMS, &nSubroutineUniforms );
		glGetProgramStageiv( program, GL_FRAGMENT_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS, &nLocationCount );
		glGetProgramStageiv( program, GL_FRAGMENT_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH, &maxSubroutineUniformLen );

    GLint sub_loc = glGetSubroutineUniformLocation( program, GL_FRAGMENT_SHADER, "subColor" );
    GLuint sub_i0 = glGetSubroutineIndex( program, GL_FRAGMENT_SHADER, "Color" );
    GLuint sub_i1 = glGetSubroutineIndex( program, GL_FRAGMENT_SHADER, "Grayscale" );
    
    GLint no_of_subroutines;
    glGetActiveSubroutineUniformiv(
        program, GL_FRAGMENT_SHADER, 
        0, // Index of the subroutine uniform, but NOT the location of the uniform variable 
        GL_NUM_COMPATIBLE_SUBROUTINES, &no_of_subroutines);

    std::vector<GLint> sub_routines( no_of_subroutines ); 
    glGetActiveSubroutineUniformiv(
        program, GL_FRAGMENT_SHADER, 
        0, // Index of the subroutine uniform, but NOT the location of the uniform variable 
        GL_COMPATIBLE_SUBROUTINES, sub_routines.data());

    std::cout << no_of_subroutines << std::endl;
}

void CWindow_Glfw::Render( double time_ms )
{
    _prog->Use();

    GLenum error1 = glGetError();
    static GLuint sub_routine = 2; 

    std::vector<GLuint> sub_routines( nLocationCount, 0 );
    sub_routines.back() = sub_routine;
    glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, (GLsizei)sub_routines.size(), sub_routines.data() );
    GLenum error2 = glGetError();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
      
    glDrawArrays( GL_TRIANGLES, 0, 3 );
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
