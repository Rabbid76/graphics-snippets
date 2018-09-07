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

  //bool Draw( )
};


} // Line

} // Render

#endif // Render_IDrawLine_h_INCLUDE