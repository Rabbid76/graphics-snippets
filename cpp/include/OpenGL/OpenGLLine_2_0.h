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
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/
class CLineOpenGL_2_00
  : public Render::Line::IRender
{
public:

  CLineOpenGL_2_00( void );
  virtual ~CLineOpenGL_2_00();

  //! Initialize the line renderer
  virtual void Init( void ) override;

  virtual Render::Line::IRender & SetColor( const Render::TColor & color ) override;
  virtual Render::Line::IRender & SetColor( const Render::TColor8 & color ) override;
  virtual Render::Line::IRender & SetStyle( const Render::Line::TStyle & style ) override;
  virtual Render::Line::IRender & SetArrowStyle( const Render::Line::TArrowStyle & style ) override;

  //! Draw a line sequence
  virtual bool Draw( Render::TPrimitive primitive_type, unsigned int tuple_size, size_t coords_size, const float *coords ) override;
  virtual bool Draw( Render::TPrimitive primitive_type, unsigned int tuple_size, size_t coords_size, const double *coords ) override;
  virtual bool Draw( Render::TPrimitive primitive_type, size_t no_of_coords, const float *x_coords, const float *y_coords ) override;
  virtual bool Draw( Render::TPrimitive primitive_type, size_t no_of_coords, const double *x_coords, const double *y_coords ) override;

  //! Start a new line sequence
  virtual bool StartSequence( Render::TPrimitive primitive_type ) override;
  
  //! Complete an active line sequence
  virtual bool EndSequence( void ) override;

  //! Specify a new vertex coordinate in an active line sequence
  virtual bool DrawSequence( float x, float y, float z ) override;
  virtual bool DrawSequence( double x, double y, double z ) override;
  
  //! Specify a sequence of new vertex coordinates in an active line sequence
  virtual bool DrawSequence( unsigned int tuple_size, size_t coords_size, const float *coords ) override;
  virtual bool DrawSequence( unsigned int tuple_size, size_t coords_size, const double *coords ) override;

protected:

  //! Trace error message
  virtual void Error( const std::string &kind, const std::string &message );

private:

  static const std::string _line_vert_110;             //!< default vertex shader for consecutive vertex attributes
  static const std::string _line_x_y_vert_110;         //!< vertex shader for separated arrays of x and y coordinates
  static const std::string _line_frag_110;             //!< fragment shader for uniform colored lines 
  static const std::string _line_stipple_vert_110;     //!< default vertex shader for consecutive vertex attributes and line stippling
  static const std::string _line_stipple_x_y_vert_110; //!< vertex shader for separated arrays of x and y coordinates and line stippling
  static const std::string _line_stipple_frag_110;     //!< fragment shader for uniform colored lines and line stippling

  bool _initilized = false;                            //!< initialization flag of the object

  Render::Program::TProgramPtr _line_prog;             //!< shader program for consecutive vertex attributes
  int                          _line_color_loc;        //!< uniform location of color  
  
  Render::Program::TProgramPtr _line_x_y_prog;         //!< shader program for separated arrays of x and y coordinates
  int                          _line_x_y_color_loc;    //!< uniform location of color  
  int                          _line_x_attrib_inx;     //!< attribute index of x coordinate 
  int                          _line_y_attrib_inx;     //!< attribute index of y coordinate 
  
  Render::Program::TProgramPtr _line_stipple_prog;     //!< shader program for uniform colored lines
  
  Render::Program::TProgramPtr _line_stipple_x_y_prog; //!< shader program for uniform colored lines and line stippling


  // TODO : comments
  // The implementation is based on the OpenGL X.XX compatibility mode specification [link] - line tipple etc.
  bool _active_sequence = false;
};


} // Line


} // OpenGL


#endif // OpenGLLine_2_00_h_INCLUDED
