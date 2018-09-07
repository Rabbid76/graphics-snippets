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

// includes

#include <stdafx.h>

#include <OpenGLError.h>
#include <OpenGLVertexBuffer_2_0.h>


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
CDrawBuffer_2_0::CDrawBuffer_2_0( void )
{}


/******************************************************************//**
* \brief   ctor
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
CDrawBuffer_2_0::CDrawBuffer_2_0( 
  Render::TDrawBufferUsage usage,       //!< ! - usage of vertex buffer
  size_t                   minVboSize ) //!< I -  minimum size of vertex buffer object
  : CDrawBuffer( usage, minVboSize )
{}


/******************************************************************//**
* \brief   dtor
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
CDrawBuffer_2_0::~CDrawBuffer_2_0()
{}


/******************************************************************//**
* \brief   move constructor
* 
* \author  gernot
* \date    2017-11-29
* \version 1.0
**********************************************************************/
CDrawBuffer_2_0::CDrawBuffer_2_0( 
  CDrawBuffer_2_0 && src ) //!< I - source object
  : CDrawBuffer( std::move(src) )
{
  _lastVaoEmulationId = src._lastVaoEmulationId, src._lastVaoEmulationId = 0;
}


/******************************************************************//**
* \brief   move assignment operator
* 
* \author  gernot
* \date    2017-11-29
* \version 1.0
**********************************************************************/
CDrawBuffer_2_0 & CDrawBuffer_2_0::operator = ( 
  CDrawBuffer_2_0 && src ) //!< I - source object
{
  CDrawBuffer::operator =( std::move(src) );
  _lastVaoEmulationId = src._lastVaoEmulationId, src._lastVaoEmulationId = 0;
  return * this;
}


/******************************************************************//**
* \brief   Unbind any vertex array objects.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
void CDrawBuffer_2_0::UnbindVAO( void )
{
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
  OPENGL_CHECK_GL_ERROR

  // find the current vertex array object emulation
  if ( _currentVAO == 0 )
    return;
  TVAO *vao_ptr = FindExistingVAO( _currentVAO );
  if ( vao_ptr == nullptr )
    return;

  const TDescription &key = std::get<c_descr>( *vao_ptr );
  int i_ibo        = key[eHeadOffset_ibo];       // index buffer id (< 0 means no index buffer)
  size_t no_of_vbo = key[eHeadOffset_no_of_vbo]; // number of array buffers

  // disable client states
  int i_key = eHeadSize;
  for ( int i_vbo=0; i_vbo<no_of_vbo; ++ i_vbo )
  {
    int buffer_inx = key[i_key + eVboOffset_index];            // internal index of the array buffer
    int stride     = key[i_key + eVboOffset_stride];           // (stride / 4) from one attribute to the next attribute
    int no_of_attr = key[i_key + eVboOffset_no_of_attributes]; // number of attributes in the set
    i_key += eVboSize;
    for ( int i_attr=0; i_attr<no_of_attr; ++ i_attr )
    {
      int attr_id   = key[i_key + eAtrributeOffset_id];     // attribute index or client state id
      int attr_size = key[i_key + eAtrributeOffset_size];   // size of the attribute 1, 2, 3 or 4
      int attr_type = key[i_key + eAtrributeOffset_type];   // type id of the attribute
      int attr_offs = key[i_key + eAtrributeOffset_offset]; // (offset / 4) of the attribute in the attribute set
      i_key += eAttributeSize;
      DisableAttribute( attr_id );
    }
 }
}


/******************************************************************//**
* \brief   Bind the currently selected vertex array object.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
void CDrawBuffer_2_0::BindVAO( void )
{
  // find the current vertex array object emulation
  assert( _currentVAO != 0 );
  if ( _currentVAO == 0 )
    return;
  TVAO *vao_ptr = FindExistingVAO( _currentVAO );
  assert( vao_ptr != nullptr );
  if ( vao_ptr == nullptr )
    return;

  // setup the client states and pointers and bind element array buffer (optional)
  PreprateAttributesAndIndices( std::get<c_descr>( *vao_ptr ) );
}


/******************************************************************//**
* \brief   If a vertex array object with the description exists,
* then it is becomes the current vertex array object.  
* If no vertex array object exists, which follows the description,
* then a new vertex array object is created and the new object is made
* the current object.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
void CDrawBuffer_2_0::SpecifyVA( 
  size_t      description_size, //!< I - size of description array
  const char *description )     //!< I - description - specification of vertices and indices
{
  // Create description key array
  THashCode hashCode = HashDescription( description_size, description );
  
  // Set current number of elements
  if ( FindExistingVAO( hashCode, description_size, description ) )
    return;

  // Check if the required buffers are exists and create them if they are not created yet.
  int    i_ibo     = description[eHeadOffset_ibo];       // index buffer id (< 0 means no index buffer)
  size_t no_of_vbo = description[eHeadOffset_no_of_vbo]; // number of array buffers
  CreateMissingBuffers( i_ibo, no_of_vbo );
  
  // Emulate vertex array object
  TDescription key( description_size );
  std::copy( description, description + description_size, key.begin() );
  _lastVaoEmulationId ++;
  _currentVAO = _lastVaoEmulationId;
  _vaos[hashCode] = std::make_tuple( _currentVAO, key );
}


} // Compatibility

} // OpenGL