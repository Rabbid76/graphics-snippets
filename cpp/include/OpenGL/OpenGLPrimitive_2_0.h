/******************************************************************//**
* \brief Implementation of OpenGL primitive shader program,
* with the use of very simple and highly optimized shaders,
* for OpenGL version 2.00 and GLSL version 1.10 (`#version 110`).
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/
#pragma once
#ifndef OpenGLPrimitive_2_00_h_INCLUDED
#define OpenGLPrimitive_2_00_h_INCLUDED

// includes

#include <Render_IDrawType.h>
#include <Render_IProgram.h>


// STL


// class definitions


/******************************************************************//**
* \brief General namespace for OpenGL implementation.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
namespace OpenGL
{


/******************************************************************//**
* \brief Implementation of OpenGL polygon shader program,
* with the use of very simple and highly optimized shaders,
* for OpenGL version 2.00 and GLSL version 1.10 (`#version 110`).
*
* The goal of this implementation is to be most efficient and to do the
* minimal requirements, that are necessary for drawing uniform colored
* primitives by the use of OpenGL compatibility profile,
* down to OpenGL version 2.00. 
** 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
class CPrimitiveOpenGL_2_00
{
public:

  CPrimitiveOpenGL_2_00( void );
  virtual ~CPrimitiveOpenGL_2_00();

  bool ActiveSequence( void )  const { return _active_sequence; }
  int  Attrib_xyzw_inx( void ) const { return _attrib_xyzw_inx; } 
  int  Attrib_y_inx( void )    const { return _attrib_y_inx; } 

  //! Initialize the primitive renderer
  virtual void Init( void );

  //! set the primitive color
  CPrimitiveOpenGL_2_00 & SetColor( const Render::TColor & color );
  CPrimitiveOpenGL_2_00 & SetColor( const Render::TColor8 & color );

  //! set depth attenuation
  CPrimitiveOpenGL_2_00 SetDeptAttenuation( Render::t_fp depth_attenuation );

  //! Notify the render that a sequence of successive primitives will follow, that is not interrupted by any other drawing operation.
  //! This allows the render to do some performance optimizations and to prepare for the primitive rendering.
  //! The render can keep states persistent from one primitive drawing to the other, without initializing and restoring them.
  bool StartSuccessivePrimitiveDrawings( void );

  //! Notify the renderer that a sequence of primitives has been finished, and that the internal states have to be restored.
  bool FinishSuccessivePrimitiveDrawings( void );

  //! Start a new primitive sequence
  bool StartSequence( void );
  
  //! Complete an active primitive sequence
  bool EndSequence( void );

  //! activate program, update uniforms and enable vertex attributes
  bool ActivateProgram( bool x_y_case );

  //! install the default program
  bool DeactivateProgram( void );

  //! set style and color parameter uniforms
  CPrimitiveOpenGL_2_00 & UpdateParameterUniforms( void );

  //! install the program object as part of current rendering state
  CPrimitiveOpenGL_2_00 & Use( void );

  //! Trace error message
 virtual void Error( const std::string &kind, const std::string &message );

private:

  static const std::string     _vert_110;                    //!< default vertex shader for consecutive vertex attributes
  static const std::string     _frag_110;                    //!< fragment shader for uniform colored primitives 
                                                                           
  Render::Program::TProgramPtr _prog;                        //!< shader program for consecutive vertex attributes
  int                          _color_loc{ -1 };             //!< uniform location of color  
  int                          _depth_attenuation_loc{ -1 }; //!< uniform location of depth attenuation parameter
  int                          _case_loc{ -1 };              //!< uniform location of attribute case  
  int                          _attrib_xyzw_inx{ -1 };       //!< attribute index of the vertex coordinate 
  int                          _attrib_y_inx{ -1 };          //!< attribute index of the vertex separated y coordinate 
                                                                           
  bool                         _initilized{ false };         //!< initialization flag of the object
  bool                         _successive_drawing{ false }; //!< true: optimized successive drawing of primitives is enabled
  int                          _attribute_case{ 0 };         //!< vertex attribute case for successive drawing of primitives
  Render::TColor               _color{ 1.0f };               //!< uniform color of the pending primitives
  Render::t_fp                 _depth_attenuation = 0.0f;    //!< attenuation of the primitive color by depth

  bool                         _active_sequence{ false };    //!< true: an draw sequence was started, but not finished yet
};


} // OpenGL


#endif // OpenGLPrimitive_2_00_h_INCLUDED

