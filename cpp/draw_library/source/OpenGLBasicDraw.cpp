/******************************************************************//**
* \brief   Implementation og generic interface for basic darwing.
* 
* \author  gernot
* \date    2018-02-04
* \version 1.0
**********************************************************************/

// includes

#include <stdafx.h>

// OpenGL

#include <OpenGLBasicDraw.h>
#include <OpenGLVertexBuffer.h>

// OpenGL wrapper

#include <GL/glew.h>
//#include <GL/gl.h> not necessary because of glew 
#include <GL/glu.h>

// stl

#include <cassert>
#include <algorithm>


// class implementation

/******************************************************************//**
* \brief General OpenGL namespace
**********************************************************************/
namespace OpenGL
{


//---------------------------------------------------------------------
// Shader
//---------------------------------------------------------------------


std::string draw_sh_vert = R"(
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

std::string draw_sh_frag = R"(
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


//---------------------------------------------------------------------
// CBasicDraw
//---------------------------------------------------------------------


/******************************************************************//**
* @brief   ctor
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
CBasicDraw::CBasicDraw( void )
{}


/******************************************************************//**
* @brief   dtor
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
CBasicDraw::~CBasicDraw()
{
  Destroy();
}


/******************************************************************//**
* \brief   Create new or retrun existion default draw buffer
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
Render::IDrawBuffer & CBasicDraw::DrawBuffer( void )
{
  bool cached = false;
  return DrawBuffer( nullptr, cached );
}


/******************************************************************//**
* \brief   Create new or retrun existion default draw buffer
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
Render::IDrawBuffer & CBasicDraw::DrawBuffer( 
  const void *key,     //!< I - key for the temporary cache
  bool       &cached ) //!< O - object was found in the cache
{
  if ( key != nullptr )
  {
    // try to find temporary buffer which already has the data
    auto keyIt = std::find( _buffer_keys.begin(), _buffer_keys.end(), key );
    if ( keyIt != _buffer_keys.end() )
    {
      size_t inx = keyIt - _buffer_keys.begin();
      if ( _nextBufferI == inx )
        _nextBufferI = _nextBufferI < _max_buffers - 1 ? _nextBufferI + 1 : 0;
      
      Render::IDrawBuffer *foundBuffer = _draw_buffers[inx];
      cached = true;
      return *foundBuffer;
    }
  }

  // ensure the buffer object is allocated
  if ( _draw_buffers[_nextBufferI] == nullptr )
    _draw_buffers[_nextBufferI]= NewDrawBuffer( Render::TDrawBufferUsage::stream_draw );
  
  // get buffer object
  Render::IDrawBuffer *currentBuffer = _draw_buffers[_nextBufferI];
  _buffer_keys[_nextBufferI] = key;
  _nextBufferI = _nextBufferI < _max_buffers - 1 ? _nextBufferI + 1 : 0;

  cached = false;
  return *currentBuffer;
}


/******************************************************************//**
* \brief   Create a new and empty draw buffer object.
* 
* \author  gernot
* \date    2017-11-26
* \version 1.0
**********************************************************************/
Render::IDrawBuffer * CBasicDraw::NewDrawBuffer( 
  Render::TDrawBufferUsage usage ) //!< I - usage of draw buffer : static_draw, dynamic_draw or stream_draw
{
  return new OpenGL::CDrawBuffer( usage, 1024 ); 
}


/******************************************************************//**
* \brief   General initializations.
*
* Specify the render buffers
* 
* \author  gernot
* \date    2018-03-16
* \version 1.0
**********************************************************************/
bool CBasicDraw::Init( void )
{
  if ( _initialized )
    return true;

  _draw_prog.reset( new OpenGL::ShaderProgram(
    {
      { draw_sh_vert, GL_VERTEX_SHADER },
      { draw_sh_frag, GL_FRAGMENT_SHADER }
    } ) );

  // TODO $$$ render process
  // TODO $$$ shaders
  // TODO $$$ uniform block model, view, projection

  _initialized = true;
  return true;
}


/******************************************************************//**
* @brief   destroy internal GPU objects
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
void CBasicDraw::Destroy( void )
{
  _initialized = false;
  _drawing     = false;

  _draw_prog.reset( nullptr );

  for ( auto & buffer : _draw_buffers )
  {
    delete buffer;
    buffer = nullptr;
  }
  for ( auto & key : _buffer_keys )
    key = nullptr;
  _nextBufferI = 0;
}


/******************************************************************//**
* \brief   Start the rendering.
*
* Specify the render buffers
* 
* \author  gernot
* \date    2018-03-16
* \version 1.0
**********************************************************************/
bool CBasicDraw::Begin( 
  const Render::TColor &background_color ) //!< in clear color for the background
{
  if ( _drawing || Init() == false )
  {
    assert( false );
    return false;
  }

  // Init matrices
  _projection = OpenGL::Identity();
  _view       = OpenGL::Identity();
  _model      = OpenGL::Identity();

  // TODO $$$

  glClearDepth( 1.0f );
  glClearColor( background_color[0], background_color[1], background_color[2], background_color[3] );  
  glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

  glEnable( GL_DEPTH_TEST );
  glEnable( GL_BLEND );
  glBlendFunci( 0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  _drawing = true;
  return true;
}


/******************************************************************//**
* \brief   Finish the rendering.
*
* Finally write to the default frame buffer.
* 
* \author  gernot
* \date    2018-03-16
* \version 1.0
**********************************************************************/
bool CBasicDraw::Finish( void )
{
  if ( _drawing == false )
  {
    assert( false );
    return false;
  }

  // TODO $$$

  _drawing = false;
  return true;
}


/******************************************************************//**
* \brief   Interim clear of the depth buffer.
* 
* \author  gernot
* \date    2018-03-16
* \version 1.0
**********************************************************************/
bool CBasicDraw::ClearDepth( void )
{
  if ( _drawing == false )
  {
    assert( false );
    return false;
  }

  // TODO $$$
  glClearDepth( 1.0f );
  glClear( GL_DEPTH_BUFFER_BIT );

  return true;
}


/******************************************************************//**
* \brief   Draw an array od primitives with a single color
* 
* \author  gernot
* \date    2018-03-15
* \version 1.0
**********************************************************************/
bool CBasicDraw::Draw( 
  Render::TPrimitive    primitive_type, //!< in: type of the primitives
  size_t                size,           //!< in: size vertex coordiantes
  size_t                coords_size,    //!< in: size of coordinate buffer
  const Render::t_fp   *coords,         //!< in: coordiant buffer
  const Render::TColor &color,          //!< in: color for drawing
  const TStyle         &style )         //!< in: additional style parameters 
{
  if ( _drawing == false )
  {
    assert( false );
    return false;
  }

  if ( size != 2 && size !=3 && size !=4 )
  {
    assert( false );
    return false;
  }

  bool is_point = primitive_type == Render::TPrimitive::points;

  bool is_line =
    primitive_type == Render::TPrimitive::lines ||
    primitive_type == Render::TPrimitive::lines_adjacency ||
    primitive_type == Render::TPrimitive::linestrip ||
    primitive_type == Render::TPrimitive::linestrip_adjacency ||
    primitive_type == Render::TPrimitive::lineloop;

  bool is_polygon =
    primitive_type == Render::TPrimitive::triangles ||
    primitive_type == Render::TPrimitive::triangle_adjacency ||
    primitive_type == Render::TPrimitive::trianglestrip ||
    primitive_type == Render::TPrimitive::trianglestrip_adjacency ||
    primitive_type == Render::TPrimitive::trianglefan;

  // buffer specification
  Render::TVA va_id = size == 2 ? Render::TVA::b0_xy__b1_rgba : (size == 3 ? Render::TVA::b0_xyz__b1_rgba : Render::TVA::b0_xyzw__b1_rgba);
  const std::vector<char> bufferdescr = Render::IDrawBuffer::VADescription( va_id );

  // color buffer
  size_t no_of_vertices = coords_size / size;
  std::vector<Render::t_fp> color_buffer( no_of_vertices * 4 );
  for ( size_t i=0; i < no_of_vertices; ++ i )
    std::copy( color.begin(), color.end(), color_buffer.begin() + i * 4 );

  // create buffer
  Render::IDrawBuffer &buffer = DrawBuffer();
  buffer.SpecifyVA( bufferdescr.size(), bufferdescr.data() );
  buffer.UpdateVB( 0, sizeof(float), coords_size, coords );
  buffer.UpdateVB( 1, sizeof(float), color_buffer );

  // set style and context
  if ( is_line )
  {
    glEnable( GL_POLYGON_OFFSET_FILL );
    glPolygonOffset( 1.0, 1.0 );
    glLineWidth( style._thickness );
  }

  // set sahder
  _draw_prog->Use();

  // setup uniforms
  // TODO $$$ automatic type selector, uniform type introspection 
  // { { "u_proj", _projection },{ "u_view", _view }, { "u_model", _model } } ;
  _draw_prog->SetUniformM44( "u_proj",  _projection );
  _draw_prog->SetUniformM44( "u_view",  _view );
  _draw_prog->SetUniformM44( "u_model", _model );

  // draw_buffer
  buffer.DrawArray( primitive_type, 0, no_of_vertices, true );
  buffer.Release();

  // set style and context
  if ( is_line )
  {
    glDisable( GL_POLYGON_OFFSET_FILL );
    glLineWidth( 1.0f );
  }

  return true;
}


} // OpenGL