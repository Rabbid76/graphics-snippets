/***********************************************************************************************//**
* \brief Types and base classes for texture handling, generation and filter.  
*
* \author  Rabbid76    \date  2019-22-02
***************************************************************************************************/

#pragma once
#ifndef __Texture_ITexture_Types__h__
#define __Texture_ITexture_Types__h__


// include

#include <object_iobject_types.h>


// STL

// ...


namespace Texture
{


class ITextureGenerator
{
public:

    virtual ~ITextureGenerator() = default;

    virtual Object::TObjectHandle Generate( void ) = 0;
};


} // Texture


#endif / /__Texture_ITexture_Types__h__
