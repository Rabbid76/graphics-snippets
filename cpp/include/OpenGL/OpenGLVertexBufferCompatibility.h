/******************************************************************//**
* \brief   Generic Interface for OpenGL vertex buffers and
* vertex array objects.
* Requires at least OpenGL 3.0.
*
* Supports Fixed Function Pipeline.
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
#ifndef OpenGLVertexBufferCompatibility_h_INCLUDED
#define OpenGLVertexBufferCompatibility_h_INCLUDED


// includes

#include <IBuffer.h>
#include <OpenGLFixedFunctionPipelineTypes.h>
#include <OpenGLVertexBuffer.h>


// class declarations


/******************************************************************//**
* \brief General OpenGL namespace
**********************************************************************/
namespace OpenGL
{


/******************************************************************//**
* \brief OpenGL compatibility (Fixed Function Pipeline) namespace
**********************************************************************/
namespace Compatibility
{


/******************************************************************//**
* \brief   OpenGL implementation of `IDrawBuffer`
* 
* Uses vertex array buffers.
* Extetends Legacy OpenGL (Fixed Function Pipeline) support
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
class CDrawBuffer 
  :  public OpenGL::CDrawBuffer
{
public: // public operations

  CDrawBuffer( void );
  CDrawBuffer( Render::TDrawBufferUsage usage, size_t minVboSize );

  CDrawBuffer( CDrawBuffer && );
  virtual CDrawBuffer & operator = ( CDrawBuffer && );

  virtual ~CDrawBuffer();

protected: // protected operations

  virtual void DefineAndEnableAttribute( int attr_id, int attr_size, Render::TAttributeType elem_type, int attr_offs, int stride ) const override;
  virtual void DisableAttribute( int attr_id ) const;
};

} // Compatibility


} // OpenGL 


#endif // OpenGLVertexBuffer_h_INCLUDED
