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
* \brief   Generic interface for attribute data.
* 
* \author  gernot
* \date    2018-06-13
* \version 1.0
**********************************************************************/
template<typename T_INDEX, typename T_DATA>
class IAttributeData
{
public:

  virtual const int       AttribSize( void ) const = 0;
  virtual const size_t    Count( void ) const = 0;
  virtual const T_DATA *  Data( void )  const = 0;

  virtual const size_t    IndexCount( void ) const = 0;
  virtual const T_INDEX * IndexData( void ) const = 0;
};


/******************************************************************//**
* \brief   Generic interface for mesh data.
* 
* \author  gernot
* \date    2018-05-29
* \version 1.0
**********************************************************************/
template<typename T_INDEX, typename T_DATA>
class IMeshData
{
public:

  enum class TIndexKind { non, common, single };
  enum class TNormalKins { non, face, vertex };
  
  // TODO $$$ ttransformer class
  virtual TNormalKind NormalKind( void ) const = 0;
  virtual bool TransformFaceNormalsToVertexNormals( void ) = 0;

  virtual TIndexKind IndexKind( void ) const = 0;

  // TODO $$$ transformer class
  virtual bool TransformMultiIndexToSingleIndex( void ) = 0;  //! if the attributes have different indices, the transform the attributes to arrays with common indices.
  virtual bool TransformIndexBufferToArrayBuffer( void ) = 0; //! get rid of all indices- transform to an attribute array, where all vertices of the faces are in consecutive order. 

  virtual IAttributeData & Verices( void ) const = 0;
  virtual IAttributeData * Normals( void ) const = 0;
  virtual int              FaceSize( void ) const = 0;
  virtual IAttributeData * FaceNormals( void ) const = 0;
  virtual IAttributeData * TextureCoors( void ) const = 0;
  virtual IAttributeData * Colors( void ) const = 0;
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

  // virtual const bool Generate( IMeshData &mesh ) = 0; // read data from file a store data to mesh 
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

  // virtual const bool Load( IMeshData &mesh ) = 0; // read data from file a store data to mesh 
};


} // Render

#endif // IMeshData_h_INCLUDED
