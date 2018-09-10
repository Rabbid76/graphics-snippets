/******************************************************************//**
* \brief Implementation of OpenGL line renderer,
* with the use of "modern" OpenGL (4+) and hight quality shaders,
* for OpenGL version 4+ and GLSL version 4.20 (`#version 420`).
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
* for OpenGL version 4+ and GLSL version 4.20 (`#version 420`).
*
* Line stippling techniques is implemented in the shader program.
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/
class CLineOpenGL_4_core
{
public:

  //CLineOpenGL_4_core( void );
  //virtual ~CLineOpenGL_4_core();

  // TODO : comments
// The implementation is based on the OpenGL X.XX compatibility mode specification [link] - line tipple etc.
};


/******************************************************************//**
* \brief Implementation of OpenGL line renderer,
* with the use of "modern" OpenGL (4+) and hight quality shaders,
* for OpenGL version 4+ and GLSL version 4.20 (`#version 420`).
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/
class CLineHighQuality
{
public:

  CLineHighQuality( void );
  virtual ~CLineHighQuality();

  // TODO : comments
  // The implementation is based on the ... link to paper
};


} // Line


} // OpenGL


#endif // OpenGLLine_4_h_INCLUDED
