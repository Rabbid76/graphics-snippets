#pragma once


/******************************************************************//**
* \brief Implementation of OpenGL line renderer,
* with the use of OpenGL core profile
* for OpenGL version 4+ and GLSL version 4.20 (`#version 420 core`)
* or OpenGL es 3(+) and GLSL ES version 3.00 (`#version 300 es`).
*
* The primitive type GL_POLYGONE is substituted by `GL_TRIANGLE_FAN`.
* The primitive types `GL_QUADS` and GL_QUAD_STRIP are emulated by
* `GL_TRIANGLES`.
* 
* \author  gernot
* \date    
* \version 1.0
**********************************************************************/
