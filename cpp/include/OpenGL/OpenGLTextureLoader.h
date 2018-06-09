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

  //! create a new but empty texture
  virtual Render::TTexturePtr CreateTexture(const Render::TTextureSize &size, size_t layers, const Render::TTextureParameters &param) override;

  //! create a new texture from an image resource
  virtual Render::TTexturePtr CreateTexture(Render::IImageResource &image, Render::TTrasformation transform, const Render::TTextureSize &size, size_t layers, const Render::TTextureParameters &param) override;

  //! load image data to texture
  virtual bool LoadToTexture(Render::IImageResource &image, Render::ITexture &texture, Render::TTexturePoint &pos, size_t layer) override;

private:

  size_t _loader_binding_point = 0;
};

} // OpenGL

#endif // OpenGLTextureLoader_h_INCLUDED
