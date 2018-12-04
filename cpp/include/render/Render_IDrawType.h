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

} // Draw

#endif // Render_ILight_h_INCLUDED