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

#include <stdafx.h>

// includes

#include <OpenGLLine_core_and_es.h>


// OpenGL wrapper

#include <OpenGL_include.h>


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
* \brief ctor  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
CLineOpenGL_core_and_es::CLineOpenGL_core_and_es( 
  Render::Program::TViewDataPtr view_data )
  : _view_data( view_data )
{}


/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
CLineOpenGL_core_and_es::~CLineOpenGL_core_and_es()
{}



} // Line


} // OpenGL
