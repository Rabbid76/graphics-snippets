/******************************************************************//**
* \brief   Generic interface for mesh data generation.
* 
* \author  gernot
* \date    2017-11-20
* \version 1.0
**********************************************************************/
#pragma once
#ifndef IMeshFactory_h_INCLUDED
#define IMeshFactory_h_INCLUDED


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
* \brief   Generic interface for generating mesh data.
* 
* \author  gernot
* \date    2018-05-29
* \version 1.0
**********************************************************************/
class IMeshFactory
{
public:

  virtual const bool Generate( IMeshData &mesh ) = 0; // read data from file a store data to mesh 
};


// TODO $$$ "meshdef_template.h" move from workbenche to examples

} // Render

#endif // IMeshFactory_h_INCLUDED
