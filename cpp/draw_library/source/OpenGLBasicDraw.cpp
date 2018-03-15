/******************************************************************//**
* \brief   Implementation og generic interface for basic darwing.
* 
* \author  gernot
* \date    2018-02-04
* \version 1.0
**********************************************************************/

// includes

#include <stdafx.h>

// OpenGL

#include <OpenGLBasicDraw.h>
#include <OpenGLVertexBuffer.h>

// stl

#include <cassert>
#include <algorithm>


// class implementation

/******************************************************************//**
* \brief General OpenGL namespace
**********************************************************************/
namespace OpenGL
{

//---------------------------------------------------------------------
// CBasicDraw
//---------------------------------------------------------------------


/******************************************************************//**
* @brief   ctor
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
CBasicDraw::CBasicDraw( void )
{}


/******************************************************************//**
* @brief   dtor
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
CBasicDraw::~CBasicDraw()
{
  Destroy();
}


/******************************************************************//**
* @brief   destroy internal GPU objects
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
void CBasicDraw::Destroy( void )
{
  for ( auto & buffer : _draw_buffers )
  {
    delete buffer;
    buffer = nullptr;
  }
  for ( auto & key : _buffer_keys )
    key = nullptr;
  _nextBufferI = 0;
}


/******************************************************************//**
* \brief   Create new or retrun existion default draw buffer
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
Render::IDrawBuffer & CBasicDraw::DrawBuffer( void )
{
  bool cached = false;
  return DrawBuffer( nullptr, cached );
}


/******************************************************************//**
* \brief   Create new or retrun existion default draw buffer
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
Render::IDrawBuffer & CBasicDraw::DrawBuffer( 
  const void *key,     //!< I - key for the temporary cache
  bool       &cached ) //!< O - object was found in the cache
{
  if ( key != nullptr )
  {
    // try to find temporary buffer which already has the data
    auto keyIt = std::find( _buffer_keys.begin(), _buffer_keys.end(), key );
    if ( keyIt != _buffer_keys.end() )
    {
      size_t inx = keyIt - _buffer_keys.begin();
      if ( _nextBufferI == inx )
        _nextBufferI = _nextBufferI < _max_buffers - 1 ? _nextBufferI + 1 : 0;
      
      Render::IDrawBuffer *foundBuffer = _draw_buffers[inx];
      cached = true;
      return *foundBuffer;
    }
  }

  // ensure the buffer object is allocated
  if ( _draw_buffers[_nextBufferI] == nullptr )
    _draw_buffers[_nextBufferI]= NewDrawBuffer( Render::TDrawBufferUsage::stream_draw );
  
  // get buffer object
  Render::IDrawBuffer *currentBuffer = _draw_buffers[_nextBufferI];
  _buffer_keys[_nextBufferI] = key;
  _nextBufferI = _nextBufferI < _max_buffers - 1 ? _nextBufferI + 1 : 0;

  cached = false;
  return *currentBuffer;
}


/******************************************************************//**
* \brief   Create a new and empty draw buffer object.
* 
* \author  gernot
* \date    2017-11-26
* \version 1.0
**********************************************************************/
Render::IDrawBuffer * CBasicDraw::NewDrawBuffer( 
  Render::TDrawBufferUsage usage ) //!< I - usage of draw buffer : static_draw, dynamic_draw or stream_draw
{
  return new OpenGL::CDrawBuffer( usage, 1024 ); 
}


/******************************************************************//**
* \brief   Draw a convex polygon with a single color
* 
* \author  gernot
* \date    2018-03-15
* \version 1.0
**********************************************************************/
bool CBasicDraw::DrawConvexPolygon( 
  size_t                size,        //!< in: size vertex coordiantes
  size_t                coords_size, //!< in: size of coordinate buffer
  const Render::t_fp   *coords,      //!< in: coordiant buffer
  const Render::TColor &color )      //!< in: color for drawing
{
  if ( size != 2 && size !=3 && size !=4 )
  {
    assert( false );
    return false;
  }

  // buffer specification
  Render::TVA va_id = size == 2 ? Render::TVA::b0_xy__b1_rgba : (size == 3 ? Render::TVA::b0_xyz__b1_rgba : Render::TVA::b0_xyzw__b1_rgba);
  const std::vector<char> bufferdescr = Render::IDrawBuffer::VADescription( va_id );

  // color buffer
  size_t no_of_vertices = coords_size / size;
  std::vector<Render::t_fp> color_buffer( no_of_vertices * 4 );
  for ( size_t i=0; i < no_of_vertices; ++ i )
    std::copy( color.begin(), color.end(), color_buffer.begin() + i * 4 );

  // create buffer
  Render::IDrawBuffer &buffer = DrawBuffer();
  buffer.SpecifyVA( bufferdescr.size(), bufferdescr.data() );
  buffer.UpdateVB( 0, sizeof(float), coords_size, coords );
  buffer.UpdateVB( 1, sizeof(float), color_buffer );

  // draw_buffer
  buffer.DrawArray( Render::TPrimitive::trianglefan, 0, no_of_vertices, true );
  buffer.Release();

  return true;
}


} // OpenGL