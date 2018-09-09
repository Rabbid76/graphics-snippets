/******************************************************************//**
* @brief   Generic interface for drawing lines.
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
#pragma once
#ifndef Render_IDrawLine_h_INCLUDE
#define Render_IDrawLine_h_INCLUDE


// includes

#include <Render_IDrawType.h>

#include <vector>
#include <memory>


/******************************************************************//**
* @brief   Namespace for renderer.
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
namespace Render
{


/******************************************************************//**
* @brief   Namespace for generic line rendering.
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
namespace Line
{


class IRender;
using IRenderPtr = std::shared_ptr<IRender>;


//---------------------------------------------------------------------
// ILineProvider
//---------------------------------------------------------------------


/******************************************************************//**
* \brief   Generic provider for generic line drawing.
* 
* \author  gernot
* \date    2018-06-08
* \version 1.0
**********************************************************************/
class IRenderProvider
{
public:

  virtual ~IRenderProvider() = default;

  virtual IRender & LineRender( void ) = 0;
};


//---------------------------------------------------------------------
// Style
//---------------------------------------------------------------------

struct TStyle
{
  t_fp _width        = 1.0f; //!< line width
  int  _stipple_type = 1;    //!< type of line stippling

  // TODO $$$ line cap
};


enum class TArrowStyleProperty
{
  arrow_from,
  arrow_to,
  // ...
  NO_OF
};
using TArrowStyleProperties = std::bitset<(int)TArrowStyleProperty::NO_OF>;


struct TArrowStyle
{
  TVec2                       _size{ 0.0f };
  Line::TArrowStyleProperties _properites;
};


//---------------------------------------------------------------------
// ILine
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

  virtual IRender & SetColor( const TColor & color ) = 0;
  virtual IRender & SetColor( const TColor8 & color ) = 0;
  virtual IRender & SetStyle( const TStyle & style ) = 0;
  virtual IRender & SetArrowStyle( const TArrowStyle & style ) = 0;

  //! Draw a line sequence
  virtual bool Draw( 
    Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - lines, line strip, line loop, lines adjacency or line strip adjacency  
    unsigned int       tuple_size,     //!< in: kind of the coordinates - 2: 2D (x, y), 3: 3D (x, y, z), 4: homogeneous (x, y, z, w)   
    size_t             coords_size,    //!< in: number of elements (size) of the coordinate array - `coords_size` = `tuple_size` * "number of coordinates" 
    const float       *coords          //!< in: pointer to an array of the vertex coordinates
  ) = 0;
  
  //! Draw a line sequence
  virtual bool Draw( 
    Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - lines, line strip, line loop, lines adjacency or line strip adjacency  
    unsigned int       tuple_size,     //!< in: kind of the coordinates - 2: 2D (x, y), 3: 3D (x, y, z), 4: homogeneous (x, y, z, w)   
    size_t             coords_size,    //!< in: number of elements (size) of the coordinate array - `coords_size` = `tuple_size` * "number of coordinates" 
    const double      *coords          //!< in: pointer to an array of the vertex coordinates
  ) = 0;

  //! Draw a line sequence
  virtual bool Draw( 
    Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - lines, line strip, line loop, lines adjacency or line strip adjacency 
    size_t             no_of_coords,   //!< in: number of coordinates and number of elements (size) of the coordinate array
    const float       *x_coords,       //!< int pointer to an array of the x coordinates
    const float       *y_coords        //!< int pointer to an array of the y coordinates
  ) = 0;
 
  //! Draw a line sequence
  virtual bool Draw( 
    Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - lines, line strip, line loop, lines adjacency or line strip adjacency 
    size_t             no_of_coords,   //!< in: number of coordinates and number of elements (size) of the coordinate array
    const double      *x_coords,       //!< int pointer to an array of the x coordinates
    const double      *y_coords        //!< int pointer to an array of the y coordinates
  ) = 0;

  //! Start a new line sequence
  virtual bool StartSequence( Render::TPrimitive primitive_type ) = 0;
  
  //! Complete an active line sequence
  virtual bool EndSequence( void ) = 0;

  //! Specify a new vertex coordinate in an active line sequence
  virtual bool DrawSequence( float x, float y, float z ) = 0;
  
  //! Specify a new vertex coordinate in an active line sequence
  virtual bool DrawSequence( double x, double y, double z ) = 0;
  
  //! Specify a sequence of new vertex coordinates in an active line sequence
  virtual bool DrawSequence( unsigned int tuple_size, size_t coords_size, const float *coords ) = 0;
  
  //! Specify a sequence of new vertex coordinates in an active line sequence
  virtual bool DrawSequence( unsigned int tuple_size, size_t coords_size, const double *coords ) = 0;
};


} // Line

} // Render

#endif // Render_IDrawLine_h_INCLUDE