/******************************************************************//**
* \brief Implementation of OpenGL polygon renderer,
* for OpenGL version 1.00 - "Software-OpenGL".
* 
* \author  gernot
* \date    2018-09-28
* \version 1.0
**********************************************************************/
#pragma once
#ifndef OpenGLPolygon_1_00_h_INCLUDED
#define OpenGLPolygon_1_00_h_INCLUDED

// includes

#include <Render_IDrawType.h>
#include <Render_IDrawPolygon.h>


// class definitions


/******************************************************************//**
* \brief General namespace for OpenGL implementation.  
* 
* \author  gernot
* \date    2018-09-28
* \version 1.0
**********************************************************************/
namespace OpenGL
{


/******************************************************************//**
* \brief Namespace for drawing polygons with the use of OpenGL.  
* 
* \author  gernot
* \date    2018-09-28
* \version 1.0
**********************************************************************/
namespace Polygon
{


/******************************************************************//**
* \brief Implementation of OpenGL polygon renderer,
* for OpenGL version 1.00 - "Software-OpenGL".
*
* See [The OpenGL Graphics System A Specification (Version 1.0)[](https://www.khronos.org/registry/OpenGL/specs/gl/glspec10.pdf).
* 
* The goal of this implementation is not high performance, but it is
* to succeed on any hardware, by the use of the lowest possible
* OpenGL version 1.00.
*
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/
class CPolygonOpenGL_1_00
  : public Render::Polygon::IRender
{
public:

  CPolygonOpenGL_1_00( void );
  virtual ~CPolygonOpenGL_1_00();

  //! Initialize the polygon renderer
  virtual void Init( void ) override;

  //! Notify the render that a sequence of successive polygons will follow, which is not interrupted by any other drawing operation.
  //! This allows the render to do some performance optimizations and to prepare for the polygon rendering.
  //! The render can keep states persistent from one polygon drawing to the other, without initializing and restoring them.
  //!
  //! Not implemented (Software OpenGL).
  virtual bool StartSuccessivePolygonDrawings( void ) override { return false; } 

 //! Notify the renderer that a sequence of polygons has been finished, and that the internal states have to be restored.
  //!
  //! Not implemented (Software OpenGL).
  virtual bool FinishSuccessivePolygonDrawings( void ) override { return false; }

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

  bool         _active_sequence{ false }; //!< true: an draw sequence was started, but not finished yet
  unsigned int _tuple_size{ 0 };          //!< tuple size (2, 3 or 4) for a sequence
};


} // Polygon


} // OpenGL


#endif // OpenGLPolygon_1_00_h_INCLUDED
