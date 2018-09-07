/******************************************************************//**
* \brief   OpenGL error handling
* 
* \author  gernot
* \date    2018-02-04
* \version 1.0
**********************************************************************/
#pragma once
#ifndef OpenGLError_h_INCLUDED
#define OpenGLError_h_INCLUDED


// includes

#include <Render_IGraphicsDebug.h>

#include <functional> 

// preprocessor definitions

#if !defined(__FRAMBUFFER_DEBUG_ERROR_CHECK__)
//#define __FRAMBUFFER_DEBUG_ERROR_CHECK__
#endif

#if defined(_DEBUG) && defined(__FRAMBUFFER_DEBUG_ERROR_CHECK__)
#define OPENGL_CHECK_GL_ERROR OpenGL::CError().Check();
#else
#define OPENGL_CHECK_GL_ERROR
#endif


// class declarations


/******************************************************************//**
* \brief General namespace for OpenGL implementation.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
namespace OpenGL
{


//*********************************************************************
// CError
//*********************************************************************

class CError
{
public: 

  unsigned int Check( void );
};


//*********************************************************************
// CDebug
//*********************************************************************


/******************************************************************//**
* \brief   OpenGL debug call back
* 
* Debug Output
* [https://www.khronos.org/opengl/wiki/Debug_Output]
* 
* \author  gernot
* \date    2018-05-10
* \version 1.0
**********************************************************************/
class CDebug
  : public Render::IDebug
{
public:

  typedef void (*TDebugProcPtr)(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message, const void* userParam);
  using TDebugCBSinganture = std::remove_pointer< TDebugProcPtr >::type;


  CDebug( void );
  virtual ~CDebug( void );

  virtual bool IsValid( void ) const override { return _valid; }
  virtual bool IsActive( void ) const override { return _active; }
  virtual bool IsSynchronous( void ) const override { return _synchronous; }

  virtual bool Init( Render::TDebugLevel level ) override; //!< initialize the debug output
  virtual void Destroy( void ) override;                   //!< destroy all internal objects and cleanup
  virtual bool Activate( bool synchronous ) override;      //!< activate the debug output
  virtual bool Deactivate( void ) override;                //!< deactivate the debug output

private:

  void EnableOutput( bool enable ) const;
  void EnableSynchronous( bool enable ) const;

  // debug callback function
  static void DebugCallback( unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message, const void* userParam );
  void DebugCallback( unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message );

  bool                _valid       = false;                    //!< valid and initialized
  bool                _active      = false;                    //!< debug output is valid
  bool                _synchronous = false;                    //!< debug output is synchronous
  Render::TDebugLevel _level       = Render::TDebugLevel::all; //!< level of debug messages
};

}  // OpenGL

#endif // OpenGLError_h_INCLUDED