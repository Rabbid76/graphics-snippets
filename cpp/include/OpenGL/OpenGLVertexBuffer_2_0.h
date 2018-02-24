/******************************************************************//**
* \brief   Generic Interface for OpenGL vertex buffers and.
* Does not vertex array objects and requires at least OpenGL 2.0.
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
#ifndef OpenGLVertexBuffer_2_0_h_INCLUDED
#define OpenGLVertexBuffer_2_0_h_INCLUDED


// includes

#include <IBuffer.h>
#include <OpenGLFixedFunctionPipelineTypes.h>
#include <OpenGLVertexBufferCompatibility.h>


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
* Does NOT vertex array buffers and requires at least OpenGL 2.0.
* Extetends Legacy OpenGL (Fixed Function Pipeline) support
*
* The vertex array object is emulated on the CPU.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
class CDrawBuffer_2_0 
  :  public CDrawBuffer
{
public: // public operations

  CDrawBuffer_2_0( void );
  CDrawBuffer_2_0( Render::TDrawBufferUsage usage, size_t minVboSize );

  CDrawBuffer_2_0( CDrawBuffer_2_0 && );
  virtual CDrawBuffer_2_0 & operator = ( CDrawBuffer_2_0 && );

  virtual ~CDrawBuffer_2_0();

  virtual void SpecifyVA( size_t description_size, const char *description ) override;

protected: // protected operations

  virtual void UnbindVAO( void );
  virtual void BindVAO( void );

private: // private attributes

  TGPUObj _lastVaoEmulationId = 0;
  
};


} // Compatibility


} // OpenGL 


#endif // OpenGLVertexBuffer_h_INCLUDED

