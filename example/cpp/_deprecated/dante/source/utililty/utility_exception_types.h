/***********************************************************************************************//**
* \brief Exception and runtime error handler  
*
* \author  Rabbid76    \date  2019-22-02
***************************************************************************************************/

#pragma once
#ifndef __Utility_Exception_Types__h__
#define __Utility_Exception_Types__h__


// STL

#include <exception>
#include <stdexcept>


namespace Utility
{

namespace Exception
{


//! Runtime error handler
using RuntimeError = std::runtime_error;


} // Exception


} // Utility


#endif / /__Utility_Exception_Types__h__
