/******************************************************************//**
* \brief   Generic Interface for OpenGL vertex buffers and
* vertex array objects.
* Requires at least OpenGL 3.0.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/

// includes

#include <stdafx.h>

#include <OpenGLVertexBuffer.h>


// OpenGL wrapper

#include <OpenGL_include.h>


// STL

#include <array>
#include <algorithm>
#include <assert.h>


// class implementations


/******************************************************************//**
* \brief General OpenGL namespace
**********************************************************************/
namespace OpenGL
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
  Render::TDrawBufferUsage  usage,       //!< ! - usage of vertex buffer
  size_t                    minVboSize ) //!< I -  minimum size of vertex buffer object
  : _usage( usage )
  , _minVboSize( minVboSize )
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
{
  _minVboSize   = src._minVboSize,   src._minVboSize = 0;
  _usage        = src._usage,        src._usage = Render::TDrawBufferUsage::stream_draw;
  _currentVAO   = src._currentVAO,   src._currentVAO = 0;
  _currNoElems  = src._currNoElems,  src._currNoElems = 0;
  _currElemSize = src._currElemSize, src._currElemSize = 0;
  _vbos         = std::move( src._vbos );
  _ibos         = std::move( src._ibos );
  _vaos         = std::move( src._vaos );
  _shortcuts    = std::move( src._shortcuts );
}


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
  _minVboSize   = src._minVboSize,   src._minVboSize = 0;
  _usage        = src._usage,        src._usage = Render::TDrawBufferUsage::stream_draw;
  _currentVAO   = src._currentVAO,   src._currentVAO = 0;
  _currNoElems  = src._currNoElems,  src._currNoElems = 0;
  _currElemSize = src._currElemSize, src._currElemSize = 0;
  _vbos         = std::move( src._vbos );
  _ibos         = std::move( src._ibos );
  _vaos         = std::move( src._vaos );
  _shortcuts    = std::move( src._shortcuts );

  return * this;
}


/******************************************************************//**
* \brief   dtor
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
CDrawBuffer::~CDrawBuffer()
{
  // unbind any vertex array
  UnbindAnyVertexArrayObject();
  OPENGL_CHECK_GL_ERROR

  // delete array buffers
  std::vector<TGPUObj> vbos( _vbos.size() );
  std::transform( _vbos.begin(), _vbos.end(), vbos.begin(), [](auto &vbo) -> TGPUObj { return std::get<c_obj>( vbo ); } );
  if ( vbos.empty() == false )
  {
    glDeleteBuffers( static_cast<GLsizei>(vbos.size()), vbos.data() );
    OPENGL_CHECK_GL_ERROR
  }

  // delete element array buffers
  std::vector<TGPUObj> ibos( _ibos.size() );
  std::transform( _ibos.begin(), _ibos.end(), ibos.begin(), [](auto &ibo) -> TGPUObj { return std::get<c_obj>( ibo.second ); } );
  if ( ibos.empty() == false )
  {
    glDeleteBuffers( static_cast<GLsizei>(ibos.size()), ibos.data() );
    OPENGL_CHECK_GL_ERROR
  }

  // delete vertex array objects
  std::vector<TGPUObj> vaos( _vaos.size() );
  std::transform( _vaos.begin(), _vaos.end(), vaos.begin(), [](auto &vao) -> TGPUObj { return std::get<c_obj>( vao.second ); } );
  if ( vaos.empty() == false )
  {
    glDeleteVertexArrays( static_cast<GLsizei>(vaos.size()), vaos.data() );
    OPENGL_CHECK_GL_ERROR
  }
}


/******************************************************************//**
* \brief   Bind a named vertex array object.
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
void CDrawBuffer::BindVertexArrayObject( 
  unsigned int currentVAO ) //!< I - vertex array object
{
  glBindVertexArray( currentVAO );
}


/******************************************************************//**
* \brief   Bind the default vertex array object 0.
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
void CDrawBuffer::UnbindAnyVertexArrayObject( void )
{
  glBindVertexArray( 0 );
}


/******************************************************************//**
* \brief   Return OpenGL primitive type enumerator for a 
* generic primitive type 
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
unsigned int CDrawBuffer::PrimitiveType( 
  Render::TPrimitive primitive_type //!< I - primitive id
  ) const
{
  switch ( primitive_type )
  {
    case Render::TPrimitive::points:                  return GL_POINTS;
    case Render::TPrimitive::lines:                   return GL_LINES;
    case Render::TPrimitive::linestrip:               return GL_LINE_STRIP;
    case Render::TPrimitive::lineloop:                return GL_LINE_LOOP;
    case Render::TPrimitive::lines_adjacency:         return GL_LINES_ADJACENCY;
    case Render::TPrimitive::linestrip_adjacency:     return GL_LINE_STRIP_ADJACENCY;
    case Render::TPrimitive::triangles:               return GL_TRIANGLES;
    case Render::TPrimitive::trianglestrip:           return GL_TRIANGLE_STRIP;
    case Render::TPrimitive::trianglefan:             return GL_TRIANGLE_FAN;
    case Render::TPrimitive::triangle_adjacency:      return GL_TRIANGLES_ADJACENCY;
    case Render::TPrimitive::trianglestrip_adjacency: return GL_POINTS;
    case Render::TPrimitive::patches:                 return GL_PATCHES;
  }

  assert( false );
  return GL_TRIANGLES;
}


/******************************************************************//**
* \brief   Return OpenGL type enumerator for an vertex array data type. 
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
unsigned int CDrawBuffer::DataType(
  Render::TAttributeType elem_type //!< I - type id of one element
  ) const
{
  static const std::vector<unsigned int> opengl_types
  {
    GL_FLOAT, GL_DOUBLE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT
  };
  return opengl_types[static_cast<int>( elem_type )];
}


/******************************************************************//**
* \brief    Return OpenGL type enumerator for an vertex element array
* data type.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
unsigned int CDrawBuffer::IndexType( 
  size_t element_size //!< I - size of one element in bytes
  ) const
{
  switch ( element_size )
  {
    case 1: return GL_UNSIGNED_BYTE;
    case 2: return GL_UNSIGNED_SHORT;
    default:
    case 4: return GL_UNSIGNED_INT;
  }
}


/******************************************************************//**
* \brief  Return OpenGL usage enumerator 
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
unsigned int CDrawBuffer::Usage( void ) const
{
  const static std::array<GLenum, 3> usage{ GL_STATIC_DRAW, GL_DYNAMIC_DRAW, GL_STREAM_DRAW };
  return usage[(int)_usage];
}


/******************************************************************//**
* \brief   Unbind any vertex array objects.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
void CDrawBuffer::UnbindVAO( void )
{
  BindVertexArrayObject( 0 );
  OPENGL_CHECK_GL_ERROR
}


/******************************************************************//**
* \brief   Bind the currently selected vertex array object.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
void CDrawBuffer::BindVAO( void )
{
  assert( _currentVAO != 0 );
  BindVertexArrayObject( _currentVAO );
  OPENGL_CHECK_GL_ERROR
}


/******************************************************************//**
* \brief   Define and enable an array of generic vertex attribute.
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
  assert( attr_id >= 0 );
  if ( attr_id < 0 )
    return;
  
  glVertexAttribPointer( attr_id, attr_size, DataType(elem_type), GL_FALSE, stride, (void*)(stride == 0 ? 0 : (size_t)attr_offs) );
  glEnableVertexAttribArray( attr_id );
  OPENGL_CHECK_GL_ERROR
}


/******************************************************************//**
* \brief   disable an array of generic vertex attribute.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
void CDrawBuffer::DisableAttribute( 
  int attr_id //!< I - attribute index
  ) const
{
  assert( attr_id >= 0 );
  if ( attr_id < 0 )
    return;
  glDisableVertexAttribArray( attr_id );
  OPENGL_CHECK_GL_ERROR
}


/******************************************************************//**
* \brief   Define and enable the arrays of generic vertex attributes
* and bind the index buffer (optional)
* 
* \author  gernot
* \date    2017-11-2z
* \version 1.0
**********************************************************************/
void CDrawBuffer::PreprateAttributesAndIndices( 
  const TDescription &key ) //!< I - description - specification of vertices and indices
{
  int i_ibo        = key[eHeadOffset_ibo];       // index buffer id (< 0 means no index buffer)
  size_t no_of_vbo = key[eHeadOffset_no_of_vbo]; // number of array buffers

  // Create a new vertex array object according to the description
  int i_key = eHeadSize;
  for ( int i_vbo=0; i_vbo<no_of_vbo; ++ i_vbo )
  {
    int buffer_inx = key[i_key + eVboOffset_index];            // internal index of the array buffer
    int stride     = key[i_key + eVboOffset_stride];           // (stride / 4) from one attribute to the next attribute
    int no_of_attr = key[i_key + eVboOffset_no_of_attributes]; // number of attributes in the set
    i_key += eVboSize;
    glBindBuffer( GL_ARRAY_BUFFER, std::get<c_obj>( _vbos[buffer_inx] ) );
    for ( int i_attr=0; i_attr<no_of_attr; ++ i_attr )
    {
      int attr_id   = key[i_key + eAtrributeOffset_id];     // attribute index or client state id
      int attr_size = key[i_key + eAtrributeOffset_size];   // size of the attribute 1, 2, 3 or 4
      int attr_type = key[i_key + eAtrributeOffset_type];   // type id of the attribute
      int attr_offs = key[i_key + eAtrributeOffset_offset]; // (offset / 4) of the attribute in the attribute set
      i_key += eAttributeSize;
      this->DefineAndEnableAttribute( attr_id, attr_size, static_cast<Render::TAttributeType>(attr_type), attr_offs*4, stride*4 );
    }
  }
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  OPENGL_CHECK_GL_ERROR

  // Associate the element array buffer (index buffer) to the vertex array object
  if ( i_ibo >= 0 )
  {
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, std::get<c_obj>( _ibos[i_ibo] ) );
    OPENGL_CHECK_GL_ERROR
  }
}


/******************************************************************//**
* \brief   Create all the required array buffers and optional the 
* element array buffer
* 
* \author  gernot
* \date    2017-11-2z
* \version 1.0
**********************************************************************/
void CDrawBuffer::CreateMissingBuffers( 
  int    i_ibo,      //!< I - id of the element array buffer
  size_t no_of_vbo ) //!< I - number of the required array buffers
{
  auto iboIt = _ibos.find( i_ibo );
  if ( i_ibo >= 0 && iboIt == _ibos.end() )
  {
    GLuint ibo = 0;
    glGenBuffers( 1, &ibo );
    OPENGL_CHECK_GL_ERROR
    _ibos[i_ibo] = std::make_tuple( ibo, 0, 0, 0 );
  }
  if ( _vbos.size() < no_of_vbo )
  {
    std::vector<GLuint> vbos( no_of_vbo - _vbos.size() );
    glGenBuffers( static_cast<GLsizei>( vbos.size() ), vbos.data() );
    OPENGL_CHECK_GL_ERROR
    for ( auto vbo : vbos )
      _vbos.emplace_back( vbo, 0, 0 );
  }
}


/******************************************************************//**
* \brief   Set data to buffer
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
size_t CDrawBuffer::UpdateBuffer( 
  int         type,      //!< I - array buffer or element array buffer (GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER)
  TGPUObj     bo,        //!< I - buffer object <GPU name>
  size_t      curr_size, //!< I - current buffer size
  size_t      min_size,  //!< I - minimum size for the buffer
  size_t      data_size, //!< I - new buffer size
  const void *data       //!< I - pointer to the data
  ) const
{
  glBindBuffer( type, bo );
  OPENGL_CHECK_GL_ERROR

  if ( curr_size == 0 || data_size > curr_size )
  {
    if ( data_size >= min_size )
    {
      glBufferData( type, static_cast<GLsizei>(data_size), data, Usage() );
      glBindBuffer( type, 0 );
      OPENGL_CHECK_GL_ERROR
      return data_size;
    }
    else
    {
      glBufferData( type, static_cast<GLsizei>(min_size), nullptr, Usage() );
      glBufferSubData( type, 0, static_cast<GLsizei>(data_size), data );
      glBindBuffer( type, 0 );
      OPENGL_CHECK_GL_ERROR
      return min_size;
    }
  }
  
  glBufferSubData( type, 0, static_cast<GLsizei>(data_size), data );
  glBindBuffer( type, 0 );
  OPENGL_CHECK_GL_ERROR
  return curr_size;
}


/******************************************************************//**
* \brief Add a shortcut to a vertex array object specification.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CDrawBuffer::AddSortcut( 
  TShortcut   shortcut,
  size_t      description_size,
  const char *description )
{
  // Create description key array
  THashCode hashCode = HashDescription( description_size, description );

  // check if the shortcut already exists
  auto it = _shortcuts.find( shortcut );
  if ( it != _shortcuts.end() )
    return hashCode == it->second; // check if the hash code is the same
  
  // add the new shortcut
  _shortcuts[shortcut] = hashCode;

  return true;
}


/******************************************************************//**
* \brief Find a vertex array object by its shortcut and  make it 
* current. 
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CDrawBuffer::SelectVA(
   TShortcut shortcut ) //!< vertex array object shortcut
{
  // find the description hash code to the shortcut 
  auto it = _shortcuts.find( shortcut );
  if ( it == _shortcuts.end() )
    return false;
  
  THashCode hashC = it->second;

  // Check if a proper vertex array object already exists
  auto vaoIt = _vaos.find( hashC );
  if ( vaoIt != _vaos.end() )
  {
    _currentVAO = std::get<c_obj>( vaoIt->second );
    return true;
  }

  return false;
}


/******************************************************************//**
* \brief   If a vertex array object with the given description exists,
* then it is becomes the current vertex array object.  
* If no vertex array object exists, which follows the description,
* then a new vertex array object is created and the new object is made
* the current object.
* Note, the vertex array object is not bound, it is unbound.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
void CDrawBuffer::SpecifyVA( 
  size_t      description_size, //!< I - size of description array
  const char *description )     //!< I - description - specification of vertices and indices
{
  // Create description key array
  THashCode hashCode = HashDescription( description_size, description );
  
  // Set current number of elements
  if ( FindExistingVAO( hashCode, description_size, description ) )
    return;

  // Check if the required buffers are exists and create them if they are not created yet.
  int    i_ibo      = description[eHeadOffset_ibo];       // index buffer id (< 0 means no index buffer)
  size_t no_of_vbo  = description[eHeadOffset_no_of_vbo]; // number of array buffers
  int    i_key      = eHeadSize;
  size_t max_buffer = 0;
  for ( int i_vbo=0; i_vbo<no_of_vbo; ++ i_vbo )
  {
    int buffer_inx = description[i_key + eVboOffset_index];            // internal index of the array buffer
    if ( buffer_inx > max_buffer )
      max_buffer = buffer_inx;

    int no_of_attr = description[i_key + eVboOffset_no_of_attributes]; // number of attributes in the set
    i_key += (eVboSize + no_of_attr*eAttributeSize);
  }
  CreateMissingBuffers( i_ibo, max_buffer+1 );
  
  // Create vertex array object
  glGenVertexArrays( 1, &_currentVAO );
  OPENGL_CHECK_GL_ERROR
  TDescription key( description_size );
  std::copy( description, description + description_size, key.begin() );
  _vaos[hashCode] = std::make_tuple( _currentVAO, key );
  this->BindVAO();

  // Create a new vertex array object according to the description
  PreprateAttributesAndIndices( key );

  // Unbind the vertex array object
  this->UnbindVAO();

  // Unbind the element array buffer
  // This has to be done after the vertex array object is unbound, otherwise the association to the vertex array object would be lost.
  if ( i_ibo >= 0 )
  {
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    OPENGL_CHECK_GL_ERROR
  }
}


/******************************************************************//**
* \brief   Update array buffer data.
* If the current buffer is large enough for the data it is reused,
* but if the current buffer is to small it is recreated.
* Reusing is much faster (`glBufferSubData`) than recreating
* (`glBufferData`)
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
bool CDrawBuffer::UpdateVB( 
  size_t      id,             //!< I - internal index of the array buffer 
  size_t      element_size,   //!< I - size of one array element 
  size_t      no_of_elements, //!< I - number of the new data
  const void *data )          //!< I - the new data
{
  if ( id >= _vbos.size() )
  {
    assert( false );
    return false;
  }

  TGPUObj vbo        = std::get<c_obj>(   _vbos[id] );
  size_t &curr_size  = std::get<c_size>(  _vbos[id] );
  size_t &curr_elems = std::get<c_count>( _vbos[id] );
  size_t vbo_size    = no_of_elements * element_size;
  curr_size  = this->UpdateBuffer( GL_ARRAY_BUFFER, vbo, curr_size, _minVboSize, vbo_size, data );
  curr_elems = no_of_elements;
  return true;
}


/******************************************************************//**
* \brief   Update array buffer data.
* If the current buffer is large enough for the data it is reused,
* but if the current buffer is to small it is recreated.
* Reusing is much faster (`glBufferSubData`) than recreating
* (`glBufferData`)
*
* see [Vertex Specification; Index buffers](https://www.khronos.org/opengl/wiki/Vertex_Specification#Index_buffers)
  The index buffer binding is stored within the VAO. If no VAO is bound, then you cannot bind a buffer object to GL_ELEMENT_ARRAY_BUFFER.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
bool CDrawBuffer::UpdateIB( 
  size_t      id,             //!< I - internal id of the element array buffer 
  size_t      element_size,   //!< I - size of one array element 
  size_t      no_of_elements, //!< I - number of the new data
  const void *data )          //!< I - the new data
{
  auto ibIt = _ibos.find( id );
  if ( ibIt == _ibos.end() )
  {
    assert( false );
    return false;
  }

  TGPUObj ibo            = std::get<c_obj>(   ibIt->second );
  size_t &curr_size      = std::get<c_size>(  ibIt->second );
  size_t &curr_noOfElems = std::get<c_count>( ibIt->second );
  size_t &curr_elemSize  = std::get<c_elemS>( ibIt->second );
  size_t data_size       = no_of_elements * element_size;
  curr_size      = this->UpdateBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo, curr_size, data_size, data_size, data );
  curr_noOfElems = no_of_elements;
  curr_elemSize  = element_size;

  // update current size of elements
  CDrawBuffer::TVAO *vao_ptr = CDrawBuffer::FindExistingVAO( this->_currentVAO );
  if ( vao_ptr != nullptr && std::get<c_descr>( *vao_ptr )[0] == id )
  {
    _currNoElems  = no_of_elements;  
    _currElemSize = element_size;
  }

  return true;
}


/******************************************************************//**
* \brief   Draw the indices of the current index buffer.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
void CDrawBuffer::DrawAllElements( 
  Render::TPrimitive primitive_type, //!< I - OpenGL primitive type
  bool               bind )          //!< I - true : vertex array object  will be bound; false: vertex array object is already bound
{
  if ( _currNoElems == 0 )
    return;
  if ( bind )
    this->BindVAO();
  glDrawElements( PrimitiveType( primitive_type ), static_cast<GLsizei>( _currNoElems ), IndexType( _currElemSize ), nullptr );
  OPENGL_CHECK_GL_ERROR
}


/******************************************************************//**
* \brief   Draw `count` indices of the current index buffer, starting
* at `start`.
* If `count == 0`, all the indices from `start` to the end of the index
* buffer will be drawn.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
void CDrawBuffer::DrawElements( 
  Render::TPrimitive primitive_type, //!< I - OpenGL primitive type
  size_t             start,          //!< I - first index to be draw
  size_t             count,          //!< I - number of indices to be drawn (if == 0, all the indices from `start` to the end of the index buffer will be drawn)
  bool               bind )          //!< I - true : vertex array object  will be bound; false: vertex array object is already bound
{
  if ( _currNoElems == 0 || start >= _currNoElems )
    return;
  if ( bind )
    this->BindVAO();
  size_t noOfElements = (count == 0 || start + count > _currNoElems) ? _currNoElems - start : count;
  glDrawElements( PrimitiveType( primitive_type ), static_cast<GLsizei>( noOfElements ), IndexType( _currElemSize ), (void*)(_currElemSize * start) );
  OPENGL_CHECK_GL_ERROR
}



/******************************************************************//**
* \brief   Draw the indices which are given by the index array.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
void CDrawBuffer::DrawElements( 
  Render::TPrimitive  primitive_type, //!< I - OpenGL primitive type
  size_t              element_size,   //!< I - size of one array element 
  size_t              no_of_elements, //!< I - number of the new data
  const void         *data,           //!< I - the new data
  bool                bind )          //!< I - true : vertex array object  will be bound; false: vertex array object is already bound
{
  if ( bind )
    this->BindVAO();
  glDrawElements( PrimitiveType( primitive_type ), static_cast<GLsizei>( no_of_elements ), IndexType( element_size ), data );
  OPENGL_CHECK_GL_ERROR
}


/******************************************************************//**
* \brief   
* 
* \author  gernot
* \date    2017-11-26
* \version 1.0
**********************************************************************/
void CDrawBuffer::DrawElements( 
  Render::TPrimitive  primitive_type, //!< I - OpenGL primitive type
  size_t              element_size,   //!< I - size of one array element 
  size_t              list_size,      //!< I - length of the list
  const int          *no_of_elements, //!< I - list of the element counts
  const void * const *data,           //!< I - list of the indices arrays
  bool                bind )          //!< I - true : vertex array object  will be bound; false: vertex array object is already bound
{
  if ( bind )
    this->BindVAO();
  glMultiDrawElements( PrimitiveType( primitive_type ), no_of_elements, IndexType( element_size ), data, static_cast<GLsizei>( list_size ) );
  OPENGL_CHECK_GL_ERROR
}


/******************************************************************//**
* \brief   Draw `count` indices of the current index buffer, starting
* at `start`.
* If `count == 0`, all the indices from `start` to the end of the index
* buffer will be drawn.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
void CDrawBuffer::DrawElementsBase( 
  Render::TPrimitive primitive_type, //!< I - OpenGL primitive type
  size_t             start,          //!< I - first index to be draw
  size_t             count,          //!< I - number of indices to be drawn (if == 0, all the indices from `start` to the end of the index buffer will be drawn)
  size_t             base_index,     //!< I - constant that should be added to each element of indices when choosing elements from the enabled vertex arrays
  bool               bind )          //!< I - true : vertex array object  will be bound; false: vertex array object is already bound
{
  if ( _currNoElems == 0 || start >= _currNoElems )
    return;
  if ( bind )
    this->BindVAO();
  size_t noOfElements = (count == 0 || start + count > _currNoElems) ? _currNoElems - start : count;
  glDrawElementsBaseVertex( PrimitiveType( primitive_type ), static_cast<GLsizei>( noOfElements ), IndexType( _currElemSize ), (void*)(_currElemSize * start), static_cast<GLint>( base_index ) );
  OPENGL_CHECK_GL_ERROR
}



/******************************************************************//**
* \brief   Draw the indices which are given by the index array.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
void CDrawBuffer::DrawElementsBase( 
  Render::TPrimitive  primitive_type, //!< I - OpenGL primitive type
  size_t              element_size,   //!< I - size of one array element 
  size_t              no_of_elements, //!< I - number of the new data
  const void         *data,           //!< I - the new data
  size_t              base_index,     //!< I - constant that should be added to each element of indices when choosing elements from the enabled vertex arrays
  bool                bind )          //!< I - true : vertex array object  will be bound; false: vertex array object is already bound
{
  if ( bind )
    this->BindVAO();
  glDrawElementsBaseVertex( PrimitiveType( primitive_type ), static_cast<GLsizei>( no_of_elements ), IndexType( element_size ), const_cast<void*>(data), static_cast<GLint>( base_index ) );
  OPENGL_CHECK_GL_ERROR
}


/******************************************************************//**
* \brief   Draw the elements of the current index buffer, if the index
* is greater or equal than `minInx` and less or equal than `maxInx` 
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
void CDrawBuffer::DrawRangeElements( 
  Render::TPrimitive primitive_type, //!< I - OpenGL primitive type
  unsigned int       minInx,         //!< I - start index
  unsigned int       maxInx,         //!< I - end index
  bool               bind )          //!< I - true : vertex array object  will be bound; false: vertex array object is already bound
{
  if ( _currNoElems == 0 )
    return;
  if ( bind )
    this->BindVAO();
  glDrawRangeElements( PrimitiveType( primitive_type ), minInx, maxInx, static_cast<GLsizei>( _currNoElems ), IndexType( _currElemSize ), nullptr );
  OPENGL_CHECK_GL_ERROR
}


/******************************************************************//**
* \brief   Draw a range of the current vertices.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
void CDrawBuffer::DrawArray( 
  Render::TPrimitive primitive_type, //!< I - OpenGL primitive type
  size_t             first,          //!< I - first vertex
  size_t             count,          //!< I - number of vertices 
  bool               bind )          //!< I - true : vertex array object  will be bound; false: vertex array object is already bound
{
  if ( bind )
    this->BindVAO();
  glDrawArrays( PrimitiveType( primitive_type ), static_cast<GLsizei>( first ), static_cast<GLsizei>( count ) );
  OPENGL_CHECK_GL_ERROR
}


/******************************************************************//**
* \brief   Bind the current vertex array objects
* 
* \author  gernot
* \date    2017-11-25
* \version 1.0
**********************************************************************/
void CDrawBuffer::Prepare( void )
{
  this->BindVAO();
}


/******************************************************************//**
* \brief   Unbind any vertex array objects
* 
* \author  gernot
* \date    2017-11-25
* \version 1.0
**********************************************************************/
void CDrawBuffer::Release( void )
{
  this->UnbindVAO();
}

 
/******************************************************************//**
* \brief   Compute hash code from description array
* 
* \author  gernot
* \date    2017-11-26
* \version 1.0
**********************************************************************/
CDrawBuffer::THashCode CDrawBuffer::HashDescription( 
  size_t      description_size, //!< I - size of description array
  const char *description       //!< I - description - specification of vertices and indices
  ) const
{
  THashCode hashCode = 5381;
  for ( size_t i = 0; i<description_size; ++i )
    hashCode = hashCode * 33 + hashCode + description[i];
  return hashCode;
}


/******************************************************************//**
* \brief   Find an existing vertex array object by a buffer description.
* Make the found object the current object.
* Note, the object is not bound on the GPU.
* 
* \author  gernot   
* \date    2017-11-27
* \version 1.0
**********************************************************************/
bool CDrawBuffer::FindExistingVAO( 
  THashCode   hashCode,         //!< I - optional the hash code of the description, (if == 0 the hash code is computed)
  size_t      description_size, //!< I - size of description array
  const char *description )     //!< I - description - specification of vertices and indices
{
  THashCode hashC = hashCode == 0 ? HashDescription( description_size, description ) : hashCode;
  
  int i_ibo = description[eHeadOffset_ibo]; // index buffer id (< 0 means no index buffer)
  auto iboIt = _ibos.find( i_ibo );
  _currNoElems  = iboIt != _ibos.end() ? std::get<c_count>( iboIt->second ) : 0;
  _currElemSize = iboIt != _ibos.end() ? std::get<c_elemS>( iboIt->second ) : 0;
 
  // Check if a proper vertex array object already exists
  auto vaoIt = _vaos.find( hashC );
  if ( vaoIt != _vaos.end() )
  {
    _currentVAO = std::get<c_obj>( vaoIt->second );
    return true;
  }

  return false;
}


/******************************************************************//**
* \brief   Find an existing vertex array object by a vertex array object.
* Note, the object is not bound on the GPU.
* 
* \author  gernot   
* \date    2017-11-27
* \version 1.0
**********************************************************************/
CDrawBuffer::TVAO * CDrawBuffer::FindExistingVAO( 
  TGPUObj vao ) //!< I - vertex array object <GPU name>
{
  auto vaoIt = std::find_if( _vaos.begin(), _vaos.end(), [&](auto &vaoElem) -> bool {
    return std::get<c_obj>( vaoElem.second ) == vao;
  } );
  return vaoIt != _vaos.end() ? &(vaoIt->second) : nullptr;
}

} // OpenGL