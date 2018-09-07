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
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/
class CLineOpenGL_2_00
{
public:

  CLineOpenGL_2_00( void );
  virtual ~CLineOpenGL_2_00();
};


} // Line


} // OpenGL


#endif // OpenGLLine_2_00_h_INCLUDED
