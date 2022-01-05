#include <stdafx.h>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// GLFW
#include <glfw3_window.h>

// stl
#include <iostream>
#include <vector>
#include <stdexcept>
#include <chrono>
#include <memory>
#include <cmath>

#define _USE_MATH_DEFINES
#include <math.h>

// Application

#include <utility_noise_perlin.h>

#include <OpenGL_3xx_program.h>
#include <OpenGL_include.h>

#include <OpenGL_Matrix_Camera.h>



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

    std::unique_ptr<Program::OpenGL::GL3xx::CProgram> _prog;

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
#version 460 core

in vec2 vUV;

out vec4 fragColor;

uniform float u_time_ms;

int permutation[256] = {
    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
    140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
    247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
    57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
    74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
    60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54,
    65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
    200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64,
    52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212,
    207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213,
    119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
    129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104,
    218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
    81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157,
    184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93,
    222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,
};

int perm(int i) { return permutation[i % 256]; }

vec3 fade(vec3 t) { return t * t * t * (t * (t * 6.0 - 15.0) + 10.0); }

float grad(int hash, float x, float y, float z)
{
    int h = hash & 15;                      
    float u = h<8 ? x : y,                 
          v = h<4 ? y : h==12||h==14 ? x : z;
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

float noise(in vec3 p)
{
    vec3  p256 = fract( abs(p) ) * 255.0;
    ivec3 i256 = ivec3( floor(p256) );
    vec3  pf   = fract( p256 );   
    vec3  uvw  = fade( pf.xyz );
    
    int A  = perm(i256.x  ) + i256.y;
    int B  = perm(i256.x+1) + i256.y;
    int AA = perm(A)   + i256.z;
    int BA = perm(B)   + i256.z;
    int AB = perm(A+1) + i256.z;      
    int BB = perm(B+1) + i256.z;   
 
    return mix(uvw.z, mix(uvw.y, mix(uvw.x, grad( perm(AA  ), pf.x  , pf.y  , pf.z   ), 
                                            grad( perm(BA  ), pf.x-1, pf.y  , pf.z   )),
                                 mix(uvw.x, grad( perm(AB  ), pf.x  , pf.y-1, pf.z   ), 
                                            grad( perm(BB  ), pf.x-1, pf.y-1, pf.z   ))),
                      mix(uvw.y, mix(uvw.x, grad( perm(AA+1), pf.x  , pf.y  , pf.z-1 ), 
                                            grad( perm(BA+1), pf.x-1, pf.y  , pf.z-1 )), 
                                 mix(uvw.x, grad( perm(AB+1), pf.x  , pf.y-1, pf.z-1 ),
                                            grad( perm(BB+1), pf.x-1, pf.y-1, pf.z-1 ))));
}

vec3 HUEtoRGB(in float H)
{
    float R = abs(H * 6.0 - 3.0) - 1.0;
    float G = 2.0 - abs(H * 6.0 - 2.0);
    float B = 2.0 - abs(H * 6.0 - 4.0);
    return clamp( vec3(R,G,B), 0.0, 1.0 );
}

void main()
{
    vec2 offs = vec2(u_time_ms / 5000.0, u_time_ms / 15000.0);
    offs = vec2(0.0);

    float n = noise(vec3(vUV.xy + offs.xy, 0.0));
    //float n = noise(vec3(vUV.x + offs.x, 0.0, vUV.y + offs.y));
    //float n = noise(vec3(vUV.xy + offs.xy, (1.0 - vUV.x)*(1.0 - vUV.y)));

    fragColor = vec4( HUEtoRGB( n ), 1.0);
    
    //fragColor = vec4(vUV.x, vUV.y, (1.0 - vUV.x)*(1.0 - vUV.y), 1.0);
}
)";


GLint proj_loc  = -1;
GLint model_loc = -1;
GLint time_loc  = -1;
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

    _prog = std::make_unique<Program::OpenGL::GL3xx::CProgram>();
    _prog->Generate(
    {
      { sh_vert, GL_VERTEX_SHADER },
      { sh_perlin_noise, GL_FRAGMENT_SHADER }
    } );
    program = _prog->ProgramObject();

    proj_loc  = glGetUniformLocation( program, "u_proj" );
    model_loc = glGetUniformLocation( program, "u_model" );
    time_loc  = glGetUniformLocation( program, "u_time_ms" );

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
    glUniform1f( time_loc, (float)time_ms );


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
