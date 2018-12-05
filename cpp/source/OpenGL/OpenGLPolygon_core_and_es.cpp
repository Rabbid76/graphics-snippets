/******************************************************************//**
* \brief Implementation of OpenGL polygon renderer,
* with the use of "modern" OpenGL 4+ core and
* GLSL version 4.20 (`#version 420 core`) or
* OpenGL es 3(+) and GLSL ES version 3.00 (`#version 300 es`).
* 
* \author  gernot
* \date    2018-12-05
* \version 1.0
**********************************************************************/

#include <stdafx.h>

// includes

#include <OpenGLPolygon_core_and_es.h>
#include <OpenGLProgram.h>


// OpenGL wrapper

#include <OpenGL_include.h>
#include <OpenGL_enumconst.h>


// STL

#include <iostream>
#include <algorithm>


// class definitions


/******************************************************************//**
* \brief General namespace for OpenGL implementation.  
* 
* \author  gernot
* \date    2018-12-05
* \version 1.0
**********************************************************************/
namespace OpenGL
{


/******************************************************************//**
* \brief Namespace for drawing polygons with the use of OpenGL.  
* 
* \author  gernot
* \date    2018-12-05
* \version 1.0
**********************************************************************/
namespace Polygon
{


/******************************************************************//**
* \brief ctor.  
* 
* \author  gernot
* \date    2018-12-05
* \version 1.0
**********************************************************************/
CPolygonOpenGL_core_and_es::CPolygonOpenGL_core_and_es( 
  size_t min_cache_elems ) //! I - size of the element cache
{}
  

/******************************************************************//**
* \brief dtor.  
* 
* \author  gernot
* \date    2018-12-05
* \version 1.0
**********************************************************************/
CPolygonOpenGL_core_and_es::~CPolygonOpenGL_core_and_es()
{}


} // Poylgon

} // OpenGL