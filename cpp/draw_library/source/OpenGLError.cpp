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


unsigned int CError::Check( void )
{
  GLenum err = glGetError();
  if ( err != GL_NO_ERROR )
    DebugWarning << "OpenGL ERROR" << err;
  return err;
}


}  // OpenGL
