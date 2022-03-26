#pragma once
#ifndef __gl_opengl_loader_h_
#define __gl_opengl_loader_h_

// GLEW [http://glew.sourceforge.net/]
#include <GL/glew.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#endif