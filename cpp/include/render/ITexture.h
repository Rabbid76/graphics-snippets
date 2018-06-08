/******************************************************************//**
* @brief   Generic interface for rendering textures.
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
#pragma once
#ifndef ITexture_h_INCLUDED
#define ITexture_h_INCLUDED


// includes

#include <IDrawType.h>


/******************************************************************//**
* @brief   Namespace for renderer.
*
* @author  gernot
* @date    2018-03-17
* @version 1.0
**********************************************************************/
namespace Render
{


/******************************************************************//**
* \brief   Type of the texture 1D, 2D, 3D
* 
* \author  gernot
* \date    2018-06-08
* \version 1.0
**********************************************************************/
enum class TTextureType
{
  T1D,       //!< 1 dimensional texture
  T2D,       //!< 2 dimensional texture
  T3D,       //!< 3 dimensional texture
  TCUBE,     //!< 2 dimensional texture with 6 layers (6 sides of the cubemap)
  T2D_ARRAY, //!< 2 dimensional texutre with layers
};


/******************************************************************//**
* \brief   Standard resource image formats.
* 
* \author  gernot
* \date    2018-06-08
* \version 1.0
**********************************************************************/
enum class TImageFormat
{
  RGB8,   //!< 3 color channels; 1 byte (unsigned)
  RGBA8,  //!< 3 colro channels and 1 alpha channel; 1 byte (unsigned) 
};


/******************************************************************//**
* \brief   Standard texture formats
* 
* \author  gernot
* \date    2018-06-08
* \version 1.0
**********************************************************************/
enum class TTextureFormat
{
  RGB8,        //!< 3 color channels; 1 byte; [0.0, 1.0]
  RGBA8,       //!< 3 color channels and 1 alpha channel; 1 byte;  [0.0, 1.0] 
  RGB8_SNORM,  //!< 3 color channels; 1 byte  [-0.0, 1.0]
  RGB16_SNORM, //!< 3 color channels; 1 byte  [-0.0, 1.0]
};


/******************************************************************//**
* \brief   Transformation when loading an image resurce to a texture. 
* 
* \author  gernot
* \date    2018-06-08
* \version 1.0
**********************************************************************/
enum class TTrasformation
{
  NON
  // ...
};


using TTextureSize = std::array<size_t, 3>;

//---------------------------------------------------------------------
// ITexture
//---------------------------------------------------------------------


/******************************************************************//**
* @brief   Generic interface for image resource data.
*
* @author  gernot
* @date    2018-03-17
* @version 1.0
**********************************************************************/
class IImageResource
{
public:

  virtual TTextureType Type( void )   const = 0; //!< type of the texture 1D, 2D, 3D
  virtual TImageFormat Format( void ) const = 0; //!< format of one texel or voxel
  virtual TTextureSize Size( void )   const = 0; //!< size for up to 3 dimensions
  virtual size_t       Layers( void ) const = 0; //!< number of layers
  virtual size_t       BPL( void )    const = 0; //!< number of byts of a single line of the image
  virtual const void * DataPtr()      const = 0; //!< pointer to the native image data
};


/******************************************************************//**
* @brief   Generic interface for loading an image from a resource.
*
* @author  gernot
* @date    2018-03-17
* @version 1.0
**********************************************************************/
class IImageResourceLoader
{
public:


};


/******************************************************************//**
* @brief Generic interface for a render texture.
*
* Implementations of this interface should use
* Resource Acquisition Is Initialization or RAII
* [http://en.cppreference.com/w/cpp/language/raii]
*
* @author  gernot
* @date    2018-03-17
* @version 1.0
**********************************************************************/
class ITexture
{
public:

  virtual ~ITexture() = default;

  virtual bool Bind( size_t binding_point ) = 0;
  virtual bool Release( size_t binding_point ) = 0;
};

using TTexturePtr = std::unique_ptr<ITexture>;


/******************************************************************//**
* @brief Generic interface for lading a render texture from a
* resource.
*
* @author  gernot
* @date    2018-03-17
* @version 1.0
**********************************************************************/
class ITextureLoader
{
public:

  virtual ~ITextureLoader() = default;

  virtual TTexturePtr NewTexture(IImageResource &image, TTrasformation transform, TTextureFormat internalformat) = 0;

};



} // Render

#endif // ITexture_h_INCLUDED