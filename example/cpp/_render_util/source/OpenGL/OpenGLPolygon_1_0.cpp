/******************************************************************//**
* \brief Implementation of OpenGL polygon renderer,
* for OpenGL version 1.00 - "Software-OpenGL".
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/

#include <stdafx.h>

// includes

#include "../../include/OpenGL/OpenGLPolygon_1_0.h"


// OpenGL wrapper

#include "../../include/OpenGL/OpenGL_include.h"
#include "../../include/OpenGL/OpenGL_enumconst.h"


// STL

#include <cassert>


// class definitions


/******************************************************************//**
* \brief General namespace for OpenGL implementation.  
* 
* \author  gernot
* \date    2018-09-28
* \version 1.0
**********************************************************************/
namespace OpenGL
{


/******************************************************************//**
* \brief Namespace for drawing polygons with the use of OpenGL.  
* 
* \author  gernot
* \date    2018-09-28
* \version 1.0
**********************************************************************/
namespace Polygon
{


/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-09-28
* \version 1.0
**********************************************************************/
CPolygonOpenGL_1_00::CPolygonOpenGL_1_00( void )
{}


/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-09-28
* \version 1.0
**********************************************************************/
CPolygonOpenGL_1_00::~CPolygonOpenGL_1_00()
{}


/******************************************************************//**
* \brief Initialize the polygon renderer.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_1_00::Init( void )
{
  return true;
}


/******************************************************************//**
* \brief Change the current polygon color, for the pending polygon drawing
* instructions
* 
* \author  gernot
* \date    2018-09-28
* \version 1.0
**********************************************************************/
Render::Polygon::IRender & CPolygonOpenGL_1_00::SetColor(
  const Render::TColor & color ) //!< I - the new color
{
  // change the vertex color
  glColor4fv( color.data() );

  return *this;
}


/******************************************************************//**
* \brief Change the current polygon color, for the pending polygon drawing
* instructions
* 
* \author  gernot
* \date    2018-09-28
* \version 1.0
**********************************************************************/
Render::Polygon::IRender & CPolygonOpenGL_1_00::SetColor( 
  const Render::TColor8 & color ) //!< I - the new color
{
  // change the vertex color
  glColor4ubv( color.data() );

  return *this;
}


/******************************************************************//**
* \brief Set the polygon style.
* 
* \author  gernot
* \date    2018-09-28
* \version 1.0
**********************************************************************/
Render::Polygon::IRender & CPolygonOpenGL_1_00::SetStyle( 
  const Render::Polygon::TStyle & style )
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

  // ...
  
  return *this;
}


/******************************************************************//**
* \brief Draw a single polygon sequence.  
* 
* \author  gernot
* \date    2018-09-28
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_1_00::Draw( 
  Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - triangles, triangle strip, triangle fan, triangle adjacency or triangle strip adjacency
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
  ASSERT( Render::BasePrimitive(primitive_type) == Render::TBasePrimitive::polygon );
  ASSERT( tuple_size == 2 || tuple_size == 3 || tuple_size == 4 );

  // start `glBegin` / `glEnd` sequence
  glBegin( OpenGL::Primitive(primitive_type) );

  // draw the polygon sequence
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
* \brief Draw a single polygon sequence.  
* 
* \author  gernot
* \date    2018-09-28
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_1_00::Draw( 
  Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - triangles, triangle strip, triangle fan, triangle adjacency or triangle strip adjacency
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
  ASSERT( Render::BasePrimitive(primitive_type) == Render::TBasePrimitive::polygon );
  ASSERT( tuple_size == 2 || tuple_size == 3 || tuple_size == 4 );

  // start `glBegin` / `glEnd` sequence
  glBegin( OpenGL::Primitive(primitive_type) );

  // draw the polygon sequence
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
* \brief Draw a single polygon sequence.  
* 
* \author  gernot
* \date    2018-09-28
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_1_00::Draw( 
  Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - triangles, triangle strip, triangle fan, triangle adjacency or triangle strip adjacency
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
  ASSERT( Render::BasePrimitive(primitive_type) == Render::TBasePrimitive::polygon );
  
  // start `glBegin` / `glEnd` sequence
  glBegin( OpenGL::Primitive(primitive_type) );

  // draw the polygon sequence
  for ( size_t i = 0; i < no_of_coords; ++ i )
    glVertex2f( x_coords[i], y_coords[i] );

  // complete sequence
  glEnd();

  return true;
}


/******************************************************************//**
* \brief Draw a single polygon sequence.  
* 
* \author  gernot
* \date    2018-09-28
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_1_00::Draw( 
  Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - triangles, triangle strip, triangle fan, triangle adjacency or triangle strip adjacency
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
  ASSERT( Render::BasePrimitive(primitive_type) == Render::TBasePrimitive::polygon );
  
  // start `glBegin` / `glEnd` sequence
  glBegin( OpenGL::Primitive(primitive_type) );

  // draw the polygon sequence
  for ( size_t i = 0; i < no_of_coords; ++ i )
    glVertex2d( x_coords[i], y_coords[i] );

  // complete sequence
  glEnd();

  return true;
}


/******************************************************************//**
* \brief Start a new polygon sequence.  
* 
* \author  gernot
* \date    2018-09-28
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_1_00::StartSequence( 
  Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - triangles, triangle strip, triangle fan, triangle adjacency or triangle strip adjacency
  unsigned int       tuple_size )    //!< in: kind of the coordinates - 2: 2D (x, y), 3: 3D (x, y, z), 4: homogeneous (x, y, z, w)   
{
  // A new sequence can't be started within an active sequence
  if ( _active_sequence )
  {
    ASSERT( false );
    return false;
  }
  ASSERT( Render::BasePrimitive(primitive_type) == Render::TBasePrimitive::polygon );
  ASSERT( tuple_size == 2 || tuple_size == 3 || tuple_size == 4 );
  
  _active_sequence = true;
  _tuple_size      = tuple_size;

  // start `glBegin` / `glEnd` sequence
  glBegin( OpenGL::Primitive(primitive_type) );

  return true;
}
  

/******************************************************************//**
* \brief Complete an active polygon sequence.  
* 
* \author  gernot
* \date    2018-09-28
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_1_00::EndSequence( void )
{
  // A sequence can't be completed if there is no active sequence
  if ( _active_sequence == false )
  {
    ASSERT( false );
    return false;
  }
 
  _active_sequence = false;
  _tuple_size      = 0;

  // complete sequence
  glEnd();

  return true;
}


/******************************************************************//**
* \brief Specify a new vertex coordinate in an active polygon sequence.  
* 
* \author  gernot
* \date    2018-09-28
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_1_00::DrawSequence( 
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
* \brief Specify a new vertex coordinate in an active polygon sequence.  
* 
* \author  gernot
* \date    2018-09-28
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_1_00::DrawSequence( 
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
* \brief Specify a sequence of new vertex coordinates in an active
* polygon sequence.  
* 
* \author  gernot
* \date    2018-09-28
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_1_00::DrawSequence( 
  size_t             coords_size, //!< in: number of elements (size) of the coordinate array - `coords_size` = `tuple_size` * "number of coordinates" 
  const float       *coords )     //!< in: pointer to an array of the vertex coordinates
{
  // A sequence has to be active, to specify new vertex coordinates
  if ( _active_sequence == false )
  {
    ASSERT( false );
    return false;
  }

  // draw the polygon sequence
  if ( _tuple_size == 2 )
  {
    for ( const float *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 2 )
      glVertex2fv( ptr );
  }
  else if ( _tuple_size == 3 )
  {
    for ( const float *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 3 )
      glVertex3fv( ptr );
  }
  else if ( _tuple_size == 4 )
  {
    for ( const float *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 4 )
      glVertex4fv( ptr );
  }

  return true;
}


/******************************************************************//**
* \brief Specify a sequence of new vertex coordinates in an active
* polygon sequence.  
* 
* \author  gernot
* \date    2018-09-28
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_1_00::DrawSequence( 
  size_t             coords_size, //!< in: number of elements (size) of the coordinate array - `coords_size` = `tuple_size` * "number of coordinates" 
  const double      *coords )     //!< in: pointer to an array of the vertex coordinates
{
  // A sequence has to be active, to specify new vertex coordinates
  if ( _active_sequence == false )
  {
    ASSERT( false );
    return false;
  }

  // draw the polygon sequence
  if ( _tuple_size == 2 )
  {
    for ( const double *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 2 )
      glVertex2dv( ptr );
  }
  else if ( _tuple_size == 3 )
  {
    for ( const double *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 3 )
      glVertex3dv( ptr );
  }
  else if ( _tuple_size == 4 )
  {
    for ( const double *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr += 4 )
      glVertex4dv( ptr );
  }

  return true;
}


} // Polygon


} // OpenGL