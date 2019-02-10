#include <stdafx.h>

// OpenGL
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// GLFW
#include <glfw3_window.h>

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


#include <utility_noise_perlin.h>


// [Switching Between windowed and full screen in OpenGL/GLFW 3.2](https://stackoverflow.com/questions/47402766/switching-between-windowed-and-full-screen-in-opengl-glfw-3-2/47462358#47462358)
/***********************************************************************************************//**
* \brief   
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
class CWindow_Glfw
{
private:

    View::GLFW3::CWindow _window;

    void Resize( int cx, int cy );

    std::chrono::high_resolution_clock::time_point _start_time;
    std::chrono::high_resolution_clock::time_point _current_time;

    std::unique_ptr<OpenGL::ShaderProgramSimple> _prog;

    void InitScene( void );
    void Render( double time_ms );

public:

    CWindow_Glfw( void ) : _window("OGL window") {}

    virtual ~CWindow_Glfw();

    void Init( int width, int height, int multisampling, bool doubleBuffer, bool debugcontext );
    void MainLoop( void );

    void CursorEvent( const View::TCursorEventData &data );
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
    // create OpenGL window and make OpenGL context current (`glfwInit` has to be done before).
    CWindow_Glfw window;
    window.Init( 800, 600, mulit_samples, true, debug_level != COpenGLContext::TDebugLevel::off );

    // OpenGL context needs to be current for `glewInit`
    glewExperimental = true;
    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    COpenGLContext context;
    context.Init( debug_level );
    
    if (mulit_samples > 1)
    {
        glEnable(GL_MULTISAMPLE); // default
        glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
    }

    window.MainLoop();
    return 0;
}


CWindow_Glfw::~CWindow_Glfw()
{}


void CWindow_Glfw::Init( int width, int height, int multisampling, bool doubleBuffer, bool debugcontext )
{
    View::TViewSettings paramter;
    paramter._size = {width, height};
    paramter._samples = multisampling;
    paramter.Set<View::TCapability::doublebuffer>( doubleBuffer );
    paramter.Set<View::TCapability::debug>( debugcontext );
    
    _window.Init( paramter );

    View::TCursorEvent cb_cursor = [this]( View::IView &view, const View::TCursorEventData &data )
    {
        this->CursorEvent( data );
    };
    _window.AddCursorEvent( 1, cb_cursor );

    _window.Activate();
}


 void CWindow_Glfw::CursorEvent( const View::TCursorEventData &data )
 {
     std::cout << (int)data._kind 
         << " (" << data._position[0] << ", " << data._position[1] << ") "
         << " (" << data._distance[0] << ", " << data._distance[1] << ") "
         << data._button << std::endl;
 }


void CWindow_Glfw::MainLoop ( void )
{
    InitScene();

    // TODO $$$
    // thread 
    // process class implementing loop and timing std::chrono::high_resolution_clock

    _start_time = std::chrono::high_resolution_clock::now();

    while (_window.Dropped() == false)
    {
        if ( _window.SizeChanged(true) )
        {
            View::TSize size = _window.Size();
            glViewport( 0, 0, size[0], size[1] );
        }

        _current_time     = std::chrono::high_resolution_clock::now();
        auto   delta_time = _current_time - _start_time;
        double time_ms    = (double)std::chrono::duration_cast<std::chrono::milliseconds>(delta_time).count();
        
        Render( time_ms );

        _window.Flush();
        _window.HandleEvents();
    }
} 


std::string sh_vert = R"(
#version 400 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;

out vec2 vUV;

uniform mat4 u_proj;
uniform mat4 u_model;

void main()
{
    vUV         = inUV;
    gl_Position = u_proj * u_model * vec4(inPos, 1.0);
}
)";


std::string sh_frag = R"(
#version 400 core

in vec2 vUV;

out vec4 fragColor;

void main()
{
    fragColor = vec4(vUV.x, vUV.y, (1.0 - vUV.x)*(1.0 - vUV.y), 1.0);
}
)";


std::string sh_perlin_noise = R"(
#version 400 core

in vec2 vUV;

out vec4 fragColor;

void main()
{
    fragColor = vec4(vUV.x, vUV.y, (1.0 - vUV.x)*(1.0 - vUV.y), 1.0);
}
)";


GLint proj_loc  = -1;
GLint model_loc = -1;
GLuint program = 0;

void CWindow_Glfw::InitScene( void )
{
    static const std::vector<float> varray
    { 
      -1.0f, -1.0f,    0.0f, 0.0f, 
       1.0f, -1.0f,    1.0f, 0.0f,
       1.0f,  1.0f,    1.0f, 1.0f,
      -1.0f,  1.0f,    0.0f, 1.0f,
    };

    static const std::vector<unsigned int> iarray
    {
        0, 1, 2, 0, 2, 3
    };

    GLuint vbo;
    glGenBuffers( 1, &vbo );

    GLuint ibo;
    glGenBuffers( 1, &ibo );

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );

    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, iarray.size()*sizeof(*iarray.data()), iarray.data(), GL_STATIC_DRAW );

    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(*varray.data()), 0 );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(*varray.data()), (void*)(2*sizeof(*varray.data())) );
    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    _prog.reset( new OpenGL::ShaderProgramSimple(
    {
      { sh_vert, GL_VERTEX_SHADER },
      { sh_perlin_noise, GL_FRAGMENT_SHADER }
    } ) );
    program = _prog->Prog();

    proj_loc  = glGetUniformLocation( program, "u_proj" );
    model_loc = glGetUniformLocation( program, "u_model" );

    glUseProgram( program );
}


void CWindow_Glfw::Render( double time_ms )
{
    View::TSize size = _window.Size();
    float aspect = (float)size[0] / (float)size[1];

    glm::mat4 proj  = glm::ortho( -aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f );
    glm::mat4 model = glm::scale( glm::mat4(1.0f), glm::vec3( 0.8f, 0.8f, 0.8f ) );

    glUniformMatrix4fv( proj_loc,  1, GL_FALSE, glm::value_ptr( proj ) );
    glUniformMatrix4fv( model_loc, 1, GL_FALSE, glm::value_ptr( model ) );

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
      
    glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr );
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
