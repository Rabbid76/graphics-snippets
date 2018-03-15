/******************************************************************//**
* @brief   Generic interface for basic drawing.
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
#pragma once
#ifndef IDraw_h_INCLUDED
#define IDraw_h_INCLUDED


// includes

#include <IDrawType.h>

// stl

#include <vector>


/******************************************************************//**
* @brief   Namespace for renderer.
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
namespace Render
{


//---------------------------------------------------------------------
// IDraw
//---------------------------------------------------------------------


/******************************************************************//**
* @brief   Generic interface for drawing commands.
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
class IDraw
{
public:

  using TBuffer = std::vector<t_fp>;

  virtual ~IDraw() = default;
  virtual void Destroy( void ) = 0;

  bool DrawConvexPolygon( size_t coord_size, const TBuffer &corrds, const TColor &color )
  {
    return DrawConvexPolygon( coord_size, corrds.size(), corrds.data(), color );
  }

  virtual bool DrawConvexPolygon( size_t size, size_t coords_size, const t_fp *coords, const TColor &color ) = 0;
};


} // Render


#endif // IDraw_h_INCLUDED
