/******************************************************************//**
* \brief   OpenGL header files.
* 
* \author  gernot
* \date    2018-02-04
* \version 1.0
**********************************************************************/
#pragma once
#ifndef OpenGL_include_h_INCLUDED
#define OpenGL_include_h_INCLUDED


// OpenGL wrapper

#include <GL/glew.h>
//#include <GL/gl.h> not necessary because of GLEW 
#include <GL/glu.h>

#include <cassert>

#ifndef ASSERT
#define ASSERT(C) assert(C)
#endif

#include <OpenGLError.h>


#endif // OpenGL_include_h_INCLUDED
