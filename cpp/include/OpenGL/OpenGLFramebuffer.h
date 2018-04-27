/******************************************************************//**
* \brief   Implementation for rendering passes with OpenGL frame
*          buffers.
* 
* \author  gernot
* \date    2018-02-04
* \version 1.0
**********************************************************************/
#pragma once
#ifndef OpenGLFramebuffer_h_INCLUDED
#define OpenGLFramebuffer_h_INCLUDED

// includes

#include <IRenderPass.h>


// class definitions

namespace OpenGL
{


//*********************************************************************
// CRenderProcess
//*********************************************************************


/******************************************************************//**
* \brief   OpenGL implementation of the render process managment.
*          
*  Make use of:
*
*  - [OpenGL Texture 2D](https://www.khronos.org/opengl/wiki/Texture)
*  - [OpenGL Framebuffer Object](https://www.khronos.org/opengl/wiki/Framebuffer_Object)         
* 
* \author  gernot
* \date    2018-02-11
* \version 1.0
**********************************************************************/
class CRenderProcess
  : public Render::IRenderProcess
{
public:

  using TScaleMap     = std::map<size_t, float>;
  using TViewportSize = Render::IRenderProcess::TSize;

  struct TTextureObject
  {
    bool operator ==( TTextureObject &B ) const
    {
      if ( B._extern )
        return _extern && _object == B._object;
      return _layers == B._layers && _size == B._size && _format == B._format;
    }

    bool operator !=( TTextureObject &B ) const { return (*this == B) == false; }

    unsigned int                _object;       //!< GPU object name
    std::array<size_t, 2>       _size;         //!< 2D texture size
    std::array<unsigned int, 3> _format;       //!< texture format
    unsigned int                _layers;       //!< number of texture layers
    unsigned int                _multisamples; //!< number of texture layers
    bool                        _linear;       //!< linear texture buffer filter
    bool                        _extern;       //!< extern texture; a texture which is not maneged in this class, but was handed over for use
  };
  using TTextureMap = std::map<size_t, TTextureObject>;

  struct TFramebufferObject
  {
    unsigned int          _object; //!< GPU object name
    std::array<size_t, 2> _size;   //!< render buffer size
  };
  using TFrambufferMap = std::map<size_t, TFramebufferObject>;

  using TTextureAndBindig = std::tuple<unsigned int, unsigned int, unsigned int>;
  struct TBufferInfoCache
  {
    TBufferInfoCache( void ) = default;

    bool                                           _targetIsDefault = false; //!< the target is the default frame buffer
    TViewportSize                                  _vp_size{ 0, 0 };         //!< frambuffer viewport size
    unsigned int                                   _fb_obj;                  //!< the named frambuffer object (GPU); if the target is the default frambuffer, the this is 0 
    unsigned int                                   _clearMask = 0;           //!< OpenGL bit mask for frambuffer clear
    std::vector<std::pair<size_t, Render::TColor>> _clearTargets;            //!< color attachmet and color for separted clear operations
    std::vector<unsigned int>                      _drawBuffers;             //!< enumerators for the draw buffers
    std::vector<TTextureAndBindig>                 _sourceTextures;          //!< source texture objects and its binding point
  };
  using TBufferInfoMap = std::map<size_t, TBufferInfoCache>;

  static const size_t c_no_pass = (size_t)-1;


  CRenderProcess( void );
  virtual ~CRenderProcess();

  const TTextureMap & Textures( void ) const { return _textures; };

  virtual const TBufferMap & Buffers( void ) const { return _buffers; };
  virtual const TPassMap   & Passes( void )  const { return _passes; };

  static std::array<unsigned int, 3> InternalFormat( Render::TBufferType buffer, Render::TBufferDataType format );

  virtual bool          IsValid( void )     const override { return _valid; }
  virtual bool          IsComplete( void )  const override { return _complete; }
  virtual TViewportSize CurrentSize( void ) const override { return _size; }                        
  
  virtual void Invalidate( void ) override;                                                     //!< invalidate, force renew of buffers and passes
  virtual void ClearBuffers( void ) override;                                                   //!< clear all render buffer specifications
  virtual void ClearBuffer( size_t bufferID ) override;                                         //!< clear a render buffer specification
  virtual bool SpecifyBuffer( size_t bufferID, const Render::TBuffer &specification ) override; //!< specify a single render buffer
  virtual bool SpecifyBuffers( const TBufferMap &buffers ) override;                            //!< specify all render pass buffers
  virtual void ClearPasses( void ) override;                                                    //!< clear all render passes specifications
  virtual void ClearPass( size_t passID ) override;                                             //!< clear a render pass specifications
  virtual bool SpecifyPass( size_t passID, const Render::TPass &specification ) override;       //!< specify a single render pass
  virtual bool SpecifyPasses( const TPassMap &passes ) override;                                //!< specify all render passes 
  virtual bool Validate( void ) override;                                                       //!< validate the specifcations
  virtual bool ValidateSize( std::array<size_t, 2> ) override;                                  //!< validate the frambuffer sizes
  virtual bool Create( TViewportSize size ) override;                                           //!< validate the specification and create the render passes                
  virtual void Destroy( void ) override;                                                        //!< destroy the buffer and the passes
  virtual bool Prepare( size_t passID, TPrepareProperties props ) override;                     //!< prepare a render pass
  virtual bool Bind( size_t passID, bool read, bool draw ) override;                            //!< binds the named famebuffer
  virtual bool ReleasePass( size_t passID ) override;                                           //!< relase the render pass
  virtual bool Release( void ) override;                                                        //!< relase the current render pass
  virtual bool GetPassObject( size_t passID, unsigned int &obj ) override;                      //!< get the implementation object for a pass

  virtual bool SetupDepthTest( Render::TPassDepthTest depth );  //!< set up the depth test
  virtual bool SetupBlending( Render::TPassBlending blending ); //!< set up the blending function


private:

  bool IsLayered(      unsigned int layers )       const { return layers > 1; }
  bool IsMultisampled( unsigned int multisamples ) const { return multisamples > 1; }

  void Destruct( void );
  void DeleteUnnecessaryTextures( void );
  void DeleteUnnecessaryFrambuffers( void );
  void UpdateTexture( size_t bufferID, const Render::TBuffer &specification );
  unsigned int CreateFrambufferRenderBuffer( const TFramebufferObject &fb, const Render::TPass::TTarget &target );
  void AttachFrambufferTextureBuffer( const TFramebufferObject &fb, const Render::TPass::TTarget &target );
  const TBufferInfoCache & EvaluateInfoCache( size_t passID, const Render::TPass &pass );
  
  size_t           _currentPass = c_no_pass; //!< current render pass
  TBufferMap       _buffers;                 //!< buffer specifications
  TPassMap         _passes;                  //!< pass specifications
  bool             _valid       = false;     //!< all process buffers and process passes have been validated successfully 
  bool             _complete    = false;     //!< all framebuffers are created an stated complete
  TScaleMap        _scales;                  //!< validated buffer scales
  TScaleMap        _passScales;              //!< validated render pass scales
  TViewportSize    _size{ 0, 0 };            //!< viewport size for which the render passes have been created
  TTextureMap      _textures;                //!< texture objects for the pass targets
  TFrambufferMap   _fbs;                     //!< frambuffer objects for the render passes
  TBufferInfoMap   _bufferInfoMap;           //!< cached information for target buffer binding, target buffer clearing and source texture binding
};


} // OpenGL


#endif // OpenGLFramebuffer_h_INCLUDED
