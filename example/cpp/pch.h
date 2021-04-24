#ifndef __PCH_H__
#define __PCH_H__

#define _CRT_SECURE_NO_WARNINGS
#include <wx/wx.h>
#undef _CRT_SECURE_NO_WARNINGS

#include <GL/glew.h>
#ifdef __WXMAC__
#include "OpenGL/glu.h"
#include "OpenGL/gl.h"
#else
#include <GL/glu.h>
#include <GL/gl.h>
#endif

// glm
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#endif