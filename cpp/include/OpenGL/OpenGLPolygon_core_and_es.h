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
#pragma once
#ifndef OpenGLPolygon_4_h_INCLUDED
#define OpenGLPolygon_4_h_INCLUDED

// includes

#include <Render_IDrawType.h>
#include <Render_IDrawPolygon.h>
#include <Render_IProgram.h>


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
* \brief Implementation of OpenGL polygon renderer,
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
class CPolygonOpenGL_core_and_es
  : public Render::Polygon::IRender
{
public: 

  CPolygonOpenGL_core_and_es( size_t min_cache_elems );
  virtual ~CPolygonOpenGL_core_and_es();

// TODO $$$


};

} // Poylgon

} // OpenGL

#endif OpenGLPolygon_4_h_INCLUDED