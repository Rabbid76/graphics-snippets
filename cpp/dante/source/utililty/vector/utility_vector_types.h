/***********************************************************************************************//**
* \brief Geometric Vector templates and types  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/

#pragma once
#ifndef __Utility_Vector_Types__h__
#define __Utility_Vector_Types__h__


// STL

#include <array>


namespace Utility
{

namespace Vector
{


/***********************************************************************************************//**
* \brief Base class for geometric locations and distances  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
template <typename TYPE_SCALE, size_t TUPLE_SIZE>
class TVector
    : public std::array<TYPE_SCALE, TUPLE_SIZE>
{};

//! specialization for TUPLE_SIZE == 2
template <typename TYPE_SCALE>
class TVector<TYPE_SCALE, 2>
    : public std::array<TYPE_SCALE, 2>
{
public:

    //using std::array<TYPE_SCALE, 2>();

    TYPE_SCALE x(void) const { return (*this)[0]; }
    TYPE_SCALE y(void) const { return (*this)[1]; }

    TYPE_SCALE & x(void) { return (*this)[0]; }
    TYPE_SCALE & y(void) { return (*this)[1]; }
};


//! specialization for TUPLE_SIZE == 3
template <typename TYPE_SCALE>
class TVector<TYPE_SCALE, 3>
    : public std::array<TYPE_SCALE, 3>
{
public:

    TYPE_SCALE x(void) const { return (*this)[0]; }
    TYPE_SCALE y(void) const { return (*this)[1]; }
    TYPE_SCALE z(void) const { return (*this)[2]; }

    TYPE_SCALE & x(void) { return (*this)[0]; }
    TYPE_SCALE & y(void) { return (*this)[1]; }
    TYPE_SCALE & z(void) { return (*this)[2]; }
};


//! specialization for TUPLE_SIZE == 3
template <typename TYPE_SCALE>
class TVector<TYPE_SCALE, 4>
    : public std::array<TYPE_SCALE, 4>
{
public:

    TYPE_SCALE x(void) const { return (*this)[0]; }
    TYPE_SCALE y(void) const { return (*this)[1]; }
    TYPE_SCALE z(void) const { return (*this)[2]; }
    TYPE_SCALE w(void) const { return (*this)[3]; }

    TYPE_SCALE & x(void) { return (*this)[0]; }
    TYPE_SCALE & y(void) { return (*this)[1]; }
    TYPE_SCALE & z(void) { return (*this)[2]; }
    TYPE_SCALE & w(void) { return (*this)[3]; }
};


} // Vector


} // Utility


#endif / /__Utility_Vector_Types__h__