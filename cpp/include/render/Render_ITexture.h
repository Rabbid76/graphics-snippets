/******************************************************************//**
* @brief   Generic interface for rendering textures.
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
#pragma once
#ifndef Render_ITexture_h_INCLUDED
#define Render_ITexture_h_INCLUDED


// includes

#include <Render_IDrawType.h>

#include <memory>


/******************************************************************//**
* @brief   Namespace for renderer.
*
* @author  gernot
* @date    2018-03-17
* @version 1.0
**********************************************************************/
namespace Render
{

class ITextureLoader;
using ITextureLoaderPtr = std::unique_ptr<ITextureLoader>;


/******************************************************************//**
* \brief Kind of the resource image content.  
* 
* \author  gernot
* \date    2018-06-09
* \version 1.0
**********************************************************************/
enum class TImageKind : t_byte
{
  NON,                    //!< not used (no texture)
  diffuse,                //!< color texture
  grayscale,              //!< grayscale texture
  displacement_map,       //!< displacment map; 1 channel (red)
  normal_map,             //!< normal map; 3 channels (RGB)
  noraml_displacment_map, //!< normal map; 3 channels (RGB) + displacment map; 1 channel (alpha)
  displacment_cone_map    //!< displacment map; 1 channel (red) + cone map; 1 channel (green) 
};


/******************************************************************//**
* \brief   
* 
* \author  gernot
* \date    2018-06-09
* \version 1.0
**********************************************************************/
enum class TTextureFilter : t_byte
{
  NON,        //!< no filter
  bilinear,   //!< linear interpolation
  trilinear   //!< mipmaps (linear interpolation)
};


/******************************************************************//**
* \brief   Texture wrap parameter
* 
* \author  gernot
* \date    2018-06-09
* \version 1.0
**********************************************************************/
enum class TTextureWrap : t_byte
{
  clamp,           //!< clamp texture to edge
  mirrored,        //!< mirror and clamp texture to edge
  tiled,           //!< tile texture
  tiled_mirrored,  //!< tile texture; even tiles are mirrored
};
using TTextureWraps = std::array<TTextureWrap, 3>;


/******************************************************************//**
* \brief   Type of the texture 1D, 2D, 3D
* 
* \author  gernot
* \date    2018-06-08
* \version 1.0
**********************************************************************/
enum class TTextureType : t_byte
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
  UNKNOWN, //!< unsupported image format
  GRAY8,   //!< 1 color channel; 1 byte (unsigned)
  RGB8,    //!< 3 color channels; 1 byte (unsigned)
  BGR8,    //!< 3 color channels; 1 byte (unsigned)
  RGBA8,   //!< 3 color channels and 1 alpha channel; 1 byte (unsigned) 
  BGRA8,   //!< 3 color channels and 1 alpha channel; 1 byte (unsigned) 
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
  R8,          //!< 1 color channel; 1 byte; [0.0, 1.0]
  RG8,         //!< 1 color channel; 1 byte; [0.0, 1.0]
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



//---------------------------------------------------------------------
// IImageResource
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

  virtual ~IImageResource() = default;

  virtual TTextureType Type( void )    const = 0; //!< type of the texture 1D, 2D, 3D
  virtual TImageFormat Format( void )  const = 0; //!< format of one texel or voxel
  virtual TTextureSize Size( void )    const = 0; //!< size for up to 3 dimensions
  virtual size_t       Layers( void )  const = 0; //!< number of layers
  virtual size_t       BPL( void )     const = 0; //!< number of byts of a single line of the image
  virtual const void * DataPtr( void ) const = 0; //!< pointer to the native image data
};
using IImageResourcePtr = std::unique_ptr<IImageResource>;



//---------------------------------------------------------------------
// IImageResourceLoader
//---------------------------------------------------------------------


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

  virtual ~IImageResourceLoader() = default;

};


//---------------------------------------------------------------------
// ITextureLoaderProvider
//---------------------------------------------------------------------


/******************************************************************//**
* \brief   Generic provider for generic texture loaders.
* 
* \author  gernot
* \date    2018-06-08
* \version 1.0
**********************************************************************/
class ITextureLoaderProvider
{
public:

  virtual ~ITextureLoaderProvider() = default;

  virtual ITextureLoaderPtr NewTextureLoader( void ) = 0;
};



//---------------------------------------------------------------------
// ITexture
//---------------------------------------------------------------------



/******************************************************************//**
* \brief   Texture setup parameters.
* 
* \author  gernot
* \date    2018-06-09
* \version 1.0
**********************************************************************/
struct TTextureParameters
{
  TTextureParameters() = default;
  TTextureParameters( const TTextureParameters & ) = default;

  TTextureParameters( TTextureType type, TTextureFormat format, TTextureWraps wrap, TTextureFilter filter, int max_mipmap, int anisotropic )
    : _type( type )
    , _format( format )
    , _wrap( wrap )
    , _filter( filter )
    , _max_mipmap( max_mipmap )
    , _anisotropic( anisotropic )
  {}

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

  virtual size_t ObjectHandle( void ) const = 0;

  virtual TTextureType Type( void ) const = 0;

  virtual bool Bind( size_t binding_point ) = 0;
  virtual bool Release( size_t binding_point ) = 0;
};

using ITexturePtr = std::unique_ptr<ITexture>;



//---------------------------------------------------------------------
// ITextureLoader
//---------------------------------------------------------------------


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
  T2D_RGBA_tiled_nofilter      = 0,
  T2D_RGBA_tiled_bilinear      = 1,
  T2D_RGBA_tiled_trilinear     = 2,
  T2D_RGBA_clamped_nofilter    = 3,
  T2D_RGBA_clamped_bilinear    = 4, //!< Recommended for texture fonts. Note texture fonts may disappear with trilinear filtering and huge minification.
  T2D_RGBA_clamped_trilinear   = 5
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
  ITexturePtr CreateTexture( const TTextureSize &size, size_t layers, TStandardTextureKind kind )
  {
    return CreateTexture(size, layers, Parameters(kind));
  }
  
  //! create a new but empty texture
  virtual ITexturePtr CreateTexture( const TTextureSize &size, size_t layers, const TTextureParameters &param ) = 0;


  //! create a new texture from an image resource
  ITexturePtr CreateTexture( const IImageResource &image, TStandardTextureKind kind)
  {
    return CreateTexture( image, TTrasformation::NON, image.Size(), image.Layers(), Parameters(kind) );
  }

  //! create a new texture from an image resource
  ITexturePtr CreateTexture( const IImageResource &image, TTrasformation transform, TStandardTextureKind kind )
  {
    return CreateTexture( image, transform, image.Size(), image.Layers(), Parameters(kind) );
  }

  //! create a new texture from an image resource
  ITexturePtr CreateTexture( const IImageResource &image, TTrasformation transform, const TTextureSize &size, size_t layers, TStandardTextureKind kind )
  {
    return CreateTexture( image, transform, size, layers, Parameters(kind) );
  }
       
  //! create a new texture from an image resource
  virtual ITexturePtr CreateTexture( const IImageResource &image, TTrasformation transform, const TTextureSize &size, size_t layers, const TTextureParameters &param ) = 0;

  //! load image data to texture
  virtual bool LoadToTexture( const IImageResource &image, ITexture &texture, const TTexturePoint &pos, size_t layer ) = 0;


  static const TTextureParameters & Parameters( TStandardTextureKind id )
  {
    static const TTextureParameters param_table[] = {

      // T2D_RGBA_tiled_nofilter
      TTextureParameters{ TTextureType::T2D, TTextureFormat::RGBA8, { TTextureWrap::tiled, TTextureWrap::tiled, TTextureWrap::tiled }, TTextureFilter::NON, 0, 0 },

      // T2D_RGBA_tiled_bilinear
      TTextureParameters{ TTextureType::T2D, TTextureFormat::RGBA8, { TTextureWrap::tiled, TTextureWrap::tiled, TTextureWrap::tiled }, TTextureFilter::bilinear, 0, 0 },
   
      // T2D_RGBA_tiled_trilinear
      TTextureParameters{ TTextureType::T2D, TTextureFormat::RGBA8, { TTextureWrap::tiled, TTextureWrap::tiled, TTextureWrap::tiled }, TTextureFilter::trilinear, 1000, 16 },

      // T2D_RGBA_clamped_nofilter
      TTextureParameters{ TTextureType::T2D, TTextureFormat::RGBA8, { TTextureWrap::clamp, TTextureWrap::clamp, TTextureWrap::clamp }, TTextureFilter::NON, 0, 0 },

      // T2D_RGBA_clamped_bilinear
      TTextureParameters{ TTextureType::T2D, TTextureFormat::RGBA8, { TTextureWrap::clamp, TTextureWrap::clamp, TTextureWrap::clamp }, TTextureFilter::bilinear, 0, 0 },
    
      // T2D_RGBA_clamped_trilinear
      TTextureParameters{ TTextureType::T2D, TTextureFormat::RGBA8, { TTextureWrap::clamp, TTextureWrap::clamp, TTextureWrap::clamp }, TTextureFilter::trilinear, 1000, 16 },
    };

    return param_table[(int)id];
  }
};


} // Render

#endif // Render_ITexture_h_INCLUDED