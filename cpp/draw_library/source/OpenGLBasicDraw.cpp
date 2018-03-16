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
#include <OpenGLFrameBuffer.h>

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
// qpaque shader
//---------------------------------------------------------------------


std::string opaque_sh_vert = R"(
#version 460

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

std::string opaque_sh_frag = R"(
#version 460

in TVertexData
{
    vec3 pos;
    vec4 col;
} in_data;

out vec4 frag_color;

void main()
{
    frag_color = in_data.col;
}
)";


//---------------------------------------------------------------------
// transparent shader
//---------------------------------------------------------------------


std::string transp_sh_vert = R"(
#version 460

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

std::string transp_sh_frag = R"(
#version 460

in TVertexData
{
    vec3 pos;
    vec4 col;
} in_data;

layout (location = 0) out vec4 transp_color;
layout (location = 1) out vec4 transp_attrib;

void main()
{                      
    float weight  = in_data.col.a * (1.0 - gl_FragCoord.z);
    transp_color  = vec4(in_data.col.rgb*weight, weight);
    transp_attrib = vec4(1.0, 0.0, 0.0, in_data.col.a);
}
)";


//---------------------------------------------------------------------
// finish shader
//---------------------------------------------------------------------


std::string finish_sh_vert = R"(
#version 460

layout (location = 0) in vec2 in_pos;

out TVertexData
{
    vec2 pos;
} out_data;

void main()
{
    out_data.pos = in_pos;
    gl_Position  = vec4(in_pos, 0.0, 1.0);
}
)";

std::string finish_sh_frag = R"(
#version 460

in TVertexData
{
    vec2 pos;
} in_data;

out vec4 frag_color;

layout (binding = 1) uniform sampler2D u_sampler_color;
layout (binding = 2) uniform sampler2D u_sampler_transp;
layout (binding = 3) uniform sampler2D u_sampler_transp_attr;

void main()
{
    vec2 tex_st = in_data.pos.xy * 0.5 + 0.5;    
    vec4 col    = texture(u_sampler_color, tex_st);
    vec4 transp = texture(u_sampler_transp, tex_st);
    vec4 t_attr = texture(u_sampler_transp_attr, tex_st);

    vec4 col_transp = vec4(0.0);
    if ( t_attr.x > 0.0 && t_attr.a > 0.0 )
    {
        vec3 colApprox      = transp.rgb / transp.a;
        float averageTransp = t_attr.w / t_attr.x;
        float alpha         = 1.0 - pow(1.0 - averageTransp, t_attr.x);
        col_transp          = vec4(colApprox, alpha); 
    }
    
    vec3 mix_col = mix(col.rgb, col_transp.rgb, col_transp.a);   
    frag_color   = vec4(mix_col.rgb, 1.0);
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
*********************************************************************/
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

  _opaque_prog.reset( nullptr );
  _transp_prog.reset( nullptr );
  _finish_prog.reset( nullptr );

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

  // specify render process
  SpecifyRenderProcess();

  // draw shader
  try
  {
    _opaque_prog.reset( new OpenGL::ShaderProgram(
      {
        { opaque_sh_vert, GL_VERTEX_SHADER },
        { opaque_sh_frag, GL_FRAGMENT_SHADER }
      } ) );
  }
  catch (...)
  {}

   // transparent shader
  try
  {
    _transp_prog.reset( new OpenGL::ShaderProgram(
      {
        { transp_sh_vert, GL_VERTEX_SHADER },
        { transp_sh_frag, GL_FRAGMENT_SHADER }
      } ) );
  }
  catch (...)
  {}

  // finish shader
  try
  {
    _finish_prog.reset( new OpenGL::ShaderProgram(
      {
        { finish_sh_vert, GL_VERTEX_SHADER },
        { finish_sh_frag, GL_FRAGMENT_SHADER }
      } ) );
  }
  catch (...)
  {}

  // TODO $$$ uniform block model, view, projection

  _initialized = true;
  return true;
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
void CBasicDraw::BackgroundColor( 
  const Render::TColor &bg_color ) //!< in: the background color
{
  _bg_color = bg_color;
  if ( _process != nullptr )
    _process->Invalidate();
}


/******************************************************************//**
* \brief   Specifies the render process
* 
* \author  gernot
* \date    2018-03-16
* \version 1.0
**********************************************************************/
bool CBasicDraw::SpecifyRenderProcess( void )
{
  if ( _vp_size[0] == 0 || _vp_size[1] == 0 )
    return false;
  if ( _process == nullptr )
    _process.reset( new OpenGL::CRenderProcess );
  if ( _process->IsValid() && _process->IsComplete() && _process->CurrentSize() == _vp_size )
    return true;

  const size_t c_depth_ID        = 0;
  const size_t c_color_ID        = 1;
  const size_t c_transp_ID       = 2;
  const size_t c_transp_attr_ID  = 3;
  
  Render::IRenderProcess::TBufferMap buffers;
  buffers.emplace( c_depth_ID,       Render::TBuffer( Render::TBufferType::DEPTH,  Render::TBufferDataType::DEFAULT ) );
  buffers.emplace( c_color_ID,       Render::TBuffer( Render::TBufferType::COLOR4, Render::TBufferDataType::DEFAULT ) );
  buffers.emplace( c_transp_ID,      Render::TBuffer( Render::TBufferType::COLOR4, Render::TBufferDataType::F16 ) );
  buffers.emplace( c_transp_attr_ID, Render::TBuffer( Render::TBufferType::COLOR4, Render::TBufferDataType::F16 ) );

  Render::IRenderProcess::TPassMap passes;

  Render::TPass opaque_pass( Render::TPassDepthTest::LESS, Render::TPassBlending::OFF );
  opaque_pass._targets.emplace_back( c_depth_ID, Render::TPass::TTarget::depth ); // depth target
  opaque_pass._targets.emplace_back( c_color_ID, 0, true, _bg_color );            // color target
  passes.emplace( c_opque_buffer, opaque_pass );

  Render::TPass transp_pass( Render::TPassDepthTest::LESS_READONLY, Render::TPassBlending::ADD );
  transp_pass._targets.emplace_back( c_depth_ID, Render::TPass::TTarget::depth ); // depth target
  transp_pass._targets.emplace_back( c_transp_ID, 0 );                            // tranparency target
  transp_pass._targets.emplace_back( c_transp_attr_ID, 1 );                       // tranparency attribute target (adaptive tranparency)
  passes.emplace( c_tranp_buffer, transp_pass );

  Render::TPass finish_pass( Render::TPassDepthTest::OFF, Render::TPassBlending::OFF );
  finish_pass._sources.emplace_back( c_color_ID,       1 ); // color buffer source
  finish_pass._sources.emplace_back( c_transp_ID,      2 ); // tranparency buffer source
  finish_pass._sources.emplace_back( c_transp_attr_ID, 3 ); // transparency attribute buffer source
  passes.emplace( c_finish_pass, finish_pass );

  _process->SpecifyBuffers( buffers );
  _process->SpecifyPasses( passes );

  return _process->Create( _vp_size );
}


/******************************************************************//**
* \brief   Draw full screen sapce.
* 
* \author  gernot
* \date    2018-03-16
* \version 1.0
**********************************************************************/
void CBasicDraw::DrawScereenspace( void )
{
  const std::vector<char>  bufferdescr = Render::IDrawBuffer::VADescription( Render::TVA::b0_xy );
  const std::vector<float> coords{ -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };

  Render::IDrawBuffer &buffer = DrawBuffer();
  buffer.SpecifyVA( bufferdescr.size(), bufferdescr.data() );
  buffer.UpdateVB( 0, sizeof(float), coords.size(), coords.data() );

  buffer.DrawArray( Render::TPrimitive::trianglestrip, 0, 4, true );
  buffer.Release();
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
bool CBasicDraw::Begin( void )
{
  if ( _drawing || Init() == false )
  {
    assert( false );
    return false;
  }

  // specify render process
  SpecifyRenderProcess();
  _process->PrepareClear( c_opque_buffer );
  _process->PrepareClear( c_tranp_buffer );
  _process->Release();
  
  // Init matrices
  _projection = OpenGL::Identity();
  _view       = OpenGL::Identity();
  _model      = OpenGL::Identity();

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

  _process->Prepare( c_finish_pass );
  _finish_prog->Use();
  DrawScereenspace();
  glUseProgram( 0 );

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

  _process->PrepareNoClear( c_opque_buffer );
  glClear( GL_DEPTH_BUFFER_BIT );
  _process->Release();

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

  // evaluate pass and shader
  bool    is_trasnsp  = color[3] < (254.5f/255.0f);
  size_t  pass_buffer = is_trasnsp ? c_tranp_buffer : c_opque_buffer;
  auto   *prog_ptr    = is_trasnsp ? _transp_prog.get() : _opaque_prog.get(); 

  // activate pass
  _process->PrepareNoClear( pass_buffer );

  // activate shader
  prog_ptr->Use();

  // setup uniforms
  // TODO $$$ automatic type selector, uniform type introspection 
  // { { "u_proj", _projection },{ "u_view", _view }, { "u_model", _model } } ;
  prog_ptr->SetUniformM44( "u_proj",  _projection );
  prog_ptr->SetUniformM44( "u_view",  _view );
  prog_ptr->SetUniformM44( "u_model", _model );

  // draw_buffer
  buffer.DrawArray( primitive_type, 0, no_of_vertices, true );
  buffer.Release();

  // set style and context
  if ( is_line )
  {
    glDisable( GL_POLYGON_OFFSET_FILL );
    glLineWidth( 1.0f );
  }

  // release shader
  glUseProgram( 0 );
  _process->Release();

  return true;
}


} // OpenGL