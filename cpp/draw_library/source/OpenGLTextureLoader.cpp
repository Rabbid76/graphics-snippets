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

size_t CTextureLoader::_max_anisotripic_filter = 0;


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
* \brief Map texture type to OpenGL target type enumerator constant.  
* 
* \author  gernot
* \date    2018-06-10
* \version 1.0
**********************************************************************/
unsigned int CTextureLoader::TargetType( 
  Render::TTextureType type ) //!< in: texture type
{
  static const std::unordered_map< Render::TTextureType, GLenum > target_map
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
* \brief Map filter type to OpenGL minifying filter enumerator constant.
*
* It make only sense to appy trilinear filter if the maximum mipmaplevel
* is greater than 1.
* 
* \author  gernot
* \date    2018-06-10
* \version 1.0
**********************************************************************/
unsigned int CTextureLoader::Minifying( 
  Render::TTextureFilter filter,            //!< in: filter
  int                    max_mipmap_level ) //!< in: maximum mip map level
{
  static const std::unordered_map< Render::TTextureFilter, std::array<GLenum, 2> > filter_map
  {
    { Render::TTextureFilter::NON,       { GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST } },
    { Render::TTextureFilter::bilinear,  { GL_LINEAR,  GL_LINEAR_MIPMAP_NEAREST  } },
    { Render::TTextureFilter::trilinear, { GL_LINEAR,  GL_LINEAR_MIPMAP_LINEAR   } }
  };
  //!< Note, `GL_NEAREST_MIPMAP_LINEAR` is not used

  auto it = filter_map.find(filter);
  ASSERT(it != filter_map.end());
  return it != filter_map.end() ? it->second[max_mipmap_level > 1 ? 1 : 0] : GL_NEAREST;
}


/******************************************************************//**
* \brief Map filter type to OpenGL magnification filter enumerator constant.  
* 
* \author  gernot
* \date    2018-06-10
* \version 1.0
**********************************************************************/
unsigned int CTextureLoader::Magnification( 
  Render::TTextureFilter filter,            //!< in: filter
  int                    max_mipmap_level ) //!< in: maximum mip map level
{
  return filter == Render::TTextureFilter::NON ? GL_LINEAR : GL_NEAREST;
}


/******************************************************************//**
* \brief Mmap texture wrap parameters to OpenGL minifying filter
* enumerator constant.  
* 
* \author  gernot
* \date    2018-06-10
* \version 1.0
**********************************************************************/
unsigned int CTextureLoader::Wrap( 
  const Render::TTextureWrap &wrap ) //!< in: texture wrap parameter
{
  static const std::unordered_map< Render::TTextureWrap, GLenum > wrap_map
  {
    { Render::TTextureWrap::clamp,          GL_CLAMP_TO_EDGE   },
    { Render::TTextureWrap::tiled,          GL_REPEAT          },
    { Render::TTextureWrap::tiled_mirrored, GL_MIRRORED_REPEAT }
  };

  auto it = wrap_map.find(wrap);
  ASSERT(it != wrap_map.end());
  return it != wrap_map.end() ? it->second : GL_CLAMP_TO_EDGE;
}


/******************************************************************//**
* \brief Mmap texture wrap parameters to OpenGL minifying filter
* enumerator constant.  
* 
* \author  gernot
* \date    2018-06-10
* \version 1.0
**********************************************************************/
std::array<unsigned int, 3> CTextureLoader::Wrap( 
  const Render::TTextureParameters &parameter )  //!< in: texture parameter
{
  return
  { 
    CTextureLoader::Wrap( parameter._wrap[0] ),
    CTextureLoader::Wrap( parameter._wrap[1] ),
    CTextureLoader::Wrap( parameter._wrap[2] )
  };
}


/******************************************************************//**
* \brief  Apply OpenGL texture paramter to current texture object.  
*
* Set the texture parameter properties. If necessary then mipmaps are
* generated. 
* The texture as to be bound to the active texture unit before.
*
* \author  gernot
* \date    2018-06-10
* \version 1.0
**********************************************************************/
bool CTextureLoader::SetTextureParameter( 
  const Render::TTextureParameters &parameter )  //!< in: texture parameter
{
  GLenum target = TargetType( parameter._type );

  GLenum minifying     = CTextureLoader::Minifying( parameter._filter, parameter._max_mipmap );
  GLenum magnification = CTextureLoader::Magnification( parameter._filter, parameter._max_mipmap );

  auto wrap = CTextureLoader::Wrap( parameter );

  glTexParameteri( target, GL_TEXTURE_WRAP_S, wrap[0] );
  if ( parameter.Is2DType() || parameter.Is3DType() )
    glTexParameteri( target, GL_TEXTURE_WRAP_T, wrap[1] );
  if ( parameter.Is3DType() )
    glTexParameteri( target, GL_TEXTURE_WRAP_T, wrap[2] );
  
  glTexParameteri( target, GL_TEXTURE_MIN_FILTER, minifying );
  //glTexParameteri( target, GL_TEXTURE_MIN_FILTER, magnification );
  glTexParameteri( target, GL_TEXTURE_MAG_FILTER, magnification );

  // generate mipmaps
  if ( parameter._max_mipmap > 1 )
  {
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, (GLint)parameter._max_mipmap );

    // [`EXT_texture_filter_anisotropic`](https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_texture_filter_anisotropic.txt)
    GLint max_anisotropic = std::min((GLint)_max_anisotripic_filter, (GLint)parameter._anisotropic);
    if ( max_anisotropic > 1 )
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropic); 

    //! [`glGenerateMipmap`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGenerateMipmap.xhtml)
    glGenerateMipmap( GL_TEXTURE_2D );
  }

  return true;
}


/******************************************************************//**
* \brief   Create a new but empty texture.
* 
* \author  gernot
* \date    2018-06-09
* \version 1.0
**********************************************************************/
Render::TTexturePtr CTextureLoader::CreateTexture(
  const Render::TTextureSize       &size,       //!< in: texture size
  size_t                            layers,     //!< in: number of layers
  const Render::TTextureParameters &parameter ) //!< in: texture properties
{
  GLenum target = TargetType( parameter._type );
  if ( target != GL_TEXTURE_2D )
  {
    DebugWarning << "creating texture with target " << target << "is not yet implemented";
    return nullptr;
  }



  // TODO $$$

  Render::TTexturePtr texture = std::make_unique<CTextureInternal>( parameter._type );
  texture->Bind( _loader_binding_point );

  glTexImage2D( target, 0, GL_RGBA, (GLsizei)size[0], (GLsizei)size[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );

  SetTextureParameter( parameter ); 

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
  Render::IImageResource           &image,      //!< in: source image resource
  Render::TTrasformation            transform,  //!< in: special transformtion algorithm
  const Render::TTextureSize       &size,       //!< in: texture size
  size_t                            layers,     //!< in: number of layers
  const Render::TTextureParameters &parameter ) //!< in: texture properties
{
  GLenum target = TargetType( parameter._type );
  if ( target != GL_TEXTURE_2D )
  {
    DebugWarning << "creating texture with target " << target << "is not yet implemented";
    return nullptr;
  }

  // TODO $$$
  DebugWarning << "creating texture from image resource is not yet implemented";
  // TODO $$$

  // TODO $$$

  Render::TTexturePtr texture = std::make_unique<CTextureInternal>( parameter._type );
  texture->Bind( _loader_binding_point );

  // TODO $$$

  SetTextureParameter( parameter );

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