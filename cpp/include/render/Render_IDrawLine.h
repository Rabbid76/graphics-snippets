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
// CStyle
//---------------------------------------------------------------------

class CStyle
{
  t_fp _width;        //!< line width
  int  _stipple_type; //!< type of line stippling

  // TODO $$$ line cap
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

  //bool Draw( )
};


} // Line

} // Render

#endif // Render_IDrawLine_h_INCLUDE