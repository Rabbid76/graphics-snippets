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


// preprocess definitions

#if !defined(__FRAMBUFFER_DEBUG_ERROR_CHECK__)
#define __FRAMBUFFER_DEBUG_ERROR_CHECK__
#endif

#if defined(_DEBUG) && defined(__FRAMBUFFER_DEBUG_ERROR_CHECK__)
#define OPENGL_CHECK_GL_ERROR OpenGL::CError().Check();
#else
#define OPENGL_CHECK_GL_ERROR
#endif


// class declarations

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


}  // OpenGL

#endif // OpenGLError_h_INCLUDED