/***********************************************************************************************//**
* \brief Base class implementation for OpenGL ES 3.x and (desktop) OpenGL 3.2+ texture filter
* and generator implementations. 
*
* \author  Rabbid76    \date  2019-22-02
***************************************************************************************************/

#pragma once
#ifndef __OpenGL_3xx_texture_factory__h__
#define __OpenGL_3xx_texture_factory__h__


// include

// [...]


// STL

// [...]


namespace Texture
{


namespace OpenGL
{


namespace GL3xx
{


/***********************************************************************************************//**
* \brief Base class for texture generators and filters.  
*
* \author  gernot Rabbid76    \date  2019-02-24
***************************************************************************************************/
class CFactory
{
public:

    CFactory( void );
    virtual ~CFactory();

};


} // GL3xx


} // OpenGL


} // Texture


#endif // __OpenGL_3xx_texture_factory__h__
