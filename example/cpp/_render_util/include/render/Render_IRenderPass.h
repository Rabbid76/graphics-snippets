/******************************************************************//**
* \brief   Generic interface for rendering passes.
* 
* \author  gernot
* \date    2018-02-04
* \version 1.0
**********************************************************************/
#pragma once
#ifndef Render_IRenderPass_h_INCLUDED
#define Render_IRenderPass_h_INCLUDED


// includes

#include "../render/Render_IDrawType.h"

// STL

#include <bitset>
#include <array>
#include <vector>
#include <map>
#include <tuple>


// interface declarations

/******************************************************************//**
* \brief namespace for generic interface of drawing operations
**********************************************************************/
namespace Render
{


class IRenderProcess;
using IRenderProcessPtr = std::unique_ptr<IRenderProcess>;



//---------------------------------------------------------------------
// IRenderProcessProvider
//---------------------------------------------------------------------


/******************************************************************//**
* \brief   Generic provider for generic draw buffers.
* 
* \author  gernot
* \date    2018-06-08
* \version 1.0
**********************************************************************/
class IRenderProcessProvider
{
public:

  virtual ~IRenderProcessProvider() = default;

  virtual IRenderProcessPtr NewRenderProcess( void ) = 0;
};



//---------------------------------------------------------------------
// IRenderProcess
//---------------------------------------------------------------------


/******************************************************************//**
* \brief   Type of a render pass target
* 
* \author  gernot
* \date    2018-02-04
* \version 1.0
**********************************************************************/
enum class TBufferType
{
  DEFAULT      =  0, //!< default type (`COLOR4`)
  COLOR1       =  1, //!< 1 color channel
  COLOR2       =  2, //!< 2 color channels
  COLOR3       =  3, //!< 3 color channels
  COLOR4       =  4, //!< 3 color channels and a alpha channel
  DEPTH        = -1, //!< depth buffer
  STENCIL      = -2, //!< stencil buffer
  DEPTHSTENCIL = -4  //!< combined depth and stencil buffer       
};


/******************************************************************//**
* \brief   Data type for a channel of a render pass target 
* 
* \author  gernot
* \date    2018-02-04
* \version 1.0
**********************************************************************/
enum class TBufferDataType
{
  DEFAULT             =  0, //!< default format; is one of `UINT8`, `DEPTH32`, `DEPTH_STENCIL_24_8`
  F_BYTE              =  1, //!< byte [0.0, 1.0]
  F_WORD              =  2, //!< word [0.0, 1.0]
  SNORM8              = 16, //!< [-1, 1] 8 bit
  SNORM16             = 17, //!< [-1, 1] 16 bit
  F16                 = 32, //!< 16 bit float; 1 bit sign, 10 bit mantissa, 5 bit exponent; 6.10 * 10^(-5) .. 6.55 * 10^4 
  F32                 = 33, //!< 32 bit float; 1 bit sign, 23 bit mantissa, 8 bit exponent; IEEE 754; 
  DEPTH16             = 48, //!< 16 bit integral depth buffer
  DEPTH24             = 49, //!< 24 bit integral depth buffer
  DEPTH32             = 50, //!< 32 bit integral depth buffer
  DEPTH32F            = 51, //!< 32 bit floating point depth buffer
  STENCIL8            = 52, //!< 8 bit stencil buffer
  DEPTH_STENCIL_24_8  = 53, //!< combined depth and stencil buffer; 24 bit depth, 8 bit stencil
  DEPTH_STENCIL_32F_8 = 54  //!< combined depth and stencil buffer; 24 bit depth, 8 bit stencil
};


/******************************************************************//**
* \brief   Depth test type of the render pass
* 
* \author  gernot
* \date    2018-02-04
* \version 1.0
**********************************************************************/
enum class TPassDepthTest
{
  OFF                    = 0, //!< disable depth test
  ALWAYS                 = 1, //!< depth test: is on but always pass             
  LESS                   = 2, //!< depth test: is less
  LESS_OR_EQUAL          = 3, //!< depth test: is less or equal
  LESS_READONLY          = 4, //!< depth test: is less, but depth buffer will not be written
  LESS_OR_EQUAL_READONLY = 5  //!< depth test: is less or equal, but depth buffer will not be written
};


/******************************************************************//**
* \brief   Blending type of the render pass
* 
* \author  gernot
* \date    2018-02-04
* \version 1.0
**********************************************************************/
enum class TPassBlending
{
  OFF                     = 0, //!< dest = source
  OVERWRITE               = 1, //!< dest = dest * 0 + source
  MIX                     = 2, //!< dest = dest * (1-alpha) + source * alpha
  MIX_PREMULTIPLIED_ALPHA = 3, //!< dest = dest * (1-alpha) + source
  ADD                     = 4  //!< dest = dest + source  
};


/******************************************************************//**
* \brief   Specification of a target buffer for a render pass
* 
* \author  gernot
* \date    2018-02-10
* \version 1.0
**********************************************************************/
struct TBuffer
{
  enum TProperty
  {
    e_linear,  //!< linear filter
    e_extern,  //!< extern texture
    e_cubemap, //!< the extern target texture is a cubmap or a side of a cubemap
    //...
    e_NO_OF  //!< number of buffer properties
  };

  using TProperties = std::bitset<e_NO_OF>;

  bool operator ==( const TBuffer &B ) const 
  {
    return 
      std::make_tuple( _type, _format, _layers, _scale, _multisamples, _flag ) == 
      std::make_tuple( B._type, B._format, B._layers, B._scale, B._multisamples, B._flag );
  }
  bool operator !=( const TBuffer &B ) const
  {
    return ( *this == B ) == false;
  }

  TBuffer( void ) = default;
  TBuffer( const TBuffer & ) = default;
  TBuffer & operator =( const TBuffer & ) = default;

  TBuffer( TBufferType type, TBufferDataType format )
    : _type( type )
    , _format( format )
  {}

  TBuffer( TBufferType type, TBufferDataType format, unsigned int layers )
    : _type( type )
    , _format( format )
    , _layers( layers )
  {}

  TBuffer( TBufferType type, TBufferDataType format, unsigned int layers, float scale )
    : _type( type )
    , _format( format )
    , _layers( layers )
    , _scale( scale )
  {}

  TBuffer( TBufferType type, TBufferDataType format, unsigned int layers, float scale, unsigned int multisamples )
    : _type( type )
    , _format( format )
    , _layers( layers )
    , _scale( scale )
    , _multisamples( multisamples )
  {}

  TBuffer( TBufferType type, TBufferDataType format, unsigned int layers, float scale, unsigned int multisamples, bool linear )
    : _type( type )
    , _format( format )
    , _layers( layers )
    , _scale( scale )
    , _multisamples( multisamples )
  {
    _flag.set( e_linear, linear );
  }

  void SetExternBufferObject( unsigned int object, bool cubemap, int cubemap_side )
  {
    _flag.set( e_extern,  object > 0 );
    _flag.set( e_cubemap, cubemap );
    _extern_object      = _flag.test( e_extern ) ? object : 0;
    _cubemap_side_index = _flag.test( e_extern ) && _flag.test( e_cubemap ) ? cubemap_side : -1;
  }

  TBufferType     _type   = TBufferType::DEFAULT;     //!< type of the buffer
  TBufferDataType _format = TBufferDataType::DEFAULT; //!< internal format of the buffer
  unsigned int    _layers = 0;                        //!< number of the texture layers
  float           _scale  = 1.0f;                     //!< buffer size scale (down scale or up scale)
  unsigned int    _multisamples = 0;                  //!< multisamples
  TProperties     _flag;                              //!< buffer properties
  unsigned int    _extern_object = 0;                 //!< extern buffer object
  int             _cubemap_side_index = -1;           //!< if >= 0 && < 6, then the side index of the cube map; else the entire cube map
};


/******************************************************************//**
* \brief   Specification of a render pass.
* 
* \author  gernot
* \date    2018-02-10
* \version 1.0
**********************************************************************/
struct TPass
{
  //!< source buffer:
  struct TSource
  {
    TSource( size_t bufferID, size_t binding )
      : _bufferID( bufferID )
      , _binding( binding )
    {}
    
    TSource( void ) : TSource( 0, 0 ) {}
    TSource( const TSource & ) = default;
    TSource & operator =( const TSource & ) = default;

    bool operator ==( const TSource &B ) const { return std::make_tuple( _bufferID, _binding ) == std::make_tuple( B._bufferID, B._binding ); }
    bool operator !=( const TSource &B ) const { return ( *this == B ) == false; }

    size_t _bufferID; //!< buffer ID
    size_t _binding;  //!< binding point
  };

  enum TTargetProperty
  {
    e_clear, //!< buffer has to be cleared
    //...
    e_NO_OF  //!< number of buffer properties
  };

  using TTargetProperties = std::bitset<e_NO_OF>;

  //!< target buffer:
  struct TTarget
  {
    TTarget( size_t bufferID, size_t attachment )
      : _bufferID( bufferID )
      , _attachment( attachment )
    {
      _prop.set(TTargetProperty::e_clear);
    }

    TTarget( size_t bufferID, size_t attachment, bool clear )
      : _bufferID( bufferID )
      , _attachment( attachment )
    {
      _prop.set(TTargetProperty::e_clear, clear);
    }

    TTarget( size_t bufferID, size_t attachment, bool clear, const TColor &clear_color )
      : _bufferID( bufferID )
      , _attachment( attachment )
      , _clear_color( clear_color )
    {
      _prop.set(TTargetProperty::e_clear, clear);
    }
    
    TTarget( void ) : TTarget( 0, 0 ) {}
    TTarget( const TTarget & ) = default;
    TTarget & operator =( const TTarget & ) = default;

    static const size_t no = (size_t)-1;
    
    static const size_t depth         = (size_t)-1;
    static const size_t stencil       = (size_t)-2;
    static const size_t depth_stencil = (size_t)-3;

    bool ClearTarget( void ) const { return _prop.test(TTargetProperty::e_clear); }

    bool operator ==( const TTarget &B ) const 
    {
      return std::make_tuple( _bufferID, _attachment, _prop, _clear_color ) == std::make_tuple( B._bufferID, B._attachment, B._prop, B._clear_color );
    }
    bool operator !=( const TTarget &B ) const
    {
      return ( *this == B ) == false;
    }

    size_t            _bufferID;                              //!< buffer ID (it -1, the use render buffer)
    size_t            _attachment;                            //!< attachment: -3 (depth/stencil), -2 (stencil),  -1 (depth), 0, 1, 2, ... , n
    TTargetProperties _prop;                                  //!< buffer target properties
    TColor            _clear_color{ 0.0f, 0.0f, 0.0f, 0.0f }; //!< clear color for the buffer
  };


  TPass( void ) = default;
  TPass( const TPass & ) = default;
  TPass & operator =( const TPass & ) = default;

  TPass( TPassDepthTest depth, TPassBlending blend )
    : _depth( depth )
    , _blend( blend )
  {}

  bool operator ==( const TPass &B ) const 
  {
    return std::make_tuple( _sources, _targets, _depth, _blend ) == std::make_tuple( B._sources, B._targets, B._depth, B._blend );
  }
  bool operator !=( const TPass &B ) const
  {
    return ( *this == B ) == false;
  }

  std::vector<TSource> _sources;                             //!< source buffers
  std::vector<TTarget> _targets;                             //!< target buffers
  TPassDepthTest       _depth         = TPassDepthTest::OFF; //!< depth test of the pass
  TPassBlending        _blend         = TPassBlending::OFF;  //!< blending function of the pass
  size_t               _default_obj   = 0;                   //!< default framebuffer obj
  bool                 _clear_default = true;                //!< clear the default buffer
};


/******************************************************************//**
* \brief   Generic interface for render pass specification and management.
* 
* The implementations of this interface should make use of 
* RAII (Resource acquisition is initialization) technique.
* [https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization]

* \author  gernot
* \date    2018-02-10
* \version 1.0
**********************************************************************/
class IRenderProcess
{
public:

  using TBufferMap = std::map<size_t, TBuffer>; //!< buffer map
  using TPassMap   = std::map<size_t, TPass>;   //!< pass map
  using TSize      = std::array<size_t, 2>;     //!< 2D size

  enum class TPrepareProperty
  {
    depth,    //!< set the depth test
    blend,    //!< set the blending function
    viewport, //!< sets the viewport size
    bind,     //!< bind the target buffers
    targets,  //!< set target buffers
    clear,    //!< clear target buffers according to the target specification
    source,   //!< bind pass source buffers
    // ...
    NO_OF
  }; 
  using TPrepareProperties = std::bitset<(int)TPrepareProperty::NO_OF>;

  IRenderProcess( void ) = default;
  virtual ~IRenderProcess() = default;

  IRenderProcess( const IRenderProcess & ) = delete;
  IRenderProcess( IRenderProcess && ) = default;

  IRenderProcess & operator =( const IRenderProcess & ) = delete;
  IRenderProcess & operator =( IRenderProcess && ) = default;

  virtual const TBufferMap & Buffers( void ) const = 0;
  virtual const TPassMap   & Passes( void ) const = 0;

  virtual void ClearBuffers( void ) = 0;                                           //!< clear all render buffer specifications
  virtual void ClearBuffer( size_t bufferID ) = 0;                                 //!< clear a render buffer specification
  virtual bool SpecifyBuffer( size_t bufferID, const TBuffer &specification ) = 0; //!< specify a single render buffer

  //! specify all render pass buffers  
  virtual bool SpecifyBuffers( const TBufferMap &buffers )
  {
    ClearBuffers();
    bool valid = true;
    for ( auto & buffer : buffers )
      if ( SpecifyBuffer( buffer.first, buffer.second ) == false ) valid = false;
    return valid;
  }

  virtual void ClearPasses( void ) = 0;                                      //!< clear all render passes specifications
  virtual void ClearPass( size_t passID ) = 0;                               //!< clear a render pass specification
  virtual bool SpecifyPass( size_t passID, const TPass &specification ) = 0; //!< specify a single render pass

  //! specify all render passes  
  virtual bool SpecifyPasses( const TPassMap &passes )
  {
    ClearPasses();
    bool valid = true;
    for ( auto & pass : passes )
      if ( SpecifyPass( pass.first, pass.second ) == false ) valid = false;
    return valid;
  }

 
  virtual bool  IsValid( void ) const = 0;                                 //!< checks if the specifications have been successfully validated
  virtual bool  IsComplete( void ) const = 0;                              //!< checks if the buffers and passes have been successfully created
  virtual void  Invalidate( void ) = 0;                                    //!< invalidate, force renew of buffers and passes
  virtual bool  Validate( void ) = 0;                                      //!< validate the specifications
  virtual bool  ValidateSize( std::array<size_t, 2> ) = 0;                 //!< validate the framebuffer sizes
  virtual bool  Create( std::array<size_t, 2> ) = 0;                       //!< validate the specification and create the render passes 
  virtual TSize CurrentSize( void ) const = 0;                             //!< current process buffer size
  virtual void  Destroy( void ) = 0;                                       //!< destroy the buffer and the passes
  virtual bool  Prepare( size_t passID, TPrepareProperties props ) = 0;    //!< prepare a render pass
  virtual bool  Bind( size_t passID, bool read, bool draw ) = 0;           //!< binds the named framebuffer for drawing
  virtual bool  SetDrawBuffers( bool firstColorAttachmentOnly ) = 0;       //!< activates the draw buffers
  virtual bool  ReleasePass( size_t passID ) = 0;                          //!< release the render pass
  virtual bool  Release( void ) = 0;                                       //!< release the current render pass
  virtual bool  GetBufferObject( size_t bufferID, unsigned int &obj ) = 0; //!< get the implementation object for a buffer
  virtual bool  GetPassObject( size_t passID, unsigned int &obj ) = 0;     //!< get the implementation object for a pass
 
  bool Prepare( size_t passID )
  {
    static const TPrepareProperties props = TPrepareProperties().
      set((int)TPrepareProperty::depth).set((int)TPrepareProperty::blend).set((int)TPrepareProperty::viewport).
      set((int)TPrepareProperty::bind).set((int)TPrepareProperty::targets).set((int)TPrepareProperty::clear).
      set((int)TPrepareProperty::source);
    return Prepare( passID, props );
  }

  bool PrepareMode( size_t passID )
  {
    static const TPrepareProperties props = TPrepareProperties().set((int)TPrepareProperty::depth).set((int)TPrepareProperty::blend);
    return Prepare( passID, props );
  }

  bool PrepareNoClear( size_t passID )
  {
    static const TPrepareProperties props = TPrepareProperties().
      set((int)TPrepareProperty::depth).set((int)TPrepareProperty::blend).set((int)TPrepareProperty::viewport).
      set((int)TPrepareProperty::bind).set((int)TPrepareProperty::targets).
      set((int)TPrepareProperty::source);
    return Prepare( passID, props );
  }

  bool PrepareClear( size_t passID )
  {
    static const TPrepareProperties props = TPrepareProperties().
      set((int)TPrepareProperty::depth).    // depth test has to be set, otherwise depth buffer may not be cleared
      set((int)TPrepareProperty::viewport).
      set( (int)TPrepareProperty::bind ).set( (int)TPrepareProperty::targets ).set( (int)TPrepareProperty::clear );
    return Prepare( passID, props );
  }

};              


} // Draw


#endif // Render_IRenderPass_h_INCLUDED