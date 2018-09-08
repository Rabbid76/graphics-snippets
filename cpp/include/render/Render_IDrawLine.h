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

  // TODO $$$
  //virtual void Draw( Render::TPrimitive primitive_type, size_t no_of_coords, float *x_coords, float *y_coords ) = 0;
  //virtual void Draw( Render::TPrimitive primitive_type, size_t no_of_coords, double *x_coords, double *y_coords ) = 0;

  // TODO $$$
  //virtual void StartSequence( Render::TPrimitive primitive_type ) = 0;
  //virtual void EndSequence( void ) = 0;
  //virtual void DrawSequence( float x, float y, float z ) = 0;
  //virtual void DrawSequence( double x, double y, double z ) = 0;
  //virtual bool DrawSequence( int tuple_size, size_t no_of_coords, const float *coords ) = 0;
  //virtual bool DrawSequence( int tuple_size, size_t no_of_coords, const double *coords ) = 0;

};


} // Line

} // Render

#endif // Render_IDrawLine_h_INCLUDE