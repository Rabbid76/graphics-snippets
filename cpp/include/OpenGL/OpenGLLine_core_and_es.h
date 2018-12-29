/******************************************************************//**
* \brief Implementation of OpenGL line renderer,
* with the use of "modern" OpenGL 4+ core and
* GLSL version 4.20 (`#version 420 core`) or
* OpenGL es 3(+) and GLSL ES version 3.00 (`#version 300 es`).
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/
#pragma once
#ifndef OpenGLLine_4_h_INCLUDED
#define OpenGLLine_4_h_INCLUDED

// includes

#include <Render_IDrawType.h>
#include <Render_IDrawLine.h>
#include <Render_IProgram.h>


// class definitions


/******************************************************************//**
* \brief General namespace for OpenGL implementation.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
namespace OpenGL
{


/******************************************************************//**
* \brief Namespace for drawing lines with the use of OpenGL.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
namespace Line
{

/******************************************************************//**
* \brief Implementation of OpenGL line renderer,
* with the use of OpenGL core profile
* for OpenGL version 4+ and GLSL version 4.20 (`#version 420 core`)
* or OpenGL es 3(+) and GLSL ES version 3.00 (`#version 300 es`).
*
* Line stippling techniques is implemented in the shader program.
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/
class CLineOpenGL_core_and_es
  : public Render::Line::IRender
{
public: 

  CLineOpenGL_core_and_es( Render::TModelAndViewPtr view_data, size_t min_cache_elems );
  virtual ~CLineOpenGL_core_and_es();

  //! Initialize the line renderer
  virtual bool Init( void ) override;

  //! Notify the render that a sequence of successive lines will follow, that is not interrupted by any other drawing operation.
  //! This allows the render to do some performance optimizations and to prepare for the line rendering.
  //! The render can keep states persistent from one line drawing to the other, without initializing and restoring them.
  virtual bool StartSuccessiveLineDrawings( void ) override;

  //! Notify the renderer that a sequence of lines has been finished, and that the internal states have to be restored.
  virtual bool FinishSuccessiveLineDrawings( void ) override;

  virtual Render::Line::IRender & SetColor( const Render::TColor & color )  override;
  virtual Render::Line::IRender & SetColor( const Render::TColor8 & color ) override;

  virtual Render::Line::IRender & SetStyle( const Render::Line::TStyle & style ) override;
  
  //! Draw a line sequence
  virtual bool Draw( Render::TPrimitive primitive_type, unsigned int tuple_size, size_t coords_size, const float *coords ) override;
  virtual bool Draw( Render::TPrimitive primitive_type, unsigned int tuple_size, size_t coords_size, const double *coords ) override;
  virtual bool Draw( Render::TPrimitive primitive_type, size_t no_of_coords, const float *x_coords, const float *y_coords ) override;
  virtual bool Draw( Render::TPrimitive primitive_type, size_t no_of_coords, const double *x_coords, const double *y_coords ) override;

  //! Start a new line sequence
  virtual bool StartSequence( Render::TPrimitive primitive_type, unsigned int tuple_size ) override;
  
  //! Complete an active line sequence
  virtual bool EndSequence( void ) override;

  //! Specify a new vertex coordinate in an active line sequence
  virtual bool DrawSequence( float x, float y, float z ) override;
  virtual bool DrawSequence( double x, double y, double z ) override;
  
  //! Specify a sequence of new vertex coordinates in an active line sequence
  virtual bool DrawSequence( size_t coords_size, const float *coords ) override;
  virtual bool DrawSequence( size_t coords_size, const double *coords ) override;

protected:

  //! Trace error message
  virtual void Error( const std::string &kind, const std::string &message );

private:

  static const std::string     _line_vert_110;                             //!< default vertex shader for consecutive vertex attributes
  static const std::string     _line_frag_110;                             //!< fragment shader for uniform colored lines 


  Render::TModelAndViewPtr      _view_data;                                 //!< model view and projection, to be applied to the shader program
  Render::Program::TProgramPtr  _line_prog;                                 //!< shader program for consecutive vertex attributes
  int                           _line_color_loc;                            //!< uniform location of color  
  int                           _line_case_loc;                             //!< uniform location of attribute case  
  int                           _line_attrib_xyzw_inx;                      //!< attribute index of the vertex coordinate 
  int                           _line_attrib_y_inx;                         //!< attribute index of the vertex separated y coordinate 
                                                                            
  bool                          _initilized{ false };                       //!< initialization flag of the object
  bool                          _successive_drawing{ false };               //!< true: optimized successive drawing of lines is enabled
  int                           _attribute_case{ 0 };                       //!< vertex attribute case for successive drawing of lines
  Render::TColor                _line_color{ 1.0f };                        //!< uniform color of the pending lines
  Render::Line::TStyle          _line_style;                                //!< line style parameters: line width and stippling
                                                                            
  bool                          _active_sequence{ false };                  //!< true: an draw sequence was started, but not finished yet
  Render::TPrimitive            _squence_type{ Render::TPrimitive::NO_OF }; //!< primitive type pf the sequence
  unsigned int                  _tuple_size{ 0 };                           //!< tuple size (2, 3 or 4) for a sequence
  size_t                        _min_cache_elems{ 0 };                      //!< minimum element size of the sequence cache
  size_t                        _sequence_size{ 0 };                        //!< current size of the sequence cache
  std::vector<float>            _elem_cache;                                //!< the sequence cache - the tuple size of the cache elements is always 3 (x, y, z coordinate)
};


} // Line


} // OpenGL


#endif // OpenGLLine_4_h_INCLUDED
