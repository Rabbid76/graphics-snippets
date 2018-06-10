/******************************************************************//**
* \brief OpenGL implementation of `ITextureLoader` for creating
* and loading textures.
* 
* \author  gernot
* \date    2018-02-04
* \version 1.0
**********************************************************************/

// includes

#include <stdafx.h>

// OpenGL

#include <OpenGLError.h>
#include <OpenGLTextureLoader.h>

// OpenGL wrapper

#include <GL/glew.h>
//#include <GL/gl.h> not necessary because of glew 
#include <GL/glu.h>

// stl

#include <algorithm> 
#include <unordered_map>
#include <iostream>


// preprocessor

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifndef DebugWarning
#define DebugWarning std::cout
#endif

#ifndef ASSERT
#include <assert.h>
#define ASSERT assert
#endif


// class implementations


/******************************************************************//**
* \brief General OpenGL namespace
**********************************************************************/
namespace OpenGL
{


//*********************************************************************
// CTextureInternal
//*********************************************************************


/******************************************************************//**
* \brief Internal OpenGL texture implementation for loaded image
* resources.
* 
* \author  gernot
* \date    2018-06-09
* \version 1.0
**********************************************************************/
class CTextureInternal
  : public Render::ITexture
{
public:

  CTextureInternal( Render::TTextureType type )
    : _type( type )
  {
    glGenTextures( 1, &_texture_obj );
  }

  virtual ~CTextureInternal()
  {
    if ( _texture_obj != 0 )
      glDeleteTextures( 1, &_texture_obj );
  }

  virtual Render::TTextureType Type( void ) const override { return _type; }


  virtual bool Bind( size_t binding_point ) override
  {
    GLenum target = CTextureLoader::TargetType( _type );

    glActiveTexture( (GLenum)(GL_TEXTURE0 + binding_point) );
    glBindTexture( target, _texture_obj );
    return true;
  }

  virtual bool Release( size_t binding_point ) override
  {
    GLenum target = CTextureLoader::TargetType( _type );

    // Bind the default texture to the texture unit.
    // In common there should be no necessity of this and there should not be any reason to do this.
    glActiveTexture( (GLenum)(GL_TEXTURE0 + binding_point) );
    glBindTexture( target, 0 );
    if ( binding_point > 0 )
      glActiveTexture( 0 );
    return true;
  }

private:

  unsigned int         _texture_obj{ 0 };
  Render::TTextureType _type{ Render::TTextureType::T2D };
};


//*********************************************************************
// CTextureLoader
//*********************************************************************


/******************************************************************//**
* \brief   ctor
* 
* \author  gernot
* \date    2018-06-09
* \version 1.0
**********************************************************************/
CTextureLoader::CTextureLoader( void )
{}


/******************************************************************//**
* \brief   ctor
* 
* \author  gernot
* \date    2018-06-09
* \version 1.0
**********************************************************************/
CTextureLoader::CTextureLoader( 
  size_t loader_binding_point ) //!< in: default binding point for loading textures
{}
  

/******************************************************************//**
* \brief   dtor
* 
* \author  gernot
* \date    2018-06-09
* \version 1.0
**********************************************************************/
CTextureLoader::~CTextureLoader()
{}


/******************************************************************//**
* \brief Map render texture type to OpenGL target enumerator constant  
* 
* \author  gernot
* \date    2018-06-10
* \version 1.0
**********************************************************************/
unsigned int CTextureLoader::TargetType( 
  Render::TTextureType type ) //!< in: texture type
{
  const  std::unordered_map< Render::TTextureType, GLenum > target_map
  {
    { Render::TTextureType::T1D,       GL_TEXTURE_1D },
    { Render::TTextureType::T2D,       GL_TEXTURE_2D },
    { Render::TTextureType::T3D,       GL_TEXTURE_3D },
    { Render::TTextureType::TCUBE,     GL_TEXTURE_CUBE_MAP }, 
    { Render::TTextureType::T2D_ARRAY, GL_TEXTURE_2D_ARRAY } 
  };

  auto it = target_map.find(type);
  ASSERT(it != target_map.end());
  return it != target_map.end() ? it->second : GL_TEXTURE_2D;
}


/******************************************************************//**
* \brief   Create a new but empty texture.
* 
* \author  gernot
* \date    2018-06-09
* \version 1.0
**********************************************************************/
Render::TTexturePtr CTextureLoader::CreateTexture(
  const Render::TTextureSize       &size,   //!< in: texture size
  size_t                            layers, //!< in: number of layers
  const Render::TTextureParameters &param ) //!< in: texture properties
{
  GLenum target = TargetType( param._type );
  if ( target != GL_TEXTURE_2D )
  {
    DebugWarning << "creating texture with target " << target << "is not yet implemented";
    return nullptr;
  }

  // TODO $$$

  Render::TTexturePtr texture = std::make_unique<CTextureInternal>( param._type );
  texture->Bind( _loader_binding_point );

  glTexParameteri( target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexImage2D( target, 0, GL_RGBA, (GLsizei)size[0], (GLsizei)size[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 ); 

  // TODO $$$

  return texture;
}


/******************************************************************//**
* \brief   Create a new texture from an image resource.
* 
* \author  gernot
* \date    2018-06-09
* \version 1.0
**********************************************************************/
Render::TTexturePtr CTextureLoader::CreateTexture(
  Render::IImageResource           &image,     //!< in: source image resource
  Render::TTrasformation            transform, //!< in: special transformtion algorithm
  const Render::TTextureSize       &size,      //!< in: texture size
  size_t                            layers,    //!< in: number of layers
  const Render::TTextureParameters &param )    //!< in: texture properties
{
  GLenum target = TargetType( param._type );
  if ( target != GL_TEXTURE_2D )
  {
    DebugWarning << "creating texture with target " << target << "is not yet implemented";
    return nullptr;
  }

  // TODO $$$
  DebugWarning << "creating texture from image resource is not yet implemented";
  // TODO $$$

  return nullptr;
}


/******************************************************************//**
* \brief   Load image data to texture.
* 
* \author  gernot
* \date    2018-06-09
* \version 1.0
**********************************************************************/
bool CTextureLoader::LoadToTexture(
  Render::IImageResource &image,   //!< in: source image resource
  Render::ITexture       &texture, //!< in: target texture 
  Render::TTexturePoint  &pos,     //!< in: target position
  size_t                  layer )  //!< in: target layer
{
  GLenum target = TargetType( texture.Type() );
  if ( target != GL_TEXTURE_2D )
  {
    DebugWarning << "loading texture with target " << target << "is not yet implemented";
    return false;
  }

  // TODO $$$

  //glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

  glTexSubImage2D(GL_TEXTURE_2D, 0, (GLsizei)pos[0], (GLsizei)pos[1], (GLsizei)image.Size()[0], (GLsizei)image.Size()[1], GL_RGBA, GL_UNSIGNED_BYTE, image.DataPtr() );

  //glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );

  // TODO $$$

  return false;
}

} // OpenGL