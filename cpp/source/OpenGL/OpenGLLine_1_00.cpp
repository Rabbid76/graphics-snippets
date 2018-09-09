/******************************************************************//**
* \brief Implementation of OpenGL line renderer,
* for OpenGL version 1.00 - "Software-OpenGL".
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/

#include <stdafx.h>

// includes

#include <OpenGLLine_1_0.h>


// OpenGL wrapper

#include <OpenGL_include.h>
#include <OpenGL_enumconst.h>

// STL

#include <cassert>


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


/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
CLineOpenGL_1_00::CLineOpenGL_1_00( void )
{}


/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
CLineOpenGL_1_00::~CLineOpenGL_1_00()
{}


/******************************************************************//**
* \brief Change the current line color, for the pending line drawing
* instructions
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
CLineOpenGL_1_00 & CLineOpenGL_1_00::SetColor(
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
CLineOpenGL_1_00 & CLineOpenGL_1_00::SetColor( 
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
CLineOpenGL_1_00 & CLineOpenGL_1_00::SetStyle( 
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
  if ( style._stipple_type == 0 )
  {
    glDisable( GL_LINE_STIPPLE );
    return *this;
  }
  glEnable( GL_LINE_STIPPLE );

  // set stipple pattern
  static const std::array<GLushort, 8> patterns
  {
    0x0000, 0xFFFF, 0x5555, 0x3333, 0x0F0F, 0x00FF,  0xDDDD, 0xFDFD
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
CLineOpenGL_1_00 & CLineOpenGL_1_00::SetArrowStyle( 
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
bool CLineOpenGL_1_00::Draw( 
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
bool CLineOpenGL_1_00::Draw( 
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


} // Line


} // OpenGL
