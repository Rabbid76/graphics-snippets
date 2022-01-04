/******************************************************************//**
* @brief   Generic interface for debugging graphics output.
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
#pragma once
#ifndef Render_IGraphicsDebug_h_INCLUDED
#define Render_IGraphicsDebug_h_INCLUDED


// includes

#include "Render_IDrawType.h"


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

enum class TDebugLevel
{
  all,
  error_only
};


/******************************************************************//**
* @brief   Generic interface for debugging graphics output.
*
* @author  gernot
* @date    2018-03-17
* @version 1.0
**********************************************************************/
class IDebug
{
public: 

  virtual ~IDebug() = default;

  virtual bool IsValid( void ) const = 0;        //!< is valid
  virtual bool IsActive( void ) const = 0;       //!< is active
  virtual bool IsSynchronous( void ) const = 0;  //!< is synchronous output

  virtual bool Init( TDebugLevel level ) = 0;    //!< initialize the debug output
  virtual void Destroy( void ) = 0;              //!< destroy all internal objects and cleanup
  virtual bool Activate( bool synchronous ) = 0; //!< activate the debug output
  virtual bool Deactivate( void ) = 0;           //!< deactivate the debug output
  
};


} // Render

#endif // Render_ITexture_h_INCLUDED
