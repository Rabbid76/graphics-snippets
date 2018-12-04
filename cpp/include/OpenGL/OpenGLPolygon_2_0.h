/******************************************************************//**
* \brief Implementation of OpenGL polygon renderer,
* with the use of very simple and highly optimized shaders,
* for OpenGL version 2.00 and GLSL version 1.10 (`#version 110`).
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
#pragma once
#ifndef OpenGLPolygon_2_00_h_INCLUDED
#define OpenGLPolygon_2_00_h_INCLUDED

// includes

#include <Render_IDrawType.h>
#include <Render_IDrawPolygon.h>


// class definitions


/******************************************************************//**
* \brief General namespace for OpenGL implementation.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
namespace OpenGL
{

class CPrimitiveOpenGL_2_00;


/******************************************************************//**
* \brief Namespace for drawing polygons with the use of OpenGL.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
namespace Polygon
{


/******************************************************************//**
* \brief Implementation of OpenGL polygon renderer
* with the use of very simple and highly optimized shaders,
* for OpenGL version 2.00 and GLSL version 1.10 (`#version 110`).
*
* The goal of this implementation is to be most efficient and to do the
* minimal requirements, that are necessary for drawing uniform colored
* polygons by the use of OpenGL compatibility profile,
* down to OpenGL version 2.00. 
* 
* `glBegin` \ `glEnd` sequences are emulated and by caching the vertices
* to a buffer, which stays persistent and is initialized by a minimum
* size provided to the constructor.
*
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
class CPolygonOpenGL_2_00
  : public Render::Polygon::IRender
{
public:

  using TProgramPtr = std::shared_ptr<CPrimitiveOpenGL_2_00>;

  CPolygonOpenGL_2_00( size_t min_cache_elems );
  virtual ~CPolygonOpenGL_2_00();

  const Render::Polygon::TStyle & PoylgonStyle( void ) const { return _polygon_style; }
  void PoylgonStyle( const Render::Polygon::TStyle &style ) { _polygon_style = style; }

  //! Initialize the polygon renderer
  virtual void Init( void ) override;
  // TODO $$$ virtual void Init( TProgramPtr &program ) override;

  //! Notify the render that a sequence of successive polygons will follow, which is not interrupted by any other drawing operation.
  //! This allows the render to do some performance optimizations and to prepare for the polygon rendering.
  //! The render can keep states persistent from one polygon drawing to the other, without initializing and restoring them.
  //!
  //! Not implemented (Software OpenGL).
  virtual bool StartSuccessivePolygonDrawings( void ) override;

 //! Notify the renderer that a sequence of polygons has been finished, and that the internal states have to be restored.
  //!
  //! Not implemented (Software OpenGL).
  virtual bool FinishSuccessivePolygonDrawings( void ) override;

  virtual Render::Polygon::IRender & SetColor( const Render::TColor & color ) override;
  virtual Render::Polygon::IRender & SetColor( const Render::TColor8 & color ) override;
  virtual Render::Polygon::IRender & SetStyle( const Render::Polygon::TStyle & style ) override;
  
  //! Draw a polygon sequence
  virtual bool Draw( Render::TPrimitive primitive_type, unsigned int tuple_size, size_t coords_size, const float *coords ) override;
  virtual bool Draw( Render::TPrimitive primitive_type, unsigned int tuple_size, size_t coords_size, const double *coords ) override;
  virtual bool Draw( Render::TPrimitive primitive_type, size_t no_of_coords, const float *x_coords, const float *y_coords ) override;
  virtual bool Draw( Render::TPrimitive primitive_type, size_t no_of_coords, const double *x_coords, const double *y_coords ) override;

  //! Start a new polygon sequence
  virtual bool StartSequence( Render::TPrimitive primitive_type, unsigned int tuple_size ) override;
  
  //! Complete an active polygon sequence
  virtual bool EndSequence( void ) override;

  //! Specify a new vertex coordinate in an active polygon sequence
  virtual bool DrawSequence( float x, float y, float z ) override;
  virtual bool DrawSequence( double x, double y, double z ) override;
  
  //! Specify a sequence of new vertex coordinates in an active polygon sequence
  virtual bool DrawSequence( size_t coords_size, const float *coords ) override;
  virtual bool DrawSequence( size_t coords_size, const double *coords ) override;

private:

  TProgramPtr             _primitive_prog;                            //!< primitive render program
  Render::Polygon::TStyle _polygon_style;                             //!< polygon style parameters                     
  Render::TPrimitive      _squence_type{ Render::TPrimitive::NO_OF }; //!< primitive type pf the sequence
  Render::TVertexCache    _vertex_cache;                              //!< cache for vertex coordinates
};


} // Polygon


} // OpenGL


#endif // OpenGLLine_2_00_h_INCLUDED

