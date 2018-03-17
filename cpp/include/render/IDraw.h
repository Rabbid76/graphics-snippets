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
#include <IRenderPass.h>

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
  using TSize   = IRenderProcess::TSize;

  struct TStyle
  {
    float _thickness = 1.0f;
  };

  virtual ~IDraw() = default;
  
  virtual void Destroy( void ) = 0;                           //!< destroy all internal objects and cleanup
  virtual bool Init( void ) = 0;                              //!< general initializations
  virtual bool Begin( void ) = 0;                             //!< start the rendering
  virtual bool ActivateBackground( void ) = 0;                //!< activate rendering to background
  virtual bool ActivateOpaque( void ) = 0;                    //!< activate rendering to the opaque buffer
  virtual bool ActivateTransparent( void ) = 0;               //!< activate rendering to the transparent buffer
  virtual bool Finish( void ) = 0;                            //!< finish the rendering
  virtual bool ClearDepth( void ) = 0;                        //!< interim clear of the depth buffer
  virtual void BackgroundColor( const TColor &bg_color ) = 0; //!< sets the background color
  virtual void ViewportSize( const TSize &vp_size ) = 0;      //!< sete the size of the viewport
  virtual void Projection( const TMat44 &proj ) = 0;          //!< set the projection matrix
  virtual void View( const TMat44 &view ) = 0;                //!< set the view matrix
  virtual void Model( const TMat44 &model ) = 0;              //!< set the model matrix

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

  bool DrawLines2D( const TPoint2 &min, const TPoint2 &max, float dist, float z, const TColor &color, float thickness )
  {
    TStyle style;
    style._thickness = thickness;
    
    TBuffer lines;
    lines.reserve( 6 * (1 + (int)((max[1]-min[1])/dist) ) );
    float center = (min[1] + max[1]) / 2.0f;
    float lne_2  = fabs(max[1] - min[1]) / 2.0f;
    
    // horizontal and vertical line in the middle
    float line[] = { min[0], center, z, max[0], center, z };
    lines.insert( lines.begin(), line, line + 12 );

    // horizontal lines
    for ( float y = dist; y <= lne_2; y += dist )
    {
      float line[] = { min[0], center+y, z, max[0], center+y, z, min[0], center-y, z, max[0], center-y, z };
      lines.insert( lines.begin(), line, line + 12 );
    }

    return Draw( TPrimitive::lines, 3, lines.size(), lines.data(), color, style );
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
