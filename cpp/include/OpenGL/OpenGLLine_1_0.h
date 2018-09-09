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

  virtual CLineOpenGL_1_00 & SetColor( const Render::TColor & color ) override;
  virtual CLineOpenGL_1_00 & SetColor( const Render::TColor8 & color ) override;
  virtual CLineOpenGL_1_00 & SetStyle( const Render::Line::TStyle & style ) override;
  virtual CLineOpenGL_1_00 & SetArrowStyle( const Render::Line::TArrowStyle & style ) override;

  //! Draw a line sequence
  virtual bool Draw( Render::TPrimitive primitive_type, unsigned int tuple_size, size_t coords_size, const float *coords ) override;
  virtual bool Draw( Render::TPrimitive primitive_type, unsigned int tuple_size, size_t coords_size, const double *coords ) override;
  
private:

  bool _active_sequence = false;
};


} // Line


} // OpenGL


#endif // OpenGLLine_1_00_h_INCLUDED