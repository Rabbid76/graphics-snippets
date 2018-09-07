/******************************************************************//**
* \brief Implementation of OpenGL line renderer,
* with the use of very simple and highly optimized shaders,
* for OpenGL version 2.00 and GLSL version 1.10 (`#version 110`).
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/

#include <stdafx.h>

// includes

#include <OpenGLLine_2_0.h>


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
CLineOpenGL_2_00::CLineOpenGL_2_00( void )
{}


/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
CLineOpenGL_2_00::~CLineOpenGL_2_00()
{}



} // Line


} // OpenGL