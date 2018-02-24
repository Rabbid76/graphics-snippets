/******************************************************************//**
* \brief   General interface for drawing bunches of primitives.
* 
* \author  gernot
* \date    2017-11-20
* \version 1.0
**********************************************************************/
#pragma once
#ifndef IDraw_h_INCLUDED
#define IDraw_h_INCLUDED


// includes

#include <IDrawType.h>



/******************************************************************//**
* \brief namespace for generic interface of drawing operations
**********************************************************************/
namespace Render
{


/******************************************************************//**
* \brief   Usage of drawing buffer.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
enum class TDrawBufferUsage
{
  static_draw,  //!< set up data once and draw repeatly
  dynamic_draw, //!< change the data sometimes
  stream_draw,  //!< set up data and draw once
};


/******************************************************************//**
* \brief   Generic primitive type
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
enum class TPrimitive
{
  points,
  lines,
  linestrip,
  lineloop,
  lines_adjacency,
  linestrip_adjacency,
  triangles,
  trianglestrip,
  trianglefan,
  triangle_adjacency,
  trianglestrip_adjacency,
  patches
};


/******************************************************************//**
* \brief   The type of an attribute element
* 
* \author  gernot
* \date    2017-11-29
* \version 1.0
**********************************************************************/
enum TAttributeType : char
{
  eFloat32,
  eFloat64,
  eAttribUInt8,
  eAttribUInt16,
  eAttribUInt32
};

/******************************************************************//**
* \brief   The type of an index element
* 
* \author  gernot
* \date    2017-11-29
* \version 1.0
**********************************************************************/
enum TIndexType : char
{
  eElemUInt8,
  eElemUInt16,
  eElemUInt32
};


/******************************************************************//**
* \brief   
* 
* interface fpr creating and managing vertex arrays with the following description
*
* <index buffer>              : index of the index buffer; 0 is default; <= -1 no index buffer is reqired
* <no of buffers>             : the number of the require vertex buffer objects - followed by the list of vbo specifications
* {
*     <array buffer index>    : index of the array buffer
*     <stride>                : stride from one vertex attribute set to the next, in float (4 byte) units) 
*     <no of attributes>      : number of the generic vertex attributes in the buffer - followed by the list of attribute specifications
*     {
*         <attribute index>   : vertex attribute index or client state
*         <attribute size>    : number of elemts of the vertex attribute
*         <attribute type>    : type of an attribute (`TAttribType`)
*         <attribute offset>  : offset of the vertex attributes from the begin of the attributes set in float (4 byte) units) 
*     }
* }
*
*
* e.g. Strided record sets:  
*      Vx0, Vy0, Vz0, Nx0, Ny0, Nz0, Tu0, Tv0,
*      Vx1, Vy1, Vz1, Nx1, Ny1, Nz1, Tu1, Tv1,
*      ....
*      
*      [
*        0, 1, 
*              0, 8, 3,
*                       -1, 3, 0, 0,
*                       -2, 3, 0, 3,
*                       -3, 2, 0, 6
*      ]
*
*
* e.g. Tightly packed vertex attributes:
*      Vx0, Vy0, Vz0, Vx1, Vy1, Vz1, .... 
*      Nx0, Ny0, Nz0, Nx1, Ny1, Nz1, ....
*      Tu0, Tv0, Tu1, Tv1 ....
*
*      [
*        0, 3,
*              0, 0, 1, 
*                       -1, 3, 0, 0,
*              1, 0, 1,
*                       -2, 3, 0, 0,
*              2, 0, 1,
*                       -3, 2, 0, 0
*      ]
*
*
* Implementations of this interface should use RAII (Resource acquisition is initialization) technique.
* [https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization]
*
*
* \author  gernot
* \date    2017-11-20
* \version 1.0
**********************************************************************/
class IDrawBuffer
{
public: 
  
  // types

  enum THeadOffset
  {
    eHeadOffset_ibo,
    eHeadOffset_no_of_vbo,
    eHeadSize
  };

  enum TVboOffset
  { 
    eVboOffset_index,
    eVboOffset_stride,
    eVboOffset_no_of_attributes,
    eVboSize
  };

  enum TAttributeOffset
  { 
    eAtrributeOffset_id,
    eAtrributeOffset_size,
    eAtrributeOffset_type,
    eAtrributeOffset_offset,
    eAttributeSize
  };

  // abstract operations

  virtual ~IDrawBuffer() {}

  virtual size_t NoOf( void ) const = 0;
  virtual bool   Selected( void ) const = 0;

  virtual void SpecifyVA( size_t description_size, const char *description ) = 0;
  virtual bool UpdateVB( size_t id, size_t element_size, size_t no_of_elements, const void *data ) = 0;
  virtual bool UpdateIB( size_t id, size_t element_size, size_t no_of_elements, const void *data ) = 0;

  virtual void DrawAllElements( TPrimitive primitive_type, bool bind ) = 0;
  virtual void DrawElements( TPrimitive primitive_type, size_t start, size_t count, bool bind ) = 0;
  virtual void DrawElements( TPrimitive primitive_type, size_t element_size, size_t no_of_elements, const void *data, bool bind ) = 0;
  virtual void DrawElements( TPrimitive primitive_type, size_t element_size, size_t list_size, const int *no_of_elements, const void * const *data, bool bind ) = 0;
  virtual void DrawRangeElements( TPrimitive primitive_type, unsigned int minInx, unsigned int maxInx, bool bind ) = 0;
  virtual void DrawArray( TPrimitive primitive_type, size_t first, size_t count, bool bind  ) = 0;

  virtual void Prepare( void ) = 0;
  virtual void Release( void ) = 0;
};


} // Draw


#endif // IDraw_h_INCLUDED