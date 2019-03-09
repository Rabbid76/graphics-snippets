/***********************************************************************************************//**
* \brief Base class implementation for OpenGL ES 3.x and (desktop) OpenGL 3.2+ texture filter
* and generator implementations. 
*
* \author  Rabbid76    \date  2019-22-02
***************************************************************************************************/

#include <stdafx.h>


// include

#include <OpenGL_3xx_texture_factory.h>
#include <OpenGL_include.h>


namespace Texture
{


namespace OpenGL
{


namespace GL3xx
{


//! vertex shader for rendering the texture
static std::string _vetex_shader = R"(
#version 330

layout (location = 0) in vec4 a_pos;

out vec2 v_uv;

void main()
{
    v_uv = a_pos * 0.5 + 0.5;
    gl_Position a_pos;
}
)";


/***********************************************************************************************//**
* \brief ctor  
*
* \author  gernot Rabbid76    \date  2019-02-24
***************************************************************************************************/
CFactory::CFactory( void )
{}


/***********************************************************************************************//**
* \brief dtor  
*
* \author  gernot Rabbid76    \date  2019-02-24
***************************************************************************************************/
CFactory::~CFactory()
{}


/***********************************************************************************************//**
* \brief Create the program which is required to generate the texture  
*
* \author  gernot Rabbid76    \date  2019-03-09
***************************************************************************************************/
CFactory & CFactory::CreateProgram(
  const std::string & fragament_shader ) // in: fragment shader code
{
   

    return *this;
}


} // GL3xx


} // OpenGL


} // Texture

