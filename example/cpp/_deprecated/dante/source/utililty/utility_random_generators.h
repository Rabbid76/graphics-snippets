/***********************************************************************************************//**
* \brief Random number generators
*
* \author  gernot Rabbid76    \date  2019-02-23
***************************************************************************************************/


#pragma once
#ifndef __Utility_Math_Generators__h__
#define __Utility_Math_Generators__h__


// STL

#include <vector>
#include <map>
#include <chrono>
#include <random>
#include <algorithm>


namespace Utility
{

namespace Random
{


// TODO $$$ use constraint `Ingtegral` for template argument of class `CUniqueIntegralSequence`
// see [Constraints and concepts (since C++20)](https://en.cppreference.com/w/cpp/language/constraints)

//template <class T>
//concept Integral = std::is_integral<T>::value;


template< typename T >
class CUniqueIntegralSequence
{
public:

    using TSequnce = std::vector<T>;

    operator const TSequnce & (void) const { return _sequence; }

    const TSequnce & operator() (void) const { return _sequence; }

    //! Generates a random sequence of the numbers in [a, b] (including a and b)  
    CUniqueIntegralSequence & Generate( T a, T b )
    {
        // setup random number generator
        auto seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count();
        std::minstd_rand0 generator( seed );

        // map random numbers to the values in the range [a, b]
        std::map<decltype(generator()), T> random_map;
        for ( T n = a; n <= b; )
        {
            auto random_val = generator();
            if ( random_map.find( random_val ) == random_map.end() )
                random_map[random_val] = n++;
        }

        // copy numbers to the sequence
        _sequence.resize(b - a + 1);
        std::transform( random_map.begin(), random_map.end(), _sequence.begin(), [](auto t) -> T { return t.second; } );

        return *this;
    }


private:

    TSequnce _sequence; //!< sequence of integral numbers
};



} // Math

} // Utility

# endif // __Utility_Math_Generators__h__
