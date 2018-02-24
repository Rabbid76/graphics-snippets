/******************************************************************//**
* \brief   General type declarations for drawing parameters.
* 
* \author  gernot
* \date    2017-11-20
* \version 1.0
**********************************************************************/
#pragma once
#ifndef IDrawType_h_INCLUDED
#define IDrawType_h_INCLUDED


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

using t_byte = unsigned char; //!< general unsigned  8bit data type (C++ 17 `std::byte`)
using t_fp   = float;         //!< general floating point data type


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

} // Draw

#endif // ILight_h_INCLUDED