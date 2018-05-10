/******************************************************************//**
* \brief   OpenGL error handling
* 
* \author  gernot
* \date    2018-02-04
* \version 1.0
**********************************************************************/

// includes

#include <stdafx.h>

// OpenGL

#include <OpenGLError.h>


// OpenGL wrapper

#include <GL/glew.h>
//#include <GL/gl.h> not necessary because of glew 
#include <GL/glu.h>


// stl

#include <vector>
#include <algorithm>
#include <iostream>



// preprocessor

#ifndef DebugWarning
#define DebugWarning std::cout
#endif


// class implementations


/******************************************************************//**
* \brief General OpenGL namespace
**********************************************************************/
namespace OpenGL
{


//*********************************************************************
// CError
//*********************************************************************

unsigned int CError::Check( void )
{
  GLenum err = glGetError();
  if ( err != GL_NO_ERROR )
    DebugWarning << "OpenGL ERROR " << err;
  return err;
}


//*********************************************************************
// CDebug
//*********************************************************************


/******************************************************************//**
* \brief   ctor
* 
* \author  gernot
* \date    2018-05-10
* \version 1.0
**********************************************************************/
CDebug::CDebug( void )
{}


/******************************************************************//**
* \brief   dtor
* 
* \author  gernot
* \date    2018-05-10
* \version 1.0
**********************************************************************/
CDebug::~CDebug( void )
{
  Destroy();
}


/******************************************************************//**
* \brief   Initilaization of th debug output
*
* Debug Output
* [https://www.khronos.org/opengl/wiki/Debug_Output]
*
*
* `glDebugMessageCallback` specifies a callback to receive debugging
*  messages from the GL.
* [https://www.khronos.org/opengl/wiki/GLAPI/glDebugMessageCallback]
*
* `glGetDebugMessageLog` retrieve messages from the debug message log
* [https://www.khronos.org/opengl/wiki/GLAPI/glGetDebugMessageLog]
*
* `glDebugMessageControl` control the reporting of debug messages
* in a debug context.
* [https://www.khronos.org/opengl/wiki/GLAPI/glDebugMessageControl]
*
* `glDebugMessageInsert` inject an application-supplied message
* into the debug message queue
* [https://www.khronos.org/opengl/wiki/GLAPI/glDebugMessageInsert]
* 
* `glPushDebugGroup` push a named debug group into the command stream
* [https://www.khronos.org/opengl/wiki/GLAPI/glPushDebugGroup]
*
* `glPopDebugGroup` pop the active debug group
* [https://www.khronos.org/opengl/wiki/GLAPI/glPopDebugGroup]
* 
* \author  gernot
* \date    2018-05-10
* \version 1.0
**********************************************************************/
bool CDebug::Init( void )
{
  if ( IsValid() )
    return true;

  // The heart of debug output is a message event.
  // Message events are generated when certain "interesting events" occur within the OpenGL implementation.
  // These message events are provided to the application via a callback function
  // Messages, once generated, can either be stored in a log or passed directly to the application via a callback function.
  // If a callback is registered, then the messages are not stored in a log.

  if ( glDebugMessageCallback != nullptr )
    glDebugMessageCallback( &CDebug::DebugCallback, this );


  // In Debug Contexts, debug output starts enabled.
  // In non-debug contexts, the OpenGL implementation may not generate messages even if debug output is enabled.
  _active = true;

  return IsValid();
}

/******************************************************************//**
* \brief   Reset debug callback.
* 
* \author  gernot
* \date    2018-05-10
* \version 1.0
**********************************************************************/
void CDebug::Destroy( void )
{
  if ( IsValid() == false )
    return;

  glDebugMessageCallback( nullptr, nullptr );
}


/******************************************************************//**
* \brief   Activate the debug output.
* 
* \author  gernot
* \date    2018-05-10
* \version 1.0
**********************************************************************/
bool CDebug::Activate( 
  bool synchronous ) //!< true: enable synchronous output
{
  if ( IsValid() == false )
    return false;

  if ( IsActive() == false )
    EnableOutput( true );
  if ( IsSynchronous() != synchronous )
    EnableSynchronous( synchronous );

  _active = true;
  _synchronous = true;
  return true;
}


 /******************************************************************//**
* \brief   Decativate the debug output
* 
* \author  gernot
* \date    2018-05-10
* \version 1.0
**********************************************************************/
bool CDebug::Deactivate( void )
{
  if ( IsValid() == false )
    return false;

  if ( IsActive() )
    EnableOutput( false );

  _active = false;
  return true;
}


/******************************************************************//**
* \brief   Enable or disable debug messages.
*
* If enabled, debug messages are produced by a debug context.
* When disabled, the debug message log is silenced.
* Note that in a non-debug context, very few, if any messages might be produced,
* even when `GL_DEBUG_OUTPUT` is enabled.
*
* \author  gernot
* \date    2018-05-10
* \version 1.0
**********************************************************************/
void CDebug::EnableOutput( 
  bool enable //!< I - true: enable; false: disable
  ) const
{
  if ( enable )
    glEnable( GL_DEBUG_OUTPUT );
  else
    glDisable( GL_DEBUG_OUTPUT );
}


/******************************************************************//**
* \brief   Enable or disable synchronouse debug messaging.
*
* If enabled, debug messages are produced synchronously by a debug context.
* If disabled, debug messages may be produced asynchronously.
* In particular, they may be delayed relative to the execution of GL commands,
* and the debug callback function may be called from a thread other
* than that in which the commands are executed.
* See `glDebugMessageCallback`
* [https://www.khronos.org/opengl/wiki/GLAPI/glDebugMessageCallback].
* 
* \author  gernot
* \date    2018-05-10
* \version 1.0
**********************************************************************/
void CDebug::EnableSynchronous( 
  bool enable //!< I - true: enable; false: disable
  ) const
{
  if ( enable )
    glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
  else
    glDisable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
}


/******************************************************************//**
* \brief   Debug callback function.  
* 
* \author  gernot
* \date    2018-05-10
* \version 1.0
**********************************************************************/
void CDebug::DebugCallback( 
  unsigned int  source,     //!< I - 
  unsigned int  type,       //!< I - 
  unsigned int  id,         //!< I - 
  unsigned int  severity,   //!< I - 
  int           length,     //!< I - length of debug message
  const char   *message,    //!< I - debug message
  const void   *userParam ) //!< I - user parameter
{
  CDebug *debugObj = (CDebug*)userParam;
  debugObj->DebugCallback( source, type, id, severity, length, message );
}


/******************************************************************//**
* \brief   Debug callback function.  
* 
* \author  gernot
* \date    2018-05-10
* \version 1.0
**********************************************************************/
void CDebug::DebugCallback( 
  unsigned int  source,   //!< I - 
  unsigned int  type,     //!< I - 
  unsigned int  id,       //!< I - 
  unsigned int  severity, //!< I - 
  int           length,   //!< I - length of debug message
  const char   *message ) //!< I - debug message
{
  static const std::vector<GLenum> error_ids
  {
    GL_INVALID_ENUM,
    GL_INVALID_VALUE,
    GL_INVALID_OPERATION,
    GL_STACK_OVERFLOW,
    GL_STACK_UNDERFLOW,
    GL_OUT_OF_MEMORY
  };

  auto error_it = std::find( error_ids.begin(), error_ids.end(), id );
  if ( error_it != error_ids.end() )
  {
    std::cout << message << std::endl;
  }
  else
  {
    std::cout << message << std::endl;
  }
  // TODO $$$
}


}  // OpenGL
