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
* @brief   Generic interface for text resource data.
*
* @author  gernot
* @date    2018-03-17
* @version 1.0
**********************************************************************/
class ITextureData
{
public:


};


/******************************************************************//**
* @brief   Generic interface loading a texture from a resource.
*
* @author  gernot
* @date    2018-03-17
* @version 1.0
**********************************************************************/
class ITextureDataLoader
{
public:


};


/******************************************************************//**
* @brief   Generic interface render texture.
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


/******************************************************************//**
* @brief   Generic interface for lading a render texture from a
*          resource.
*
* @author  gernot
* @date    2018-03-17
* @version 1.0
**********************************************************************/
class ITextureLoader
{
public:

  virtual ~ITextureLoader() = default;

};



} // Render

#endif // ITexture_h_INCLUDED