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

#include <render/Render_IDrawType.h>

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

  virtual ~IAttributeData() = default;

  virtual int            tuple_size( void ) const = 0;  //!< number of the components of an attribute 1, 2, 3 or 4 
  virtual int            stride( void ) const = 0;      //!< number of elements (`T_DATA`) from one attribute to the next attribute
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

  virtual ~TAttributeVector() = default;

  TAttrbutes       AV( void )       { return _av; } //!< access to raw attribute vector
  const TAttrbutes AV( void ) const { return _av; } //!< access to raw attribute vector

  virtual int            tuple_size( void ) const override { return V_TUPLE_SIZE; }
  virtual int            stride( void )     const override { return V_STRIDE == 0 ? V_TUPLE_SIZE : V_STRIDE; }
  virtual int            offset( void )     const override { return V_OFFSET; }
  virtual size_t         size( void )       const override { return _av.size() * V_TUPLE_SIZE; }
  virtual const T_DATA * data( void )       const override { return reinterpret_cast<const T_DATA *>(_av.data()); }

  TAttrbutes _av;
};

template<typename T_DATA>
class TAttributeVectorN
  : public IAttributeData<T_DATA>
{
public:

  using TAttrbutes = std::vector<T_DATA>;

  virtual ~TAttributeVectorN() = default;

  TAttrbutes       AV( void )       { return _av; } //!< access to raw attribute vector
  const TAttrbutes AV( void ) const { return _av; } //!< access to raw attribute vector

  virtual int            tuple_size( void ) const override { return _tuple_size; }
  virtual int            stride( void )     const override { return _tuple_size; }
  virtual int            offset( void )     const override { return 0; }
  virtual size_t         size( void )       const override { return _av.size(); }
  virtual const T_DATA * data( void )       const override { return _av.data(); }

  TAttrbutes _av;
  int        _tuple_size = 0;
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

  virtual ~IIndexData() = default;

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

  virtual ~TIndexVector() = default;

  TIndices       IV( void )       { return _iv; } //!< access to raw index vector
  const TIndices IV( void ) const { return _iv; } //!< access to raw index vector

  virtual size_t          size( void ) const override { return _iv.size() * V_PRIMITIVE_SIZE; }
  virtual const T_INDEX * data( void ) const override { return reinterpret_cast<const T_INDEX *>(_iv.data()); }

  TIndices _iv;
};

template<typename T_INDEX>
class TIndexVector<T_INDEX, 1>
  : public IIndexData<T_INDEX>
{
public:

  using TIndices = std::vector<T_INDEX>;

  virtual ~TIndexVector() = default;

  TIndices       IV( void )       { return _iv; } //!< access to raw index vector
  const TIndices IV( void ) const { return _iv; } //!< access to raw index vector

  virtual size_t          size( void ) const override { return _iv.size(); }
  virtual const T_INDEX * data( void ) const override { return _iv.data(); }

  TIndices _iv;
};

template<typename T_INDEX>
class TIndexVectorN
  : public IIndexData<T_INDEX>
{
public:

  using TIndices = std::vector<T_INDEX>;

  virtual ~TIndexVectorN() = default;

  TIndices       IV( void )       { return _iv; } //!< access to raw index vector
  const TIndices IV( void ) const { return _iv; } //!< access to raw index vector

  virtual size_t          size( void ) const override { return _iv.size(); }
  virtual const T_INDEX * data( void ) const override { return _iv.data(); }

  TIndices _iv;
};


enum class TMeshIndexKind  { non, common, multiple };
enum class TMeshNormalKind { non, face, vertex, both };
  
//! Specification of the shape of a primitive (face)
enum class TMeshFaceType
{ 
  // point primitives
  point,
    
  // line primitives
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
  polygons, //!< same as a triangle fan with a separated size for each polygon

  // special primitives
  encoded   //!< the type of the primitive is encoded to the index array data
};

//! Specification of the size (number of vertices) of a primitive (face)  
enum class TMeshFaceSizeKind
{
  constant,        //!< The size of the faces is constant and equal for each face
  separated_array, //!< The size of the faces is stored in an separated array
  encoded_restart, //!< The size of the faces is encoded in the index array, by an primitive restart marker
  encoded_size     //!< The size of the faces is encoded in the index array, the size is specified at the start of each primitive.
};

//!< specifies the general packing of the attributes
enum class TMeshAttributePack
{
  separated_tightly, //!< the attributes are tightly packed to separated buffers
  record_stride      //!< the vertex and its attributes are placed to a tuple recored and stored in one buffer
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

  using TIndex              = T_INDEX;
  using TAttributeContainer = Render::IAttributeData<T_DATA>;
  using TIndexContainer     = Render::IIndexData<T_INDEX>;

  virtual ~IMeshData() = default;

  virtual TMeshFaceType                FaceType( void ) const = 0;            //!< Type of primitives
  virtual TMeshFaceSizeKind            FaceSizeKind( void ) const = 0;        //!< Specifies how the size of a single primitive is defined 
  virtual TMeshIndexKind               IndexKind( void ) const = 0;           //!< Specifies whether the attributes have separated, common or no indices.
                                                                              //!< If the attributes have no indices, then all the attributes are consecutively sorted according to the faces in the attribute arrays.
  virtual TMeshAttributePack           Pack( void ) const = 0;                //!< specifies the buffer packing
  virtual const TAttributeContainer  & Vertices( void ) const = 0;            //!< Container for the vertex coordinates
  
  virtual const TIndexContainer      * Indices( void ) const = 0;             //!< Container for the common indices or vertex indices
  virtual TIndex                       FaceSize( void ) const = 0;            //!< Size of a single primitive (0, if the primitives have different sizes)
  virtual const TIndexContainer      * FaceSizes( void ) const = 0;           //!< Array of face sizes
  virtual TIndex                       FaceRestart( void ) const = 0;         //!< Primitive restart marker

  virtual TMeshNormalKind              NormalKind( void ) const = 0;          //!< Specifies whether there are face, vertex or both kind of normals.
  virtual const TAttributeContainer  * FaceNormals( void ) const = 0;         //!< Container for the vertex normals
  virtual const TIndexContainer      * FaceNormalIndices( void ) const = 0;   //!< Container for the separated indices of the vertex normals
  virtual const TAttributeContainer  * Normals( void ) const = 0;             //!< Container for face normals
  virtual const TIndexContainer      * NormalIndices( void ) const = 0;       //!< Container for the separated indices of the face normals
  
  virtual const TAttributeContainer  * TextureCoordinates( void ) const = 0;  //!< Container for the texture coordinates
  virtual const TIndexContainer      * TextureCoordIndices( void ) const = 0; //!< Container for the separated indices of the texture coordinates
  
  virtual const TAttributeContainer  * Colors( void ) const = 0;              //!< Container for the color attributes
  virtual const TIndexContainer      * ColorIndices( void ) const = 0;        //!< Container for the separated indices of the color attributes

  virtual bool                         Smooth( void ) const { return false; } //!< Additional attribute, which indicates that the mesh is completely smooth

  virtual size_t NoOfVertices( void ) const { return Vertices().NoOfAttributes(); } //!< number of vertex coordinates

  bool ValidFaceNormals( void ) const { auto kind = NormalKind(); return kind == TMeshNormalKind::face || kind == TMeshNormalKind::both; }
  bool ValidNormals( void )     const { auto kind = NormalKind(); return kind == TMeshNormalKind::vertex || kind == TMeshNormalKind::both; }                        
};

template<typename T_DATA, typename T_INDEX>
using IMeshPtr = std::unique_ptr<IMeshData<T_DATA, T_INDEX>>;


/******************************************************************//**
* \brief   Generic interface for mesh data cache.
* 
* \author  gernot
* \date    2018-05-29
* \version 1.0
**********************************************************************/
template<typename T_DATA, typename T_INDEX>
class IMeshCache
{
public: 

  virtual ~IMeshCache( void ) = default;

  // add a mesh to the cache, without any key
  virtual IMeshData<T_DATA, T_INDEX> & Add( IMeshPtr<T_DATA, T_INDEX> &&mesh ) = 0;

  // manages meshes by an identifier and associates a time stamp to the mesh
  virtual IMeshData<T_DATA, T_INDEX> * Get( size_t key, time_t &time_stamp ) = 0;
  virtual IMeshData<T_DATA, T_INDEX> * AddOrReplace( size_t key, time_t time_stamp, IMeshPtr<T_DATA, T_INDEX> &&mesh ) = 0;

  virtual IMeshData<T_DATA, T_INDEX> * Add( size_t key, time_t time_stamp, IMeshPtr<T_DATA, T_INDEX> &&mesh )
  {
    time_t cached_time;
    if ( Get( key, cached_time ) != nullptr && cached_time >= time_stamp )
      return nullptr;
    return AddOrReplace( key, time_stamp, std::move(mesh) );
  }

  // manages meshes by a name and associates a time stamp to the mesh
  virtual IMeshData<T_DATA, T_INDEX> * Get( const std::string &name, time_t &time_stamp ) = 0;
  virtual IMeshData<T_DATA, T_INDEX> * AddOrReplace( const std::string &name, time_t time_stamp, IMeshPtr<T_DATA, T_INDEX> &&mesh ) = 0;

  virtual IMeshData<T_DATA, T_INDEX> * Add( const std::string &name, time_t time_stamp, IMeshPtr<T_DATA, T_INDEX> &&mesh )
  {
    time_t cached_time;
    if ( Get( name, cached_time ) != nullptr && cached_time >= time_stamp )
      return nullptr;
    return AddOrReplace( name, time_stamp, std::move(mesh) );
  }
  
};



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
  using TUniqueMesh = IMeshPtr<T_DATA, T_INDEX>;

  virtual ~IMeshFormatTransformer() = default;

  virtual TUniqueMesh Transform( const TMesh &mesh ) const = 0;

  virtual bool Transform( TUniqueMesh &mesh ) const 
  {
    if ( mesh == nullptr )
      return false;

    TUniqueMesh new_mesh = Transform( *mesh.get() );
    if ( new_mesh == nullptr )
      return false;

    mesh = std::move(new_mesh);
    return true;
  }
};


//---------------------------------------------------------------------
// IMeshNormalGenerator
//---------------------------------------------------------------------


/******************************************************************//**
* \brief Generic interface for automatic normal vector 
* generation.
* 
* \author  gernot
* \date    2018-05-29
* \version 1.0
**********************************************************************/
template<typename T_DATA, typename T_INDEX>
class IMeshNormalGenerator
{
public:

  virtual ~IMeshNormalGenerator() = default;

  // TODO $$$
};


//---------------------------------------------------------------------
// IMeshUVGenerator
//---------------------------------------------------------------------


/******************************************************************//**
* \brief Generic interface for automatic 2D texture coordinate 
* generation.
* 
* \author  gernot
* \date    2018-05-29
* \version 1.0
**********************************************************************/
template<typename T_DATA, typename T_INDEX>
class IMeshUVGenerator
{
public:

  virtual ~IMeshUVGenerator() = default;

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

  virtual ~IMeshTessellator() = default;

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

  virtual ~IMeshFactory() = default;

  // virtual const bool Generate( IMeshData &mesh ) = 0; // read data from file a store data to mesh 
};


// TODO $$$ "meshdef_template.h" move from workbench to examples



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

  using TMesh = IMeshData<T_DATA, T_INDEX>;
  using TUniqueMesh = IMeshPtr<T_DATA, T_INDEX>;

  virtual ~IMeshResource() = default;

  virtual TUniqueMesh Load( void ) const = 0; // read data from file a store data to mesh 
};


} // Render

#endif // Render_IMeshData_h_INCLUDED
