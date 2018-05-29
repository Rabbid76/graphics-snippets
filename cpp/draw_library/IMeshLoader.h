/******************************************************************//**
* \brief   Generic interface for mesh data loading.
* 
* \author  gernot
* \date    2017-11-20
* \version 1.0
**********************************************************************/
#pragma once
#ifndef IMeshLoader_h_INCLUDED
#define IMeshLoader_h_INCLUDED


// includes


#include <IDrawType.h>
#include <IMeshData.h>


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
* \brief   Generic interface for mesh data file.
* 
* \author  gernot
* \date    2018-05-29
* \version 1.0
**********************************************************************/
class IMeshFile
{
public:

  virtual const bool Load( IMeshData &mesh ) = 0; // read data from file a store data to mesh 
};


} // Render

#endif // IMeshLoader_h_INCLUDED
