/***************************************************************************//**
* \brief OpenGL `std140` layout Uniform Block implementations.    
* 
* \author  gernot
* \date    2018-12-17
* \version 1.0
*******************************************************************************/
#pragma once
#ifndef OpenGLDataBuffer_std140_h_INCLUDED
#define OpenGLDataBuffer_std140_h_INCLUDED


// includes


// render

#include <Render_IDrawType.h>


// OpenGL

#include <OpenGL_include.h>
#include <OpenGL_enumconst.h>

// STL

#include <memory>
#include <set>
#include <string>


// class declarations


/******************************************************************//**
* \brief General namespace for OpenGL implementation.  
* 
* \author  gernot
* \date    2018-12-05
* \version 1.0
**********************************************************************/
namespace OpenGL
{


/***************************************************************************//**
* \brief OpenGL `std140` layout Uniform Block implementation for
* model, view and projection matrices, viewport size and depth range. 
*
* The implementation uses the RAII (Resource acquisition is initialization)
* technique.
* [https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization]
*
* \author  gernot
* \date    2018-12-17
* \version 1.0
*******************************************************************************/
class CModelAndViewBuffer_std140
{
public:

  CModelAndViewBuffer_std140( void );
  virtual ~CModelAndViewBuffer_std140();

  CModelAndViewBuffer_std140( const CModelAndViewBuffer_std140 & ) = delete;

  CModelAndViewBuffer_std140 & operator = ( const CModelAndViewBuffer_std140 & ) = delete;

  size_t BufferBinding( void ) const { return _buffer_binding; }

  bool Init( size_t binding );                                    //!< initialization of the object
  bool Init( size_t binding, Render::TModelAndViewPtr data_ptr ); //!< initialization of the object
  void Destroy( void );                                           //!< destroy the object resources

  bool Update( void );                                            //!< Update buffer data

private:

  bool                     _initialized{ false }; //!< initialization state
  Render::TModelAndViewPtr _data;                 //!< data structure 
  size_t                   _buffer_binding{ 0 };  //!< binding point of the buffer
  unsigned int             _buffer_object{ 0 };   //!< named OpenGL buffer object
  unsigned int             _data_stamp{ 0 };      //!< modification and synchronization stamp of the data 
  unsigned int             _model_stamp{ 0 };     //!< modification and synchronization stamp of the model matrix
};



} // OpenGL


#endif OpenGLDataBuffer_std140_h_INCLUDED
