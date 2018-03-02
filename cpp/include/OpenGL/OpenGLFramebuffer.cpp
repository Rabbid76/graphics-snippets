/******************************************************************//**
* \brief   Implementation for rendering passes with OpenGL frame
*          buffers.
* 
* \author  gernot
* \date    2018-02-04
* \version 1.0
**********************************************************************/

// includes

#include <stdafx.h>

#include <OpenGLFramebuffer.h>

#include <algorithm>
#include <iostream>

#include <GL/glew.h>
//#include <GL/gl.h> not necessary because of glew 
#include <GL/glu.h>

// stl

#include <cassert>


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

#if !defined(__FRAMBUFFER_DEBUG_ERROR_CHECK__)
//#define __FRAMBUFFER_DEBUG_ERROR_CHECK__
#endif

#if defined(_DEBUG) && defined(__FRAMBUFFER_DEBUG_ERROR_CHECK__)
#define TEST_GL_ERROR ErrorCheck();
#else
#define TEST_GL_ERROR
#endif

// class implementations


/******************************************************************//**
* \brief General OpenGL namespace
**********************************************************************/
namespace OpenGL
{


GLenum ErrorCheck( void )
{
  GLenum err = glGetError();
  if ( err != GL_NO_ERROR )
    DebugWarning << "OpenGL ERROR" << err;
  return err;
}



//*********************************************************************
// CRenderProcess
//*********************************************************************


/******************************************************************//**
* \brief   default ctor.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
CRenderProcess::CRenderProcess( void )
{}


/******************************************************************//**
* \brief   dtor
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
CRenderProcess::~CRenderProcess()
{
  Destruct();
}


/******************************************************************//**
* \brief   Invalidate, force renew of buffers and passes.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
void CRenderProcess::Invalidate( void )
{
  _valid = _complete = false;
}


/******************************************************************//**
* \brief   Destroy the buffer opjects and the passes objects.
* Destroy textue and framebuffer objects.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
void CRenderProcess::Destruct( void ) 
{
  // destroy frambuffer objects (GPU)
  std::vector<unsigned int> delFbs;
  for ( auto & fbs : _fbs )
  {
    if ( fbs.second._object != 0 )
      delFbs.push_back( fbs.second._object );
  }
  glDeleteFramebuffers( (GLsizei)delFbs.size(), delFbs.data() );
  TEST_GL_ERROR

  // destroy texture opbjects (GPU)
  std::vector<unsigned int> delTex;
  for ( auto & tex : _textures )
  {
    if ( tex.second._object != 0 &&  tex.second._extern == false )
      delTex.push_back( tex.second._object );
  }
  glDeleteTextures( (GLsizei)delTex.size(), delTex.data() );
  TEST_GL_ERROR

  Invalidate();
  _buffers.clear();
  _scales.clear();
  _passScales.clear();
  _textures.clear();
  _fbs.clear();
}


/******************************************************************//**
* \brief  Delete the GPU texture objects, which are not required
*         anymore.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
void CRenderProcess::DeleteUnnecessaryTextures( void )
{
  // collect all buffer/texture IDs and GPU texture objects which have to be deleted
  std::vector<size_t>       texDelIDs;
  std::vector<unsigned int> texDelObjects;
  for ( auto & tex : _textures )
  {
    bool texRequired = false;
    for ( auto & pass : _passes )
    {
      size_t         passId   = pass.first;  // name of the pass
      Render::TPass &passSpec = pass.second; // specification of the pass
      
      // search the buffer/texture ID in the list of the pass targets
      auto tragetIt = std::find_if( passSpec._targets.begin(), passSpec._targets.end(), [tex]( const auto & target ) -> bool {
        return tex.first == target._bufferID;
      } );
      texRequired   = tragetIt != passSpec._targets.end();
      if ( texRequired )
        break;
    }
    if ( texRequired )
      continue;

    // note thes textures which have to be deleted (GPU)
    if ( tex.second._extern == false)
      texDelObjects.emplace_back(tex.second._object);

    // note the texture to be removed from the target textures
    texDelIDs.emplace_back(tex.first);
  }

  // erase texture objects from map
  for ( auto texId : texDelIDs )
    _textures.erase( texId );

  // delte texture GPU objects
  if ( texDelObjects.empty() == false )
  {
    glDeleteTextures( (GLsizei)texDelObjects.size(), texDelObjects.data() );
    TEST_GL_ERROR
  }
}


/******************************************************************//**
* \brief  Delete the GPU framebuffer objects, which are not required
*         anymore.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
void CRenderProcess::DeleteUnnecessaryFrambuffers( void )
{
  // collect all pass/frambuffer IDs and GPU framebuffer objects which have to be deleted
  std::vector<size_t>       fbDelIDs;
  std::vector<unsigned int> fbDelObjects;
  for ( auto & fb : _fbs )
  {
    // Search the pass/frambuffer ID in the list of the passes.
    // Note, a frambuffer is only requred for a pass with target specification else the default buffer is used.
    auto passIt = _passes.find( fb.first );
    bool fbRequired = passIt != _passes.end() && passIt->second._targets.empty() == false;

    if ( fbRequired == false )
    {
      fbDelIDs.emplace_back( fb.first );
      fbDelObjects.emplace_back( fb.second._object );
    }
  }

  // erase frambuffer objects from map
  for ( auto fbID : fbDelIDs )
    _fbs.erase( fbID );

  // delte frambuffer GPU objects
  if ( fbDelObjects.empty() == false )
  {
    glDeleteFramebuffers( (GLsizei)fbDelObjects.size(), fbDelObjects.data() );
    TEST_GL_ERROR
  }
}


/******************************************************************//**
* \brief   Get the internal buffer format for a buffer type and the 
*  internal buffer component format.
*
* return values, see [`glTexImage2D `](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml):
*    internal format, format, type
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
std::array<unsigned int, 3> CRenderProcess::InternalFormat(
  Render::TBufferType     buffer,  //!< in: type of the buffer
  Render::TBufferDataType format ) //!< in: interrnal format
{
  static const std::array<unsigned int, 3> return_fail{ 0, 0, 0 };

  constexpr const int TYPES = 8;
  static const std::array<Render::TBufferType, TYPES> bufferTypes{
    Render::TBufferType::COLOR1, 
    Render::TBufferType::COLOR2,
    Render::TBufferType::COLOR3,
    Render::TBufferType::COLOR4,
    Render::TBufferType::DEPTH,
    Render::TBufferType::STENCIL,
    Render::TBufferType::DEPTHSTENCIL
  };

  constexpr const int FORMATS = 13;
  static const std::array<Render::TBufferDataType, FORMATS> formatTypes{
    Render::TBufferDataType::DEFAULT,
    Render::TBufferDataType::UINT8,
    Render::TBufferDataType::SNORM8,
    Render::TBufferDataType::SNORM16,
    Render::TBufferDataType::F16,
    Render::TBufferDataType::F32,
    Render::TBufferDataType::DEPTH16,
    Render::TBufferDataType::DEPTH24,
    Render::TBufferDataType::DEPTH32,
    Render::TBufferDataType::DEPTH32F,
    Render::TBufferDataType::STENCIL8,
    Render::TBufferDataType::DEPTH_STENCIL_24_8,
    Render::TBufferDataType::DEPTH_STENCIL_32F_8
  };

  // Internal format mapping
  // [Image Format](https://www.khronos.org/opengl/wiki/Image_Format)
  static const unsigned int internalFormats[FORMATS][TYPES]
  {
    //                          COLOR1         COLOR2         COLOR3          COLOR4           DEPTH                  STENCIL              STENCIL DEPTHSTENCIL
    /* DEFAULT             */  { GL_RED,       GL_RG,         GL_RGB,         GL_RGBA,         GL_DEPTH_COMPONENT,    GL_STENCIL_INDEX8,   GL_DEPTH24_STENCIL8 },
    /* UINT8               */  { GL_R8,        GL_RG8,        GL_RGB8,        GL_RGBA8,        0,                     0,                   0 },
    /* SNORM8              */  { GL_R8_SNORM,  GL_RG8_SNORM,  GL_RGB8_SNORM,  GL_RGBA8_SNORM,  0,                     0,                   0 },
    /* SNORM16             */  { GL_R16_SNORM, GL_RG16_SNORM, GL_RGB16_SNORM, GL_RGBA16_SNORM, 0,                     0,                   0 },
    /* F16                 */  { GL_R16F,      GL_RG16F,      GL_RGB16F,      GL_RGBA16F,      0,                     0,                   0 },
    /* F32                 */  { GL_R32F,      GL_RG32F,      GL_RGB32F,      GL_RGBA32F,      0,                     0,                   0 },
    /* DEPTH16             */  { 0,            0,             0,              0,               GL_DEPTH_COMPONENT16,  0,                   0 },
    /* DEPTH24             */  { 0,            0,             0,              0,               GL_DEPTH_COMPONENT24,  0,                   GL_DEPTH24_STENCIL8 }, 
    /* DEPTH32             */  { 0,            0,             0,              0,               GL_DEPTH_COMPONENT32,  0,                   0 },
    /* DEPTH32F            */  { 0,            0,             0,              0,               GL_DEPTH_COMPONENT32F, 0,                   GL_DEPTH32F_STENCIL8 }, 
    /* STENCIL8            */  { 0,            0,             0,              0,               0,                     GL_STENCIL_INDEX8,   0 },
    /* DEPTH_STENCIL_24_8  */  { 0,            0,             0,              0,               0,                     0,                   GL_DEPTH24_STENCIL8 }, 
    /* DEPTH_STENCIL_32F_8 */  { 0,            0,             0,              0,               0,                     0,                   GL_DEPTH32F_STENCIL8 } 
  };

  Render::TBufferType useBuffer = buffer == Render::TBufferType::DEFAULT ? Render::TBufferType::COLOR4 : buffer;

  // check buffer type
  auto bufferIt = std::find( bufferTypes.begin(), bufferTypes.end(), useBuffer );
  if ( bufferIt == bufferTypes.end() )
  {
    DebugWarning << "illegal buffer type " << (int)buffer;
    return return_fail;
  }

  // check format
  auto formatIt = std::find( formatTypes.begin(), formatTypes.end(), format );
  if ( formatIt == formatTypes.end() )
  {
    DebugWarning << "illegal format " << (int)format;
    return return_fail;
  }

  // get the internal format
  size_t bufferInx      = bufferIt - bufferTypes.begin();
  size_t formatInx      = formatIt - formatTypes.begin();
  auto   internalFormat = internalFormats[formatInx][bufferInx];
  if ( internalFormat == 0 )
    DebugWarning << "illegal buffer type and format combination " << (int)useBuffer << ", " << (int)format; 

  // get the format
  static const unsigned int textureformats[TYPES] {
    GL_RED, GL_RG, GL_RGB, GL_RGBA, GL_DEPTH_COMPONENT, GL_STENCIL_INDEX, GL_DEPTH_STENCIL 
  };
  unsigned int textureFormat = textureformats[bufferInx];

  // get the data type
  unsigned int dataType = 0;
  switch ( useBuffer )
  {
    case Render::TBufferType::DEPTH:   dataType = GL_FLOAT; break;
    case Render::TBufferType::STENCIL: dataType = GL_UNSIGNED_BYTE; break;
    
    case Render::TBufferType::DEPTHSTENCIL: 
      if ( internalFormat == GL_DEPTH24_STENCIL8 )       dataType = GL_UNSIGNED_INT_24_8;	
      else if ( internalFormat == GL_DEPTH32F_STENCIL8 ) dataType = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
      else                                               dataType = GL_FLOAT;
      break;
    
    default:
      dataType = ( format == Render::TBufferDataType::DEFAULT || format == Render::TBufferDataType::UINT8 ) ? GL_UNSIGNED_BYTE : GL_FLOAT;
      break;
  } 
  
  return { internalFormat, textureFormat, dataType };
}


/******************************************************************//**
* \brief   Clear all render buffer specifications.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
void CRenderProcess::CRenderProcess::ClearBuffers( void )
{
  Invalidate();
  _buffers.clear();
}


/******************************************************************//**
* \brief   Clear a render buffer specification.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
void CRenderProcess::CRenderProcess::ClearBuffer( 
  size_t bufferID ) //!< in: name of the buffer
{
  auto it = _buffers.find( bufferID );
  if ( it == _buffers.end() )
    return;
  Invalidate();
  _buffers.erase( it );
}


/******************************************************************//**
* \brief   Specify a single render buffer.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
bool CRenderProcess::SpecifyBuffer( 
  size_t                 bufferID,       //!< in: name of the buffer 
  const Render::TBuffer &specification ) //!< in: buffer specification
{
  auto it = _buffers.find( bufferID );
  if ( it != _buffers.end() && it->second == specification )
    return true;
  Invalidate();
  _buffers[bufferID] = specification;
  return true;
}


/******************************************************************//**
* \brief   Specify all render pass buffers.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
bool CRenderProcess::SpecifyBuffers( 
  const TBufferMap &buffers ) //!< in new buffer map
{
  // find the buffers which are not further required
  std::vector<size_t> unnecessaryBuffers;
  for ( auto &buffer : _buffers )
  {
    auto it = buffers.find( buffer.first );
    if ( it == buffers.end() )
      unnecessaryBuffers.emplace_back( buffer.first );
  }

  // clear the deprecated buffer specifications
  for ( auto bufferID : unnecessaryBuffers )
    ClearBuffer( bufferID );

  // update or add the new buffer specifications
  bool valid = true;
  for ( auto & buffer : buffers )
  {
    if ( SpecifyBuffer( buffer.first, buffer.second ) == false )
      valid = false;
  }
  return valid;
}


/******************************************************************//**
* \brief   Clear all render passes specifications.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
void CRenderProcess::ClearPasses( void )
{
  Invalidate();
  _passes.clear();
}


/******************************************************************//**
* \brief   Clear a render pass specification.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
void CRenderProcess::ClearPass( 
  size_t passID ) //!< in: name of the pass
{
  auto it = _passes.find( passID );
  if ( it == _passes.end() )
    return;
  _valid = _complete = false;
  _passes.erase( it );
}


/******************************************************************//**
* \brief   Specify a single render pass.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
bool CRenderProcess::SpecifyPass( 
  size_t               passID,         //!< in: name of the render pass
  const Render::TPass &specification ) //!< in: specification of the render pass
{
  auto it = _passes.find( passID );
  if ( it != _passes.end() && it->second == specification )
    return true;
  Invalidate();
  _passes[passID] = specification;
  return true;
}


/******************************************************************//**
* \brief   Specify all render passes.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
bool CRenderProcess::SpecifyPasses( 
  const TPassMap &passes ) //!< in: new pass map
{
  // find the passes which are not further required
  std::vector<size_t> unnecessaryPasses;
  for ( auto &pass : _passes )
  {
    auto it = passes.find( pass.first );
    if ( it == passes.end() )
      unnecessaryPasses.emplace_back( pass.first );
  }

  // clear the deprecated pass specifications
  for ( auto passID : unnecessaryPasses )
    ClearPass( passID );

  // update or add the new buffer specifications
  bool valid = true;
  for ( auto & pass : passes )
  {
    if ( SpecifyPass( pass.first, pass.second ) == false )
      valid = false;
  }
  return valid;
}


/******************************************************************//**
* \brief   Validate the specifcations.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
bool CRenderProcess::Validate( void )
{
  if ( _valid )
    return true;

  _valid = true;
  _scales.clear();
  _passScales.clear();
  _bufferInfoMap.clear();
  
  // for each pass
  for ( auto & pass : _passes )
  {
    size_t         passId   = pass.first;  // name of the pass
    Render::TPass &passSpec = pass.second; // specification of the pass

    // calclate the pass scale 
    // (all OpenGL color attachments to farmebuffer have to have the same scale to achieve frambuffbuffer completness)
    std::array<float, 2> scale_range{ 1.0e10, 0.0 };
    for ( auto & target : passSpec._targets )
    {
      auto bufferIt = _buffers.find( target._bufferID );
      if ( bufferIt == _buffers.end() )
      {
        Invalidate();
        DebugWarning << "buffer" << target._bufferID << " is not specified";
        continue;
      }
      Render::TBuffer &bufferSpec = bufferIt->second;

      scale_range = {
        std::min( scale_range[0], bufferSpec._scale ),
        std::max( scale_range[1], bufferSpec._scale )
      };
    }
    float scale = ( scale_range[0] < scale_range[1] + 0.001 ) ? scale_range[0] : 1.0f;
    _passScales[passId] = scale;
    bool any_layered = false;
    for ( auto & target : passSpec._targets )
    {
      auto bufferIt = _buffers.find( target._bufferID );
      if ( bufferIt == _buffers.end() )
        continue;

      // if the target buffers have different scales then set the scale to 1.0
      if ( _scales.find(target._bufferID) != _scales.end() )
      {
        if ( fabs(_scales[target._bufferID] - scale ) > 0.0001 )
        {
          Invalidate();
          DebugWarning << "buffer scale missmatch (" << _scales[target._bufferID] << " <-> " << scale << ") for buffer " << target._bufferID << " in pass " << passId;
        }
      }
      else
      {
        _scales[target._bufferID] = scale;
      }

      if ( IsLayered( bufferIt->second._layers ) )
        any_layered = true;
    }

    if ( any_layered )
    {
      for ( auto & target : passSpec._targets )
      {
        auto bufferIt = _buffers.find( target._bufferID );
        if ( bufferIt == _buffers.end() )
        { 
          // if ther is no buffer specification, then a render buffer is used, but render buffer cannot be layered
          Invalidate();
          DebugWarning << "buffer layer missmatch (" << target._bufferID << ") in pass " << passId;
        }

        // See [Framebuffer Completeness - Completeness Rules](https://www.khronos.org/opengl/wiki/Framebuffer_Object#Completeness_Rules)
        // If a layered image is attached to one attachment, then all attachments must be layered attachments.
        // The attached layers do not have to have the same number of layers,
        // nor do the layers have to come from the same kind of texture (a cubemap color texture can be paired with an array depth texture). 
        bool is_layerd = IsLayered( bufferIt->second._layers );
        if ( is_layerd == false )
        { 
          Invalidate();
          DebugWarning << "buffer layer missmatch (" << target._bufferID << ") in pass " << passId;
        }
      }
    }
  }

  // for each buffer
  bool is_layered = _buffers.empty() ? false : _buffers[0]._layers > 0;
  for ( auto buffer : _buffers )
  {
    size_t         bufferID     = buffer.first;  //!< name of the buffer
    Render::TBuffer &bufferSpec = buffer.second; //!< specification of the buffer
    
    auto bufferFormat = InternalFormat( bufferSpec._type, bufferSpec._format );
    if ( bufferFormat[0] == 0 || bufferFormat[1] == 0 || bufferFormat[2] == 0 )
      Invalidate();
  }
  
  return _valid;
}


/******************************************************************//**
* \brief   Update a buffer texture acoording to the buffer specification. 
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
void CRenderProcess::UpdateTexture( 
  size_t                 bufferID,       //!< in: the name of the buffer
  const Render::TBuffer &specification ) //!< in: the buffer specification
{
  // When in the following comments is talked about an "external" texture, the a texture is meant,
  // that is not managed in this class, but was handed over for use.

  TTextureObject newTexture; //!< new texture object for the buffer

  // get texture size
  newTexture._size = { (size_t)(_size[0] * _scales[bufferID] + 0.5f), (size_t)(_size[1] * _scales[bufferID] + 0.5f) };

  // get the buffer format
  newTexture._format = InternalFormat( specification._type, specification._format );

  // set the layers
  newTexture._layers = specification._layers;

  // set the the extern textue object
  newTexture._extern = false; // TODO $$$ extern textures
  newTexture._object = 0;     // TODO $$$ newTexture._extern ? specification_extern_object : 0;

  // find the existing texture object 
  auto texIt = _textures.find( bufferID );

  // if the texture is an extern texture, then it is sufficient to update the texture format information
  if ( newTexture._extern )
  {
    // delte the "old" texture if it was not an external texture
    if ( texIt != _textures.end() && texIt->second._extern == false )
    {
      glDeleteTextures( 1, &texIt->second._object );
      TEST_GL_ERROR
    }
    
    // update texture information
    texIt->second = newTexture;
    return;
  }

  // check if the textue object exists and has a proper format and size
  if ( texIt != _textures.end() )
  {
    // If an identical and proper object exists, nothing more has to done.
    if ( texIt->second == newTexture )
      return;   
    
    // Delete the "old" texture object, of course if it is not extern.
    if ( texIt->second._extern == false )
    {
      glDeleteTextures( 1, &texIt->second._object );
      TEST_GL_ERROR
    }
  }

  // create the new texture
  glGenTextures( 1, &newTexture._object ); TEST_GL_ERROR

  // setup the texture size and the format
  if ( IsLayered( newTexture._layers ) == false )
  {
    glBindTexture( GL_TEXTURE_2D, newTexture._object ); TEST_GL_ERROR
    glTexImage2D( GL_TEXTURE_2D, 0, (GLint)newTexture._format[0],
      (GLsizei)newTexture._size[0], (GLsizei)newTexture._size[1],
      0, (GLenum)newTexture._format[1], (GLenum)newTexture._format[2], nullptr ); TEST_GL_ERROR
  }
  else
  {
    glBindTexture( GL_TEXTURE_2D_ARRAY, newTexture._object ); TEST_GL_ERROR
    glTexImage3D( GL_TEXTURE_2D_ARRAY, 0, (GLint)newTexture._format[0],
      (GLsizei)newTexture._size[0], (GLsizei)newTexture._size[1], (GLsizei)newTexture._layers,
      0, (GLenum)newTexture._format[1], (GLenum)newTexture._format[2], nullptr ); TEST_GL_ERROR
  }
  
  // TODO $$$ GL_NEAREST blur buffer ?
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); TEST_GL_ERROR
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); TEST_GL_ERROR
  
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ); TEST_GL_ERROR
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ); TEST_GL_ERROR
  if ( IsLayered( newTexture._layers ) )
  {
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
    TEST_GL_ERROR
  }

  // add the texture to the texture map
  _textures[bufferID] = newTexture;
}


/******************************************************************//**
* \brief   Create a render buffer according to the target specification
* and attach the render buffer to the framebuffer attachment point,
* which is specified in the target specification.
* The frmabuffer has to be bound.
* The name of the new renderbuffer object (GPU) is returned.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
unsigned int CRenderProcess::CreateFrambufferRenderBuffer( 
  const TFramebufferObject     &fb,      //!< in: frambuffer specification 
  const Render::TPass::TTarget &target ) //!< in: the target specification
{
  if ( target._bufferID != Render::TPass::TTarget::no )
  {
    assert( false );
    return 0;
  }

  // create render buffer object
  unsigned int rbo;
  glGenRenderbuffers( 1, &rbo ); TEST_GL_ERROR
  glBindRenderbuffer( GL_RENDERBUFFER, rbo ); TEST_GL_ERROR

  GLenum                      attachType = 0;
  std::array<unsigned int, 3> format{ 0, 0, 0 };
  if ( target._attachment == Render::TPass::TTarget::depth ||
       target._attachment == Render::TPass::TTarget::stencil ||
       target._attachment == Render::TPass::TTarget::depth_stencil )
  {
    // depth/stencil attachmnet 

    // type of the attachment
    if ( target._attachment == Render::TPass::TTarget::depth )
      attachType = GL_DEPTH_ATTACHMENT;
    else if ( target._attachment == Render::TPass::TTarget::stencil )
      attachType = GL_STENCIL_ATTACHMENT;
    else // if ( target._location == Render::TPass::TTarget::depth_stencil )
      attachType = GL_DEPTH_STENCIL_ATTACHMENT;

    // format of the render buffer
    Render::TBufferType bufferType = Render::TBufferType::DEPTH;
    if ( target._bufferID == Render::TPass::TTarget::stencil )
      bufferType = Render::TBufferType::STENCIL;
    else if ( target._bufferID == Render::TPass::TTarget::depth_stencil )
      bufferType = Render::TBufferType::DEPTHSTENCIL;
    format = CRenderProcess::InternalFormat( bufferType, Render::TBufferDataType::DEFAULT );
  }
  else
  {
    // color attachmnet

    // type of the attachment
    GLenum attachType = (GLenum)(GL_COLOR_ATTACHMENT0 + target._attachment);

    // add render buffer attachment

    // format of the render buffer
    // TODO $$$ special render buffer format
    format = CRenderProcess::InternalFormat( Render::TBufferType::DEFAULT, Render::TBufferDataType::DEFAULT );
  }

  // specify the format of the render buffer
  glRenderbufferStorage( GL_RENDERBUFFER, format[0], (GLsizei)fb._size[0], (GLsizei)fb._size[0] ); TEST_GL_ERROR

  // unbind the render buffer
  glBindRenderbuffer( GL_RENDERBUFFER, 0 ); TEST_GL_ERROR
  
  // attach render buffer to frambuffer 
  glFramebufferRenderbuffer( GL_FRAMEBUFFER, attachType, GL_RENDERBUFFER, rbo ); TEST_GL_ERROR

  return rbo;
}


/******************************************************************//**
* \brief   Attach a texture to the framebuffer attachmnet point,
* which is specified in the target specification.
* The textue has to exist and frmabuffer has to be bound.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
void CRenderProcess::AttachFrambufferTextureBuffer( 
  const TFramebufferObject     &fb,      //!< in: frambuffer specification 
  const Render::TPass::TTarget &target ) //!< in: the target specification
{
  if ( target._bufferID == Render::TPass::TTarget::no )
  {
    assert( false );
    return;
  }

  GLenum attachType = 0;
  switch ( target._attachment )
  {
    case Render::TPass::TTarget::depth:         attachType = GL_DEPTH_ATTACHMENT; break;
    case Render::TPass::TTarget::stencil:       attachType = GL_STENCIL_ATTACHMENT; break;
    case Render::TPass::TTarget::depth_stencil: attachType = GL_DEPTH_STENCIL_ATTACHMENT; break;
    default:                                    attachType = (GLenum)(GL_COLOR_ATTACHMENT0 + target._attachment); break;

  }

  
  // add texture attachmnet
  if ( IsLayered( _textures[target._bufferID]._layers ) == false )
  {
    glFramebufferTexture2D( GL_FRAMEBUFFER, attachType, GL_TEXTURE_2D, _textures[target._bufferID]._object, 0 );
    TEST_GL_ERROR
  }
  else
  {
    // layerd texture attachment
    glFramebufferTexture( GL_FRAMEBUFFER, attachType, _textures[target._bufferID]._object, 0 );
    TEST_GL_ERROR
  }
  // TODO $$$ _textures[target._bufferID]._layers == 1 : glFramebufferTexture3D ???
      
  // TODO cubemaps: 
  //   GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 
  //   GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
  //   GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
  //   e.g.: glFramebufferTexture2D( GL_FRAMEBUFFER, attachType, GL_TEXTURE_CUBE_MAP_POSITIVE_X, _textures[target._bufferID]._object, 0 );

}


/******************************************************************//**
* \brief   Prepare a render pass.
* 
* Note, renderbuffers are deleted immediatly after they are attached to
* the frambuffer, because:
* 
* See:
* [OpenGL 4.6 API core profile specification, Chapter 5.1.3 Deleted Object and Object Name Lifetimes, page 54](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf)<br/>
* [OpenGL ES Specification 3.2, Chapter 5.1.3 Deleted Object and Object Name Lifetimes, page 45](https://www.khronos.org/registry/OpenGL/specs/es/3.2/es_spec_3.2.pdf)<br/>
* [Draw call succeeds even VBO is deleted](https://stackoverflow.com/questions/47289353/webgl-2-0-draw-call-succeeds-even-vbo-is-deleted/47290706#47290706)
*
* > When a buffer, texture, sampler, renderbuffer, query, or sync object is deleted, its
* > name immediately becomes invalid (e.g. is marked unused), but the underlying
* > object will not be deleted until it is no longer in use.
*
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
bool CRenderProcess::Create( 
  TViewportSize size ) //!< in: size of the viewport (size of the framebuffer targets, if the scale is 1.0)
{
  if ( IsValid() == false && Validate() == false )
    return false;
  if ( _complete && _size == size )
    return true; 
  _size = size;
  _complete = true;
  _bufferInfoMap.clear();

  // delete all frambuffer objects which are not required anymore
  DeleteUnnecessaryTextures();

  // delete all texture objects which are not required anymore
  DeleteUnnecessaryFrambuffers();

  // for each buffer
  if ( _buffers.empty() == false )
  {
    glActiveTexture( GL_TEXTURE0 );
    TEST_GL_ERROR
  }
  for ( auto & buffer : _buffers )
    UpdateTexture( buffer.first, buffer.second );

  // unbind any textures
  if ( _buffers.empty() == false )
  {
    glBindTexture( GL_TEXTURE_2D, 0 );
    TEST_GL_ERROR
  }

  // for each pass
  for ( auto & pass : _passes )
  {
    size_t              passID   = pass.first;  // name of the pass
    Render::TPass      &passSpec = pass.second; // specification of the pass
    TFramebufferObject  newFb;                  // new frambuffer object for the render pass

    // get renderbuffer size
    newFb._size = { (size_t)(_size[0] * _passScales[passID] + 0.5f), (size_t)(_size[1] * _passScales[passID] + 0.5f) };

    // $$$ 
    // TODO : check if the frambuffer has changed 
    // current implementation (workaround): delete the framebuffer in any case fo the moment
    auto fbIt = _fbs.find( passID );
    if ( fbIt != _fbs.end() )
    {
      glDeleteFramebuffers( 1, &fbIt->second._object );
      TEST_GL_ERROR
    }

    // Check if a new frambuffer has to be created
    // Note, a frambuffer is only requred for a pass with target specification  else the default buffer is used.
    if ( passSpec._targets.empty() )
      continue;
    
    // create the new buffer object
    glGenFramebuffers( 1, &newFb._object ); TEST_GL_ERROR
    glBindFramebuffer( GL_FRAMEBUFFER, newFb._object ); TEST_GL_ERROR
    _fbs[passID] = newFb;

    // for each frambuffer target
    std::vector<unsigned int> preDelRenderBufObjs;
    bool has_depth   = false;
    bool has_stencil = false;
    for ( auto target : passSpec._targets )
    {
      // check if there is any deth or stencil attachmnet
      if ( target._attachment == Render::TPass::TTarget::depth || target._attachment == Render::TPass::TTarget::depth_stencil )
        has_depth = true;
      if ( target._attachment == Render::TPass::TTarget::stencil || target._attachment == Render::TPass::TTarget::depth_stencil )
        has_stencil = true;

      if ( target._bufferID == Render::TPass::TTarget::no ) 
      {
        // If there is no textue buffer specified, the create a render buffer
        unsigned int rbo = CreateFrambufferRenderBuffer( newFb, target );
        if ( rbo != 0 )
          preDelRenderBufObjs.emplace_back( rbo ); // attach it to the renderbuffer to the pre deleted objects
      }
      else
      {
        // Attach the existing texureto the frambuffer
        AttachFrambufferTextureBuffer( newFb, target );
      }
    }

    // disable depth and stencil attachment
    if ( has_depth == false && has_stencil == false )
    {
      glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0 );
      TEST_GL_ERROR
    }
    else if ( has_depth == false )
    {
      glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0 );
      TEST_GL_ERROR
    }
    else if ( has_stencil == false )
    {
      glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0 );
      TEST_GL_ERROR
    }

    // check for frame buffer completness
    // See:
    // - [Framebuffer Completeness - Completeness Rules](https://www.khronos.org/opengl/wiki/Framebuffer_Object#Completeness_Rules)
    // - [OpenGL 4.6 API core profile specification; 9.4.2 Whole Framebuffer Completeness; page 325](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf)
    GLenum fb_status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
    if ( fb_status != GL_FRAMEBUFFER_COMPLETE )
    {
      _complete = false;
      assert( false );
    }

    // unbind the framebuffer before pre deleting the render buffers
    glBindFramebuffer( GL_FRAMEBUFFER, 0 ); TEST_GL_ERROR

    if ( preDelRenderBufObjs.empty() == false )
    {
      // pre delete the render buffer objects (see comment block of this method)
      glDeleteRenderbuffers( (GLsizei)preDelRenderBufObjs.size(), preDelRenderBufObjs.data() );
      TEST_GL_ERROR
    }
  }

  // unbind any framebuffers
  if ( _passes.empty() == false )
  {
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    TEST_GL_ERROR
  }

  return _complete;
}


/******************************************************************//**
* \brief   Destroy the buffer opjects and the passes objects.
* Destroy textue and framebuffer objects.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
void CRenderProcess::Destroy( void ) 
{
  Destruct();
}


/******************************************************************//**
* \brief   Check if the cache for target buffer binding,
* target buffer clearing and source texture binding is valid.
* Initialize the chach if it is not valid. 
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
const CRenderProcess::TBufferInfoCache & CRenderProcess::EvaluateInfoCache( 
  size_t               passID, //!< in: the name of the render pass
  const Render::TPass &pass )  //!< in: render pass data
{
  auto it = _bufferInfoMap.find( passID );
  if ( it != _bufferInfoMap.end() )
    return it->second;
  _bufferInfoMap[passID] = TBufferInfoCache();
  it = _bufferInfoMap.find( passID );
  TBufferInfoCache &info = it->second;
  
  // search the pass frambuffer
  // Note, not all passes have to use a frambuffer.
  // if there is not frambuffer, the target of the pass is the default frambuffer.
  auto fbIt = _fbs.find( passID );
  info._targetIsDefault = fbIt == _fbs.end();

  // set the size of the view port for the render pass
  info._vp_size = info._targetIsDefault ? _size : fbIt->second._size;

  // set the named frambuffer object
  info._fb_obj = info._targetIsDefault ? 0 : fbIt->second._object;

  // init the buffer clear mask
  info._clearMask = info._targetIsDefault ? ( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT ) : 0;

  // evaluate target buffers and target buffer clearing
  bool clear_all_color = true;
  bool clear_any_color = false;
  for ( auto target : pass._targets )
  {
    auto texIt = _textures.find( target._bufferID );
    if ( texIt == _textures.end() )
      continue;
    TTextureObject &texture = texIt->second;

    if ( target._attachment == Render::TPass::TTarget::depth_stencil )
    {
      if ( target.ClearTarget() )
        info._clearMask = info._clearMask | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT; 
    }
    else if ( target._attachment == Render::TPass::TTarget::depth )
    {
      if ( target.ClearTarget() )
        info._clearMask = info._clearMask | GL_DEPTH_BUFFER_BIT; 
    }
    else if ( target._attachment == Render::TPass::TTarget::stencil )
    {
      if ( target.ClearTarget() )
        info._clearMask = info._clearMask | GL_STENCIL_BUFFER_BIT; 
    }
    else
    {
      clear_any_color = true;
      info._drawBuffers.push_back( (GLenum)(GL_COLOR_ATTACHMENT0 + target._attachment) );

      if ( target.ClearTarget() == false || target._clear_color != Render::TColor{0.0f, 0.0f, 0.0f, 0.0f} )
        clear_all_color = false;
      if ( target.ClearTarget() )
        info._clearTargets.emplace_back( (GLint)target._attachment, target._clear_color );
    }
  }
  if ( clear_all_color && clear_any_color )
  {
    info._clearTargets.clear();
    info._clearMask = info._clearMask | GL_COLOR_BUFFER_BIT;
  }

  // setup source textures
  for ( auto source : pass._sources )
  {
    auto texIt = _textures.find( source._bufferID );
    if ( texIt == _textures.end() )
      continue;
    TTextureObject &texture = texIt->second;
    info._sourceTextures.emplace_back( GL_TEXTURE0 + (unsigned int)source._binding, texture._object );
  }

  return it->second;
}


/******************************************************************//**
* \brief   Prepare and activate a render pass. 
* 
* The following steps are processed:
* 
* - set the depth test
* - set the blendig mode
* - set the viewport 
* - bind the frame buffer
* - set the drawing buffers
* - clear the frame buffers
* - bind the source textures
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
bool CRenderProcess::Prepare( 
  size_t             passID, //!< in: the name of the render pass
  TPrepareProperties props ) //!< in: conditons
{
  if ( IsValid() == false || _complete == false )
    return false;

  // check if the pass is valid
  auto passIt = _passes.find( passID );
  if ( passIt == _passes.end() )
    return false;
  _currentPass = passID;
  Render::TPass &pass = passIt->second;

  // Update information for target buffer binding, target buffer clearing and source texture binding
  const TBufferInfoCache &bufferInfo = EvaluateInfoCache( passID, pass );

  // setup depth test
  if ( props.test((int)TPrepareProperty::depth) )
    SetupDepthTest( pass._depth );

  // setup blending
  if ( props.test((int)TPrepareProperty::blend) )
    SetupBlending( pass._blend );

  // render pass viewport size
  if ( props.test((int)TPrepareProperty::viewport) )
  {
    glViewport( 0, 0, (GLsizei)bufferInfo._vp_size[0], (GLsizei)bufferInfo._vp_size[1] );
    TEST_GL_ERROR
  }

  // bind the frambuffer
  if ( props.test((int)TPrepareProperty::bind) )
  {
    glBindFramebuffer( GL_FRAMEBUFFER, bufferInfo._fb_obj );
    TEST_GL_ERROR
  }

  // setup tartget buffer
  if ( props.test((int)TPrepareProperty::targets) &&  bufferInfo._targetIsDefault == false )
  {
    if ( bufferInfo._drawBuffers.empty() )
      glDrawBuffer( GL_NONE );
    else
      glDrawBuffers( (GLsizei)bufferInfo._drawBuffers.size(),  bufferInfo._drawBuffers.data() );
    TEST_GL_ERROR
  }

  // clear the buffers
  if ( props.test((int)TPrepareProperty::clear) )
  {
    for ( auto & clearTarget : bufferInfo._clearTargets )
    {
      glClearBufferfv( GL_COLOR, (GLint)clearTarget.first, clearTarget.second.data() );
      TEST_GL_ERROR
    }
    if ( bufferInfo._clearMask != 0 )
    {
      glClear( bufferInfo._clearMask );
      TEST_GL_ERROR
    }
  }

  // bind source textures
  if ( props.test((int)TPrepareProperty::source) )
  {
    for ( auto source : bufferInfo._sourceTextures )
    {
      glActiveTexture( source.first ); TEST_GL_ERROR
      glBindTexture( GL_TEXTURE_2D, source.second ); TEST_GL_ERROR
    }
  }
  
  return true;
}


/******************************************************************//**
* \brief   Relase the current render pass.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
bool CRenderProcess::ReleasePass( 
  size_t passID ) //!< in: the name of the render pass
{
  if ( IsValid() == false || _complete == false )
    return false;

  // check if the pass is valid
  auto passIt = _passes.find( passID );
  if ( passIt == _passes.end() )
    return false;
  Render::TPass &pass = passIt->second;

  // release source textures
  auto it = _bufferInfoMap.find( passID );
  if ( it != _bufferInfoMap.end() )
  {
    for ( auto source : it->second._sourceTextures )
    {
      glActiveTexture( source.first ); TEST_GL_ERROR
      glBindTexture( GL_TEXTURE_2D, 0 ); TEST_GL_ERROR
    }
  }

  // restore the viewport size
  glViewport( 0, 0, (GLsizei)_size[0], (GLsizei)_size[1] );
  TEST_GL_ERROR

  // release any framebuffer
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  TEST_GL_ERROR

  return true;
}


/******************************************************************//**
* \brief   Relase the current render pass.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
bool CRenderProcess::Release( void )
{
  if ( _currentPass == c_no_pass )
    return false;
  bool ret = ReleasePass( _currentPass );
  _currentPass = c_no_pass;
  return ret;
}


/******************************************************************//**
* \brief   Bind a frambuffer for reading and/or drawing.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
bool CRenderProcess::Bind( 
  size_t passID, //!< in: the name of the render pass
  bool   read,   //!< in: bind for reading
  bool   draw )  //!< in: bind for drawing
{
  if ( IsValid() == false || _complete == false )
    return false;

  // check if the pass is valid
  auto passIt = _passes.find( passID );
  if ( passIt == _passes.end() )
    return false;
  _currentPass = passID;
  Render::TPass &pass = passIt->second;

  // Update information for target buffer binding, target buffer clearing and source texture binding
  const TBufferInfoCache &bufferInfo = EvaluateInfoCache( passID, pass );

  // bind the frambuffer
  if ( read && draw == false )
    glBindFramebuffer( GL_READ_FRAMEBUFFER, bufferInfo._fb_obj );
  else if ( read == false && draw )
    glBindFramebuffer( GL_DRAW_FRAMEBUFFER, bufferInfo._fb_obj );
  else
    glBindFramebuffer( GL_FRAMEBUFFER, bufferInfo._fb_obj );
  TEST_GL_ERROR

  return true;
}


/******************************************************************//**
* \brief   Setup the depth function.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
bool CRenderProcess::SetupDepthTest( 
  Render::TPassDepthTest depth ) //!< in: depth function
{
  switch ( depth )
  {
    default:
    case Render::TPassDepthTest::OFF:
      glDisable( GL_DEPTH_TEST );
      break;

    case Render::TPassDepthTest::ALWAYS:
      glEnable( GL_DEPTH_TEST );
      glDepthFunc( GL_ALWAYS );
      glDepthMask( GL_TRUE );
      break;

    case Render::TPassDepthTest::LESS:
      glEnable( GL_DEPTH_TEST );
      glDepthFunc( GL_LESS );
      glDepthMask( GL_TRUE );
      break;

    case Render::TPassDepthTest::LESS_OR_EQUAL:
      glEnable( GL_DEPTH_TEST );
      glDepthFunc( GL_LEQUAL );
      glDepthMask( GL_TRUE );
      break;

    case Render::TPassDepthTest::LESS_READONLY:
      glEnable( GL_DEPTH_TEST );
      glDepthFunc( GL_LESS );
      glDepthMask( GL_FALSE );
      break;

    case Render::TPassDepthTest::LESS_OR_EQUAL_READONLY:
      glEnable( GL_DEPTH_TEST );
      glDepthFunc( GL_LEQUAL );
      glDepthMask( GL_FALSE );
      break;
  }
 
  TEST_GL_ERROR
  return true;
}

/******************************************************************//**
* \brief   Setup the blending function.
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
bool CRenderProcess::SetupBlending( 
  Render::TPassBlending blending ) //!< in: blending function
{
  switch( blending )
  {
    default:
    case Render::TPassBlending::OFF:
      glDisable( GL_BLEND );
      break;

    case Render::TPassBlending::OVERWRITE:
      glEnable( GL_BLEND );
      glBlendFunc( GL_ONE, GL_ZERO );
      break;

    case Render::TPassBlending::MIX:
      glEnable( GL_BLEND );
      glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
      break;

    case Render::TPassBlending::MIX_PREMULTIPLIED_ALPHA:
      glEnable( GL_BLEND );
      glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
      break;

    case Render::TPassBlending::ADD:
      glEnable( GL_BLEND );
      glBlendFunc( GL_ONE, GL_ONE );
      break;
  }

  TEST_GL_ERROR
  return true;
}

} // OpenGL