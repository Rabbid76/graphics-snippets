/***********************************************************************************************//**
* \brief Base class implementation for OpenGL ES 3.x and (desktop) OpenGL 3.2+ Perlin-Noise
* textures.
*
* \author  Rabbid76    \date  2019-22-02
***************************************************************************************************/

#include <stdafx.h>


// include

#include <opengl_3xx_perlin_noise.h>
#include <OpenGL_include.h>


namespace Texture
{


namespace OpenGL
{


namespace GL3xx
{


/***********************************************************************************************//**
* \brief  ctor. 
*
* \author  gernot Rabbid76    \date  2019-02-24
***************************************************************************************************/
CPerlinNoiseGenerator::CPerlinNoiseGenerator( 
    const TPerlinNoisParameter &param ) //! noise specification
    : CFactory()
    , Utility::Noise::Perlin::CPerlinNoiseGenerator(param)
{}
    

/***********************************************************************************************//**
* \brief dtor  
*
* \author  gernot Rabbid76    \date  2019-02-24
***************************************************************************************************/
CPerlinNoiseGenerator::~CPerlinNoiseGenerator()
{}


/***********************************************************************************************//**
* \brief Generate a new noise texture  
*
* \author  gernot Rabbid76    \date  2019-02-24
***************************************************************************************************/
Object::TObjectHandle CPerlinNoiseGenerator::Generate( void ) 
{
    Object::TObjectHandle handle;

    // ...

    return handle;
}


} // GL3xx


} // OpenGL


} // Texture