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

// scale type for texture size and window coordinates
typedef int TScale;


class ITextureGenerator
{
public:

    ITextureGenerator( void ) = default;
    ITextureGenerator( ITextureGenerator && ) = default;
    ITextureGenerator & operator = ( ITextureGenerator && ) = default;

    ITextureGenerator( const ITextureGenerator & ) = delete;
    ITextureGenerator & operator = ( const ITextureGenerator & ) = delete;

    virtual ~ITextureGenerator() = default;

    virtual Object::TObjectHandle Generate( void ) = 0;
};


} // Texture


#endif // __Texture_ITexture_Types__h__
