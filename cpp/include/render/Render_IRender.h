/******************************************************************//**
* @brief   Generic interface for render.
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
#pragma once
#ifndef Render_IRender_h_INCLUDED
#define Render_IRender_h_INCLUDED


// includes

#include <Render_IDrawType.h>
#include <Render_ITexture.h>
#include <Render_IBuffer.h>


// type declarations


/*XEOMETRIC********************************************************//**
* @brief   Namespace for render tables.
*
* @author  gernot76
* @date    2018-02-06
* @version 1.0
**********************************************************************/
namespace Render
{


/******************************************************************//**
* \brief   Provider collection.
* 
* \author  gernot
* \date    2018-06-15
* \version 1.0
**********************************************************************/
class IProvider
  : public IDrawBufferProvider
  // TODO $$$
{
};


}; // Render


#endif // Render_IRender_h_INCLUDED