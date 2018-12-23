/******************************************************************//**
* \brief Implementation of OpenGL primitive renderer,
* with the use of "modern" OpenGL 4+ core and
* GLSL version 4.30 (`#version 430 core`) or
* OpenGL es 3(+) and GLSL ES version 3.00 (`#version 300 es`).
* 
* \author  gernot
* \date    2018-12-10
* \version 1.0
**********************************************************************/
#pragma once
#ifndef OpenGLPrimitive_core_and_es_h_INCLUDED
#define OpenGLPrimitive_core_and_es_h_INCLUDED

// includes

#include <Render_IDrawType.h>
#include <Render_IProgram.h>
#include <Render_IBuffer.h>


// STL


// class definitions


/******************************************************************//**
* \brief General namespace for OpenGL implementation.  
* 
* \author  gernot
* \date    2018-12-10
* \version 1.0
**********************************************************************/
namespace OpenGL
{

class CModelAndViewBuffer_std140;


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
* \date    2018-12-10
* \version 1.0
**********************************************************************/
class CPrimitiveOpenGL_core_and_es
{
public:

  using TDrawBufferPtr = std::shared_ptr<Render::IDrawBuffer>;
  using TMVPBufferPtr  = std::shared_ptr<CModelAndViewBuffer_std140>;


  CPrimitiveOpenGL_core_and_es( void );
  virtual ~CPrimitiveOpenGL_core_and_es();

  bool SuccessiveDrawing( void ) const { return _successive_drawing; }
  bool ActiveSequence( void )    const { return _active_sequence; }
  int  Attrib_xyzw_inx( void )   const { return _attrib_xyzw_inx; } 
  int  Attrib_y_inx( void )      const { return _attrib_y_inx; } 

  Render::IDrawBuffer* DrawBuffer( void ) const { return _mesh_buffer.get(); }
  TMVPBufferPtr        MVPBuffer( void ) { return _mvp_buffer; }

  //! Initialize the primitive renderer
  virtual void Init( size_t min_buffer_size, TMVPBufferPtr mvp_buffer );

  //! set the primitive color
  CPrimitiveOpenGL_core_and_es & SetColor( const Render::TColor & color );
  CPrimitiveOpenGL_core_and_es & SetColor( const Render::TColor8 & color );

  //! set depth attenuation
  CPrimitiveOpenGL_core_and_es SetDeptAttenuation( Render::t_fp depth_attenuation );

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
  bool ActivateProgram( Render::TVA requested_va_type );

  //! install the default program
  bool DeactivateProgram( void );

  //! set style and color parameter uniforms
  CPrimitiveOpenGL_core_and_es & UpdateParameterUniforms( void );

  //! install the program object as part of current rendering state
  CPrimitiveOpenGL_core_and_es & Use( void );

  //! Trace error message
 virtual void Error( const std::string &kind, const std::string &message );

private:

  void InitMVPBuffer( TMVPBufferPtr mvp_buffer );
  void InitDrawBuffer( size_t min_buffer_size );
  void InitProgram( void );

  static const std::string     _vert_430;                    //!< default vertex shader for consecutive vertex attributes
  static const std::string     _frag_430;                    //!< fragment shader for uniform colored primitives 
                                                                           
  TMVPBufferPtr                _mvp_buffer;                  //!< model, view, projection and viewport data
  Render::Program::TProgramPtr _prog;                        //!< shader program for consecutive vertex attributes
  int                          _color_loc{ -1 };             //!< uniform location of color  
  int                          _depth_attenuation_loc{ -1 }; //!< uniform location of depth attenuation parameter
  int                          _case_loc{ -1 };              //!< uniform location of attribute case  
  int                          _attrib_xyzw_inx{ -1 };       //!< attribute index of the vertex coordinate 
  int                          _attrib_y_inx{ -1 };          //!< attribute index of the vertex separated y coordinate 
  
  Render::TVA                  _va_type{ Render::unknown };  //!< vertex array object type
  TDrawBufferPtr               _mesh_buffer;                 //!< vertex array object and array buffer 
  size_t                       _min_buffer_size;             //!< minimum size of the vertex buffer

  bool                         _initilized{ false };         //!< initialization flag of the object
  bool                         _successive_drawing{ false }; //!< true: optimized successive drawing of primitives is enabled
  int                          _attribute_case{ 0 };         //!< vertex attribute case for successive drawing of primitives
  Render::TColor               _color{ 1.0f };               //!< uniform color of the pending primitives
  Render::t_fp                 _depth_attenuation = 0.0f;    //!< attenuation of the primitive color by depth

  bool                         _active_sequence{ false };    //!< true: an draw sequence was started, but not finished yet
};


} // OpenGL


#endif // OpenGLPrimitive_core_and_es_h_INCLUDED

