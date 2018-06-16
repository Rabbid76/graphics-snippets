/******************************************************************//**
* @brief   Generic interface for debuging graphics output.
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
#pragma once
#ifndef Render_IGraphicsDebug_h_INCLUDED
#define Render_IGraphicsDebug_h_INCLUDED


// includes

#include <Render_IDrawType.h>


/******************************************************************//**
* @brief   Namespace for renderer.
*
* @author  gernot
* @date    2018-05-10
* @version 1.0
**********************************************************************/
namespace Render
{


//---------------------------------------------------------------------
// IFont
//---------------------------------------------------------------------


/******************************************************************//**
* @brief   Generic interface for debuging graphics output.
*
* @author  gernot
* @date    2018-03-17
* @version 1.0
**********************************************************************/
class IDebug
{
public: 

  virtual ~IDebug() = default;

  virtual bool IsValid( void ) const = 0;        //!< is valide
  virtual bool IsActive( void ) const = 0;       //!< is active
  virtual bool IsSynchronous( void ) const = 0;  //!< is synchronous output

  virtual bool Init( void ) = 0;                 //!< init the debug output
  virtual void Destroy( void ) = 0;              //!< destroy all internal objects and cleanup
  virtual bool Activate( bool synchronous ) = 0; //!< activate the debug output
  virtual bool Deactivate( void ) = 0;           //!< decativate the debug output
  
};


} // Render

#endif // Render_ITexture_h_INCLUDED
