#include <stdafx.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <tuple>
#include <array>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <math.h>
#include <iomanip>      // std::setprecision

// project includes
#include <gl/gl_debug.h>
#include <gl/gl_shader.h>

const float cPI = 3.141593f;

using vec3 = std::array<float, 3>;
vec3 BarycentricCoordinates( double x, double y, double vp_cx, double vp_cy );

// vertex array object
class VAObject
{
private:

  GLuint                _obj;
  std::vector< GLuint > _indices;

public:
  
  using TBuffer = std::tuple< GLint, std::vector< float > >;
  VAObject( const std::vector< TBuffer > & dataArrays, const std::vector< GLuint > & indices  )
    : _indices( indices )
  {
    GLuint indexBuffer;
    glGenBuffers( 1, &indexBuffer );
    

    glGenVertexArrays( 1, &_obj );
    std::vector<GLuint> buffers( dataArrays.size() );
    glGenBuffers( (GLsizei)buffers.size(), buffers.data() );
    glBindVertexArray( _obj );
    for ( size_t i_buffer = 0; i_buffer < buffers.size(); ++ i_buffer )
    {
      GLint elemSize = std::get<0>( dataArrays[i_buffer] );
      std::vector< float > buffer = std::get<1>( dataArrays[i_buffer] );
      glBindBuffer( GL_ARRAY_BUFFER, buffers[i_buffer] );
      glBufferData( GL_ARRAY_BUFFER, buffer.size() * sizeof( float ), buffer.data(), GL_STATIC_DRAW );
      glEnableVertexAttribArray( (GLuint)i_buffer );
      glVertexAttribPointer( (GLuint)i_buffer, elemSize, GL_FLOAT, GL_FALSE, 0, nullptr );
    }
    
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexBuffer );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof( GLuint ), _indices.data(), GL_STATIC_DRAW );
    glBindVertexArray( 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
  }

  virtual ~VAObject()
  {
    glDeleteVertexArrays( 1, &_obj );
  }
  
  void Draw( void ) const
  {  
    glBindVertexArray( _obj );
    //glDrawArrays( GL_TRIANGLES, 0, (GLsizei)_indices.size() );
    glDrawElements( GL_TRIANGLES, (GLsizei)_indices.size(), GL_UNSIGNED_INT, NULL );
    //glDrawElements( GL_TRIANGLES, (GLsizei)_indices.size(), GL_UNSIGNED_INT, _indices.data() );
  }
};

class Window
{
private:

  double               _startTime = 0.0;
  std::array<double, 2> _cursorPos;

  std::array< int, 2 > _wndPos         {0, 0};
  std::array< int, 2 > _wndSize        {0, 0};
  std::array< int, 2 > _vpSize         {0, 0};
  bool                 _updateViewport = true;
  GLFWwindow *         _wnd            = nullptr;
  GLFWmonitor *        _monitor        = nullptr;

public:

  Window( std::array< int, 2 > size )
  {
    if ( glfwInit() == 0 )
      throw std::runtime_error( "error initializing glfw" ); 
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glewExperimental = true;
#endif
    Init( size[0], size[1] );

    std::string ver = (const char*)glGetString( GL_VERSION );
    std::string ven = (const char*)glGetString( GL_VENDOR );
    std::cout << ver.data() << std::endl;
    std::cout << ven.data() << std::endl;
  }

  virtual ~Window()
  {
    if ( _wnd == nullptr )
      return;
    glfwDestroyWindow( _wnd );
    glfwTerminate();
  }

  void Init( int width, int height )
  {
    _wnd = glfwCreateWindow( width, height, "OGL window", nullptr, nullptr );
    if ( _wnd == nullptr )
    {
      glfwTerminate();
      throw std::runtime_error( "error initializing window" ); 
    }

    glfwMakeContextCurrent(_wnd);

    // get monitor
    _monitor = glfwGetPrimaryMonitor();

    // get window size and posiiton and invalidate viewport size
    glfwGetFramebufferSize(_wnd, &_vpSize[0], &_vpSize[1]);
    glfwGetWindowSize(_wnd, &_wndSize[0], &_wndSize[1]);
    glfwGetWindowPos(_wnd, &_wndPos[0], &_wndPos[1]);
    _updateViewport = true;

    // get mouse position
    glfwGetCursorPos(_wnd, &_cursorPos[0], &_cursorPos[1]);

    // get current time
    _startTime = glfwGetTime();

    // event handler (user pointer)
    glfwSetWindowUserPointer(_wnd, this);

    // resize event
    glfwSetWindowSizeCallback(_wnd, &Window::window_size_callback);

     // mouse events
    glfwSetCursorPosCallback(_wnd, Window::cursor_pos_callback);
    glfwSetMouseButtonCallback(_wnd, &Window::mouse_button_callback);

    // keyboard events
    glfwSetKeyCallback(_wnd, &Window::key_callback);
  }

  static void window_size_callback(GLFWwindow* window, int cx, int cy)
  {
    void *ptr = glfwGetWindowUserPointer( window );
    if ( Window *wndPtr = static_cast<Window*>( ptr ) )
      wndPtr->Resize( cx, cy );
  }

  static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
  {
    void *ptr = glfwGetWindowUserPointer( window );
    if ( Window *wndPtr = static_cast<Window*>( ptr ) )
      wndPtr->Key(key, scancode, action, mods);
  }

  static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
  {
    void *ptr = glfwGetWindowUserPointer( window );
    if ( Window *wndPtr = static_cast<Window*>( ptr ) )
      wndPtr->MouseMove(xpos, ypos);
  }

  static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
  {
    void *ptr = glfwGetWindowUserPointer( window );
    if ( Window *wndPtr = static_cast<Window*>( ptr ) )
      wndPtr->MouseButton(button, action, mods);
  }

  void Resize(int cx, int cy)
  {
    _updateViewport = true;
  }

  void Key(int key, int scancode, int action, int mods)
  {
    if (key == GLFW_KEY_F && action == GLFW_RELEASE)
    {
      bool isFullScreen = IsFullscreen();
      SetFullScreen(isFullScreen == false);
    }
    //activate_airship();
  }

  void MouseMove(double xpos, double ypos)
  {
    _cursorPos = { xpos, ypos };
    // ...
  }

  void MouseButton(int button, int action, int mods)
  {
    if ( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS )
    {
        vec3 b_coord = BarycentricCoordinates( _cursorPos[0], _cursorPos[1], (double)_wndSize[0], (double)_wndSize[1] );
        std::cout << std::fixed << std::setprecision( 3 ) << "barycentric : (" << b_coord[0] << ", " << b_coord[1] << ", " << b_coord[2] << ")" << std::endl;
    }

    // ...
  }

  bool IsFullscreen( void )
  {
    return glfwGetWindowMonitor( _wnd ) != nullptr;
  }

  void SetFullScreen( bool fullscreen )
  {
    if ( IsFullscreen() == fullscreen )
      return;

    if ( fullscreen )
    {
      // backup windwo position and window size
      glfwGetWindowPos( _wnd, &_wndPos[0], &_wndPos[1] );
      glfwGetWindowSize( _wnd, &_wndSize[0], &_wndSize[1] );
      
      // get reolution of monitor
      const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

      // swithc to full screen
      glfwSetWindowMonitor( _wnd, _monitor, 0, 0, mode->width, mode->height, 0 );
    }
    else
    {
      // restor last window
      glfwSetWindowMonitor( _wnd, nullptr,  _wndPos[0], _wndPos[1], _wndSize[0], _wndSize[1], 0 );
    }

    _updateViewport = true;
  }

  void MainLoop ( void )
  {
    while (!glfwWindowShouldClose(_wnd))
    {
      if ( _updateViewport )
      {
        glfwGetFramebufferSize( _wnd, &_vpSize[0], &_vpSize[1] );
        glViewport( 0, 0, _vpSize[0], _vpSize[1] );
        _updateViewport = false;
        glfwGetWindowSize(_wnd, &_wndSize[0], &_wndSize[1]);
      }

      vec3 b_coord = BarycentricCoordinates( _cursorPos[0], _cursorPos[1], (double)_wndSize[0], (double)_wndSize[1] );
      glClearColor(b_coord[0] < 0.0f ? 1.0f : 0.0f, b_coord[1] < 0.0f ? 1.0f : 0.0f, b_coord[2] < 0.0f ? 1.0f : 0.0f, 0.0f);

      Render();

      glfwSwapBuffers(_wnd);
      glfwPollEvents();
    }

    glfwDestroyWindow( _wnd );
    _wnd = nullptr;
    glfwTerminate();
  }

  
  const OpenGL::ShaderProgramSimple *prog = nullptr;
  const VAObject *vao = nullptr;
  void Set( const OpenGL::ShaderProgramSimple *p, const VAObject *v )
  {
    prog = p;
    vao = v;
  }

    
  glm::mat4 testTrans;
  void Render( void )
  {
      if ( _wnd == nullptr )
          throw std::runtime_error( "error creating glfw window" );

      double currentTime = glfwGetTime();

      // set up attributes and shader program
      glEnable( GL_DEPTH_TEST );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      prog->Use();
        
      // draw object 
      vao->Draw();
  }
};

static const std::string shader_vert = R"(
#version 410 core

layout (location=0) in vec3 in_pos;
layout (location=1) in vec4 in_col;

// layout locations are distributed incrementally to the members, starting with the specified location
//layout (location=0)
out TVertexData
{
    vec4 color;
} out_data; 

void main ()
{
    out_data.color = in_col;
    gl_Position    = vec4(in_pos.xyz, 1.0);
}
)";

static const std::string shader_frag = R"(
#version 410 core

// layout locations are distributed incrementally to the members, starting with the specified location
//layout (location=0)
in TVertexData
{
    vec4 color;
} in_data; 

layout (location = 0) out vec4 fragColor; 

void main ()
{
    fragColor = in_data.color;
}
)";

std::vector< float > vertices{ -0.707f, -0.75f, 0.0f, 0.707f, -0.75f, 0.0f, 0.0f, 0.75f, 0.0f };

int main(void)
{ 
  try
  {
    // create window
    Window *wnd = new Window( { 800, 600 } );
    
    // initialize glut
    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
    OpenGL::CContext context;
    context.Init( debug_level );

    // define tetrahedron vertex array opject
    const float sin120 = 0.8660254f;
    VAObject * vaObj = new VAObject(
    { { 3, vertices },
      { 3, { 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f } }
    },
    { 0, 1, 2 } );

    // load, compile and link shader
    OpenGL::ShaderProgramSimple * prog = new OpenGL::ShaderProgramSimple(
    { { shader_vert, GL_VERTEX_SHADER },
      { shader_frag, GL_FRAGMENT_SHADER }
    } );
  
    // start main loop
    wnd->Set( prog, vaObj );
    wnd->MainLoop();

    delete vaObj;
    delete prog;
    delete wnd;
  }
  catch( const std::runtime_error& re )
  {
    std::cerr << "runtime error: " << re.what() << std::endl;
  }
  catch( const std::exception& ex )
  {
    std::cerr << "exception: " << ex.what() << std::endl;
  }
  catch(...)
  {
    std::cerr << "exception exception" << std::endl;
  }
   
  return 0;
}


float dot( const vec3 &a, const vec3 &b )
{
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

vec3 Barycentric( const vec3 &v_AB, const vec3 &v_AC, const vec3 &v_AX )
{
    float d00 = dot(v_AB, v_AB);
    float d01 = dot(v_AB, v_AC);
    float d11 = dot(v_AC, v_AC);
    float d20 = dot(v_AX, v_AB);
    float d21 = dot(v_AX, v_AC);
    float denom = d00 * d11 - d01 * d01; // determinant
    vec3  b_coord;
    b_coord[1] = (d11 * d20 - d01 * d21) / denom;
    b_coord[2] = (d00 * d21 - d01 * d20) / denom;
    b_coord[0] = 1.0f - b_coord[1] - b_coord[2];
    return b_coord;
}

vec3 BarycentricCoordinates( double x, double y, double vp_cx, double vp_cy )
{
  double tex_x = x / vp_cx;
  double tex_y = y / vp_cy;
  double pos_x = tex_x * 2.0 - 1.0;
  double pos_y = 1.0 - tex_y * 2.0;

  vec3 v_AB{ (float)vertices[3] - (float)vertices[0], (float)vertices[4] - (float)vertices[1], (float)vertices[5] - (float)vertices[2] };
  vec3 v_AC{ (float)vertices[6] - (float)vertices[0], (float)vertices[7] - (float)vertices[1], (float)vertices[8] - (float)vertices[2] };
  vec3 v_AX{ (float)pos_x       - (float)vertices[0], (float)pos_y       - (float)vertices[1], 0.0f               - (float)vertices[2] };
 
  vec3 b_coord = Barycentric( v_AB, v_AC, v_AX );
  return b_coord;
}