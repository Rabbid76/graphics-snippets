/******************************************************************//**
* @brief   Generic interface for rendering text.
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
#pragma once
#ifndef ITexture_h_INCLUDED
#define ITexture_h_INCLUDED


// includes

#include <IDrawType.h>


/******************************************************************//**
* @brief   Namespace for renderer.
*
* @author  gernot
* @date    2018-03-17
* @version 1.0
**********************************************************************/
namespace Render
{


//---------------------------------------------------------------------
// IText
//---------------------------------------------------------------------


/******************************************************************//**
* @brief   Generic interface for rendering text.
*
* @author  gernot
* @date    2018-03-17
* @version 1.0
**********************************************************************/
class IText
{
public: 

  virtual ~IText() = default;

  virtual void Destroy( void ) = 0; //!< destroy all internal objects and cleanup
  virtual bool Load( void ) = 0;    //!< load the glyphs

};


} // Render

#endif // ITexture_h_INCLUDED
