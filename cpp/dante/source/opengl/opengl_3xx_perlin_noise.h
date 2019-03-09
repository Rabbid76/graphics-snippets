/***********************************************************************************************//**
* \brief Base class implementation for OpenGL ES 3.x and (desktop) OpenGL 3.2+ texture filter
* and generator implementations. 
*
* \author  Rabbid76    \date  2019-22-02
***************************************************************************************************/

#pragma once
#ifndef __OpenGL_3xx_perlin_noise__h__
#define __OpenGL_3xx_perlin_noise__h__


// include

#include <texture_itexture_types.h>
#include <opengl_3xx_texture_factory.h>
#include <utility_noise_perlin.h>


// STL

// [...]


namespace Texture
{


namespace OpenGL
{


namespace GL3xx
{


/***********************************************************************************************//**
* \brief Perlin-Noise generator.  
*
* \author  gernot Rabbid76    \date  2019-02-24
***************************************************************************************************/
class CPerlinNoiseGenerator
    : public CFactory
    , public Utility::Noise::Perlin::CPerlinNoiseGenerator
{
public:

    using TPerlinNoisParameter = Utility::Noise::Perlin::TPerlinNoisParameter;

    CPerlinNoiseGenerator( const TPerlinNoisParameter & );
    virtual ~CPerlinNoiseGenerator();

    virtual Object::TObjectHandle Generate( void ) override;

};


} // GL3xx


} // OpenGL


} // Texture


#endif // __OpenGL_3xx_perlin_noise__h__
