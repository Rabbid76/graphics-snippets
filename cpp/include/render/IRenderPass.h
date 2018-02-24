/******************************************************************//**
* \brief   Generic interface for rendering passes.
* 
* \author  gernot
* \date    2018-02-04
* \version 1.0
**********************************************************************/
#pragma once
#ifndef IRenderPass_h_INCLUDED
#define IRenderPass_h_INCLUDED


// includes

#include <IDrawType.h>

// stl

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
  UINT8               =  1, //!< byte
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
  OFF           = 0, //!< disable depth test
  LESS          = 1, //!< depth test: is less
  LESS_OR_EQUAL = 2  //!< depth test: is less or equal
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
    e_cubemap, //!< buffer has to be cleared
    //...
    e_NO_OF  //!< number of buffer properties
  };

  using TProperties = std::bitset<e_NO_OF>;

  bool operator ==( const TBuffer &B ) const 
  {
    return std::make_tuple( _type, _format, _scale, _flag ) == std::make_tuple( B._type, B._format, B._scale, B._flag );
  }
  bool operator !=( const TBuffer &B ) const
  {
    return ( *this == B ) == false;
  }

  TBufferType     _type   = TBufferType::DEFAULT;     //!< type of the buffer
  TBufferDataType _format = TBufferDataType::DEFAULT; //!< internal format of the buffer
  unsigned int    _layers = 0;                        //!< number of the texture layers
  float           _scale  = 1.0f;                     //!< buffer size scale (down scale or up scale)
  TProperties     _flag;                              //!< buffer properties
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
    TTarget( size_t bufferID, size_t location )
      : _bufferID( bufferID )
      , _location( location )
    {
      _prop.set(TTargetProperty::e_clear);
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
      return std::make_tuple( _bufferID, _location, _prop, _clear_color ) == std::make_tuple( B._bufferID, B._location, B._prop, B._clear_color );
    }
    bool operator !=( const TTarget &B ) const
    {
      return ( *this == B ) == false;
    }

    size_t            _bufferID;                              //!< buffer ID (it -1, the use render buffer)
    size_t            _location;                              //!< -3 (depth/stencil), -2 (stencil),  -1 (depth), 0, 1, 2, ... , n
    TTargetProperties _prop;                                  //!< buffer target properties
    TColor            _clear_color{ 0.0f, 0.0f, 0.0f, 0.0f }; //!< clear color for the buffer
  };


  bool operator ==( const TPass &B ) const 
  {
    return std::make_tuple( _sources, _targets, _depth, _blend ) == std::make_tuple( B._sources, B._targets, B._depth, B._blend );
  }
  bool operator !=( const TPass &B ) const
  {
    return ( *this == B ) == false;
  }

  std::vector<TSource> _sources;                     //!< source buffers
  std::vector<TTarget> _targets;                     //!< target buffers
  TPassDepthTest       _depth = TPassDepthTest::OFF; //!< depth test of the pass
  TPassBlending        _blend = TPassBlending::OFF;  //!< blending function of the pass
};



//*********************************************************************
// IContextManager
//*********************************************************************


/******************************************************************//**
* \brief   Generic interface for render pass specification and managment.
* 
* \author  gernot
* \date    2018-02-10
* \version 1.0
**********************************************************************/
class IRenderProcess
{
public:

  using TBufferMap = std::map<size_t, TBuffer>; //!< buffer map
  using TPassMap   = std::map<size_t, TPass>;   //!< pass map


  IRenderProcess( void ) = default;
  virtual ~IRenderProcess() = default;

  IRenderProcess( const IRenderProcess & ) = delete;
  IRenderProcess( IRenderProcess && ) = default;

  IRenderProcess & operator =( const IRenderProcess & ) = delete;
  IRenderProcess & operator =( IRenderProcess && ) = default;


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

  virtual bool IsValid( void ) = 0;                 //!< check if the specifications have been successfully validated
  virtual bool Validate( void ) = 0;                //!< validate the specifcations
  virtual bool Create( std::array<size_t, 2> ) = 0; //!< validate the specification and create the render passes 
  virtual void Destroy( void ) = 0;                 //!< detroy the buffer and the passes
  virtual bool Prepare( size_t passID ) = 0;        //!< prepare a render pass
  virtual bool ReleasePass( size_t passID ) = 0;    //!< relase the render pass
  virtual bool Release( void ) = 0;                 //!< relase the current render pass
};


} // Draw


#endif // IRenderPass_h_INCLUDED