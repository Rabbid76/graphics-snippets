/******************************************************************//**
* \brief   General type declarations for drawing parameters.
* 
* \author  gernot
* \date    2017-11-20
* \version 1.0
**********************************************************************/
#pragma once
#ifndef Render_IDrawType_h_INCLUDED
#define Render_IDrawType_h_INCLUDED


// includes


#include <cstddef>
#include <cmath>
#include <bitset>
#include <array>
#include <vector>
#include <tuple>


//---------------------------------------------------------------------
// suppress '#define' of 'min' and 'max'
//---------------------------------------------------------------------


#if defined( min )
#define cutil__define_min_pushed
#pragma push_macro( "min" )
#undef min
#endif

#if defined( max )
#define cutil__define_max_pushed
#pragma push_macro( "max" )
#undef max
#endif


/******************************************************************//**
* \brief namespace for generic interface of drawing operations
**********************************************************************/
namespace Render
{

using t_byte  = unsigned char; //!< general unsigned  8bit data type (C++ 17 `std::byte`)
using t_sbyte = char;          //!< general signed 8bit data
using t_fp    = float;         //!< general floating point data type

inline t_byte MixProperty( t_byte a, t_byte b, t_fp w )
{
  return (t_byte)( a * ( 1.0f - w ) + b * w );
}

inline t_byte MixProperty( t_sbyte a, t_sbyte b, t_fp w )
{
  return (t_sbyte)( a * ( 1.0f - w ) + b * w );
}

inline t_fp MixProperty( t_fp a, t_fp b, t_fp w )
{
  return a * ( 1.0f - w ) + b * w ;
}


//! point data types
using TPoint2 = std::array< t_fp, 2 >;
using TPoint3 = std::array< t_fp, 3 >;
using TPoint4 = std::array< t_fp, 4 >;


//! direction vector data types
using TVec2  = std::array< t_fp, 2 >;
using TVec3  = std::array< t_fp, 3 >;
using TVec4  = std::array< t_fp, 4 >;


//! matrix data types
using TMat33 = std::array< TVec3, 3 >;
using TMat44 = std::array< TVec4, 4 >;

const TMat33 c_m33_ident{ TVec3{1, 0, 0}, TVec3{0, 1, 0}, TVec3{0, 0, 1} };
const TMat44 c_m44_ident{ TVec4{1, 0, 0, 0}, TVec4{0, 1, 0, 0}, TVec4{0, 0, 1, 0}, TVec4{0, 0, 0, 1} };

//! 8bit color data type
using TColor8 = std::array<t_byte, 4>;

//! floating point color data type
using TColor = std::array<t_fp, 4>;

//! normalize value
inline t_fp NormalizedValue( t_byte val )  { return static_cast<t_fp>(val) / static_cast<t_fp>(255.0); }
inline t_fp NormalizedValue( t_sbyte val ) { return val >= 0 ? static_cast<t_fp>(val) / static_cast<t_fp>(128.0f) : static_cast<t_fp>(val) / static_cast<t_fp>(127.0f); }
inline t_fp NormalizedValue( t_fp val )    { return val; }

//! convert floating point value to singed byte
inline t_sbyte UByte( t_byte val ) { return val; }
inline t_sbyte UByte( t_fp val )   { return static_cast<t_byte>(val * 255.0f); }

//! convert floating point value to singed byte
inline t_sbyte SByte( t_sbyte val ) { return val; }
inline t_sbyte SByte( t_fp val )    { return static_cast<t_sbyte>(val < 0.0 ? val * 128.0f : val * 127.0f); }

//! convert byte color to floating point color
inline TColor toColor( const TColor8 &col )
{ 
  return TColor{
    static_cast<t_fp>(col[0]) / static_cast<t_fp>(255.0),
    static_cast<t_fp>(col[1]) / static_cast<t_fp>(255.0),
    static_cast<t_fp>(col[2]) / static_cast<t_fp>(255.0),
    static_cast<t_fp>(col[3]) / static_cast<t_fp>(255.0)
  };
};

//! convert floating point color to byte color
inline TColor8 toColor8( const TColor &col8 )
{ 
  return TColor8{
    static_cast<t_byte>(col8[0] * static_cast<t_fp>(255.0)),
    static_cast<t_byte>(col8[1] * static_cast<t_fp>(255.0)),
    static_cast<t_byte>(col8[2] * static_cast<t_fp>(255.0)),
    static_cast<t_byte>(col8[3] * static_cast<t_fp>(255.0)),
  };
};

//! mix 2 floating point colors
inline TColor MixColor( const TColor &a, const TColor &b, t_fp w )
{
  return TColor{
    MixProperty(a[0], b[0], w),
    MixProperty(a[1], b[1], w),
    MixProperty(a[2], b[2], w),
    MixProperty(a[3], b[3], w)
  };
}

//! mix 2 floating point colors
inline TColor MixColorRGB( const TColor &a, const TColor &b, t_fp w, t_fp alpha )
{
  return TColor{
    MixProperty(a[0], b[0], w),
    MixProperty(a[1], b[1], w),
    MixProperty(a[2], b[2], w),
    alpha
  };
}

//! mix 2 byte colors
inline TColor8 MixColor( const TColor8 &a, const TColor8 &b, t_fp w )
{
  return TColor8{
    MixProperty(a[0], b[0], w),
    MixProperty(a[1], b[1], w),
    MixProperty(a[2], b[2], w),
    MixProperty(a[3], b[3], w)
  };
}


//! get the identity matrix
inline constexpr TMat44 Identity( void )
{
  return TMat44( { TVec4{ 1.0f, 0.0f, 0.0f, 0.0f },
                   TVec4{ 0.0f, 1.0f, 0.0f, 0.0f },
                   TVec4{ 0.0f, 0.0f, 1.0f, 0.0f },
                   TVec4{ 0.0f, 0.0f, 0.0f, 1.0f } } );
}

//! sketch mode
enum class TSketchMode
{
  NON,
  lead_pencil,          //! lead pencil
  //edge,                 //! edge only
  sketch_1,             //! sketch hatch
  sketch_2,             //! sketch hatch
  perlin_hatch,         //! perlin hatch
  perlin_hatch_spotted, //! spotted perlin hatch
  texture_hatch,        //! texture hatch
  toon,                 //! toon effect specular light source
  toon_opaque,          //! toon effect opaque color source
  toon_opaque_mapped,   //! toon effect opaque color source and luminosity map 
  toon_flat,            //! toon effect flat opaque color source
  // ...
  NUMBER_OF
};

//---------------------------------------------------------------------
// Primitives
//---------------------------------------------------------------------


/******************************************************************//**
* \brief   Generic primitive type
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
enum class TPrimitive
{
  points                   = 0,
  lines                    = 1,
  linestrip                = 2,
  lineloop                 = 3,
  lines_adjacency          = 4,
  linestrip_adjacency      = 5,
  triangles                = 6,
  trianglestrip            = 7,
  trianglefan              = 8,
  triangle_adjacency       = 9,
  trianglestrip_adjacency  = 10,
  patches                  = 11,
  // ...
  NO_OF
};


/******************************************************************//**
* \brief   Generic base primitives
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
enum class TBasePrimitive
{
  point = 0,
  line = 1,
  polygon = 2,
  patch = 3
};


/******************************************************************//**
* \brief   Identify the bas primitive
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
inline TBasePrimitive BasePrimitive( TPrimitive primitive )
{
  static_assert((int)TPrimitive::NO_OF == 12, "invalid number of primitives");
  static const std::array<TBasePrimitive, (int)TPrimitive::NO_OF> primitive_map
  {
    TBasePrimitive::point,   // TPrimitive::points
    TBasePrimitive::line,    // TPrimitive::lines
    TBasePrimitive::line,    // TPrimitive::linestrip
    TBasePrimitive::line,    // TPrimitive::lineloop
    TBasePrimitive::line,    // TPrimitive::lines_adjacency
    TBasePrimitive::line,    // TPrimitive::linestrip_adjacency
    TBasePrimitive::polygon, // TPrimitive::triangles
    TBasePrimitive::polygon, // TPrimitive::trianglestrip
    TBasePrimitive::polygon, // TPrimitive::trianglefan
    TBasePrimitive::polygon, // TPrimitive::triangle_adjacency
    TBasePrimitive::polygon, // TPrimitive::trianglestrip_adjacency
    TBasePrimitive::patch    // TPrimitive::patches
  };
  return primitive_map[(int)primitive];
}


//---------------------------------------------------------------------
// VertexCache
//---------------------------------------------------------------------


/******************************************************************//**
* \brief Simple cache for vertices.  
* 
* \author  gernot
* \date    2018-12-04
* \version 1.0
**********************************************************************/
class  TVertexCache
{
public:

  TVertexCache(void) = default;
  TVertexCache(const TVertexCache &) = default;
  TVertexCache(TVertexCache &&) = default;

  TVertexCache(size_t min_cache_elems)
  : _min_cache_elems( min_cache_elems )
  , _cache( min_cache_elems, 0.0f )
  {}

  virtual ~TVertexCache() = default;

  TVertexCache & operator = (const TVertexCache &) = default;
  TVertexCache & operator = (TVertexCache &&) = default;

  TVertexCache & clear( void )
  {
    _cache = std::vector<t_fp>( _min_cache_elems, 0.0f );
    _sequence_size = 0;
    return *this;
  }

  TVertexCache & Reset( void )
  {
    _sequence_size = 0;
    return *this;
  }

  TVertexCache & TupleSize( unsigned int tuple_size ) { _tuple_size = tuple_size; return *this; }
 
  unsigned int TupleSize( void )    const { return _tuple_size; }
  size_t       SequenceSize( void ) const { return _sequence_size; }
  const t_fp * VertexData( void )   const { return _cache.data(); }
 
  TVertexCache & Add( t_fp x, t_fp y, t_fp z )
  {
    // reserve the cache
    if ( _cache.size() < _sequence_size + _tuple_size )
      _cache.resize( _cache.size() + std::max((size_t)_tuple_size, _min_cache_elems) );

    // add the vertex coordinate to the cache
    _cache.data()[_sequence_size++] = x;
    _cache.data()[_sequence_size++] = y;
    if ( _tuple_size >= 3 )
      _cache.data()[_sequence_size++] = z;
    if ( _tuple_size == 4 )
      _cache.data()[_sequence_size++] = 1.0f;

    return *this;
  }
  
  TVertexCache & Add( double x, double y, double z )
  {
    // reserve the cache
    if ( _cache.size() < _sequence_size + _tuple_size )
      _cache.resize( _cache.size() + std::max((size_t)_tuple_size, _min_cache_elems) );

    // add the vertex coordinate to the cache
    _cache.data()[_sequence_size++] = (float)x;
    _cache.data()[_sequence_size++] = (float)y;
    if ( _tuple_size >= 3 )
      _cache.data()[_sequence_size++] = (float)z;
    if ( _tuple_size == 4 )
      _cache.data()[_sequence_size++] = 1.0f;

    return *this;
  }
  
  TVertexCache & Add( size_t coords_size, const t_fp *coords )
  {
    // reserve the cache
    if ( _cache.size() < _sequence_size + coords_size )
      _cache.resize( _cache.size() + std::max(coords_size, _min_cache_elems) );

    // add the vertex coordinates to the cache
    std::memcpy( _cache.data() + _sequence_size, coords, coords_size * sizeof( float ) );
    _sequence_size += coords_size;

    return *this;
  }

  TVertexCache & Add( size_t coords_size, const double *coords )
  {
    // reserve the cache
    if ( _cache.size() < _sequence_size + coords_size )
      _cache.resize( _cache.size() + std::max(coords_size, _min_cache_elems) );

    // add the vertex coordinate to the cache
    float *cache_ptr = _cache.data() + _sequence_size;
    for ( const double *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr ++, cache_ptr ++ )
      *cache_ptr = (float)(*ptr);
    _sequence_size += coords_size;

    return *this;
  }

private:

  unsigned int      _tuple_size{ 0 };      //!< tuple size (2, 3 or 4) for a sequence
  size_t            _min_cache_elems{ 0 }; //!< minimum element size of the sequence cache
  size_t            _sequence_size{ 0 };   //!< current size of the sequence cache
  std::vector<t_fp> _cache;                //!< the sequence cache
};


//---------------------------------------------------------------------
// CModelAndView
//---------------------------------------------------------------------


/******************************************************************//**
* \brief Data buffer which contains all the information which are
 necessary to transform a vertex coordinate form the model space
to the normalized device space and window coordinates to normalized 
device coordinates.

The `_model` matrix transforms a coordinate from model space to world space.

The `_view` matrix transforms a coordinate from world space to view space.

The `_projection` matrix transforms a coordinate from view space to clip space.
The result is a homogeneous coordinate, which is transformed by a perspective 
divide to a cartesian coordinate in normalized device space.

With the viewport size (`_vp_size`) and the and the `_near` and `_far` plane
a window coordinate can be transformed to normalized device space.

For further informations about the coordinate transformations see \ref pagShaderProjectionModelViewDepth.

\attention This data is not allowed to be changed, because it has to proper fit the corresponding types
of uniform blocks or shader storage buffer objects in the GLSL code.

The data structure completely full fills the requirements of the GLSL `std140` and `std340` layout.  

*std140*

[OpenGL 4.6 API Core Profile Specification; 7.6.2.2 Standard Uniform Block Layout; page 144](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf),  
[OpenGL 4.6 API Compatibility Profile Specification; 7.6.2.2 Standard Uniform Block Layout; page 145](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.compatibility.pdf)  
[OpenGL ES 3.2 Specification; 7.6.2.2 Standard Uniform Block Layout; page 113](https://www.khronos.org/registry/OpenGL/specs/es/3.2/es_spec_3.2.pdf):

> When using the `std140` storage layout, structures will be laid out in buffer storage with their members stored in monotonically increasing order based on their location in the declaration.
A structure and each structure member have a base offset and a base alignment, from which an aligned offset is computed by rounding the base offset up to a multiple of the base alignment.
The base offset of the first member of a structure is taken from the aligned offset of the structure itself.
The base offset of all other structure members is derived by taking the offset of the last basic machine unit consumed by the previous member and adding one. 
Each structure member is stored in memory at its aligned offset. The members of a toplevel uniform block are laid out in buffer storage by treating the uniform block as a structure with a base offset of zero.
>
> 1. If the member is a scalar consuming N basic machine units, the base alignment is N.
> 2. If the member is a two- or four-component vector with components consuming N basic machine units, the base alignment is 2N or 4N, respectively.
> 3. If the member is a three-component vector with components consuming N basic machine units, the base alignment is 4N.
> 4. If the member is an array of scalars or vectors, the base alignment and array stride are set to match the base alignment of a single array element, according to rules (1), (2), and (3), and rounded up to the base alignment of a `vec4`.
>    The array may have padding at the end; the base offset of the member following the array is rounded up to the next multiple of the base alignment.
> 5. If the member is a column-major matrix with C columns and R rows, the matrix is stored identically to an array of C column vectors with R components each, according to rule (4).
> 6. If the member is an array of S column-major matrices with C columns and R rows, the matrix is stored identically to a row of S × C column vectors with R components each, according to rule (4).
> 7. If the member is a row-major matrix with C columns and R rows, the matrix is stored identically to an array of R row vectors with C components each, according to rule (4).
> 8. If the member is an array of S row-major matrices with C columns and R rows, the matrix is stored identically to a row of S × R row vectors with C components each, according to rule (4).
> 9. If the member is a structure, the base alignment of the structure is N, where N is the largest base alignment value of any of its members, and rounded up to the base alignment of a `vec4`.
>    The individual members of this substructure are then assigned offsets by applying this set of rules recursively, where the base offset of the first member of the sub-structure is equal to the aligned offset of the structure.
>    The structure may have padding at the end; the base offset of the member following the sub-structure is rounded up to the next multiple of the base alignment of the structure.
> 10. If the member is an array of S structures, the S elements of the array are laid out in order, according to rule (9).

*std340*

[OpenGL 4.6 API Compatibility Profile Specification; 7.6.2.2 Standard Uniform Block Layout; page 144](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.compatibility.pdf)

> Shader storage blocks (see section 7.8) also support the `std140` layout qualifier, as well as a `std430` qualifier not supported for uniform blocks. When using the `std430` storage layout,
> shader storage blocks will be laid out in buffer storage identically to uniform and shader storage blocks using the `std140` layout,
> except that the base alignment and stride of arrays of scalars and vectors in rule 4 and of structures in rule 9 are not rounded up a multiple of the base alignment of a `vec4`.

Corresponding GLSL data structures

Uniform block:

\code{.glsl}
layout (std140) uniform UB_ViewData
{
    mat4  _model;      // model matrix
    mat4  _view;       // view matrix
    mat4  _projection; // projection matrix
    vec4  _vp_rect;    // viewport rectangle
    float _near;       // near plane 
    float _far;        // far plane 

} view_data;
\endcode

Shader storage block:

\code{.glsl}
layout (std340) buffer SSBO_ViewData
{
    mat4  _model;      // model matrix
    mat4  _view;       // view matrix
    mat4  _projection; // projection matrix
    vec4  _vp_rect;    // viewport rectangle
    float _near;       // near plane 
    float _far;        // far plane 

} view_data;
\endcode

* \author  gernot
* \date    2018-12-05
* \version 1.0
**********************************************************************/
struct TModelAndView
{
  TMat44 _model;      //!< model matrix
  TMat44 _view;       //!< view matrix
  TMat44 _projection; //!< projection matrix 
  TVec4  _vp_rect;    //!< viewport rectangle
  t_fp   _near;       //!< near plane
  t_fp   _far;        //!< far plane
};


/******************************************************************//**
* \brief Manges a `TModelAndView` data structure   
* 
* \author  gernot
* \date    2018-12-05
* \version 1.0
**********************************************************************/
class CModelAndView
{
public:

  CModelAndView( void )
    : _data( {c_m44_ident, c_m44_ident, c_m44_ident, { 0, 0, 0, 0 }, -1.0f, 1.0f} )
  {}

  CModelAndView( const CModelAndView & ) = default;
  CModelAndView( CModelAndView && ) = default;

  ~CModelAndView() = default;

  CModelAndView & operator = ( const CModelAndView & ) = default;
  CModelAndView & operator = ( CModelAndView && ) = default;

  void Model( const TMat44 & m )                  { _data._model = m;                               ++ _model_modifier; }
  void Model( const t_fp * m )                    { memcpy(&_data._model[0][0], m, sizeof(TMat44)); ++ _model_modifier; }
  void View( const TMat44 & v )                   { _data._view = v;                                ++ _data_modifier; }
  void View( const t_fp * v )                     { memcpy(&_data._model[0][0], v, sizeof(TMat44)); ++ _data_modifier; }
  void Projection( const TMat44 & p )             { _data._projection = p;                          ++ _data_modifier; }
  void Projection( const t_fp * p )               { memcpy(&_data._model[0][0], p, sizeof(TMat44)); ++ _data_modifier; }
  void Viewport( t_fp x, t_fp y, t_fp w, t_fp h ) { _data._vp_rect = {x, y, w, h};                  ++ _data_modifier; }
  void DepthRage( t_fp np, t_fp fp )              { _data._near = np; _data._far = fp;              ++ _data_modifier; }

  const TModelAndView * Data( void ) const { return &_data; }

  unsigned int DataModifier( void )  const { return _data_modifier; }
  unsigned int ModelModifier( void ) const { return _model_modifier; }

private:

  TModelAndView _data;                //!< buffer of the transformation data
  unsigned int  _data_modifier{ 0 };  //!< every time when any data (except the model matrix) is modified this is incremented 
  unsigned int  _model_modifier{ 0 }; //!< every time when the model matrix is modified, then this is incremented 
};
using TModelAndViewPtr = std::shared_ptr<CModelAndView>;


} // Draw


#if defined( cutil__define_min_pushed )
#pragma pop_macro( "min" )
#endif

#if defined( cutil__define_max_pushed )
#pragma pop_macro( "max" )
#endif

#endif // Render_ILight_h_INCLUDED