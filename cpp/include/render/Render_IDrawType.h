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


//! direction vector data tpyes
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

} // Draw

#endif // Render_ILight_h_INCLUDED