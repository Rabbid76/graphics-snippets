/******************************************************************//**
* \brief Generic interface for a render program data storage.  
* 
* \author  gernot
* \date    2018-08-05
* \version 1.0
**********************************************************************/
#pragma once
#ifndef Render_IDrawBuffer_h_INCLUDED
#define Render_IDrawBuffer_h_INCLUDED


// includes

#include <Render_IDrawType.h>


// stl




/******************************************************************//**
* \brief Namespace for renderer.   
* 
* \author  gerno
* \date    2018-08-05
* \version 1.0
**********************************************************************/
namespace Render
{


/******************************************************************//**
* \brief Generic interface for a render program data buffer or storage.    
* 
* \author  gernot
* \date    2018-08-05
* \version 1.0
**********************************************************************/
class IDataBuffer
{
public:

  ~IDataBuffer() = default;

  // TODO $$$ implementation fro unifrm block and shader storage buffer
};


} // Render


#endif // Render_IDrawBuffer_h_INCLUDED
