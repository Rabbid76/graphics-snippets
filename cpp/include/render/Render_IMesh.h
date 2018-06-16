/******************************************************************//**
* \brief   Generic interface for mesh data array access.
* 
* \author  gernot
* \date    2017-11-20
* \version 1.0
**********************************************************************/
#pragma once
#ifndef Render_IMesh_h_INCLUDED
#define Render_IMesh_h_INCLUDED


// includes

#include <Render_IDrawType.h>

#include <memory>
#include <vector>
#include <array>


/******************************************************************//**
* @brief   Namespace for renderer.
*
* @author  gernot
* @date    2018-03-17
* @version 1.0
**********************************************************************/
namespace Render
{


//---------------------------------------------------------------------
// IMeshData
//---------------------------------------------------------------------


/******************************************************************//**
* \brief   Generic interface for attribute data.
* 
* \author  gernot
* \date    2018-06-13
* \version 1.0
**********************************************************************/
template<typename T_DATA>
class IAttributeData
{
public:

  using const_iterator = const T_DATA *;

  virtual int            tuple_size( void ) const = 0;  //!< number of the components of an attrbute 1, 2, 3 or 4 
  virtual int            stride( void ) const = 0;      //!< number of elements (`T_DATA`) from one attrbute to the next attribute
  virtual int            offset( void ) const = 0;      //!< index of elements (`T_DATA`) where the first attribute starts in the array
  virtual size_t         size( void ) const = 0;        //!< number of attributes 
  virtual const T_DATA * data( void ) const = 0;        //!< pointer to the raw attribute data

  const size_t NoOfAttributes( void ) const { return size() / tuple_size(); }
  
  bool           empty( void ) const { return size() == 0; }
  const_iterator begin( void ) const { return data(); }
  const_iterator end( void )   const { return data() + size(); }
};

template<typename T_DATA, int V_TUPLE_SIZE, int V_STRIDE=0, int V_OFFSET=0>
class TAttributeVector
  : public IAttributeData<T_DATA>
{
public:

  using TVector    = std::array<T_DATA, V_TUPLE_SIZE>; 
  using TAttrbutes = std::vector<TVector>;

  TAttrbutes       AV( void )       { return _va; } //!< access to raw attribute vector
  const TAttrbutes AV( void ) const { return _va; } //!< access to raw attribute vector

  virtual int            tuple_size( void ) const override { return V_TUPLE_SIZE; }
  virtual int            stride( void )     const override { return V_STRIDE == 0 ? V_TUPLE_SIZE : V_STRIDE; }
  virtual int            offset( void )     const override { return V_OFFSET; }
  virtual size_t         size( void )       const override { return _av.size() * V_TUPLE_SIZE; }
  virtual const T_DATA * data( void )       const override { return reinterpret_cast<const T_DATA *>(_av.data()); }

  TAttrbutes _av;
};


/******************************************************************//**
* \brief   Generic interface for attribute indices.
* 
* \author  gernot
* \date    2018-06-16
* \version 1.0
**********************************************************************/
template<typename T_INDEX>
class IIndexData
{
public:

  using const_iterator = const T_INDEX *;

  virtual size_t          size( void ) const = 0; //!< number of indices 
  virtual const T_INDEX * data( void ) const = 0; //!< pointer to the raw index data

  bool           empty( void ) const { return size() == 0; }
  const_iterator begin( void ) const { return data(); }
  const_iterator end( void )   const { return data() + size(); }
};

template<typename T_INDEX, int V_PRIMITIVE_SIZE>
class TIndexVector
  : public IIndexData<T_INDEX>
{
public:

  using TFace    = std::array<T_INDEX, V_PRIMITIVE_SIZE>; 
  using TIndices = std::vector<TFace>;

  TIndices       IV( void )       { return _iv; } //!< access to raw index vector
  const TIndices IV( void ) const { return _iv; } //!< access to raw index vector

  virtual size_t          size( void ) const override { return _iv.size() * V_PRIMITIVE_SIZE; }
  virtual const T_INDEX * data( void ) const override { return reinterpret_cast<const T_INDEX *>(_iv.data()); }

  TIndices _iv;
};


enum class TMeshIndexKind  { non, common, multiple };
enum class TMeshNormalKind { non, face, vertex, both };
  
//! Specification of the shape of a primitive (face)
enum class TMeshFaceType
{ 
  // point primitives
  point,
    
  // line primitves
  lines,
  line_strips,
  line_loops,
  lines_adjacencies,
  line_strips_adjacencies,

  // triangle (polygon) primitives
  triangles,
  triangle_fans,
  triangle_strips,
  triangles_adjacencies,
  triangle_strips_adjacencies,  

  // polygon primitives
  quads,    //!< same as a triangle fan with a size of 4 for each polygon 
  polygons, //!< same as a triangle fan with a separated size for each polgon

  // special primitives
  encoded   //!< the type of the primitive is encoded to the index array data
};

//! Specification of the size (numberr of vertices) of a primitive (face)  
enum class TMeshFaceSizeKind
{
  constant,        //!< The size of the faces is constant and equal for each face
  separated_array, //!< The size of the faces is stored in an separated array
  encoded_restart, //!< The size of the faces is encoded in the index array, by an primitive restrat marker
  encoded_size     //!< The size of the faces is encoded in the index array, the size is specified at the start of each primitive.
};


/******************************************************************//**
* \brief   Generic interface for mesh data.
* 
* \author  gernot
* \date    2018-05-29
* \version 1.0
**********************************************************************/
template<typename T_DATA, typename T_INDEX>
class IMeshData
{
public:

  using TIndex         = T_INDEX;
  using TAttributeData = Render::IAttributeData<T_DATA>;
  using TIndexData     = Render::IIndexData<T_INDEX>;

  virtual TMeshFaceType           FaceType( void ) const = 0;            //!< Type of primitives
  virtual TMeshFaceSizeKind       FaceSizeKind( void ) const = 0;        //!< Specifies how the size of a single primtive is defined 
  virtual TMeshIndexKind          IndexKind( void ) const = 0;           //!< Specifies whether the attributes have separated, common or no indices.
                                                                         //!< If the attributes have no indices, then all the attributes are consecutively sorted according to the faces in the attribute arrays.

  virtual const TAttributeData  & Vertices( void ) const = 0;            //!< Container for the vertex coordiantes
  
  virtual const TIndexData      * Indices( void ) const = 0;             //!< Container for the common indices or vertex indices
  virtual TIndex                  FaceSize( void ) const = 0;            //!< Size of a single primitive (0, if the primitives hacve different sizes)
  virtual const TIndexData      * FaceSizes( void ) const = 0;           //!< Array of face sizes
  virtual TIndex                  FaceRestart( void ) const = 0;         //!< Primitive restart marker

  virtual TMeshNormalKind         NormalKind( void ) const = 0;          //!< Specifies whether there are face, vertex or both kind of normals.
  virtual const TAttributeData  * FaceNormals( void ) const = 0;         //!< Container for the vertex normals
  virtual const TIndexData      * FaceNormalIndices( void ) const = 0;   //!< Container for the separated indices of the vertex normals
  virtual const TAttributeData  * Normals( void ) const = 0;             //!< Container for face normals
  virtual const TIndexData      * NormalIndices( void ) const = 0;       //!< Container for the separated indices of the face normals
  
  virtual const TAttributeData  * TextureCoordinates( void ) const = 0;  //!< Container for the texture coordiantes
  virtual const TIndexData      * TextureCoordIndices( void ) const = 0; //!< Container for the separated indices of the texture coordiantes
  
  virtual const TAttributeData  * Colors( void ) const = 0;              //!< Container for the color attributes
  virtual const TIndexData      * ColorIndices( void ) const = 0;        //!< Container for the separated indices of the color attributes

  bool ValidFaceNormals( void ) const { auto kind = NormalKind(); return kind == TNormalKind::face || kind == TNormalKind::both; }
  bool ValidNormals( void )     const { auto kind = NormalKind(); return kind == TNormalKind::vertex || kind == TNormalKind::both; }
};

template<typename T_DATA, typename T_INDEX>
using TMeshPtr = std::unique_ptr<IMeshData<T_DATA, T_INDEX>>;



//---------------------------------------------------------------------
// IMeshFormatTransformer
//---------------------------------------------------------------------


/******************************************************************//**
* \brief   Generic interface for transforming mesh data.
* 
* \author  gernot
* \date    2018-05-29
* \version 1.0
**********************************************************************/
template<typename T_DATA, typename T_INDEX>
class IMeshFormatTransformer
{
public:

  using TMesh = IMeshData<T_DATA, T_INDEX>;
  using TUniqueMesh = TMeshPtr<T_DATA, T_INDEX>;

  virtual TUniqueMesh Transform( const TMesh &mesh, TMeshIndexKind mesh_kind, bool triangles ) const = 0;

  // TODO $$$
  //virtual TUniqueMesh TransformFaceNormalsToVertexNormals( const TMesh &mesh ) = 0;
  //virtual TUniqueMesh TransformMultiIndexToSingleIndex( const TMesh &mesh ) = 0;  //! if the attributes have different indices, the transform the attributes to arrays with common indices.
  //virtual TUniqueMesh TransformIndexBufferToArrayBuffer( const TMesh &mesh ) = 0; //! get rid of all indices- transform to an attribute array, where all vertices of the faces are in consecutive order. 
 
};


//---------------------------------------------------------------------
// IMeshNormalGenerator
//---------------------------------------------------------------------


/******************************************************************//**
* \brief Generic interface for automatic normal vector 
* gerneration.
* 
* \author  gernot
* \date    2018-05-29
* \version 1.0
**********************************************************************/
template<typename T_DATA, typename T_INDEX>
class IMeshNormalGenerator
{
public:

  // TODO $$$
};


//---------------------------------------------------------------------
// IMeshUVGenerator
//---------------------------------------------------------------------


/******************************************************************//**
* \brief Generic interface for automatic 2D texture coordinate 
* gerneration.
* 
* \author  gernot
* \date    2018-05-29
* \version 1.0
**********************************************************************/
template<typename T_DATA, typename T_INDEX>
class IMeshUVGenerator
{
public:

  // TODO $$$
};


//---------------------------------------------------------------------
// IMeshTessellator
//---------------------------------------------------------------------


/******************************************************************//**
* \brief Generic interface for mesh tessellation.
* 
* \author  gernot
* \date    2018-05-29
* \version 1.0
**********************************************************************/
template<typename T_DATA, typename T_INDEX>
class IMeshTessellator
{
public:

  // TODO $$$
};


//---------------------------------------------------------------------
// IMeshFactory
//---------------------------------------------------------------------


/******************************************************************//**
* \brief   Generic interface for generating mesh data.
* 
* \author  gernot
* \date    2018-05-29
* \version 1.0
**********************************************************************/
template<typename T_DATA, typename T_INDEX>
class IMeshFactory
{
public:

  // virtual const bool Generate( IMeshData &mesh ) = 0; // read data from file a store data to mesh 
};


// TODO $$$ "meshdef_template.h" move from workbenche to examples



//---------------------------------------------------------------------
// IMeshResource
//---------------------------------------------------------------------


/******************************************************************//**
* \brief   Generic interface for mesh data file.
* 
* \author  gernot
* \date    2018-05-29
* \version 1.0
**********************************************************************/
template<typename T_DATA, typename T_INDEX>
class IMeshResource
{
public:

  // virtual const bool Load( IMeshData &mesh ) = 0; // read data from file a store data to mesh 
};


} // Render

#endif // Render_IMeshData_h_INCLUDED
