/******************************************************************//**
* \brief Implementation of OpenGL line renderer,
* with the use of very simple and highly optimized shaders,
* for OpenGL version 2.00 and GLSL version 1.10 (`#version 110`).
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/
#pragma once
#ifndef OpenGLLine_2_00_h_INCLUDED
#define OpenGLLine_2_00_h_INCLUDED

// includes

#include <Render_IDrawType.h>
#include <Render_IDrawLine.h>
#include <Render_IProgram.h>


// STL

#include <string>
#include <vector>


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
* with the use of very simple and highly optimized shaders,
* for OpenGL version 2.00 and GLSL version 1.10 (`#version 110`).
*
* The goal of this implementation is to be most efficient and to do the
* minimal requirements, that are necessary for drawing uniform colored
* and stippled lines by the use of OpenGL compatibility profile,
* down to OpenGL version 2.00. 
*
* `glBegin` \ `glEnd` sequences are emulated and by caching the vertices
* to a buffer, which stays persistent and is initialized by a minimum
* size provided to the constructor.
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/
class CLineOpenGL_2_00
  : public Render::Line::IRender
{
public:

  CLineOpenGL_2_00( size_t min_cache_elems );
  virtual ~CLineOpenGL_2_00();

  const Render::Line::TStyle & LineStyle( void ) const { return _line_style; }
  void LineStyle( const Render::Line::TStyle &style ) { _line_style = style; }

  //! Initialize the line renderer
  virtual void Init( void ) override;

  //! Notify the render that a sequence of successive lines will follow, that is not interrupted by any other drawing operation.
  //! This allows the render to do some performance optimizations and to prepare for the line rendering.
  //! The render can keep states persistent from one line drawing to the other, without initializing and restoring them.
  virtual bool StartSuccessiveLineDrawings( void ) override;

  //! Notify the renderer that a sequence of lines has been finished, and that the internal states have to be restored.
  virtual bool FinishSuccessiveLineDrawings( void ) override;

  virtual Render::Line::IRender & SetColor( const Render::TColor & color )  override;
  virtual Render::Line::IRender & SetColor( const Render::TColor8 & color ) override;

  virtual Render::Line::IRender & SetStyle( const Render::Line::TStyle & style ) override;
  virtual Render::Line::IRender & SetArrowStyle( const Render::Line::TArrowStyle & style ) override;

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

  //! set style and color parameter uniforms
  void UpdateParameterUniforms( void );

  //! Trace error message
  virtual void Error( const std::string &kind, const std::string &message );

private:

  static const std::string     _line_vert_110;                             //!< default vertex shader for consecutive vertex attributes
  static const std::string     _line_frag_110;                             //!< fragment shader for uniform colored lines 
                                                                           
  Render::Program::TProgramPtr _line_prog;                                 //!< shader program for consecutive vertex attributes
  int                          _line_color_loc{ -1 };                      //!< uniform location of color  
  int                          _line_depth_attenuation_loc{ -1 };          //!< uniform location of depth attenuation parameter
  int                          _line_case_loc{ -1 };                       //!< uniform location of attribute case  
  int                          _line_attrib_xyzw_inx{ -1 };                //!< attribute index of the vertex coordinate 
  int                          _line_attrib_y_inx{ -1 };                   //!< attribute index of the vertex separated y coordinate 
                                                                           
  bool                         _initilized{ false };                       //!< initialization flag of the object
  bool                         _successive_drawing{ false };               //!< true: optimized successive drawing of lines is enabled
  int                          _attribute_case{ 0 };                       //!< vertex attribute case for successive drawing of lines
  Render::TColor               _line_color{ 1.0f };                        //!< uniform color of the pending lines
  Render::Line::TStyle         _line_style;                                //!< line style parameters: line width and stippling
                                                                           
  bool                         _active_sequence{ false };                  //!< true: an draw sequence was started, but not finished yet
  Render::TPrimitive           _squence_type{ Render::TPrimitive::NO_OF }; //!< primitive type pf the sequence
  unsigned int                 _tuple_size{ 0 };                           //!< tuple size (2, 3 or 4) for a sequence
  size_t                       _min_cache_elems{ 0 };                      //!< minimum element size of the sequence cache
  size_t                       _sequence_size{ 0 };                        //!< current size of the sequence cache
  std::vector<float>           _elem_cache;                                //!< the sequence cache - the tuple size of the cache elements is always 3 (x, y, z coordinate)
};


} // Line


} // OpenGL


#endif // OpenGLLine_2_00_h_INCLUDED
