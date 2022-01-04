/******************************************************************//**
* \brief   OpenG Lheader files.
* 
* \author  gernot
* \date    2018-02-04
* \version 1.0
**********************************************************************/
#pragma once
#ifndef OpenGL_enumconst_h_INCLUDED
#define OpenGL_enumconst_h_INCLUDED


// includes

#include "../render/Render_IDrawType.h"


// OpenGL wrapper

#include "OpenGL_include.h"


// STL

#include <array>



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
* \brief   Convert primitive type to OpenGL enumerator constant. 
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
inline GLenum Primitive( Render::TPrimitive primitive )
{
  static_assert((int)Render::TPrimitive::NO_OF == 12, "invalid number of primitives");
  static const std::array<GLenum, (int)Render::TPrimitive::NO_OF> primitive_map
  {
    GL_POINTS,                   // TPrimitive::points
    GL_LINES,                    // TPrimitive::lines
    GL_LINE_STRIP,               // TPrimitive::linestrip
    GL_LINE_LOOP,                // TPrimitive::lineloop
    GL_LINES_ADJACENCY,          // TPrimitive::lines_adjacency
    GL_LINE_STRIP_ADJACENCY,     // TPrimitive::linestrip_adjacency
    GL_TRIANGLES,                // TPrimitive::triangles
    GL_TRIANGLE_STRIP,           // TPrimitive::trianglestrip
    GL_TRIANGLE_FAN,             // TPrimitive::trianglefan
    GL_TRIANGLES_ADJACENCY,      // TPrimitive::triangle_adjacency
    GL_TRIANGLE_STRIP_ADJACENCY, // TPrimitive::trianglestrip_adjacency
    GL_PATCHES                   // TPrimitive::patches
  };
  return primitive_map[(int)primitive];
}

 
}


#endif // OpenGL_enumconst_h_INCLUDED
