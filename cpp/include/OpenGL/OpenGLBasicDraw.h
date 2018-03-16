/******************************************************************//**
* \brief   Implementation og generic interface for basic darwing.
* 
* \author  gernot
* \date    2018-02-04
* \version 1.0
**********************************************************************/
#pragma once
#ifndef OpenGLBasicDraw_h_INCLUDED
#define OpenGLBasicDraw_h_INCLUDED

// includes

#include <IDraw.h>
#include <IBuffer.h>


// calss declarations

/******************************************************************//**
* \brief General OpenGL namespace
**********************************************************************/
namespace OpenGL
{


//---------------------------------------------------------------------
// CBasicDraw
//---------------------------------------------------------------------


/******************************************************************//**
* @brief   Implementation for basic drawing.
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
class CBasicDraw
  : public Render::IDraw
{
public:

  CBasicDraw( void );
  virtual ~CBasicDraw();

  virtual void Destroy( void ) override;

  virtual Render::IDrawBuffer * NewDrawBuffer( Render::TDrawBufferUsage usage );
  virtual Render::IDrawBuffer & DrawBuffer( void );
  virtual Render::IDrawBuffer & DrawBuffer( const void *key, bool &cached );



  virtual bool Draw( Render::TPrimitive primitive_type, size_t size, size_t coords_size, const Render::t_fp *coords, const Render::TColor &color, const TStyle &style ) override;

private:

  const size_t                        _max_buffers = 8;
  size_t                              _nextBufferI = 0;
  std::array<const void*, 8>          _buffer_keys{ nullptr };
  std::array<Render::IDrawBuffer*, 8> _draw_buffers{ nullptr };
};

} // OpenGL

#endif // OpenGLBasicDraw_h_INCLUDED
