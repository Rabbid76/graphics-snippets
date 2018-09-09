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


const std::string CLineOpenGL_2_00::_line_vert_110 = R"(
#version 110

void main()
{
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
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

const std::string _line_stipple_vert_110;     // TODO $$$
const std::string _line_stipple_x_y_vert_110; // TODO $$$
const std::string _line_stipple_frag_110;     // TODO $$$


/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
CLineOpenGL_2_00::CLineOpenGL_2_00( void )
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
  _line_color_loc = glGetUniformLocation( (GLuint)_line_prog->ObjectHandle(), "u_color" );

  _line_x_y_prog = std::make_shared<CShaderProgram>();
  *_line_x_y_prog << _line_x_y_vert_shader << _line_frag_shader;
  _line_x_y_prog->Link();
  if ( _line_x_y_prog->Verify( msg ) == false )
    Error( "link error", msg );
  _line_x_y_color_loc = glGetUniformLocation( (GLuint)_line_x_y_prog->ObjectHandle(), "u_color" );
  _line_x_attrib_inx  = glGetAttribLocation( (GLuint)_line_x_y_prog->ObjectHandle(), "attr_x" );
  _line_y_attrib_inx  = glGetAttribLocation( (GLuint)_line_x_y_prog->ObjectHandle(), "attr_y" );

  // TODO $$$
}


/******************************************************************//**
* \brief Change the current line color, for the pending line drawing
* instructions
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
Render::Line::IRender & CLineOpenGL_2_00::SetColor(
  const Render::TColor & color ) //!< I - the new color
{
  // change the vertex color
  glColor4fv( color.data() );

  return *this;
}


/******************************************************************//**
* \brief Change the current line color, for the pending line drawing
* instructions
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
Render::Line::IRender & CLineOpenGL_2_00::SetColor( 
  const Render::TColor8 & color ) //!< I - the new color
{
  // change the vertex color
  glColor4ubv( color.data() );

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

  // start `glBegin` / `glEnd` sequence
  glBegin( OpenGL::Primitive(primitive_type) );

  // draw the line sequence
  if ( tuple_size == 2 )
  {
    for ( const float *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 2 )
      glVertex2fv( ptr );
  }
  else if ( tuple_size == 3 )
  {
    for ( const float *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 3 )
      glVertex3fv( ptr );
  }
  else if ( tuple_size == 4 )
  {
    for ( const float *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 4 )
      glVertex4fv( ptr );
  }

  // complete sequence
  glEnd();

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

  // start `glBegin` / `glEnd` sequence
  glBegin( OpenGL::Primitive(primitive_type) );

  // draw the line sequence
  if ( tuple_size == 2 )
  {
    for ( const double *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 2 )
      glVertex2dv( ptr );
  }
  else if ( tuple_size == 3 )
  {
    for ( const double *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 3 )
      glVertex3dv( ptr );
  }
  else if ( tuple_size == 4 )
  {
    for ( const double *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 4 )
      glVertex4dv( ptr );
  }

  // complete sequence
  glEnd();

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
  
  // start `glBegin` / `glEnd` sequence
  glBegin( OpenGL::Primitive(primitive_type) );

  // draw the line sequence
  for ( size_t i = 0; i < no_of_coords; ++ i )
    glVertex2f( x_coords[i], y_coords[i] );

  // complete sequence
  glEnd();

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
  
  // start `glBegin` / `glEnd` sequence
  glBegin( OpenGL::Primitive(primitive_type) );

  // draw the line sequence
  for ( size_t i = 0; i < no_of_coords; ++ i )
    glVertex2d( x_coords[i], y_coords[i] );

  // complete sequence
  glEnd();

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

  // start `glBegin` / `glEnd` sequence
  glBegin( OpenGL::Primitive(primitive_type) );

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
 
  _active_sequence = false;

  // complete sequence
  glEnd();

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

  // specify the vertex coordinate
  glVertex3f( x, y, z );

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

  // specify the vertex coordinate
  glVertex3d( x, y, z );

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

  // draw the line sequence
  if ( tuple_size == 2 )
  {
    for ( const float *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 2 )
      glVertex2fv( ptr );
  }
  else if ( tuple_size == 3 )
  {
    for ( const float *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 3 )
      glVertex3fv( ptr );
  }
  else if ( tuple_size == 4 )
  {
    for ( const float *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 4 )
      glVertex4fv( ptr );
  }

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

  // draw the line sequence
  if ( tuple_size == 2 )
  {
    for ( const double *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 2 )
      glVertex2dv( ptr );
  }
  else if ( tuple_size == 3 )
  {
    for ( const double *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 3 )
      glVertex3dv( ptr );
  }
  else if ( tuple_size == 4 )
  {
    for ( const double *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 4 )
      glVertex4dv( ptr );
  }

  return true;
}


} // Line


} // OpenGL