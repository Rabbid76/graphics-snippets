/******************************************************************//**
* \brief Type definitions for the abstraction of deprecated OpenGL
* Fixed Function Pipeline.
*
* See:
*   - [Khronos wiki - Fixed Function Pipeline](https://www.khronos.org/opengl/wiki/Fixed_Function_Pipeline)
*   - [Khronos wiki - Legacy OpenGL](https://www.khronos.org/opengl/wiki/Legacy_OpenGL)
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
#pragma once
#ifndef OpenGLFixedFunctionPiplineTypes_h_INCLUDED
#define OpenGLFixedFunctionPiplineTypes_h_INCLUDED


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
* \brief OpenGL compatibility (Fixed Function Pipeline) namespace
**********************************************************************/
namespace Compatibility
{


/******************************************************************//**
* \brief   Compatibility mode attributes kinds and client states
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
enum TClinetAttribute
{
  eCLIENT_VERTEX    = -1, //!< GL_VERTEX_ARRAY
  eCLIENT_NORMAL    = -2, //!< GL_NORMAL_ARRAY
  eCLIENT_TEXTURE   = -3, //!< GL_TEXTURE_COORD_ARRAY
  eCLIENT_COLOR     = -4, //!< GL_COLOR_ARRAY
  eCLIENT_COLOR_2   = -5, //!< GL_SECONDARY_COLOR_ARRAY
  eCLIENT_INDEX     = -6, //!< GL_INDEX_ARRAY
  eCLIENT_EDGE_FLAG = -7, //!< GL_EDGE_FLAG_ARRAY
  eCLIENT_FOG_COORD = -8, //!< GL_FOG_COORD_ARRAY  
};


} // Compatibility


} // OpenGL


#endif // OpenGLFixedFunctionPiplineTypes_h_INCLUDED
