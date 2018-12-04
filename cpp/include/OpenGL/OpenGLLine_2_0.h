/******************************************************************//**
* \brief Implementation of OpenGL line renderer,
* with the use of very simple and highly optimized shaders,
* for OpenGL version 2.00 and GLSL version 1.10 (`#version 110`).
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/
#pragma once
#ifndef OpenGLLine_2_00_h_INCLUDED
#define OpenGLLine_2_00_h_INCLUDED

// includes

#include <Render_IDrawType.h>
#include <Render_IDrawLine.h>


// STL

#include <string>
#include <vector>


// class definitions


/******************************************************************//**
* \brief General namespace for OpenGL implementation.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
namespace OpenGL
{

class CPrimitiveOpenGL_2_00;


/******************************************************************//**
* \brief Namespace for drawing lines with the use of OpenGL.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
namespace Line
{


/******************************************************************//**
* \brief Implementation of OpenGL line renderer,
* with the use of very simple and highly optimized shaders,
* for OpenGL version 2.00 and GLSL version 1.10 (`#version 110`).
*
* The goal of this implementation is to be most efficient and to do the
* minimal requirements, that are necessary for drawing uniform colored
* and stippled lines by the use of OpenGL compatibility profile,
* down to OpenGL version 2.00. 
*
* `glBegin` \ `glEnd` sequences are emulated and by caching the vertices
* to a buffer, which stays persistent and is initialized by a minimum
* size provided to the constructor.
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/
class CLineOpenGL_2_00
  : public Render::Line::IRender
{
public:

  using TProgramPtr = std::shared_ptr<CPrimitiveOpenGL_2_00>;

  CLineOpenGL_2_00( size_t min_cache_elems );
  virtual ~CLineOpenGL_2_00();

  const Render::Line::TStyle & LineStyle( void ) const { return _line_style; }
  void LineStyle( const Render::Line::TStyle &style ) { _line_style = style; }

  //! Initialize the line renderer
  virtual void Init( void ) override;
  // TODO $$$ virtual void Init( TProgramPtr &program ) override;

  //! Notify the render that a sequence of successive lines will follow, that is not interrupted by any other drawing operation.
  //! This allows the render to do some performance optimizations and to prepare for the line rendering.
  //! The render can keep states persistent from one line drawing to the other, without initializing and restoring them.
  virtual bool StartSuccessiveLineDrawings( void ) override;

  //! Notify the renderer that a sequence of lines has been finished, and that the internal states have to be restored.
  virtual bool FinishSuccessiveLineDrawings( void ) override;

  virtual Render::Line::IRender & SetColor( const Render::TColor & color )  override;
  virtual Render::Line::IRender & SetColor( const Render::TColor8 & color ) override;

  virtual Render::Line::IRender & SetStyle( const Render::Line::TStyle & style ) override;
  virtual Render::Line::IRender & SetArrowStyle( const Render::Line::TArrowStyle & style ) override;

  //! Draw a line sequence
  virtual bool Draw( Render::TPrimitive primitive_type, unsigned int tuple_size, size_t coords_size, const float *coords ) override;
  virtual bool Draw( Render::TPrimitive primitive_type, unsigned int tuple_size, size_t coords_size, const double *coords ) override;
  virtual bool Draw( Render::TPrimitive primitive_type, size_t no_of_coords, const float *x_coords, const float *y_coords ) override;
  virtual bool Draw( Render::TPrimitive primitive_type, size_t no_of_coords, const double *x_coords, const double *y_coords ) override;

  //! Start a new line sequence
  virtual bool StartSequence( Render::TPrimitive primitive_type, unsigned int tuple_size ) override;
  
  //! Complete an active line sequence
  virtual bool EndSequence( void ) override;

  //! Specify a new vertex coordinate in an active line sequence
  virtual bool DrawSequence( float x, float y, float z ) override;
  virtual bool DrawSequence( double x, double y, double z ) override;
  
  //! Specify a sequence of new vertex coordinates in an active line sequence
  virtual bool DrawSequence( size_t coords_size, const float *coords ) override;
  virtual bool DrawSequence( size_t coords_size, const double *coords ) override;

private:

  TProgramPtr             _primitive_prog;                            //!< primitive render program
  Render::Line::TStyle    _line_style;                                //!< line style parameters: line width and stippling
  Render::TPrimitive      _squence_type{ Render::TPrimitive::NO_OF }; //!< primitive type pf the sequence
  Render::TVertexCache    _vertex_cache;                              //!< cache for vertex coordinates
};


} // Line


} // OpenGL


#endif // OpenGLLine_2_00_h_INCLUDED
