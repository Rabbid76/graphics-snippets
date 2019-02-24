/***********************************************************************************************//**
* \brief Interfaces related to object and GPU objects.
*
* \author  Rabbid76    \date  2019-22-02
***************************************************************************************************/

#pragma once

#pragma once
#ifndef __Object_IObjecct_Types__h__
#define __Object_IObjecct_Types__h__

// include

#include <utility_exception_types.h>

// STL

#include <bitset>
#include <memory>


// view assert
#define OBJECT_ASSERT(CONDITION) assert(CONDITION);

namespace Object
{ 

// object exception
using Error = Utility::Exception::RuntimeError;


//! Object handle
using THandle = uint32_t;


//! Class of the object 
enum class TObjectClass : uint32_t
{
    opengl, //! OpenGL object
};


//! Object capabilities which strongly depend on the class of the object (`TObjectClass`)
using TObjectCapability = std::bitset<32>;


//! Current state of the object
enum class TObjectState : uint32_t
{
    initilized = 0, //! object is initialized

    // ...
    NO_OF_STATES,  //! number of object states 
};



/***********************************************************************************************//**
* \brief Base class for objects.
*
* Objects are not trivial copyable and may use 
* [Resource Acquisition Is Initialization (RAII)](http://en.cppreference.com/w/cpp/language/raii)
* technology.
*
* \author  gernot Rabbid76    \date  2019-02-22
***************************************************************************************************/
class CObject
{
public:

    CObject( TObjectClass obj_calss, TObjectCapability obj_cap )
        : _handle( NewHandle() )
        , _class( obj_calss )
        , _capability( obj_cap )
    {}

    CObject( const CObject & ) = delete;
    CObject( CObject && ) = default;

    virtual ~CObject() {} 

    CObject & operator = ( const CObject & ) = delete;
    CObject & operator = ( CObject && ) = default;

    operator THandle (void) const { return _handle; } 

    THandle ResourceHandle( void ) const { return _resource_handle; }

    static THandle NewHandle( void )
    {
        static THandle next_handle = 0;
        THandle handle = next_handle ++;
        return handle;
    }

private:

    THandle           _handle;              //!< internal object handle
    TObjectClass      _class;               //!< class (type) of the object
    TObjectCapability _capability;          //!< capabilities of the object
    TObjectState      _state;               //!< current state of the object
    THandle           _resource_handle = 0; //!< handle for the implementation dependent object resource
};

using TObjectHandle = std::shared_ptr<CObject>;


} // Object

#endif // __Object_IObjecct_Types__h__
