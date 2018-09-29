/******************************************************************//**
* @brief   Generic interface for drawing polygons.
*
* @author  gernot
* @date    2018-09-28
* @version 1.0
**********************************************************************/
#pragma once
#ifndef Render_IDrawPolygon_h_INCLUDE
#define Render_IDrawPolygon_h_INCLUDE


// includes

#include <Render_IDrawType.h>

#include <vector>
#include <memory>


/******************************************************************//**
* @brief   Namespace for renderer.
*
* @author  gernot
* @date    2018-09-28
* @version 1.0
**********************************************************************/
namespace Render
{


/******************************************************************//**
* @brief   Namespace for generic polygon rendering.
*
* @author  gernot
* @date    2018-09-28
* @version 1.0
**********************************************************************/
namespace Polygon
{


class IRender;
using IRenderPtr = std::shared_ptr<IRender>;


//---------------------------------------------------------------------
// IPolgonProvider
//---------------------------------------------------------------------


/******************************************************************//**
* \brief   Generic provider for generic polygon drawing.
* 
* \author  gernot
* \date    2018-06-08
* \version 1.0
**********************************************************************/
class IRenderProvider
{
public:

  virtual ~IRenderProvider() = default;

  virtual IRender & PolygonRender( void ) = 0;
};


//---------------------------------------------------------------------
// Style
//---------------------------------------------------------------------

struct TStyle
{
  t_fp _depth_attenuation = 0.0f; //!< attenuation of the line color by depth
};


//---------------------------------------------------------------------
// Polygon
//---------------------------------------------------------------------


/******************************************************************//**
* @brief   Generic interface for drawing lines.
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
class IRender
{
public:

  virtual ~IRender() = default;

  //! Initialize the line renderer
  virtual void Init( void ) = 0;

  //! Notify the render that a sequence of successive polygons will follow, which is not interrupted by any other drawing operation.
  //! This allows the render to do some performance optimizations and to prepare for the polygon rendering.
  //! The render can keep states persistent from one polygon drawing to the other, without initializing and restoring them.
  virtual bool StartSuccessiveLineDrawings( void ) = 0;

  //! Notify the renderer that a sequence of polygons has been finished, and that the internal states have to be restored.
  virtual bool FinishSuccessiveLineDrawings( void ) = 0;

  virtual IRender & SetColor( const TColor & color ) = 0;
  virtual IRender & SetColor( const TColor8 & color ) = 0;
  virtual IRender & SetStyle( const TStyle & style ) = 0;
  
  //! Draw a polygon sequence
  virtual bool Draw( 
    Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - triangles, triangle strip, triangle fan, triangle adjacency or triangle strip adjacency  
    unsigned int       tuple_size,     //!< in: kind of the coordinates - 2: 2D (x, y), 3: 3D (x, y, z), 4: homogeneous (x, y, z, w)   
    size_t             coords_size,    //!< in: number of elements (size) of the coordinate array - `coords_size` = `tuple_size` * "number of coordinates" 
    const float       *coords          //!< in: pointer to an array of the vertex coordinates
  ) = 0;
  
  //! Draw a polygon sequence
  virtual bool Draw( 
    Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - triangles, triangle strip, triangle fan, triangle adjacency or triangle strip adjacency
    unsigned int       tuple_size,     //!< in: kind of the coordinates - 2: 2D (x, y), 3: 3D (x, y, z), 4: homogeneous (x, y, z, w)   
    size_t             coords_size,    //!< in: number of elements (size) of the coordinate array - `coords_size` = `tuple_size` * "number of coordinates" 
    const double      *coords          //!< in: pointer to an array of the vertex coordinates
  ) = 0;

  //! Draw a polygon sequence
  virtual bool Draw( 
    Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - triangles, triangle strip, triangle fan, triangle adjacency or triangle strip adjacency
    size_t             no_of_coords,   //!< in: number of coordinates and number of elements (size) of the coordinate array
    const float       *x_coords,       //!< int pointer to an array of the x coordinates
    const float       *y_coords        //!< int pointer to an array of the y coordinates
  ) = 0;
 
  //! Draw a polygon sequence
  virtual bool Draw( 
    Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - triangles, triangle strip, triangle fan, triangle adjacency or triangle strip adjacency
    size_t             no_of_coords,   //!< in: number of coordinates and number of elements (size) of the coordinate array
    const double      *x_coords,       //!< int pointer to an array of the x coordinates
    const double      *y_coords        //!< int pointer to an array of the y coordinates
  ) = 0;

  //! Start a new polygon sequence
  virtual bool StartSequence( Render::TPrimitive primitive_type, unsigned int tuple_size ) = 0;
  
  //! Complete an active line sequence
  virtual bool EndSequence( void ) = 0;

  //! Specify a new vertex coordinate in an active polygon sequence
  virtual bool DrawSequence( float x, float y, float z ) = 0;
  
  //! Specify a new vertex coordinate in an active polygon sequence
  virtual bool DrawSequence( double x, double y, double z ) = 0;
  
  //! Specify a sequence of new vertex coordinates in an active polygon sequence
  virtual bool DrawSequence( size_t coords_size, const float *coords ) = 0;
  
  //! Specify a sequence of new vertex coordinates in an active polygon sequence
  virtual bool DrawSequence( size_t coords_size, const double *coords ) = 0;
};


} // Polygon

} // Render

#endif // Render_IDrawPolygon_h_INCLUDE
