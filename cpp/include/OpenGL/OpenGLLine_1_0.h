/******************************************************************//**
* \brief Implementation of OpenGL line renderer,
* for OpenGL version 1.00 - "Software-OpenGL".
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/
#pragma once
#ifndef OpenGLLine_1_00_h_INCLUDED
#define OpenGLLine_1_00_h_INCLUDED

// includes

#include <Render_IDrawType.h>
#include <Render_IDrawLine.h>


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
class CLineOpenGL_1_00
  : public Render::Line::IRender
{
public:

  CLineOpenGL_1_00( void );
  virtual ~CLineOpenGL_1_00();

  //! Initialize the line renderer
  virtual void Init( void ) override;

  //! Notify the render that a sequence of successive lines will follow, that is not interrupted by any other drawing operation.
  //! This allows the render to do some performance optimizations and to prepare for the line rendering.
  //! The render can keep states persistent from one line drawing to the other, without initializing and restoring them.
  //!
  //! Not implemented (Software OpenGL).
  virtual bool StartSuccessiveLineDrawings( void ) override { return false; } 

  //! Notify the renderer that a sequence of lines has been finished, and that the internal states have to be restored.
  //!
  //! Not implemented (Software OpenGL).
  virtual bool FinishSuccessiveLineDrawings( void ) override { return false; }

  virtual Render::Line::IRender & SetColor( const Render::TColor & color ) override;
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

  bool         _active_sequence{ false }; //!< true: an draw sequence was started, but not finished yet
  unsigned int _tuple_size{ 0 };          //!< tuple size (2, 3 or 4) for a sequence
};


} // Line


} // OpenGL


#endif // OpenGLLine_1_00_h_INCLUDED