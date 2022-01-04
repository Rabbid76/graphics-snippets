/******************************************************************//**
* \brief   General type declarations for drawing parameters.
* 
* \author  gernot
* \date    2017-11-20
* \version 1.0
**********************************************************************/
#pragma once
#ifndef Render_GLM_h_INCLUDED
#define Render_GLSRender_GLM_h_INCLUDEDL_h_INCLUDED


// includes

#include "Render_GLSL.h"

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// STL

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
* \brief Namespace for data excahnge to GLM library 
* 
* \author  gernot
* \date    2018-09-03
* \version 1.0
**********************************************************************/
namespace GLM
{

struct CMat4
{
  GLSL::mat4 &_m;

  CMat4( GLSL::mat4 &m ) : _m( m ) {}

  GLSL::mat4 & operator = (const glm::mat4 &source)
  {
    std::copy( glm::value_ptr(source), glm::value_ptr(source)+16, (float*)(&_m) );
    return _m;
  }
};

} // GLM

} // Render


#endif // Render_GLM_h_INCLUDED
