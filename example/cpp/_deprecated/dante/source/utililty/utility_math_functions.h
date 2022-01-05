/***********************************************************************************************//**
* \brief Mathematical functions  
*
* \author  gernot Rabbid76    \date  2019-02-09
***************************************************************************************************/


#pragma once
#ifndef __Utility_Math_Functions__h__
#define __Utility_Math_Functions__h__


// STL

#include <cmath>


namespace Utility
{

namespace Math
{


/***********************************************************************************************//**
* \brief clamp value in range [minimum, maximum]  
*
* \author  gernot Rabbid76    \date  2019-02-09
***************************************************************************************************/
template< typename T >
T clamp( T val, T minimum, T maximum )
{ 
    return std::min( std::max( val, minimum ), maximum ); 
}


/***********************************************************************************************//**
* \brief  Linear interpolate between value `a` and `b` according to `w`. `w` is in [0, 1]. 
*
* `mix` calculates `a * (1 - w) + b * w`
* `lerp` calculates `a + (b - a) * w` 
*
* \author  gernot Rabbid76    \date  2019-02-09
***************************************************************************************************/
template< typename TV, typename TW >
TV mix( TV a, TV b, TW w )
{ 
    return static_cast<TV>(a * (static_cast<TW>(1) - w) + b * w); 
}

template< typename TV, typename TW >
TV lerp( TV a, TV b, TW w )
{ 
    return a + static_cast<TV>((b - a) * w); 
}


} // Math

} // Utility

# endif // __Utility_Math_Functions__h__
