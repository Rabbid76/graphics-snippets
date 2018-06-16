/******************************************************************//**
* \brief   Obj-File loader implementation.
*
* [https://de.wikipedia.org/wiki/Wavefront_OBJ]
* [https://en.wikipedia.org/wiki/Wavefront_.obj_file]
* [https://www.cs.utah.edu/~boulos/cs3505/obj_spec.pdf]
* 
* \author  gernot
* \date    2017-11-20
* \version 1.0
**********************************************************************/
#pragma once
#ifndef RenderUtil_ObjLoader_h_INCLUDED
#define RenderUtil_ObjLoader_h_INCLUDED


// includes

#include <Render_IMesh.h>


/******************************************************************//**
* @brief   Namespace for renderer.
*
* @author  gernot
* @date    2018-03-17
* @version 1.0
**********************************************************************/
namespace Render
{



/******************************************************************//**
* \brief   Implementation of obj-file loader.
* 
* \author  gernot
* \date    2018-05-29
* \version 1.0
**********************************************************************/
class CObjFileLoader
  : public IMeshResource
{
public:

};



} // Render

#endif // RenderUtil_ObjLoader_h_INCLUDED
