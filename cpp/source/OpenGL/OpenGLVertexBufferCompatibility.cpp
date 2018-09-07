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

// includes

#include <stdafx.h>

#include <OpenGLError.h>
#include <OpenGLVertexBufferCompatibility.h>


// OpenGL wrapper

#include <OpenGL_include.h>


// STL

#include <assert.h>



// class implementations


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
* \brief   default ctor
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
CDrawBuffer::CDrawBuffer( void )
{}


/******************************************************************//**
* \brief   ctor
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
CDrawBuffer::CDrawBuffer( 
  Render::TDrawBufferUsage usage,       //!< ! - usage of vertex buffer
  size_t                   minVboSize ) //!< I -  minimum size of vertex buffer object
  : OpenGL::CDrawBuffer( usage, minVboSize )
{}


/******************************************************************//**
* \brief   dtor
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
CDrawBuffer::~CDrawBuffer()
{}


/******************************************************************//**
* \brief   move constructor
* 
* \author  gernot
* \date    2017-11-29
* \version 1.0
**********************************************************************/
CDrawBuffer::CDrawBuffer( 
  CDrawBuffer && src ) //!< I - source object
  : OpenGL::CDrawBuffer( std::move(src) )
{}


/******************************************************************//**
* \brief   move assignment operator
* 
* \author  gernot
* \date    2017-11-29
* \version 1.0
**********************************************************************/
CDrawBuffer & CDrawBuffer::operator = ( 
  CDrawBuffer && src ) //!< I - source object
{
  OpenGL::CDrawBuffer::operator =( std::move(src) );
   return * this;
}


/******************************************************************//**
* \brief   Define and enable an array of generic vertex attribute or
* define and enable a client array.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
void CDrawBuffer::DefineAndEnableAttribute( 
  int                    attr_id,   //!< I - attribute index
  int                    attr_size, //!< I - size of attribute: 1, 2, 3 or 4
  Render::TAttributeType elem_type, //!< I - type id of an element
  int                    attr_offs, //!< I - offset of the attribute in the attribute record set
  int                    stride     //!< I - stride between two attribute record sets 
  ) const
{
  if ( attr_id >= 0 )
  {
    OpenGL::CDrawBuffer::DefineAndEnableAttribute(attr_id, attr_size, elem_type, attr_offs, stride);
    return;
  }
  
  unsigned int  opebgl_type       = DataType( elem_type );
  void         *opengl_offset_ptr = (void*)( stride == 0 ? 0 : (size_t)attr_offs );

  switch ( attr_id )
  {
    default: assert( false ); break;

    case eCLIENT_VERTEX:   
      glVertexPointer( attr_size, opebgl_type, stride, opengl_offset_ptr );
      glEnableClientState( GL_VERTEX_ARRAY );
      break;

    case eCLIENT_NORMAL:  
      glNormalPointer( opebgl_type, stride, opengl_offset_ptr ); 
      glEnableClientState( GL_NORMAL_ARRAY );
      break;

    case eCLIENT_TEXTURE:  
      glTexCoordPointer( attr_size, opebgl_type, stride, opengl_offset_ptr );
      glEnableClientState( GL_TEXTURE_COORD_ARRAY );
      break;

    case eCLIENT_COLOR:    
      glColorPointer( attr_size, opebgl_type, stride, opengl_offset_ptr ); 
      glEnableClientState( GL_COLOR_ARRAY );
      break;

    case eCLIENT_COLOR_2:
      glSecondaryColorPointer( attr_size, opebgl_type, stride, opengl_offset_ptr ); 
      glEnableClientState( GL_SECONDARY_COLOR_ARRAY );
      break;

    case eCLIENT_INDEX:    
      glIndexPointer( attr_size, stride, opengl_offset_ptr ); 
      glEnableClientState( GL_INDEX_ARRAY );
      break;

    case eCLIENT_EDGE_FLAG:
      glEdgeFlagPointer ( stride, opengl_offset_ptr ); 
      glEnableClientState( GL_EDGE_FLAG_ARRAY );
      break;

    case eCLIENT_FOG_COORD:
      glFogCoordPointer ( attr_size, stride, opengl_offset_ptr ); 
      glEnableClientState( GL_FOG_COORD_ARRAY );
      break;
  }
  OPENGL_CHECK_GL_ERROR
}


/******************************************************************//**
* \brief   disable an array of generic vertex attribute or
* disable a client array.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
void CDrawBuffer::DisableAttribute( 
  int attr_id //!< I - attribute index
  ) const
{
  if ( attr_id >= 0 )
    OpenGL::CDrawBuffer::DisableAttribute(attr_id);

  switch ( attr_id )
  {
    default: assert( false ); break;
    case eCLIENT_VERTEX:    glDisableClientState( GL_VERTEX_ARRAY ); break;
    case eCLIENT_NORMAL:    glDisableClientState( GL_NORMAL_ARRAY );break;
    case eCLIENT_TEXTURE:   glDisableClientState( GL_TEXTURE_COORD_ARRAY ); break;
    case eCLIENT_COLOR:     glDisableClientState( GL_COLOR_ARRAY ); break;
    case eCLIENT_COLOR_2:   glDisableClientState( GL_SECONDARY_COLOR_ARRAY ); break;
    case eCLIENT_INDEX:     glDisableClientState( GL_INDEX_ARRAY ); break;
    case eCLIENT_EDGE_FLAG: glDisableClientState( GL_EDGE_FLAG_ARRAY ); break;
    case eCLIENT_FOG_COORD: glDisableClientState( GL_FOG_COORD_ARRAY ); break;
  }
  OPENGL_CHECK_GL_ERROR
}

} // Compatibility

} // OpenGL