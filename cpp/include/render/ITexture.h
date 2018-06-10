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


using TTexturePoint = std::array<size_t, 3>;
using TTextureSize  = std::array<size_t, 3>;


/******************************************************************//**
* \brief   Texture wrap parameter
* 
* \author  gernot
* \date    2018-06-09
* \version 1.0
**********************************************************************/
enum class TTextureWrap
{
  clamp,
  tiled,
  tiled_mirrored
};

using TTextureWraps = std::array<TTextureWrap, 3>;


/******************************************************************//**
* \brief   
* 
* \author  gerno
* \date    2018-06-09
* \version 1.0
**********************************************************************/
enum class TTextureFilter
{
  NON,
  bilinear,
  trilinear
};


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
* \brief   Texture setup parameters.
* 
* \author  gernot
* \date    2018-06-09
* \version 1.0
**********************************************************************/
struct TTextureParameters
{
  TTextureType   _type{ TTextureType::T2D };
  TTextureFormat _format{ TTextureFormat::RGBA8 };
  TTextureWraps  _wrap{ TTextureWrap::clamp, TTextureWrap::clamp, TTextureWrap::clamp };
  TTextureFilter _filter{ TTextureFilter::trilinear };
  int            _max_mipmap{ 1000 };
  int            _anisotropic{ 16 };

  bool Is1DType( void ) const { return _type == TTextureType::T1D; }
  bool Is2DType( void ) const { return _type == TTextureType::T2D || _type == TTextureType::T2D_ARRAY || _type == TTextureType::TCUBE; }
  bool Is3DType( void ) const { return _type == TTextureType::T3D; }
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

  virtual TTextureType Type( void ) const = 0;

  virtual bool Bind( size_t binding_point ) = 0;
  virtual bool Release( size_t binding_point ) = 0;
};

using TTexturePtr = std::unique_ptr<ITexture>;


/******************************************************************//**
* \brief   Predfined texture kinds
* 
* Note, the enum constant is used as an index for a paramter table.
* 
* \author  gernot
* \date    2018-06-09
* \version 1.0
**********************************************************************/
enum TStandardTextureKind
{
  T2D_RGBA_tiled_nofilter    = 0,
  T2D_RGBA_tiled_trilinear   = 1,
  T2D_RGBA_clamped_nofilter  = 2,
  T2D_RGBA_clamped_trilinear = 3,
};


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

  //! create a new but empty texture
  TTexturePtr CreateTexture(const TTextureSize &size, size_t layers, TStandardTextureKind kind)
  {
    return CreateTexture(size, layers, Parameters(kind));
  }
  
  //! create a new but empty texture
  virtual TTexturePtr CreateTexture(const TTextureSize &size, size_t layers, const TTextureParameters &param) = 0;

  //! create a new texture from an image resource
  TTexturePtr CreateTexture(IImageResource &image, TTrasformation transform, const TTextureSize &size, size_t layers, TStandardTextureKind kind)
  {
    return CreateTexture( image, transform, size, layers, Parameters(kind) );
  }
       
  //! create a new texture from an image resource
  virtual TTexturePtr CreateTexture(IImageResource &image, TTrasformation transform, const TTextureSize &size, size_t layers, const TTextureParameters &param) = 0;

  //! load image data to texture
  virtual bool LoadToTexture(IImageResource &image, ITexture &texture, TTexturePoint &pos, size_t layer) = 0;


  static const TTextureParameters & Parameters( TStandardTextureKind id )
  {
    static const TTextureParameters param_table[] = {

      // T2D_RGBA_tiled_nofilter
      TTextureParameters{ { TTextureType::T2D }, { TTextureFormat::RGBA8 }, { TTextureWrap::tiled, TTextureWrap::tiled, TTextureWrap::tiled }, { TTextureFilter::NON }, { 0 }, { 0 } },

      // T2D_RGBA_tiled_trilinear
      TTextureParameters{ { TTextureType::T2D }, { TTextureFormat::RGBA8 }, { TTextureWrap::tiled, TTextureWrap::tiled, TTextureWrap::tiled }, { TTextureFilter::trilinear }, { 1000 }, { 16 } },

      // T2D_RGBA_clamped_nofilter
      TTextureParameters{ { TTextureType::T2D }, { TTextureFormat::RGBA8 }, { TTextureWrap::clamp, TTextureWrap::clamp, TTextureWrap::clamp }, { TTextureFilter::NON }, { 0 }, { 0 } },

      // T2D_RGBA_clamped_trilinear
      TTextureParameters{ { TTextureType::T2D }, { TTextureFormat::RGBA8 }, { TTextureWrap::clamp, TTextureWrap::clamp, TTextureWrap::clamp }, { TTextureFilter::trilinear }, { 1000 }, { 16 } },
    };

    return param_table[(int)id];
  }
};



} // Render

#endif // ITexture_h_INCLUDED