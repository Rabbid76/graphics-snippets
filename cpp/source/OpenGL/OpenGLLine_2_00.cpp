/******************************************************************//**
* \brief Implementation of OpenGL line renderer,
* with the use of very simple and highly optimized shaders,
* for OpenGL version 2.00 and GLSL version 1.10 (`#version 110`).
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/

#include <stdafx.h>

// includes

#include <OpenGLLine_2_0.h>
#include <OpenGLProgram.h>


// OpenGL wrapper

#include <OpenGL_include.h>
#include <OpenGL_enumconst.h>


// STL

#include <iostream>
#include <algorithm>


// class definitions


/******************************************************************//**
* \brief General namespace for OpenGL implementation.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
namespace OpenGL
{


/******************************************************************//**
* \brief Namespace for drawing lines with the use of OpenGL.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
namespace Line
{



/*

Evaluate if it would be even more efficient, to combine the vertex shader `_line_vert_110` and `_line_x_y_vert_110`
and to switch between the different vertex attribute representations by uniform conditions.
Since the vertex shader is executed per if vertex only, one additional uniform condition should not gain a notifiable performance loss.
Note, with a modern hardware, a uniform condition would be evaluated once and different code dependent on the uniform would be generated and executed.
But this would give the chance to avoid shader program switches, which probably are more time consuming operations. 

*/

const std::string CLineOpenGL_2_00::_line_vert_110 = R"(
#version 110

attribute vec4 attr_vert;

void main()
{
  gl_Position = gl_ModelViewProjectionMatrix * attr_vert;
}
)";

const std::string CLineOpenGL_2_00::_line_x_y_vert_110 = R"(
#version 110

attribute float attr_x;
attribute float attr_y;

void main()
{
  gl_Position = gl_ModelViewProjectionMatrix * vec4(attr_x, attr_y, 0.0, 1.0);
}
)";

const std::string CLineOpenGL_2_00::_line_frag_110 = R"(
#version 110

uniform vec4 u_color;

void main()
{
  gl_FragColor = u_color;
}
)";


/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
CLineOpenGL_2_00::CLineOpenGL_2_00( 
  size_t min_cache_elems ) //! I - size of the element cache
  : _min_cache_elems( min_cache_elems )
  , _elem_cache( min_cache_elems, 0.0f )
{}


/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
CLineOpenGL_2_00::~CLineOpenGL_2_00()
{}


/******************************************************************//**
* \brief Trace error message.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
void CLineOpenGL_2_00::Error( 
  const std::string &kind,     //!< in: message kind
  const std::string &message ) //!< in: error message
{
  std::cout << kind << ": " << message;
}


/******************************************************************//**
* \brief Initialize the line renderer. 
*
* For the initialization a current and valid OpenGL context is required. 
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
void CLineOpenGL_2_00::Init( void )
{
  if ( _initilized )
    return;
  _initilized = true;

  std::string msg;


  // compile shader objects

  auto _line_vert_shader = std::make_shared<CShaderObject>( Render::Program::TShaderType::vertex );
  *_line_vert_shader << _line_vert_110;
  _line_vert_shader->Compile();
  if ( _line_vert_shader->Verify( msg ) == false )
    Error( "compile error", msg );

  auto _line_x_y_vert_shader = std::make_shared<CShaderObject>( Render::Program::TShaderType::vertex );
  *_line_x_y_vert_shader << _line_x_y_vert_110;
   _line_x_y_vert_shader->Compile();
  if ( _line_x_y_vert_shader->Verify( msg ) == false )
    Error( "compile error", msg );

  auto _line_frag_shader = std::make_shared<CShaderObject>( Render::Program::TShaderType::fragment );
  *_line_frag_shader << _line_frag_110;
   _line_frag_shader->Compile();
  if ( _line_frag_shader->Verify( msg ) == false )
    Error( "compile error", msg );


  // link shader programs

  _line_prog = std::make_shared<CShaderProgram>();
  *_line_prog << _line_vert_shader << _line_frag_shader;
  _line_prog->Link();
  if ( _line_prog->Verify( msg ) == false )
    Error( "link error", msg );
  _line_color_loc       = glGetUniformLocation( (GLuint)_line_prog->ObjectHandle(), "u_color" );
  _line_vert_attrib_inx = glGetAttribLocation( (GLuint)_line_prog->ObjectHandle(), "attr_vert" );

  _line_x_y_prog = std::make_shared<CShaderProgram>();
  *_line_x_y_prog << _line_x_y_vert_shader << _line_frag_shader;
  _line_x_y_prog->Link();
  if ( _line_x_y_prog->Verify( msg ) == false )
    Error( "link error", msg );
  _line_x_y_color_loc = glGetUniformLocation( (GLuint)_line_x_y_prog->ObjectHandle(), "u_color" );
  _line_x_attrib_inx  = glGetAttribLocation( (GLuint)_line_x_y_prog->ObjectHandle(), "attr_x" );
  _line_y_attrib_inx  = glGetAttribLocation( (GLuint)_line_x_y_prog->ObjectHandle(), "attr_y" );
}


/******************************************************************//**
* \brief Change the current line with and the current line pattern,
* for the pending line drawing instructions.
*
* The line width and line pattern can't be changed within a drawing
* sequence. 
* 
* \author  gernot
* \date    2018-09-09
* \version 1.0
**********************************************************************/
Render::Line::IRender & CLineOpenGL_2_00::SetStyle( 
  const Render::Line::TStyle & style )
{
  //! This is impossible, while an drawing sequence is active.  
  //! The only possible operations within a `glBegin`/`glEnd` sequence are those operations,
  //! which directly change fixed function attributes or specify a new vertex coordinate.  
  //! See [`glBegin`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glBegin.xml)
  if ( _active_sequence )
  {
    assert( false );
    return *this;
  }

  _line_style = style;

  // set line width
  glLineWidth( style._width );

  // activate line stipple
  if ( style._stipple_type == 1 )
  {
    glDisable( GL_LINE_STIPPLE );
    return *this;
  }
  glEnable( GL_LINE_STIPPLE );

  // set stipple pattern
  static const std::array<GLushort, 8> patterns
  {
    0x0000, 0xFFFF, 0x5555, 0x3333, 0x0F0F, 0x00FF, 0x7D7D, 0x7FFD
  };
  GLushort pattern = style._stipple_type >= 0 && style._stipple_type < patterns.size() ? patterns[style._stipple_type] : 0xFFFF;
  glLineStipple( (GLint)(style._width + 0.5f), pattern );

  return *this;
}


/******************************************************************//**
* \brief TODO: not yet implemented!  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
Render::Line::IRender & CLineOpenGL_2_00::SetArrowStyle( 
  const Render::Line::TArrowStyle & style ) //! I - new style of the arrows at the endings of coherently lines
{
  assert( false );
  return *this;
}


/******************************************************************//**
* \brief Draw a single line sequence.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CLineOpenGL_2_00::Draw( 
  Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - lines, line strip, line loop, lines adjacency or line strip adjacency  
  unsigned int       tuple_size,     //!< in: kind of the coordinates - 2: 2D (x, y), 3: 3D (x, y, z), 4: homogeneous (x, y, z, w)   
  size_t             coords_size,    //!< in: number of elements (size) of the coordinate array - `coords_size` = `tuple_size` * "number of coordinates" 
  const float       *coords )        //!< in: pointer to an array of the vertex coordinates
{ 
  // A new sequence can't be started within an active sequence
  if ( _active_sequence )
  {
    ASSERT( false );
    return false;
  }
  ASSERT( Render::BasePrimitive(primitive_type) == Render::TBasePrimitive::line );
  ASSERT( tuple_size == 2 || tuple_size == 3 || tuple_size == 4 );

  _line_prog->Use();
  glUniform4fv( _line_x_y_color_loc, 1, _line_color.data() );

  glVertexAttribPointer( _line_vert_attrib_inx, tuple_size, GL_FLOAT, GL_FALSE, 0, coords );
  glEnableVertexAttribArray( _line_vert_attrib_inx );
  glDrawArrays( OpenGL::Primitive( primitive_type ), 0, (GLsizei)(coords_size / tuple_size) );
  glDisableVertexAttribArray( _line_vert_attrib_inx );
  
  glUseProgram( 0 );

  return true;
}
 

/******************************************************************//**
* \brief Draw a single line sequence.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CLineOpenGL_2_00::Draw( 
  Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - lines, line strip, line loop, lines adjacency or line strip adjacency  
  unsigned int       tuple_size,     //!< in: kind of the coordinates - 2: 2D (x, y), 3: 3D (x, y, z), 4: homogeneous (x, y, z, w)   
  size_t             coords_size,    //!< in: number of elements (size) of the coordinate array - `coords_size` = `tuple_size` * "number of coordinates" 
  const double      *coords )        //!< in: pointer to an array of the vertex coordinates
{
   // A new sequence can't be started within an active sequence
  if ( _active_sequence )
  {
    ASSERT( false );
    return false;
  }
  ASSERT( Render::BasePrimitive(primitive_type) == Render::TBasePrimitive::line );
  ASSERT( tuple_size == 2 || tuple_size == 3 || tuple_size == 4 );

  _line_prog->Use();
  glUniform4fv( _line_x_y_color_loc, 1, _line_color.data() );

  glVertexAttribPointer( _line_vert_attrib_inx, tuple_size, GL_DOUBLE, GL_FALSE, 0, coords );
  glEnableVertexAttribArray( _line_vert_attrib_inx );
  glDrawArrays( OpenGL::Primitive( primitive_type ), 0, (GLsizei)(coords_size / tuple_size) );
  glDisableVertexAttribArray( _line_vert_attrib_inx );
  
  glUseProgram( 0 );

  return true;
}


/******************************************************************//**
* \brief Draw a single line sequence.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CLineOpenGL_2_00::Draw( 
  Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - lines, line strip, line loop, lines adjacency or line strip adjacency 
  size_t             no_of_coords,   //!< in: number of coordinates and number of elements (size) of the coordinate array
  const float       *x_coords,       //!< int pointer to an array of the x coordinates
  const float       *y_coords )      //!< int pointer to an array of the y coordinates
{
  // A new sequence can't be started within an active sequence
  if ( _active_sequence )
  {
    ASSERT( false );
    return false;
  }
  ASSERT( Render::BasePrimitive(primitive_type) == Render::TBasePrimitive::line );
  ASSERT( _line_x_y_prog != nullptr );

  _line_x_y_prog->Use();
  glUniform4fv( _line_x_y_color_loc, 1, _line_color.data() );

  glVertexAttribPointer( _line_x_attrib_inx, 1, GL_FLOAT, GL_FALSE, 0, x_coords );
  glVertexAttribPointer( _line_y_attrib_inx, 1, GL_FLOAT, GL_FALSE, 0, y_coords );
  glEnableVertexAttribArray( _line_x_attrib_inx );
  glEnableVertexAttribArray( _line_y_attrib_inx );
  glDrawArrays( OpenGL::Primitive( primitive_type ), 0, (GLsizei)no_of_coords );
  glDisableVertexAttribArray( _line_x_attrib_inx );
  glDisableVertexAttribArray( _line_y_attrib_inx );

  glUseProgram( 0 );

  return true;
}


/******************************************************************//**
* \brief Draw a single line sequence.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CLineOpenGL_2_00::Draw( 
  Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - lines, line strip, line loop, lines adjacency or line strip adjacency 
  size_t             no_of_coords,   //!< in: number of coordinates and number of elements (size) of the coordinate array
  const double      *x_coords,       //!< int pointer to an array of the x coordinates
  const double      *y_coords )      //!< int pointer to an array of the y coordinates
{
  // A new sequence can't be started within an active sequence
  if ( _active_sequence )
  {
    ASSERT( false );
    return false;
  }
  ASSERT( Render::BasePrimitive(primitive_type) == Render::TBasePrimitive::line );
  
  // TODO $$$ evaluate line stippling

  ASSERT( _line_x_y_prog != nullptr );

  _line_x_y_prog->Use();
  glUniform4fv( _line_x_y_color_loc, 1, _line_color.data() );

  glVertexAttribPointer( _line_x_attrib_inx, 1, GL_DOUBLE, GL_FALSE, 0, x_coords );
  glVertexAttribPointer( _line_y_attrib_inx, 1, GL_DOUBLE, GL_FALSE, 0, y_coords );
  glEnableVertexAttribArray( _line_x_attrib_inx );
  glEnableVertexAttribArray( _line_y_attrib_inx );
  glDrawArrays( OpenGL::Primitive( primitive_type ), 0, (GLsizei)no_of_coords );
  glDisableVertexAttribArray( _line_x_attrib_inx );
  glDisableVertexAttribArray( _line_y_attrib_inx );

  glUseProgram( 0 );

  return true;
}


/******************************************************************//**
* \brief Start a new line sequence.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CLineOpenGL_2_00::StartSequence( 
  Render::TPrimitive primitive_type ) //!< in: primitive type of the coordinates - lines, line strip, line loop, lines adjacency or line strip adjacency 
{
  // A new sequence can't be started within an active sequence
  if ( _active_sequence )
  {
    ASSERT( false );
    return false;
  }
  ASSERT( Render::BasePrimitive(primitive_type) == Render::TBasePrimitive::line );
  
  _active_sequence = true;
  _squence_type    = primitive_type;
  return true;
}
  

/******************************************************************//**
* \brief Complete an active line sequence.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CLineOpenGL_2_00::EndSequence( void )
{
  // A sequence can't be completed if there is no active sequence
  if ( _active_sequence == false )
  {
    ASSERT( false );
    return false;
  }

  // draw the line

  _line_prog->Use();
  glUniform4fv( _line_x_y_color_loc, 1, _line_color.data() );

  glVertexAttribPointer( _line_vert_attrib_inx, 3, GL_FLOAT, GL_FALSE, 0, _elem_cache.data() );
  glEnableVertexAttribArray( _line_vert_attrib_inx );
  glDrawArrays( OpenGL::Primitive( _squence_type ), 0, (GLsizei)(_sequence_size / 3) );
  glDisableVertexAttribArray( _line_vert_attrib_inx );
  
  glUseProgram( 0 );

  _active_sequence = false;
  _sequence_size   = 0;

  return true;
}


/******************************************************************//**
* \brief Specify a new vertex coordinate in an active line sequence.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CLineOpenGL_2_00::DrawSequence( 
  float x,  //!< in: x coordinate
  float y,  //!< in: y coordinate
  float z ) //!< in: z coordinate
{
  // A sequence has to be active, to specify a new vertex coordinate
  if ( _active_sequence == false )
  {
    ASSERT( false );
    return false;
  }

  // reserve the cache
  if ( _elem_cache.size() < _sequence_size + 3 )
    _elem_cache.resize( _elem_cache.size() + std::max((size_t)3, _min_cache_elems) );

  // add the vertex coordinate to the cache

  _elem_cache.data()[_sequence_size + 0] = x;
  _elem_cache.data()[_sequence_size + 1] = y;
  _elem_cache.data()[_sequence_size + 2] = z;
  _sequence_size += 3;

  return true;
}


/******************************************************************//**
* \brief Specify a new vertex coordinate in an active line sequence.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CLineOpenGL_2_00::DrawSequence( 
  double x,  //!< in: x coordinate
  double y,  //!< in: y coordinate
  double z ) //!< in: z coordinate
{
  // A sequence has to be active, to specify a new vertex coordinate
  if ( _active_sequence == false )
  {
    ASSERT( false );
    return false;
  }

  // reserve the cache
  if ( _elem_cache.size() < _sequence_size + 3 )
    _elem_cache.resize( _elem_cache.size() + std::max((size_t)3, _min_cache_elems) );

  // add the vertex coordinate to the cache

  _elem_cache.data()[_sequence_size + 0] = (float)x;
  _elem_cache.data()[_sequence_size + 1] = (float)y;
  _elem_cache.data()[_sequence_size + 2] = (float)z;
  _sequence_size += 3;

  return true;
}
  

/******************************************************************//**
* \brief Specify a sequence of new vertex coordinates in an active line sequence.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CLineOpenGL_2_00::DrawSequence( 
  unsigned int       tuple_size,  //!< in: kind of the coordinates - 2: 2D (x, y), 3: 3D (x, y, z), 4: homogeneous (x, y, z, w)   
  size_t             coords_size, //!< in: number of elements (size) of the coordinate array - `coords_size` = `tuple_size` * "number of coordinates" 
  const float       *coords )     //!< in: pointer to an array of the vertex coordinates
{
  // A sequence has to be active, to specify new vertex coordinates
  if ( _active_sequence == false )
  {
    ASSERT( false );
    return false;
  }

  // reserve the cache
  size_t increment = coords_size * 3 / tuple_size;
  if ( _elem_cache.size() < _sequence_size + increment )
    _elem_cache.resize( _elem_cache.size() + std::max(increment, _min_cache_elems) );

  // add the vertex coordinate to the cache

  if ( tuple_size == 2 )
  {
    float *cache_ptr = _elem_cache.data() + _sequence_size;
    for ( const float *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 2, cache_ptr += 3 )
    {
      cache_ptr[0] = ptr[0];
      cache_ptr[1] = ptr[1];
    }
  }
  else if ( tuple_size == 3 )
  {
    std::memcpy( _elem_cache.data() + _sequence_size, coords, increment * sizeof( float ) );
  }
  else if ( tuple_size == 4 )
  {
    float *cache_ptr = _elem_cache.data() + _sequence_size;
    for ( const float *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 4, cache_ptr += 3 )
    {
      cache_ptr[0] = ptr[0] / ptr[3];
      cache_ptr[1] = ptr[1] / ptr[3];
      cache_ptr[2] = ptr[2] / ptr[3];
    }
  }
  _sequence_size += increment;
  
  return true;
}


/******************************************************************//**
* \brief Specify a sequence of new vertex coordinates in an active line sequence.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CLineOpenGL_2_00::DrawSequence( 
  unsigned int       tuple_size,  //!< in: kind of the coordinates - 2: 2D (x, y), 3: 3D (x, y, z), 4: homogeneous (x, y, z, w)   
  size_t             coords_size, //!< in: number of elements (size) of the coordinate array - `coords_size` = `tuple_size` * "number of coordinates" 
  const double      *coords )     //!< in: pointer to an array of the vertex coordinates
{
  // A sequence has to be active, to specify new vertex coordinates
  if ( _active_sequence == false )
  {
    ASSERT( false );
    return false;
  }

  // reserve the cache
  size_t increment = coords_size * 3 / tuple_size;
  if ( _elem_cache.size() < _sequence_size + increment )
    _elem_cache.resize( _elem_cache.size() + std::max(increment, _min_cache_elems) );

  // add the vertex coordinate to the cache

  if ( tuple_size == 2 )
  {
    float *cache_ptr = _elem_cache.data() + _sequence_size;
    for ( const double *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 2, cache_ptr += 3 )
    {
      cache_ptr[0] = (float)ptr[0];
      cache_ptr[1] = (float)ptr[1];
    }
  }
  else if ( tuple_size == 3 )
  {
    float *cache_ptr = _elem_cache.data() + _sequence_size;
    for ( const double *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 3, cache_ptr += 3 )
    {
      cache_ptr[0] = (float)ptr[0];
      cache_ptr[1] = (float)ptr[1];
      cache_ptr[2] = (float)ptr[2];
    }
  }
  else if ( tuple_size == 4 )
  {
    float *cache_ptr = _elem_cache.data() + _sequence_size;
    for ( const double *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 4, cache_ptr += 3 )
    {
      cache_ptr[0] = (float)(ptr[0] / ptr[3]);
      cache_ptr[1] = (float)(ptr[1] / ptr[3]);
      cache_ptr[2] = (float)(ptr[2] / ptr[3]);
    }
  }
  _sequence_size += increment;

  return true;
}


} // Line


} // OpenGL