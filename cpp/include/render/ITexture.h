/******************************************************************//**
* @brief   Generic interface for rendering textures.
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
// ITexture
//---------------------------------------------------------------------


/******************************************************************//**
* @brief   Generic interface for rendering text.
*
* @author  gernot
* @date    2018-03-17
* @version 1.0
**********************************************************************/
class ITexture
{
public:

  virtual ~ITexture() = default;

};


} // Render

#endif // ITexture_h_INCLUDED