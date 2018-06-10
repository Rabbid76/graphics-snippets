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

#include <ITexture.h>

#include <array>


// class definitions

namespace OpenGL
{

//*********************************************************************
// CTextureLoader
//*********************************************************************

/******************************************************************//**
* \brief OpenGL implmentation of generic interface `ITextureLoader`  
* 
* \author  gernot
* \date    2018-06-09
* \version 1.0
**********************************************************************/
class CTextureLoader
  : public Render::ITextureLoader
{
public: 

  CTextureLoader( void );                        //!< default constructor
  CTextureLoader( size_t loader_binding_point ); //!< constructor
  virtual ~CTextureLoader();                     //!< destructor

  //! enable anisotropic filter
  static void SetMaxAnisotropicFilter( size_t max_anisotripic_filter ) { _max_anisotripic_filter = max_anisotripic_filter; }

  //! create a new but empty texture
  virtual Render::TTexturePtr CreateTexture(const Render::TTextureSize &size, size_t layers, const Render::TTextureParameters &parameter) override;

  //! create a new texture from an image resource
  virtual Render::TTexturePtr CreateTexture(Render::IImageResource &image, Render::TTrasformation transform, const Render::TTextureSize &size, size_t layers, const Render::TTextureParameters &parameter) override;

  //! load image data to texture
  virtual bool LoadToTexture(Render::IImageResource &image, Render::ITexture &texture, Render::TTexturePoint &pos, size_t layer) override;


  //! map texture type to OpenGL target type enumerator constant
  static unsigned int TargetType( Render::TTextureType type );

  //! map filter type to OpenGL minifying filter enumerator constant
  static unsigned int Minifying( Render::TTextureFilter filter, int max_mipmap_level );

  //! map filter type to OpenGL magnification filter enumerator constant
  static unsigned int Magnification ( Render::TTextureFilter filter, int max_mipmap_level );

  //! map texture wrap parameter to OpenGL minifying filter enumerator constant
  static unsigned int Wrap( const Render::TTextureWrap &parameter );

  //! map texture wrap parameters to OpenGL magnification  filter enumerator constant
  static std::array<unsigned int, 3> Wrap( const Render::TTextureParameters &parameter );

  //! Apply OpenGL texture paramter to current texture object
  static bool SetTextureParameter( const Render::TTextureParameters &parameter );

private:

  static size_t _max_anisotripic_filter;

  size_t _loader_binding_point = 0;
};

} // OpenGL

#endif // OpenGLTextureLoader_h_INCLUDED
