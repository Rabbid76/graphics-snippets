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
#include <GL/freeglut.h>

// stl
#include <vector>
#include <stdexcept>
#include <chrono>
#include <memory>
#include <cmath>

#define _USE_MATH_DEFINES
#include <math.h>

// stb 
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Own
#include <OpenGL_Matrix_Camera.h>
#include <OpenGL_SimpleShaderProgram.h>


std::string sh_vert = R"(
#version 460

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;

out TVertexData
{
    vec3 pos;
    vec4 col;
    vec2 uv;
} out_data;

void main()
{
    out_data.pos = inPos;
    out_data.col = inColor;
    out_data.uv  = inPos.xy * 0.5 + 0.5;
		gl_Position  = vec4(inPos, 1.0);
}
)";

std::string test_frag = R"(
#version 460

in TVertexData
{
    vec3 pos;
    vec4 col;
    vec2 uv;
} in_data;

out vec4 fragColor;

void main()
{
    fragColor = vec4(in_data.uv.s, in_data.uv.t, (1.0-in_data.uv.s)*(1.0-in_data.uv.t), 1.0);
    //fragColor = in_data.col;
}
)";

std::string test_diff_frag = R"(
#version 460

in TVertexData
{
    vec3 pos;
    vec4 col;
    vec2 uv;
} in_data;

out vec4 fragColor;

void main()
{
    vec3 col1 = vec3(in_data.uv.s, in_data.uv.t, (1.0-in_data.uv.s)*(1.0-in_data.uv.t));
    vec3 col2 = in_data.col.rgb; 
    fragColor = vec4(abs(col1 - col2), 1.0);
}
)";

std::string example_texture_1_frag = R"(
#version 460

in TVertexData
{
    vec3 pos;
    vec4 col;
    vec2 uv;
} in_data;

out vec4 out_color;

uniform vec2 u_vp_size;

void main()
{
    const float margin = 6.0;
    
    vec2 tile_size  = u_vp_size*0.5 - margin*1.5;
    vec2 tile_coord = abs( gl_FragCoord.xy - u_vp_size.xy*0.5 ) - margin*0.5;    
         

    vec3 frag_col = vec3( 0.5 );
    if ( all( greaterThanEqual(tile_coord, vec2(0.0)) ) && all( lessThanEqual(tile_coord, tile_size) ) )
    {
        if ( tile_coord.x > tile_size.x * 2.0/3.0 )
        {
            if ( tile_coord.y > tile_size.y * 2.0/3.0 )  frag_col = vec3( 1.0, 0.0, 0.0 );
            else if ( tile_coord.y > tile_size.y / 3.0 ) frag_col = vec3( 1.0, 0.0, 1.0 );
            else                                         frag_col = vec3( 0.0, 0.0, 1.0 );
        }
        else if ( tile_coord.x > tile_size.x / 3.0 ) 
        {
            if ( tile_coord.y > tile_size.y * 2.0/3.0 )  frag_col = vec3( 1.0, 0.5, 0.0 );
            else if ( tile_coord.y > tile_size.y / 3.0 ) frag_col = vec3( 1.0, 1.0, 1.0 );
            else                                         frag_col = vec3( 0.0, 1.0, 1.0 );
        }
        else 
        {
            if ( tile_coord.y > tile_size.y * 2.0/3.0 )  frag_col = vec3( 1.0, 1.0, 0.0 );
            else if ( tile_coord.y > tile_size.y / 3.0 ) frag_col = vec3( 0.5, 0.5, 0.5 );
            else                                         frag_col = vec3( 0.0, 1.0, 0.0 );
        }
    }

    out_color = vec4(frag_col.rgb, 1.0);
}
)";

std::string example_height_1_frag = R"(
#version 460

in TVertexData
{
    vec3 pos;
    vec4 col;
    vec2 uv;
} in_data;

out vec4 out_color;

uniform vec2 u_vp_size;

void main()
{
    const float pi = 3.141593;
    const float margin = 6.0;
    
    vec2 tile_size  = u_vp_size*0.5 - margin*1.5;
    vec2 quad_id    = gl_FragCoord.xy - u_vp_size.xy*0.5;
    vec2 tile_coord = abs( gl_FragCoord.xy - u_vp_size.xy*0.5 ) - margin*0.5;    
         
    vec2 tile_fac = tile_coord / tile_size;

    vec3 frag_col = vec3( 0.0 );
    if ( all( greaterThanEqual(tile_coord, vec2(0.0)) ) && all( lessThanEqual(tile_coord, tile_size) ) )
    {
        if ( quad_id.x < 0.0 )
        {
            if ( quad_id.y < 0.0 )
            {
                vec2 curve = cos(tile_fac * pi * 2.0 * 3.0 ) * -0.5 + 0.5;
                frag_col = vec3(curve.x * curve.y);
            }
            else
            {
                vec2 i_t  = tile_fac * 2.0 - 1.0;
                vec2 t_co = abs(i_t) * 2.0 - 1.0;
                vec2 t_cu = max(vec2(0.0), cos(abs(t_co) * pi * 0.6));
                float h = t_cu.x * t_cu.y;
                frag_col = vec3( mix(h, 1.0-h, step(0.0, i_t.x*i_t.y)) );
            }
        }
        else 
        {
            if ( quad_id.y < 0.0 )
            {
                vec2 curve = 2.0 - 2.0 * abs( vec2(fract(tile_fac * 5.0))*2.0 - 1.0);
                frag_col = vec3( clamp(min(curve.x, curve.y), 0.0, 1.0) );
            }
            else
            {
                vec2 curve = 1.0 - abs( vec2(fract(tile_fac * 3.0)) * 2.0 - 1.0);
                frag_col = vec3( min(curve.x, curve.y) );
            }
        }
    }

    out_color = vec4(frag_col.rgb, 1.0);
}
)";


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


std::unique_ptr<OpenGL::ShaderProgramSimple> g_prog;

std::chrono::high_resolution_clock::time_point g_start_time;

bool valid_viewport = false;

void OnIdle( void );
void Resize( int, int );
void Display( void );


COpenGLContext::TDebugLevel debug_level = COpenGLContext::TDebugLevel::all;

int main(int argc, char** argv)
{
    // Initialise FreeGLUT
    glutInit(&argc, argv);

    // Setup OpenGL window properties
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_BORDERLESS | GLUT_CAPTIONLESS);

    // Create window
    glutInitWindowSize(512, 512);
    int wnd = glutCreateWindow("test");
    if ( wnd == 0 )
        throw std::runtime_error( "error initializing window" ); 

    // Register display callback function
    glutDisplayFunc( Display ); 
    glutReshapeFunc( Resize );
    glutIdleFunc( OnIdle );

    glewExperimental = true;
    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    COpenGLContext context;
    context.Init( debug_level );

    enum TFragShaderType
    {
      test, test_diff,
      example_texture_1, example_height_1
    };
    static TFragShaderType frag_type = TFragShaderType::example_height_1;

    std::string sh_frag;
    switch ( frag_type )
    {
      default:
      case  TFragShaderType::test:              sh_frag = test_frag; break;
      case  TFragShaderType::test_diff:         sh_frag = test_diff_frag; break;
      case  TFragShaderType::example_texture_1: sh_frag = example_texture_1_frag; break;
      case  TFragShaderType::example_height_1:  sh_frag = example_height_1_frag; break;
    }

    g_prog.reset( new OpenGL::ShaderProgramSimple(
    {
      { sh_vert, GL_VERTEX_SHADER },
      { sh_frag, GL_FRAGMENT_SHADER }
    } ) );

    static const std::vector<float> varray
    { 
      -1.0f, -1.0f,    0.0f, 0.0f, 1.0f, 1.0f, 
       1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,
      -1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,
       1.0f,  1.0f,    1.0f, 1.0f, 0.0f, 1.0f,
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

    g_prog->Use();

    g_start_time = std::chrono::high_resolution_clock::now();
    glutMainLoop();

    g_prog.reset( nullptr );

    glutDestroyWindow(wnd);
    return 0;
}

void OnIdle( void )
{
  glutPostRedisplay();
  // ....
}

void Resize( int cx, int cy )
{
  valid_viewport = false;
  // ....
}


void Display( void )
{
    std::chrono::high_resolution_clock::time_point current_time = std::chrono::high_resolution_clock::now();
    auto   delta_time = current_time - g_start_time;
    double time_ms    = (double)std::chrono::duration_cast<std::chrono::milliseconds>(delta_time).count();

    static int vpSize[2]{ 0 };
    if ( valid_viewport == false )
    {
      vpSize[0] = glutGet( GLUT_WINDOW_WIDTH );
      vpSize[1] = glutGet( GLUT_WINDOW_HEIGHT );

      glViewport( 0, 0, vpSize[0], vpSize[1] );
      valid_viewport = true;

      g_prog->SetUniformF2( "u_vp_size", { (float)vpSize[0], (float)vpSize[1] } );
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
      
    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

    static bool read_pixel = false;
    if ( read_pixel )
    {
      std::vector<unsigned char> buffer(4 * vpSize[0] * vpSize[1]);
      glReadPixels( 0, 0, vpSize[0], vpSize[1], GL_RGBA, GL_UNSIGNED_BYTE, buffer.data() );
      const char *filename = "d:\\temp\temp01.bmp";
      if ( filename != nullptr )
      {
        // TODO $$$
      }
    }

    glutSwapBuffers();
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