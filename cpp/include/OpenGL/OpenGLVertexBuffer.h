/******************************************************************//**
* \brief   Generic Interface for OpenGL vertex buffers and
* vertex array objects.
* Requires at least OpenGL 3.0.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
#pragma once
#ifndef OpenGLVertexBuffer_h_INCLUDED
#define OpenGLVertexBuffer_h_INCLUDED


// includes

#include <IBuffer.h>

#include <tuple>
#include <vector>
#include <map>


// class declarations


/******************************************************************//**
* \brief General OpenGL namespace
**********************************************************************/
namespace OpenGL
{


//*********************************************************************
// CDrawBuffer
//*********************************************************************


/******************************************************************//**
* \brief   OpenGL implementation of `IDrawBuffer`
* 
* Uses vertex array buffers.
*
* Uses RAII (Resource acquisition is initialization) technique.
* [https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization]
* This means that this class is not copyable or copy constructible,
* but move assignable and move constructible.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
class CDrawBuffer 
  :  public Render::IDrawBuffer
{
public: // public types

  using TGPUObj      = unsigned int;                                //!< GPU object name
  using THashCode    = size_t;                                      //!< key of a vertex array content
  using TVBO         = std::tuple<TGPUObj, size_t, size_t>;         //!< <GPU name> of an array buffer, size of the buffer  and number of array elements
  using TIBO         = std::tuple<TGPUObj, size_t, size_t, size_t>; //!< <GPU name> of an element array buffer, size of the element buffer, number of array elements, size of one array element
  using TVAO         = std::tuple<TGPUObj, TDescription>;           //!< <GPU name> of a vertex array object and the description of its content
  using TVBOs        = std::vector<TVBO>;                           //!< array buffer container type 
  using TIBOs        = std::map<size_t, TIBO>;                      //!< element array buffer container type
  using TVAOs        = std::map<THashCode, TVAO>;                   //!< vertex array object container type 

  constexpr static const int c_obj   = 0; //!< tuple index for GPU object
  constexpr static const int c_descr = 1; //!< tuple index for descripton
  constexpr static const int c_size  = 1; //!< tuple index for size of buffer
  constexpr static const int c_count = 2; //!< tuple index for number of elements
  constexpr static const int c_elemS = 3; //!< tuple index for size of one elements

public: // public operations

  CDrawBuffer( void );
  CDrawBuffer( Render::TDrawBufferUsage usage, size_t minVboSize );

  CDrawBuffer( const CDrawBuffer & ) = delete;
  CDrawBuffer & operator = ( const CDrawBuffer & ) = delete;

  CDrawBuffer( CDrawBuffer && );
  virtual CDrawBuffer & operator = ( CDrawBuffer && );

  virtual ~CDrawBuffer();

  virtual size_t NoOf( void ) const { return _vaos.size(); }
  virtual bool   Selected( void ) const { return _currentVAO != 0; }

  virtual void SpecifyVA( size_t description_size, const char *description ) override;
  virtual bool UpdateVB( size_t id, size_t element_size, size_t no_of_elements, const void *data ) override;
  virtual bool UpdateIB( size_t id, size_t element_size, size_t no_of_elements, const void *data ) override;

  virtual void DrawAllElements( Render::TPrimitive primitive_type, bool bind ) override;
  virtual void DrawElements( Render::TPrimitive primitive_type, size_t start, size_t count, bool bind ) override;
  virtual void DrawElements( Render::TPrimitive primitive_type, size_t element_size, size_t no_of_elements, const void *data, bool bind ) override;
  virtual void DrawElements( Render::TPrimitive primitive_type, size_t element_size, size_t list_size, const int *no_of_elements, const void * const *data, bool bind ) override;
  virtual void DrawRangeElements( Render::TPrimitive primitive_type, unsigned int minInx, unsigned int maxInx, bool bind ) override;
  virtual void DrawArray( Render::TPrimitive primitive_type, size_t start, size_t count, bool bind  ) override;

  virtual void Prepare( void ) override;
  virtual void Release( void ) override;


protected: // protected operations

  unsigned int PrimitiveType( Render::TPrimitive primitove_type ) const;
  unsigned int DataType( Render::TAttributeType elem_type ) const;
  unsigned int IndexType( size_t element_size ) const;
  unsigned int Usage( void ) const;
  virtual void UnbindVAO( void );
  virtual void BindVAO( void );
  virtual void DefineAndEnableAttribute( int attr_id, int attr_size, Render::TAttributeType elem_type, int attr_offs, int stride ) const;
  virtual void DisableAttribute( int attr_id ) const;
  void         PreprateAttributesAndIndices( const TDescription &key );
  void         CreateMissingBuffers( int i_ibo, size_t no_of_vbo );
  size_t       UpdateBuffer( int type, TGPUObj bo, size_t curr_size, size_t min_size, size_t data_size, const void *data ) const;
  THashCode    HashDescription( size_t description_size, const char *description ) const;
  bool         FindExistingVAO( THashCode hashCode, size_t description_size, const char *description );
  TVAO *       FindExistingVAO( TGPUObj vao );
 
protected: // protected attributes

  size_t                 _minVboSize   = 0;                                   //!< minimum size for a array buffe
  Render::TDrawBufferUsage _usage        = Render::TDrawBufferUsage::stream_draw; //!< usage of drawing buffer
  TGPUObj                _currentVAO   = 0;                                   //!< current selected vertex array object <GPU name>
  size_t                 _currNoElems  = 0;                                   //!< number of elements in the curently selected vertex array object
  size_t                 _currElemSize = 0;                                   //!< size of an element in the curently selected vertex array object
  TVBOs                  _vbos;                                               //!< map destcription -> (vertex array object <GPU name>, description)
  TIBOs                  _ibos;                                               //!< map index -> ( element array buffer <GPU name>, size <count> of element array buffer )
  TVAOs                  _vaos;                                               //!< list of array buffers ( array buffer <GPU name>, size <count> of array buffer )
};


} // OpenGL 


#endif // OpenGLVertexBuffer_h_INCLUDED
