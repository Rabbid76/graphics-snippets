/******************************************************************//**
* \brief Implementation of OpenGL line renderer,
* with the use of "modern" OpenGL 4+ core and
* GLSL version 4.20 (`#version 420 core`) or
* OpenGL es 3(+) and GLSL ES version 3.00 (`#version 300 es`).
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/

#include <stdafx.h>

// includes

#include <OpenGLLine_core_and_es.h>
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


// TODO : comments
// The implementation is based on the OpenGL X.XX core mode specification [link] - line tipple etc.
// Line stipple specification
// TODO #version 420 core / #version 300 es

// TODO get rid of matrix stack

// TODO $$$ depth attenuation

const std::string CLineOpenGL_core_and_es::_line_vert_110 = R"(
#version 110

attribute vec4  attr_xyzw;
attribute float attr_y;

uniform int u_attr_case;

void main()
{
  vec4 vert_pos = u_attr_case == 0 ? attr_xyzw : vec4(attr_xyzw.x, attr_y, 0.0, 1.0);
  gl_Position   = gl_ModelViewProjectionMatrix * vert_pos;
  gl_ClipVertex = gl_Position;
}
)";


const std::string CLineOpenGL_core_and_es::_line_frag_110 = R"(
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
CLineOpenGL_core_and_es::CLineOpenGL_core_and_es( 
  Render::TModelAndViewPtr view_data,        //! I - view data provider (mode, view and projection)
  size_t                   min_cache_elems ) //! I - size of the element cache
  : _view_data( view_data )
  , _min_cache_elems( min_cache_elems )
  , _elem_cache( min_cache_elems, 0.0f )
{}


/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
CLineOpenGL_core_and_es::~CLineOpenGL_core_and_es()
{}


/******************************************************************//**
* \brief Trace error message.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
void CLineOpenGL_core_and_es::Error( 
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
void CLineOpenGL_core_and_es::Init( void )
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
  _line_case_loc        = glGetUniformLocation( (GLuint)_line_prog->ObjectHandle(), "u_attr_case" );
  _line_attrib_xyzw_inx = glGetAttribLocation(  (GLuint)_line_prog->ObjectHandle(), "attr_xyzw" );
  _line_attrib_y_inx    = glGetAttribLocation(  (GLuint)_line_prog->ObjectHandle(), "attr_y" );
}


/******************************************************************//**
* \brief Notify the render that a sequence of successive lines will
* follow, that is not interrupted by any other drawing operation.
* This allows the render to do some performance optimizations and to
* prepare for the line rendering.
* The render can keep states persistent from one line drawing to the
* other, without initializing and restoring them.  
* 
* \author  gernot
* \date    2018-09-12
* \version 1.0
**********************************************************************/
bool CLineOpenGL_core_and_es::StartSuccessiveLineDrawings( void )
{
  if ( _active_sequence )
  {
    assert( false );
    return false;
  }
  if ( _successive_drawing )
    return true;

  // activate the shader program
  _line_prog->Use();

  // initialize uniforms
  glUniform4fv( _line_color_loc, 1, _line_color.data() );
  glUniform1i(  _line_case_loc,  0 );

  // TODO $$$ buffer / vertex array object

  // enable and disable vertex attributes
  glEnableVertexAttribArray( _line_attrib_xyzw_inx );
  glDisableVertexAttribArray( _line_attrib_y_inx );

  _attribute_case     = 0;
  _successive_drawing = true;
  return true;
}


/******************************************************************//**
* \brief Notify the renderer that a sequence of lines has been
* finished, and that the internal states have to be restored.  
* 
* \author  gernot
* \date    2018-09-12
* \version 1.0
**********************************************************************/
bool CLineOpenGL_core_and_es::FinishSuccessiveLineDrawings( void )
{
  if ( _active_sequence )
  {
    assert( false );
    return false;
  }
  if ( _successive_drawing == false )
    return true;

  // TODO $$$ buffer / vertex array object

  // disable vertex attributes
  glDisableVertexAttribArray( _line_attrib_xyzw_inx );
  glDisableVertexAttribArray( _line_attrib_y_inx );

  // activate the shader program 0
  glUseProgram( 0 );

  _attribute_case     = 0;
  _successive_drawing = false;
  return true;
}


/******************************************************************//**
* \brief Set the stroke color of the line. 
* 
* \author  gernot
* \date    2018-09-12
* \version 1.0
**********************************************************************/
Render::Line::IRender & CLineOpenGL_core_and_es::SetColor( 
  const Render::TColor & color ) //!< in: new color
{ 
  _line_color = color; 

  if ( _successive_drawing )
    glUniform4fv( _line_color_loc, 1, _line_color.data() );
  
  return *this; 
}


/******************************************************************//**
* \brief Set the stroke color of the line. 
* 
* \author  gernot
* \date    2018-09-12
* \version 1.0
**********************************************************************/
Render::Line::IRender & CLineOpenGL_core_and_es::SetColor( 
  const Render::TColor8 & color ) //!< in: new color
{ 
  _line_color = Render::toColor( color );

  if ( _successive_drawing )
    glUniform4fv( _line_color_loc, 1, _line_color.data() );
  
  return *this; 
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
Render::Line::IRender & CLineOpenGL_core_and_es::SetStyle( 
  const Render::Line::TStyle & style )
{
  // TODO $$$ in sahder

  //! This is impossible, while an drawing sequence is active.  
  //! The only possible operations within a `glBegin`/`glEnd` sequence are those operations,
  //! which directly change fixed function attributes or specify a new vertex coordinate.  
  //! See [`glBegin`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glBegin.xml)
  if ( _active_sequence )
  {
    assert( false );
    return *this;
  }

  // TODO $$$ set depth attenuation

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
* \brief Draw a single line sequence.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CLineOpenGL_core_and_es::Draw( 
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

  // activate program, update uniforms and enable vertex attributes
  if ( _successive_drawing == false )
  {
    _line_prog->Use();
   
    glUniform4fv( _line_color_loc, 1, _line_color.data() );
    glUniform1i(  _line_case_loc,  0 );
    glEnableVertexAttribArray( _line_attrib_xyzw_inx );
    glDisableVertexAttribArray( _line_attrib_y_inx );
  }
  else if ( _attribute_case != 0 )
  {
    glUniform1i(  _line_case_loc,  0 );
    glDisableVertexAttribArray( _line_attrib_y_inx );
    _attribute_case = 0;
  }

  // set vertex attribute pointer and draw the line
  glVertexAttribPointer( _line_attrib_xyzw_inx, tuple_size, GL_FLOAT, GL_FALSE, 0, coords );
  glDrawArrays( OpenGL::Primitive( primitive_type ), 0, (GLsizei)(coords_size / tuple_size) );
  
  // disable vertex attributes and activate program 0
  if ( _successive_drawing == false )
  {
    glDisableVertexAttribArray( _line_attrib_xyzw_inx );
    glUseProgram( 0 );
  }

  return true;
}
 

/******************************************************************//**
* \brief Draw a single line sequence.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CLineOpenGL_core_and_es::Draw( 
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

  // activate program, update uniforms and enable vertex attributes
  if ( _successive_drawing == false )
  {
    _line_prog->Use();
   
    glUniform4fv( _line_color_loc, 1, _line_color.data() );
    glUniform1i(  _line_case_loc,  0 );
    glEnableVertexAttribArray( _line_attrib_xyzw_inx );
    glDisableVertexAttribArray( _line_attrib_y_inx );
  }
  else if ( _attribute_case != 0 )
  {
    glUniform1i(  _line_case_loc,  0 );
    glDisableVertexAttribArray( _line_attrib_y_inx );
    _attribute_case = 0;
  }

  // set vertex attribute pointer and draw the line
  glVertexAttribPointer( _line_attrib_xyzw_inx, tuple_size, GL_DOUBLE, GL_FALSE, 0, coords );
  glDrawArrays( OpenGL::Primitive( primitive_type ), 0, (GLsizei)(coords_size / tuple_size) );
 
  // disable vertex attributes and activate program 0
  if ( _successive_drawing == false )
  {
    glDisableVertexAttribArray( _line_attrib_xyzw_inx );
    glUseProgram( 0 );
  }

  return true;
}


/******************************************************************//**
* \brief Draw a single line sequence.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CLineOpenGL_core_and_es::Draw( 
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

  // activate program, update uniforms and enable vertex attributes
  if ( _successive_drawing == false )
  {
    _line_prog->Use();
   
    glUniform4fv( _line_color_loc, 1, _line_color.data() );
    glUniform1i(  _line_case_loc,  1 );
    glEnableVertexAttribArray( _line_attrib_xyzw_inx );
    glEnableVertexAttribArray( _line_attrib_y_inx );
  }
  else if ( _attribute_case == 0 )
  {
    glUniform1i(  _line_case_loc,  1 );
    glEnableVertexAttribArray( _line_attrib_y_inx );
    _attribute_case = 1;
  }

  // set vertex attribute pointers and draw the line
  glVertexAttribPointer( _line_attrib_xyzw_inx, 1, GL_FLOAT, GL_FALSE, 0, x_coords );
  glVertexAttribPointer( _line_attrib_y_inx, 1, GL_FLOAT, GL_FALSE, 0, y_coords );
  glDrawArrays( OpenGL::Primitive( primitive_type ), 0, (GLsizei)no_of_coords );
   
  // disable vertex attributes and activate program 0
  if ( _successive_drawing == false )
  {
    glDisableVertexAttribArray( _line_attrib_xyzw_inx );
    glDisableVertexAttribArray( _line_attrib_y_inx );
    glUseProgram( 0 );
  }

  return true;
}


/******************************************************************//**
* \brief Draw a single line sequence.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CLineOpenGL_core_and_es::Draw( 
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

  // activate program, update uniforms and enable vertex attributes
  if ( _successive_drawing == false )
  {
    _line_prog->Use();
   
    glUniform4fv( _line_color_loc, 1, _line_color.data() );
    glUniform1i(  _line_case_loc,  1 );
    glEnableVertexAttribArray( _line_attrib_xyzw_inx );
    glEnableVertexAttribArray( _line_attrib_y_inx );
  }
  else if ( _attribute_case == 0 )
  {
    glUniform1i(  _line_case_loc,  1 );
    glEnableVertexAttribArray( _line_attrib_y_inx );
    _attribute_case = 1;
  }

  // set vertex attribute pointers and draw the line
  glVertexAttribPointer( _line_attrib_xyzw_inx, 1, GL_DOUBLE, GL_FALSE, 0, x_coords );
  glVertexAttribPointer( _line_attrib_y_inx, 1, GL_DOUBLE, GL_FALSE, 0, y_coords );
  glDrawArrays( OpenGL::Primitive( primitive_type ), 0, (GLsizei)no_of_coords );

  // disable vertex attributes and activate program 0
  if ( _successive_drawing == false )
  {
    glDisableVertexAttribArray( _line_attrib_xyzw_inx );
    glDisableVertexAttribArray( _line_attrib_y_inx );
    glUseProgram( 0 );
  }

  return true;
}


/******************************************************************//**
* \brief Start a new line sequence.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CLineOpenGL_core_and_es::StartSequence( 
  Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - lines, line strip, line loop, lines adjacency or line strip adjacency 
  unsigned int       tuple_size )    //!< in: kind of the coordinates - 2: 2D (x, y), 3: 3D (x, y, z), 4: homogeneous (x, y, z, w)   
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
  _tuple_size      = tuple_size;
  return true;
}
  

/******************************************************************//**
* \brief Complete an active line sequence.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CLineOpenGL_core_and_es::EndSequence( void )
{
  // A sequence can't be completed if there is no active sequence
  if ( _active_sequence == false )
  {
    ASSERT( false );
    return false;
  }

  // draw the line

  // activate program, update uniforms and enable vertex attributes
  if ( _successive_drawing == false )
  {
    _line_prog->Use();
   
    glUniform4fv( _line_color_loc, 1, _line_color.data() );
    glUniform1i(  _line_case_loc,  0 );
    glEnableVertexAttribArray( _line_attrib_xyzw_inx );
    glDisableVertexAttribArray( _line_attrib_y_inx );
  }
  else if ( _attribute_case != 0 )
  {
    glUniform1i(  _line_case_loc,  0 );
    glDisableVertexAttribArray( _line_attrib_y_inx );
    _attribute_case = 0;
  }

  // TODO vertex array object

  //  set vertex attribute pointer and draw the line
  glVertexAttribPointer( _line_attrib_xyzw_inx, _tuple_size, GL_FLOAT, GL_FALSE, 0, _elem_cache.data() );
  glDrawArrays( OpenGL::Primitive( _squence_type ), 0, (GLsizei)(_sequence_size / _tuple_size) );
  
  // disable vertex attributes and activate program 0
  if ( _successive_drawing == false )
  {
    glDisableVertexAttribArray( _line_attrib_xyzw_inx );
    glUseProgram( 0 );
  }

  _active_sequence = false;
  _tuple_size      = 0;
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
bool CLineOpenGL_core_and_es::DrawSequence( 
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

  _elem_cache.data()[_sequence_size++] = x;
  _elem_cache.data()[_sequence_size++] = y;
  if ( _tuple_size >= 3 )
    _elem_cache.data()[_sequence_size++] = z;
  if ( _tuple_size == 4 )
    _elem_cache.data()[_sequence_size++] = 1.0f;

  return true;
}


/******************************************************************//**
* \brief Specify a new vertex coordinate in an active line sequence.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CLineOpenGL_core_and_es::DrawSequence( 
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

  _elem_cache.data()[_sequence_size++] = (float)x;
  _elem_cache.data()[_sequence_size++] = (float)y;
  if ( _tuple_size >= 3 )
    _elem_cache.data()[_sequence_size++] = (float)z;
  if ( _tuple_size == 4 )
    _elem_cache.data()[_sequence_size++] = 1.0f;

  return true;
}
  

/******************************************************************//**
* \brief Specify a sequence of new vertex coordinates in an active line sequence.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CLineOpenGL_core_and_es::DrawSequence( 
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
  if ( _elem_cache.size() < _sequence_size + coords_size )
    _elem_cache.resize( _elem_cache.size() + std::max(coords_size, _min_cache_elems) );

  // add the vertex coordinate to the cache

  std::memcpy( _elem_cache.data() + _sequence_size, coords, coords_size * sizeof( float ) );
  _sequence_size += coords_size;
  
  return true;
}


/******************************************************************//**
* \brief Specify a sequence of new vertex coordinates in an active line sequence.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CLineOpenGL_core_and_es::DrawSequence( 
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
  if ( _elem_cache.size() < _sequence_size + coords_size )
    _elem_cache.resize( _elem_cache.size() + std::max(coords_size, _min_cache_elems) );

  // add the vertex coordinate to the cache

  float *cache_ptr = _elem_cache.data() + _sequence_size;
  for ( const double *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr ++, cache_ptr ++ )
    *cache_ptr = (float)(*ptr);
  _sequence_size += coords_size;

  return true;
}


} // Line


} // OpenGL
