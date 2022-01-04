/******************************************************************//**
* \brief   General interface for drawing bunches of primitives.
* 
* \author  gernot
* \date    2017-11-20
* \version 1.0
**********************************************************************/
#pragma once
#ifndef Render_IBuffer_h_INCLUDED
#define Render_IBuffer_h_INCLUDED


// includes

#include <Render_IDrawType.h>

#include <vector>
#include <memory>



/******************************************************************//**
* \brief namespace for generic interface of drawing operations
**********************************************************************/
namespace Render
{


class IDrawBuffer;
using IDrawBufferPtr = std::unique_ptr<IDrawBuffer>;



//---------------------------------------------------------------------
// IDrawBufferProvider
//---------------------------------------------------------------------


/******************************************************************//**
* \brief   Usage of drawing buffer.
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
enum class TDrawBufferUsage
{
  static_draw,  //!< set up data once and draw repeatedly
  dynamic_draw, //!< change the data sometimes
  stream_draw,  //!< set up data and draw once
};


/******************************************************************//**
* \brief   Generic provider for generic draw buffers.
* 
* \author  gernot
* \date    2018-06-08
* \version 1.0
**********************************************************************/
class IDrawBufferProvider
{
public:

  virtual ~IDrawBufferProvider() = default;

  virtual IDrawBufferPtr   NewDrawBuffer( TDrawBufferUsage usage ) = 0;
  virtual IDrawBuffer    & DrawBuffer( void ) = 0;
  virtual IDrawBuffer    & DrawBuffer( const void *key, bool &cached ) = 0;
};



//---------------------------------------------------------------------
// IDrawBuffer
//---------------------------------------------------------------------


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


enum TVA
{
  unknown,                   // unknown: dummy for initialization

  b0_xy,                     // 1 vertex buffer (no index buffer): 2 component vertex coordinate
  b0_xyz,                    // 1 vertex buffer (no index buffer): 3 component vertex coordinate
  b0_xyzw,                   // 1 vertex buffer (no index buffer): 4 component vertex coordinate

  b0_x__b1_y,                // 2 vertex buffer (no index buffer): x component vertex coordinate and separated y component vertex coordinate

  d__b0_xy,                  // double precision: 1 vertex buffer (no index buffer): 2 component vertex coordinate
  d__b0_xyz,                 // double precision: 1 vertex buffer (no index buffer): 3 component vertex coordinate
  d__b0_xyzw,                // double precision: 1 vertex buffer (no index buffer): 4 component vertex coordinate

  d__b0_x__b1_y,             // double precision: 2 vertex buffer (no index buffer): x component vertex coordinate and separated y component vertex coordinate
                  
  b0_xyz_uv,                 // 1 vertex buffer record (no index buffer): 3 component vertex coordinate, 2 component texture coordinates
  b0_xyz_nnn,                // 1 vertex buffer record (no index buffer): 3 component vertex coordinate, normal vector
  b0_xyz_nnn_uv,             // 1 vertex buffer record (no index buffer): 3 component vertex coordinate, normal vector, 2 component texture coordinate

  b0_xyz__b1_uv,             // 2 vertex buffers (no index buffer): 3 component vertex coordinate, 2 component texture coordinate
  b0_xyz__b1_nnn,            // 2 vertex buffers (no index buffer): 3 component vertex coordinate, normal vector
  b0_xyz__b1_nnn__b2_uv,     // 3 vertex buffers (no index buffer): 3 component vertex coordinate, normal vector, 2 component texture coordinate

  i0__b0_xy,                 // 1 index buffer; 1 vertex buffer: 2 component vertex coordinate
  i0__b0_xyz,                // 1 index buffer; 1 vertex buffer: 3 component vertex coordinate
                  
  i0__b0_xyz_uv,             // 1 index buffer; 1 vertex buffer:  3 component vertex coordinate, 2 component texture coordinate
  i0__b0_xyz_nnn,            // 1 index buffer; 1 vertex buffer:  3 component vertex coordinate, normal vector
  i0__b0_xyz_nnn_uv,         // 1 index buffer; 1 vertex buffer:  3 component vertex coordinate, normal vector, 2 component texture coordinate

  i0__b0_xyz__b1_uv,         // 1 index buffer; 2 vertex buffers: 3 component vertex coordinate, 2 component texture coordinate
  i0__b0_xyz__b1_nnn,        // 1 index buffer; 2 vertex buffers: 3 component vertex coordinate, normal vector
  i0__b0_xyz__b1_nnn__b2_uv, // 1 index buffer; 3 vertex buffers: 3 component vertex coordinate, normal vector, 2 component texture coordinate

  b0_xy__b1_rgba,            // 2 vertex buffers (no index buffer): 2 component vertex coordinate, RGBA color
  b0_xyz__b1_rgba,           // 2 vertex buffers (no index buffer): 3 component vertex coordinate, RGBA color
  b0_xyzw__b1_rgba,          // 2 vertex buffers (no index buffer): 3 component vertex coordinate, RGBA color
};


/******************************************************************//**
* \brief   
* 
* interface for creating and managing vertex arrays with the following description
*
* <index buffer>              : index of the index buffer; 0 is default; <= -1 no index buffer is required
* <no of buffers>             : the number of the require vertex buffer objects - followed by the list of VBO specifications
* {
*     <array buffer index>    : index of the array buffer
*     <stride>                : stride from one vertex attribute set to the next, in float (4 byte) units) 
*     <no of attributes>      : number of the generic vertex attributes in the buffer - followed by the list of attribute specifications
*     {
*         <attribute index>   : vertex attribute identification; e.g. attribute index 0, 1, 2 ... or client state -1, -2, -3 ...
*         <attribute size>    : number of elements of the vertex attribute
*         <attribute type>    : type of an attribute (`TAttribType`)
*         <attribute offset>  : offset of the vertex attributes from the begin of the attributes set in float (4 byte) units) 
*     }
* }
*
*
* e.g. Stride record sets:  
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
* The implementations of this interface should make use of 
* RAII (Resource acquisition is initialization) technique.
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

  using TShortcut = int;

  // abstract operations

  virtual ~IDrawBuffer() {}

  virtual size_t NoOf( void ) const = 0;
  virtual bool   Selected( void ) const = 0;

  bool SpecifyVA( TVA description_id )
  {
    if ( SelectVA( description_id ) )
      return true;

    const auto &description = VADescription(description_id); 
    SpecifyVA( description.size(), description.data() );
    return AddSortcut( (int)description_id, description.size(), description.data() );
  }

  bool AddSortcut( TShortcut shortcut, const std::vector<char> & description )
  {
    return AddSortcut( shortcut, description.size(), description.data() );
  }

  void SpecifyVA( const std::vector<char> & description )
  {
    SpecifyVA( description.size(), description.data() );
  }

  void UpdateVB( size_t id, size_t element_size, const std::vector<t_fp> &buffer )
  {
    UpdateVB( id, element_size, buffer.size(), buffer.data() );
  }

  virtual bool AddSortcut( TShortcut shortcut, size_t description_size, const char *description ) = 0;
  virtual bool SelectVA( TShortcut shortcut ) = 0;
  virtual void SpecifyVA( size_t description_size, const char *description ) = 0;
  virtual bool UpdateVB( size_t id, size_t element_size, size_t no_of_elements, const void *data ) = 0;
  virtual bool UpdateIB( size_t id, size_t element_size, size_t no_of_elements, const void *data ) = 0;

  virtual void DrawAllElements( TPrimitive primitive_type, bool bind ) = 0;
  virtual void DrawElements( TPrimitive primitive_type, size_t start, size_t count, bool bind ) = 0;
  virtual void DrawElements( TPrimitive primitive_type, size_t element_size, size_t no_of_elements, const void *data, bool bind ) = 0;
  virtual void DrawElements( TPrimitive primitive_type, size_t element_size, size_t list_size, const int *no_of_elements, const void * const *data, bool bind ) = 0;
  virtual void DrawElementsBase( TPrimitive primitive_type, size_t start, size_t count, size_t base_index, bool bind ) = 0;
  virtual void DrawElementsBase( TPrimitive primitive_type, size_t element_size, size_t no_of_elements, const void *data, size_t base_index, bool bind ) = 0;
  virtual void DrawRangeElements( TPrimitive primitive_type, unsigned int minInx, unsigned int maxInx, bool bind ) = 0;
  virtual void DrawArray( TPrimitive primitive_type, size_t first, size_t count, bool bind  ) = 0;

  virtual void Prepare( void ) = 0;
  virtual void Release( void ) = 0;


  // buffer specification shortcuts

  using TDescription = std::vector<char>; //!< description of a vertex array object content
  
  static const TDescription & VADescription( TVA id )
  {
    static const TDescription spec_table[] = {
    
      // unknown
      TDescription(),

      // b0_xy
      TDescription{
        -1, 1, 
        0, 0, 1, 
        0, 2, Render::TAttributeType::eFloat32, 0
      },

      // b0_xyz
      TDescription{
        -1, 1, 
        0, 0, 1, 
        0, 3, Render::TAttributeType::eFloat32, 0
      },

      // b0_xyzw
      TDescription{
        -1, 1, 
        0, 0, 1, 
        0, 4, Render::TAttributeType::eFloat32, 0
      },

      // b0_x__b1_y
      TDescription{
        -1, 2, 
        0, 0, 1, 0, 1, Render::TAttributeType::eFloat32, 0,
        1, 0, 1, 1, 1, Render::TAttributeType::eFloat32, 0,
      },

      // d__b0_xy
      TDescription{
        -1, 1, 
        0, 0, 1, 
        0, 2, Render::TAttributeType::eFloat64, 0
      },

      // d__b0_xyz
      TDescription{
        -1, 1, 
        0, 0, 1, 
        0, 3, Render::TAttributeType::eFloat64, 0
      },

      // d__b0_xyzw
      TDescription{
        -1, 1, 
        0, 0, 1, 
        0, 4, Render::TAttributeType::eFloat64, 0
      },

      // d__b0_x__b1_y
      TDescription{
        -1, 2, 
        0, 0, 1, 0, 1, Render::TAttributeType::eFloat64, 0,
        1, 0, 1, 1, 1, Render::TAttributeType::eFloat64, 0,
      },

      // b0_xyz_uv
      TDescription{
        -1, 1, 
        0, 5, 2, 
        0, 3, Render::TAttributeType::eFloat32, 0,
        1, 2, Render::TAttributeType::eFloat32, 3,
      },

      // b0_xyz_nnn
      TDescription{
        -1, 1, 
        0, 6, 2, 
        0, 3, Render::TAttributeType::eFloat32, 0,
        1, 3, Render::TAttributeType::eFloat32, 3,
      },

      // b0_xyz_nnn_uv
      TDescription{
        -1, 1, 
        0, 8, 3, 
        0, 3, Render::TAttributeType::eFloat32, 0,
        1, 3, Render::TAttributeType::eFloat32, 3,
        2, 2, Render::TAttributeType::eFloat32, 6,
      },

      // b0_xyz__b1_uv
      TDescription{
        -1, 2, 
        0, 0, 1, 0, 3, Render::TAttributeType::eFloat32, 0,
        1, 0, 1, 1, 2, Render::TAttributeType::eFloat32, 0,
      },

      // b0_xyz__b1_nnn
      TDescription{
        -1, 2, 
        0, 0, 1, 0, 3, Render::TAttributeType::eFloat32, 0,
        1, 0, 1, 1, 3, Render::TAttributeType::eFloat32, 0,
      },

      // b0_xyz__b1_nnn__b2_uv
      TDescription{
        -1, 3, 
        0, 0, 1, 0, 3, Render::TAttributeType::eFloat32, 0,
        1, 0, 1, 1, 3, Render::TAttributeType::eFloat32, 0,
        2, 0, 1, 2, 2, Render::TAttributeType::eFloat32, 0,
      },

      // i0__b0_xy
      TDescription{
        0, 1, 
        0, 2, 1, 
        0, 2, Render::TAttributeType::eFloat32, 0
      },

      // i0__b0_xyz
      TDescription{
        0, 1, 
        0, 3, 1, 
        0, 3, Render::TAttributeType::eFloat32, 0
      },

      // i0__b0_xyz_uv
      TDescription{
        0, 1, 
        0, 5, 2, 
        0, 3, Render::TAttributeType::eFloat32, 0,
        1, 2, Render::TAttributeType::eFloat32, 3,
      },

      // i0__b0_xyz_nnn
      TDescription{
        0, 1, 
        0, 6, 2, 
        0, 3, Render::TAttributeType::eFloat32, 0,
        1, 3, Render::TAttributeType::eFloat32, 3,
      },

      // i0__b0_xyz_nnn_uv
      TDescription{
        0, 1, 
        0, 8, 3, 
        0, 3, Render::TAttributeType::eFloat32, 0,
        1, 3, Render::TAttributeType::eFloat32, 3,
        2, 2, Render::TAttributeType::eFloat32, 6,
      },

      // i0__b0_xyz__b1_uv
      TDescription{
        0, 2, 
        0, 0, 1, 0, 3, Render::TAttributeType::eFloat32, 0,
        1, 0, 1, 1, 2, Render::TAttributeType::eFloat32, 0,
      },

      // i0__b0_xyz__b1_nnn
      TDescription{
        0, 2, 
        0, 0, 1, 0, 3, Render::TAttributeType::eFloat32, 0,
        1, 0, 1, 1, 3, Render::TAttributeType::eFloat32, 0,
      },

      // i0__b0_xyz__b1_nnn__b2_uv
      TDescription{
        0, 3, 
        0, 0, 1, 0, 3, Render::TAttributeType::eFloat32, 0,
        1, 0, 1, 1, 3, Render::TAttributeType::eFloat32, 0,
        2, 0, 1, 2, 2, Render::TAttributeType::eFloat32, 0,
      },

      // b0_xy__b1_rgba
      TDescription{
        -1, 2, 
        0, 0, 1, 0, 2, Render::TAttributeType::eFloat32, 0,
        1, 0, 1, 1, 4, Render::TAttributeType::eFloat32, 0,
      },

      // b0_xyz__b1_rgba
      TDescription{
        -1, 2, 
        0, 0, 1, 0, 3, Render::TAttributeType::eFloat32, 0,
        1, 0, 1, 1, 4, Render::TAttributeType::eFloat32, 0,
      },

      // b0_xyzw__b1_rgba
      TDescription{
        -1, 2, 
        0, 0, 1, 0, 4, Render::TAttributeType::eFloat32, 0,
        1, 0, 1, 1, 4, Render::TAttributeType::eFloat32, 0,
      },
    };

    return spec_table[(int)id];
  }
};


} // Draw


#endif // Render_IBuffer_h_INCLUDED