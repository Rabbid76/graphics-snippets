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
#include <Render_IProgram.h>


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
{
public:

  CLineOpenGL_core_and_es( Render::Program::TViewDataPtr view_data );
  virtual ~CLineOpenGL_core_and_es();



private:

  Render::Program::TViewDataPtr _view_data; //!< model view and projection, to be applied to the shader program

  // TODO : comments
  // The implementation is based on the OpenGL X.XX core mode specification [link] - line tipple etc.
  // Line stipple specification
};


} // Line


} // OpenGL


#endif // OpenGLLine_4_h_INCLUDED
