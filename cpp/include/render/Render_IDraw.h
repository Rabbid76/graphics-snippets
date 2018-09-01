/******************************************************************//**
* @brief   Generic interface for basic drawing.
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
#pragma once
#ifndef Render_IDraw_h_INCLUDED
#define Render_IDraw_h_INCLUDED


// includes

#include <Render_IDrawType.h>
#include <Render_IBuffer.h>
#include <Render_IRenderPass.h>
#include <Render_IFont.h>

// stl

#include <vector>
#include <bitset>


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
  using TFontId = size_t;

  enum class TStyleProperty
  {
    arrow_from,
    arrow_to,
    // ...
    NO_OF
  };
  using TStyleProperties = std::bitset<(int)TStyleProperty::NO_OF>;

  struct TStyle
  {
    float            _thickness = 1.0f;
    TVec2            _size      { 0.0f };
    TStyleProperties _properites;
  };

  virtual ~IDraw() = default;

  void MVP( const TMat44 &proj, const TMat44 &view, const TMat44 &model )
  {
    Projection( proj );
    View( view );
    Model( model );
  }

  virtual const TMat44 & Projection( void ) const = 0; //!< get the projection matrix
  virtual const TMat44 & View( void ) const = 0;       //!< get the view matrix
  virtual const TMat44 & Model( void ) const = 0;      //!< get the model matrix
  
  virtual void  BackgroundColor( const TColor &bg_color ) = 0; //!< sets the background color
  virtual void  ViewportSize( const TSize &vp_size ) = 0;      //!< sete the size of the viewport
  virtual void  Projection( const TMat44 &proj ) = 0;          //!< set the projection matrix
  virtual void  View( const TMat44 &view ) = 0;                //!< set the view matrix
  virtual void  Model( const TMat44 &model ) = 0;              //!< set the model matrix
  virtual TVec3 Project( const TVec3 &pt ) const = 0;          //!< project by projection, view and model
  virtual void  Destroy( void ) = 0;                           //!< destroy all internal objects and cleanup
  virtual bool  Init( void ) = 0;                              //!< general initializations
  virtual bool  LoadFont( TFontId font_id, IFont *&font ) = 0; //!< load and return a font by its id
  virtual bool  Begin( void ) = 0;                             //!< start the rendering
  virtual bool  ActivateBackground( void ) = 0;                //!< activate rendering to background
  virtual bool  ActivateOpaque( void ) = 0;                    //!< activate rendering to the opaque buffer
  virtual bool  ActivateTransparent( void ) = 0;               //!< activate rendering to the transparent buffer
  virtual bool  Finish( void ) = 0;                            //!< finish the rendering
  virtual bool  ClearDepth( void ) = 0;                        //!< interim clearing  of the depth buffer
  
  bool DrawConvexPolygon( size_t size, const TBuffer &corrds, const TColor &color )
  {
    return DrawConvexPolygon( size, corrds.size(), corrds.data(), color );
  }

  bool DrawConvexPolygon( const std::vector<TPoint2> &corrds, const TColor &color )
  {
    return DrawConvexPolygon( 2, corrds.size()*2, &corrds[0][0], color );
  }

  bool DrawConvexPolygon( const std::vector<TPoint3> &corrds, const TColor &color )
  {
    return DrawConvexPolygon( 3, corrds.size()*3, &corrds[0][0], color );
  }

  bool DrawConvexPolygon( const std::vector<TPoint4> &corrds, const TColor &color )
  {
    return DrawConvexPolygon( 4, corrds.size()*4, &corrds[0][0], color );
  }

  virtual bool DrawConvexPolygon( size_t size, size_t coords_size, const t_fp *coords, const TColor &color )
  {
    return Draw( TPrimitive::trianglefan, size, coords_size, coords, color, TStyle() );
  }

  bool DrawPolyline( size_t tuple_size, const TBuffer &corrds, const TColor &color, float thickness, bool closed )
  {
    return DrawPolyline( tuple_size, corrds.size(), corrds.data(), color, thickness, closed );
  }

  virtual bool DrawPolyline( size_t tuple_size, size_t coords_size, const t_fp *coords, const TColor &color, float thickness, bool closed )
  {
    TStyle style;
    style._thickness = thickness;
    return Draw( closed ? TPrimitive::lineloop : TPrimitive::linestrip, tuple_size, coords_size, coords, color, style );
  }

  bool DrawArrow( size_t tuple_size, const TBuffer &corrds, const TColor &color, float thickness, const TVec2 &arrow_size, bool arrow_from, bool arrow_to )
  {
    return DrawArrow( tuple_size, corrds.size(), corrds.data(), color, thickness, arrow_size, arrow_from, arrow_to );
  }

  virtual bool DrawArrow( size_t tuple_size, size_t coords_size, const t_fp *coords, const TColor &color, float thickness, const TVec2 &arrow_size, bool arrow_from, bool arrow_to )
  {
    TStyle style;
    style._thickness = thickness;
    style._size      = arrow_size;
    style._properites.set( (int)TStyleProperty::arrow_from, arrow_from );
    style._properites.set( (int)TStyleProperty::arrow_to, arrow_to );
    return Draw( TPrimitive::linestrip, tuple_size, coords_size, coords, color, style );
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

  bool DrawPoint2D( Render::TPoint2 pt, const TColor &color, float point_size )
  {
    TStyle style;
    style._thickness = point_size;
    return Draw( TPrimitive::points, 2, { pt[0], pt[1] }, color, style );
  }

  bool Draw( TPrimitive primitive_type, size_t size, const TBuffer &corrds, const TColor &color, const TStyle &style )
  {
    return Draw( primitive_type, size, corrds.size(), corrds.data(), color, style );
  }

  virtual bool Draw( TPrimitive primitive_type, size_t size, size_t coords_size, const t_fp *coords, const TColor &color, const TStyle &style ) = 0;

  virtual bool CalculateTextSize( TFontId font_id, const char *str, float height, float &box_x, float &box_btm, float &box_top )
  {
    IFont *font = nullptr;
    if ( LoadFont( font_id, font ) == false )
      return false;
    return font->CalculateTextSize( str, height, box_x, box_btm, box_top );
  }
  
  virtual bool DrawText2D( TFontId font_id, const char *text, float height, float width_scale, const TPoint3 &pos, const TColor &color ) = 0;

  virtual bool DrawText2D( TFontId font_id, const char *text, int origin, float height, float width_scale, float margin, const TPoint3 &pos, const TColor &color )
  {
    std::array<float, 4> text_rect{ 0.0f };
    CalculateTextSize( font_id, text, height, text_rect[2], text_rect[1], text_rect[3] );
    float width = fabs(text_rect[2]) * width_scale;
    TPoint3 origin_pos = pos;
    if ( origin % 3 == 0 ) // 3, 6, 9
      origin_pos[0] -= width + margin;
    else if ( origin % 3 == 2 ) // 2, 5, 8
      origin_pos[0] -= width * 0.5f;
    else
      origin_pos[0] += margin;
    if ( origin >= 7 && origin <= 9 ) // 7, 8, 9
      origin_pos[1] -= text_rect[3] + margin;
    else if ( origin >= 4 && origin <= 6 ) // 4, 5, 6
      origin_pos[1] -= text_rect[3] * 0.5f;
    else
      origin_pos[1] += margin;
    return DrawText2D( font_id, text, height, width_scale, origin_pos, color );
  }

  virtual bool DrawText2DProjected( TFontId font_id, const char *text, int origin, float height, float width_scale, float margin, const TPoint3 &pos, const TColor &color )
  {
     TPoint3 prj_pos = Project( pos );

     TMat44 m[]{ Projection(), View(), Model() };
     MVP( Identity(), Identity(), Identity() );
     
     bool ret = DrawText2D( font_id, text, origin, height, width_scale, margin, prj_pos, color );

     MVP( m[0], m[1], m[2] );

     return ret;
  }
};


} // Render


#endif // Render_IDraw_h_INCLUDED
