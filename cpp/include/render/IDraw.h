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
#include <IBuffer.h>

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

  struct TStyle
  {
    float _thickness = 1.0f;
  };

  virtual ~IDraw() = default;
  virtual void Destroy( void ) = 0;

  bool DrawConvexPolygon( size_t size, const TBuffer &corrds, const TColor &color )
  {
    return DrawConvexPolygon( size, corrds.size(), corrds.data(), color );
  }

  virtual bool DrawConvexPolygon( size_t size, size_t coords_size, const t_fp *coords, const TColor &color )
  {
    return Draw( TPrimitive::trianglefan, size, coords_size, coords, color, TStyle() );
  }

  bool DrawPolyline( size_t size, const TBuffer &corrds, const TColor &color, float thickness )
  {
    return DrawPolyline( size, corrds.size(), corrds.data(), color, thickness );
  }

  virtual bool DrawPolyline( size_t size, size_t coords_size, const t_fp *coords, const TColor &color, float thickness )
  {
    TStyle style;
    style._thickness = thickness;
    return Draw( TPrimitive::linestrip, size, coords_size, coords, color, style );
  }

  bool DrawRectangle2D( const TPoint2 &min, const TPoint2 &max, float z, const TColor &color, float thickness )
  {
    TStyle style;
    style._thickness = thickness;
    TBuffer line_loop{ min[0], min[1], z, max[0], min[1], z, max[0], max[1], z, min[0], max[1], z };
    return Draw( TPrimitive::lineloop, 3, line_loop.size(), line_loop.data(), color, style );
  }

  bool DrawGrid2D( const TPoint2 &min, const TPoint2 &max, const TVec2 &dist, float z, const TColor &color, float thickness )
  {
    TStyle style;
    style._thickness = thickness;
    
    TBuffer lines;
    lines.reserve( 6 * (2 + (int)((max[0]-min[0])/dist[0]) + (int)((max[1]-min[1])/dist[1]) ) );
    TPoint2 center{ (min[0] + max[0]) / 2.0f, (min[1] + max[1]) / 2.0f };
    TPoint2 lne_2{ fabs(max[0] - min[0]) / 2.0f, fabs(max[1] - min[1]) / 2.0f };
    
    // horizontal and vertical line in the middle
    float line[] = { center[0], min[1], z, center[0], max[1], z, min[0], center[1], z, max[0], center[1], z };
    lines.insert( lines.begin(), line, line + 12 );

    // vertical lines
    for ( float x = dist[0]; x <= lne_2[0]; x += dist[0] )
    {
      float line[] = { center[0]+x, min[1], z, center[0]+x, max[1], z, center[0]-x, min[1], z, center[0]-x, max[1], z, };
      lines.insert( lines.begin(), line, line + 12 );
    }
    
    // horizontal lines
    for ( float y = dist[1]; y <= lne_2[1]; y += dist[1] )
    {
      float line[] = { min[0], center[1]+y, z, max[0], center[1]+y, z, min[0], center[1]-y, z, max[0], center[1]-y, z };
      lines.insert( lines.begin(), line, line + 12 );
    }

    return Draw( TPrimitive::lines, 3, lines.size(), lines.data(), color, style );
  }

  bool Draw( TPrimitive primitive_type, size_t size, const TBuffer &corrds, const TColor &color, const TStyle &style )
  {
    return Draw( primitive_type, size, corrds.size(), corrds.data(), color, style );
  }

  virtual bool Draw( TPrimitive primitive_type, size_t size, size_t coords_size, const t_fp *coords, const TColor &color, const TStyle &style ) = 0;
};


} // Render


#endif // IDraw_h_INCLUDED
