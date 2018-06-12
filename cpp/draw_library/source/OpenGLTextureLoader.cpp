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

  size_t ObjectHandle( void ) const override { return _texture_obj; }

  virtual Render::TTextureType Type( void ) const override { return _type; }

  virtual bool Bind( size_t binding_id ) override
  {
    GLenum target = CTextureLoader::TargetType( _type );
    return CTextureLoader::BindTexture( target, _texture_obj, binding_id );
  }

  virtual bool Release( size_t binding_id ) override
  {
    GLenum target = CTextureLoader::TargetType( _type );
    return CTextureLoader::BindTexture( target, 0, binding_id );
  }

private:

  unsigned int         _texture_obj{ 0 };
  Render::TTextureType _type{ Render::TTextureType::T2D };
};


//*********************************************************************
// CTextureLoader
//*********************************************************************

bool   CTextureLoader::_capabilities_evaluated = false;
bool   CTextureLoader::_dsa = false;
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
  size_t loader_binding_id ) //!< in: default binding point for loading textures
{
  EvaluateCapabilities();
}
  

/******************************************************************//**
* \brief   dtor
* 
* \author  gernot
* \date    2018-06-09
* \version 1.0
**********************************************************************/
CTextureLoader::~CTextureLoader()
{
  EvaluateCapabilities();
}


/******************************************************************//**
* \brief Evaluate OpenGL version capabilities.  
* 
* \author  gernot
* \date    2018-06-12
* \version 1.0
**********************************************************************/
void CTextureLoader::EvaluateCapabilities( void )
{
  if ( _capabilities_evaluated == true )
    return;

  _dsa = true;
  if ( glBindTextureUnit == nullptr )
    _dsa = false;
  if ( glTextureParameteri == nullptr )
    _dsa = false;
  if ( glTextureStorage2D == nullptr )
    _dsa = false;
  if ( glGenerateTextureMipmap == nullptr )
    _dsa = false;
  _dsa == false; // TODO $$$

  _capabilities_evaluated = true;
}

 
/******************************************************************//**
* \brief Bind a texture to a texture unit. 
* 
* \author  gernot
* \date    2018-06-12
* \version 1.0
**********************************************************************/
bool CTextureLoader::BindTexture( 
  unsigned int target,         //!< I - in: target for conventional (old) interface
  unsigned int texture_object, //!< I - in: named texture object for direct state access
  size_t       binding_id )    //!< I - in: texture unit
{
  if ( _dsa )
  {
    glBindTextureUnit((GLuint)binding_id, texture_object);
  }
  else if ( glActiveTexture != nullptr )
  {
    // Bind the default texture to the texture unit.
    // In common there should be no necessity of this and there should not be any reason to do this.
    glActiveTexture( (GLenum)(GL_TEXTURE0 + binding_id) );
    glBindTexture( target, texture_object );
    if ( binding_id != 0 && texture_object == 0 )
      glActiveTexture( GL_TEXTURE0 );
  }
  else
  {
    glBindTexture( target, texture_object );
  }
  return true;
}


/******************************************************************//**
* \brief Bind a texture to a texture unit.  
* 
* \author  gernot
* \date    2018-06-12
* \version 1.0
**********************************************************************/
bool CTextureLoader::SetTextureParameterI( 
  unsigned int target,         //!< in: target for conventional (old) interface 
  unsigned int texture_object, //!< in: named texture object for direct state access 
  unsigned int parameter,      //!< in: the named parameter
  int          value )         //!< in: the new value
{
  if ( _dsa )
  {
    glTextureParameteri( texture_object, parameter, value );
  }
  else
  {
    glTexParameteri( target, parameter, value );
  }
  return true;
}


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
* \brief Map texture format to OpenGL internal format enumerator
* constant. 
* 
* \author  gernot
* \date    2018-06-12
* \version 1.0
**********************************************************************/
unsigned int CTextureLoader::InternalFormat( 
  Render::TTextureFormat format ) //!< in: requested texture format
{
  static const std::unordered_map< Render::TTextureFormat, GLenum > internal_foramt_map
  {
    { Render::TTextureFormat::R8,          GL_R8          },
    { Render::TTextureFormat::RG8,         GL_RG8         },
    { Render::TTextureFormat::RGB8,        GL_RGB8        }, 
    { Render::TTextureFormat::RGBA8,       GL_RGBA8       },
    { Render::TTextureFormat::RGB8_SNORM,  GL_RGB8_SNORM  },
    { Render::TTextureFormat::RGB16_SNORM, GL_RGBA8_SNORM } 
  };

  auto it = internal_foramt_map.find(format);
  ASSERT(it != internal_foramt_map.end());
  return it != internal_foramt_map.end() ? it->second : GL_RGBA8;
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
  unsigned int                      texture_object, //!< in: named texture object for direct state access
  const Render::TTextureParameters &parameter )     //!< in: texture parameter
{
  GLenum target = TargetType( parameter._type );

  GLenum minifying     = CTextureLoader::Minifying( parameter._filter, parameter._max_mipmap );
  GLenum magnification = CTextureLoader::Magnification( parameter._filter, parameter._max_mipmap );

  auto wrap = CTextureLoader::Wrap( parameter );

  //glTextureParameteri()

  SetTextureParameterI( target, texture_object, GL_TEXTURE_WRAP_S, wrap[0] );
  if ( parameter.Is2DType() || parameter.Is3DType() )
    SetTextureParameterI( target, texture_object, GL_TEXTURE_WRAP_T, wrap[1] );
  if ( parameter.Is3DType() )
    SetTextureParameterI( target, texture_object, GL_TEXTURE_WRAP_T, wrap[2] );
  
  SetTextureParameterI( target, texture_object, GL_TEXTURE_MIN_FILTER, minifying );
  SetTextureParameterI( target, texture_object, GL_TEXTURE_MAG_FILTER, magnification );

  // set mipmaps parameters
  if ( parameter._max_mipmap > 1 )
  {
    SetTextureParameterI( target, texture_object, GL_TEXTURE_BASE_LEVEL, 0 );
    SetTextureParameterI( target, texture_object, GL_TEXTURE_MAX_LEVEL, (GLint)parameter._max_mipmap );

    // [`EXT_texture_filter_anisotropic`](https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_texture_filter_anisotropic.txt)
    GLint max_anisotropic = std::min((GLint)_max_anisotripic_filter, (GLint)parameter._anisotropic);
    if ( max_anisotropic > 1 )
      SetTextureParameterI( target, texture_object, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropic); 
  }

  return true;
}

 
/******************************************************************//**
* \brief Generate mipmap according to the parameters.
* 
* \author  gernot
* \date    2018-06-12
* \version 1.0
**********************************************************************/
bool CTextureLoader::GenerateMipmaps( 
  unsigned int                      texture_object, //!< in: named texture object for direct state access
  const Render::TTextureParameters &parameter )     //!< in: texture parameter
{
  if ( parameter._max_mipmap <= 1 )
    return false;

  //! [`glGenerateMipmap`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGenerateMipmap.xhtml)

  if ( _dsa )
  {
    glGenerateTextureMipmap( texture_object );
  }
  else
  {
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

  // create and bind texture
  Render::TTexturePtr texture = std::make_unique<CTextureInternal>( parameter._type );
  if ( _dsa == false )
    texture->Bind( _loader_binding_id );

  // calculate texture levels
  GLint levels = 1;
  if ( parameter._max_mipmap > 1 )
  {
    size_t n = std::max( size[0], size[1] );
    double p = std::ceil(std::log2( (double)n ));
    levels   = (GLint)p;
  }

  // load texture image
  GLenum internal_format = InternalFormat( parameter._format );
  if ( _dsa )
  {
    glTextureStorage2D( (GLuint)texture->ObjectHandle(), 1, internal_format, (GLsizei)size[0], (GLsizei)size[1] );
  }
  else if ( glTexStorage2D != nullptr )
  {
    // for texture level size and allocation see
    // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml
    glTexStorage2D( target, levels, internal_format, (GLsizei)size[0], (GLsizei)size[1] );
  }
  else
  {
    static const std::unordered_map< Render::TTextureFormat, std::tuple<GLenum, GLenum> > compatible_foramt_map
    {
      { Render::TTextureFormat::R8,          { GL_RED,  GL_BYTE  } },
      { Render::TTextureFormat::RG8,         { GL_RG,   GL_BYTE  } },
      { Render::TTextureFormat::RGB8,        { GL_RGB,  GL_BYTE  } }, 
      { Render::TTextureFormat::RGBA8,       { GL_RGBA, GL_BYTE  } },
      { Render::TTextureFormat::RGB8_SNORM,  { GL_RGB,  GL_FLOAT } },
      { Render::TTextureFormat::RGB16_SNORM, { GL_RGBA, GL_FLOAT } } 
    };

    auto it = compatible_foramt_map.find(parameter._format);
    ASSERT( it != compatible_foramt_map.end() );
    GLenum format = it != compatible_foramt_map.end() ? std::get<0>( it->second ) : GL_RGBA;
    GLenum type   = it != compatible_foramt_map.end() ? std::get<1>( it->second ) : GL_BYTE;
    glTexImage2D( target, 0, GL_RGBA, (GLsizei)size[0], (GLsizei)size[1], 0, format, type, nullptr );
  }

  // set the texture paramters
  SetTextureParameter( (GLuint)texture->ObjectHandle(), parameter );

  // genrate the mipmaps
  GenerateMipmaps( (GLuint)texture->ObjectHandle(), parameter );

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
  texture->Bind( _loader_binding_id );

  // TODO $$$

  // GL_BGR, GL_BGRA

  // TODO $$$

  SetTextureParameter( (GLuint)texture->ObjectHandle(), parameter ); 
  GenerateMipmaps( (GLuint)texture->ObjectHandle(), parameter );

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