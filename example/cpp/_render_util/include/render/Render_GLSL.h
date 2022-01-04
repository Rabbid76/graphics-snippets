/******************************************************************//**
* \brief   General type declarations for drawing parameters.
* 
* \author  gernot
* \date    2017-11-20
* \version 1.0
**********************************************************************/
#pragma once
#ifndef Render_GLSL_h_INCLUDED
#define Render_GLSL_h_INCLUDED


// includes


#include <cstddef>
#include <cmath>
#include <bitset>
#include <array> 
#include <vector>
#include <tuple>


/******************************************************************//**
* \brief Namespace for generic interfaces for drawing operations.  
* 
* \author  gernot
* \date    2018-09-03
* \version 1.0
**********************************************************************/
namespace Render
{


/******************************************************************//**
* \brief Namespace for GLSL compatible data types  
* 
* \author  gernot
* \date    2018-09-03
* \version 1.0
**********************************************************************/
namespace GLSL
{

using vec2 = std::array<float, 4>;
using vec3 = std::array<float, 4>;
using vec4 = std::array<float, 4>;
using mat2 = std::array<vec2, 2>;
using mat3 = std::array<vec3, 3>;
using mat4 = std::array<vec4, 4>;

} // GLSL

} // Render


#endif // Render_GLSL_h_INCLUDED
