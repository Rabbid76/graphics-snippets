/******************************************************************//**
* \brief   Generic interface for mesh data array access.
* 
* \author  gernot
* \date    2017-11-20
* \version 1.0
**********************************************************************/
#pragma once
#ifndef IMesh_h_INCLUDED
#define IMesh_h_INCLUDED


// includes

#include <IDrawType.h>


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
* \brief   Generic interface for mesh data.
* 
* \author  gernot
* \date    2018-05-29
* \version 1.0
**********************************************************************/
class IMeshData
{
public:

};


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


/******************************************************************//**
* \brief   Generic interface for mesh data file.
* 
* \author  gernot
* \date    2018-05-29
* \version 1.0
**********************************************************************/
class IMeshResource
{
public:

  virtual const bool Load( IMeshData &mesh ) = 0; // read data from file a store data to mesh 
};


} // Render

#endif // IMeshData_h_INCLUDED
