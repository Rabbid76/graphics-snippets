/******************************************************************//**
* \brief Implementation of OpenGL line renderer,
* with the use of "modern" OpenGL (4+) and hight quality shaders,
* for OpenGL version 4+ and GLSL version 4.20 (`#version 420`).
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/

#include <stdafx.h>

// includes

#include "../../include/OpenGL/OpenGLLine_highquality.h"


// OpenGL wrapper

#include "../../include/OpenGL/OpenGL_include.h"


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
CLineHighQuality::CLineHighQuality( void )
{}


/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
CLineHighQuality::~CLineHighQuality()
{}



} // Line


} // OpenGL