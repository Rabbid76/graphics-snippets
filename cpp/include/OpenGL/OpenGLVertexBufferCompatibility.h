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

#include <Render_IBuffer.h>
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

  // compatibility buffer specification shortcuts

  enum TCompSpecificationID
  {
    comp__b0_xyz,                // compatibility 1 buffer (no index buffer): 3 vertex coordiantes
    
    comp__b0_xyz_uv,             // compatibility 1 buffer record (no index buffer): 3 vertex coordiantes, 2 texture coordinates
    comp__b0_xyz_nnn,            // compatibility 1 buffer record (no index buffer): 3 vertex coordiantes, normal vector
    comp__b0_xyz_nnn_uv,         // compatibility 1 buffer record (no index buffer): 3 vertex coordiantes, normal vector, 2 texture coordinates

    comp__b0_xyz__b1_uv,         // compatibility 2 buffers (no index buffer): 3 vertex coordiantes, 2 texture coordinates
    comp__b0_xyz__b1_nnn,        // compatibility 2 buffers (no index buffer): 3 vertex coordiantes, normal vector
    comp__b0_xyz__b1_nnn__b2_uv, // compatibility 3 buffers (no index buffer): 3 vertex coordiantes, normal vector, 2 texture coordinates
  };

  static const TDescription & CompatibilitySpecification( TCompSpecificationID id )
  {
    static const TDescription spec_table[] = {
    
      // comp__b0_xyz_uv
      TDescription{
        -1, 1, 
        0, 5, 2, 
        OpenGL::Compatibility::eCLIENT_VERTEX,  3, Render::TAttributeType::eFloat32, 0,
        OpenGL::Compatibility::eCLIENT_TEXTURE, 2, Render::TAttributeType::eFloat32, 3,
      },

      // comp__b0_xyz_nnn
      TDescription{
        -1, 1, 
        0, 6, 2, 
        OpenGL::Compatibility::eCLIENT_VERTEX, 3, Render::TAttributeType::eFloat32, 0,
        OpenGL::Compatibility::eCLIENT_NORMAL, 3, Render::TAttributeType::eFloat32, 3,
      },

      // comp__b0_xyz_nnn_uv
      TDescription {
        -1, 1, 
        0, 8, 3, 
        OpenGL::Compatibility::eCLIENT_VERTEX,  3, Render::TAttributeType::eFloat32, 0,
        OpenGL::Compatibility::eCLIENT_NORMAL,  3, Render::TAttributeType::eFloat32, 3,
        OpenGL::Compatibility::eCLIENT_TEXTURE, 2, Render::TAttributeType::eFloat32, 6,
      },

      // comp__b0_xyz__b1_uv
      TDescription{
        -1, 3, 
        0, 0, 1, OpenGL::Compatibility::eCLIENT_VERTEX,  3, Render::TAttributeType::eFloat32, 0,
        2, 0, 1, OpenGL::Compatibility::eCLIENT_TEXTURE, 2, Render::TAttributeType::eFloat32, 0 
      },

      // comp__b0_xyz__b1_nnn
      TDescription{
        -1, 2, 
        0, 0, 1, OpenGL::Compatibility::eCLIENT_VERTEX,  3, Render::TAttributeType::eFloat32, 0,
        1, 0, 1, OpenGL::Compatibility::eCLIENT_NORMAL,  3, Render::TAttributeType::eFloat32, 0
      },

      // comp__b0_xyz__b1_nnn__b2_uv
      TDescription{
        -1, 3, 
        0, 0, 1, OpenGL::Compatibility::eCLIENT_VERTEX,  3, Render::TAttributeType::eFloat32, 0,
        1, 0, 1, OpenGL::Compatibility::eCLIENT_NORMAL,  3, Render::TAttributeType::eFloat32, 0,
        2, 0, 1, OpenGL::Compatibility::eCLIENT_TEXTURE, 2, Render::TAttributeType::eFloat32, 0
      }
    };

    return spec_table[(int)id];
  }
};

} // Compatibility


} // OpenGL 


#endif // OpenGLVertexBuffer_h_INCLUDED
