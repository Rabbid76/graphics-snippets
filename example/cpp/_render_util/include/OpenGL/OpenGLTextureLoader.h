/******************************************************************//**
* \brief OpenGL implementation of `ITextureLoader` for creating
* and loading textures.
* 
* \author  gernot
* \date    2018-02-04
* \version 1.0
**********************************************************************/
#pragma once
#ifndef OpenGLTextureLoader_h_INCLUDED
#define OpenGLTextureLoader_h_INCLUDED

// includes

#include <Render_ITexture.h>

// STL

#include <array>


// class definitions


/******************************************************************//**
* \brief General namespace for OpenGL implementation.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
namespace OpenGL
{

class CTextureInternal;


//*********************************************************************
// CTextureLoader
//*********************************************************************

/******************************************************************//**
* \brief OpenGL implementation of generic interface `ITextureLoader`  
* 
* \author  gernot
* \date    2018-06-09
* \version 1.0
**********************************************************************/
class CTextureLoader
  : public Render::ITextureLoader
{
  friend class CTextureInternal;

public: 

  using Render::ITextureLoader::CreateTexture;

  CTextureLoader( void );                     //!< default constructor
  CTextureLoader( size_t loader_binding_id ); //!< constructor
  virtual ~CTextureLoader();                  //!< destructor

  
  size_t BindingId( void )             const { return _loader_binding_id; }
  int    MaxAnisotropicSamples( void ) const { return _max_anisotripic_samples >= 0 ? _max_anisotripic_samples : _max_anisotripic_samples_default; }

  //! set default anisotropic filter samples
  static void SetMaxAnisotropicSamplesDefault( int max_anisotripic_samples ) { _max_anisotripic_samples_default = max_anisotripic_samples; }

  //! enable anisotropic filter
  void SetMaxAnisotropicSamples( int max_anisotripic_samples ) { _max_anisotripic_samples = max_anisotripic_samples; }

  //! create a new but empty texture
  virtual Render::ITexturePtr CreateTexture(const Render::TTextureSize &size, size_t layers, const Render::TTextureParameters &parameter) override;

  //! create a new texture from an image resource
  virtual Render::ITexturePtr CreateTexture(const Render::IImageResource &image, Render::TImageTransformations transform, const Render::TTextureSize &size, size_t layers, const Render::TTextureParameters &parameter) override;

  //! load image data to texture
  virtual bool LoadToTexture(const Render::IImageResource &image, Render::ITexture &texture, const Render::TTexturePoint &pos, size_t layer) override;


  //! map texture type to OpenGL target type enumerator constant
  static unsigned int TargetType( Render::TTextureType type );

  //! map texture format to OpenGL source format
  static unsigned int ImageFormat( Render::TImageFormat format );

  //! map texture format to OpenGL source format
  static unsigned int ImageDataType( Render::TImageFormat format );

  //! map texture format to OpenGL internal format
  static unsigned int InternalFormat( Render::TTextureFormat format );

  //! map filter type to OpenGL minifying filter enumerator constant
  static unsigned int Minifying( Render::TTextureFilter filter, int max_mipmap_level );

  //! map filter type to OpenGL magnification filter enumerator constant
  static unsigned int Magnification ( Render::TTextureFilter filter, int max_mipmap_level );

  //! map texture wrap parameter to OpenGL minifying filter enumerator constant
  static unsigned int Wrap( const Render::TTextureWrap &parameter );

  //! map texture wrap parameters to OpenGL magnification  filter enumerator constant
  static std::array<unsigned int, 3> Wrap( const Render::TTextureParameters &parameter );

  //! Get texture parameters for the initial creation of a texture
  static Render::TTextureParameters TextureParamterLeve0( const Render::TTextureParameters &parameter );

  //! Apply OpenGL texture parameter to current texture object
  static bool SetTextureParameter( unsigned int texture_object, const Render::TTextureParameters &parameter, int max_anisotropic_samples );

  //! Generate mipmap according to the parameters
  static bool GenerateMipmaps( unsigned int texture_object, const Render::TTextureParameters &parameter );

protected: 

  //! create a new but empty texture
  Render::ITexturePtr CreateTexture(const Render::TTextureSize &size, size_t layers, const Render::TTextureParameters &parameter, bool set_parameters);


private:

  //! evaluate OpenGL version capabilities
  static void EvaluateCapabilities( void );

  //! bind a texture to a texture unit
  static bool BindTexture( unsigned int target, unsigned int texture_object, size_t binding_id );

  //! bind a texture to a texture unit
  static bool SetTextureParameterI( unsigned int target, unsigned int texture_object, unsigned int parameter, int value );

  //! true: OpenGL version capabilities have been evaluated and dependencies have been stated
  static bool _capabilities_evaluated; 
  
  //!< true: use direct state access; depends on OpenGL version
  static bool _dsa;

  //! maximum allowed anisotropic filter
  static int _max_anisotripic_samples_default;

  //! maximum allowed anisotropic filter
  int _max_anisotripic_samples = -1; 

  size_t _loader_binding_id = 0;
};

} // OpenGL

#endif // OpenGLTextureLoader_h_INCLUDED
