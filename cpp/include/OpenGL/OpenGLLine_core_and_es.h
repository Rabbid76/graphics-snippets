/******************************************************************//**
* \brief Implementation of OpenGL line renderer,
* with the use of "modern" OpenGL 4+ core and
* GLSL version 4.20 (`#version 420 core`) or
* OpenGL es 3(+) and GLSL ES version 3.00 (`#version 300 es`).
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/
#pragma once
#ifndef OpenGLLine_4_h_INCLUDED
#define OpenGLLine_4_h_INCLUDED

// includes

#include <Render_IDrawType.h>
#include <Render_IDrawLine.h>
#include <Render_IBuffer.h>

#include <OpenGLDataBuffer_std140.h>


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


class CPrimitiveOpenGL_core_and_es;


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
* with the use of OpenGL core profile
* for OpenGL version 4+ and GLSL version 4.20 (`#version 420 core`)
* or OpenGL es 3(+) and GLSL ES version 3.00 (`#version 300 es`).
*
* Line stippling techniques is implemented in the shader program.
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/
class CLineOpenGL_core_and_es
  : public Render::Line::IRender
{
public: 

  using TProgramPtr = std::shared_ptr<CPrimitiveOpenGL_core_and_es>;

  CLineOpenGL_core_and_es( size_t min_cache_elems );
  virtual ~CLineOpenGL_core_and_es();

  const Render::Line::TStyle & LineStyle( void ) const { return _line_style; }
  void LineStyle( const Render::Line::TStyle &style ) { _line_style = style; }

  //! Initialize the line renderer
  virtual bool Init( void ) override;
  virtual bool Init( Render::TModelAndViewPtr mvp_data );
  virtual bool Init( TMVPBufferPtr mvp_buffer );
  virtual bool Init( TProgramPtr program );

  //! Notify the render that a sequence of successive lines will follow, that is not interrupted by any other drawing operation.
  //! This allows the render to do some performance optimizations and to prepare for the line rendering.
  //! The render can keep states persistent from one line drawing to the other, without initializing and restoring them.
  virtual bool StartSuccessiveLineDrawings( void ) override;

  //! Notify the renderer that a sequence of lines has been finished, and that the internal states have to be restored.
  virtual bool FinishSuccessiveLineDrawings( void ) override;

  virtual Render::Line::IRender & SetColor( const Render::TColor & color )  override;
  virtual Render::Line::IRender & SetColor( const Render::TColor8 & color ) override;
  virtual Render::Line::IRender & SetStyle( const Render::Line::TStyle & style ) override;
  
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

  bool                    _initialized{ false };                      //!< initialization state of the object               
  bool                    _successive_draw_started{ false };          //!< successive drawing was started by this renderer
  size_t                  _min_buffer_size;                           //!< minimum size of the vertex buffer
  TProgramPtr             _primitive_prog;                            //!< primitive render program
  Render::Line::TStyle    _line_style;                                //!< line style parameters: line width and stippling               
  Render::TPrimitive      _squence_type{ Render::TPrimitive::NO_OF }; //!< primitive type pf the sequence
  Render::TVertexCache    _vertex_cache;                              //!< cache for vertex coordinates
};


} // Line


} // OpenGL


#endif // OpenGLLine_4_h_INCLUDED
