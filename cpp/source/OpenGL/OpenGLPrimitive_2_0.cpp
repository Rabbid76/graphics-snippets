/******************************************************************//**
* \brief Implementation of OpenGL primitive shader program,
* with the use of very simple and highly optimized shaders,
* for OpenGL version 2.00 and GLSL version 1.10 (`#version 110`).
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/

#include <stdafx.h>

// includes

#include <OpenGLPrimitive_2_0.h>
#include <OpenGLProgram.h>


// OpenGL wrapper

#include <OpenGL_include.h>
#include <OpenGL_enumconst.h>


// STL

#include <iostream>
#include <algorithm>


// preprocessor definitions

#if defined(max)
#undef max
#endif

#if defined(min)
#undef min
#endif


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


/*!

Vertex attributes:

- case 1: 1 attribute with 2 to 4 components: x, y, z, w coordinates

- case 2: 2 attributes, which are: 1. x coordinates; 2. y coordinates

It is more efficient, to combine the different vertex attribute representations in one vertex shader and to switch between them by uniform conditions,
instead of using 2 different vertex shaders, which would lead to 2 different shader programs.
Since the vertex shader is executed per vertex only, one additional uniform condition should not gain a notifiable performance loss.
Note, with a modern hardware, a uniform condition would be evaluated once and different code dependent on the uniform would be generated and executed.
But this would give the chance to avoid shader program switches, which probably are more time consuming operations. 

*/

const std::string CPrimitiveOpenGL_2_00::_vert_110 = R"(
#version 110

attribute vec4  attr_xyzw;
attribute float attr_y;

varying vec4 v_color;

uniform int   u_attr_case;
uniform vec4  u_color;
uniform float u_depth_att;

void main()
{
  vec4 vert_pos = u_attr_case == 0 ? attr_xyzw : vec4(attr_xyzw.x, attr_y, 0.0, 1.0);
  gl_Position   = gl_ModelViewProjectionMatrix * vert_pos;
  gl_ClipVertex = gl_Position;

  float depth   = 0.5 + 0.5 * gl_Position.z / gl_Position.w;
  v_color       = vec4( mix(u_color.rgb, vec3(0.0), depth * u_depth_att), u_color.a );
}
)";


const std::string CPrimitiveOpenGL_2_00::_frag_110 = R"(
#version 110

varying vec4 v_color;

void main()
{
  gl_FragColor = v_color;
}
)";


/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
CPrimitiveOpenGL_2_00::CPrimitiveOpenGL_2_00( void )
{}


/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
CPrimitiveOpenGL_2_00::~CPrimitiveOpenGL_2_00()
{}


/******************************************************************//**
* \brief Trace error message.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
void CPrimitiveOpenGL_2_00::Error( 
  const std::string &kind,     //!< in: message kind
  const std::string &message ) //!< in: error message
{
  std::cout << kind << ": " << message;
}


/******************************************************************//**
* \brief Initialize the primitive renderer. 
*
* For the initialization a current and valid OpenGL context is required. 
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
void CPrimitiveOpenGL_2_00::Init( void )
{
  if ( _initilized )
    return;
  _initilized = true;

  std::string msg;


  // compile shader objects

  auto vert_shader = std::make_shared<CShaderObject>( Render::Program::TShaderType::vertex );
  *vert_shader << _vert_110;
  vert_shader->Compile();
  if ( vert_shader->Verify( msg ) == false )
    Error( "compile error", msg );

  auto frag_shader = std::make_shared<CShaderObject>( Render::Program::TShaderType::fragment );
  *frag_shader << _frag_110;
  frag_shader->Compile();
  if ( frag_shader->Verify( msg ) == false )
    Error( "compile error", msg );


  // link shader programs

  _prog = std::make_shared<CShaderProgram>();
  *_prog << vert_shader << frag_shader;
  _prog->Link();
  if ( _prog->Verify( msg ) == false )
    Error( "link error", msg );
  _color_loc             = glGetUniformLocation( (GLuint)_prog->ObjectHandle(), "u_color" );
  _case_loc              = glGetUniformLocation( (GLuint)_prog->ObjectHandle(), "u_attr_case" );
  _depth_attenuation_loc = glGetUniformLocation( (GLuint)_prog->ObjectHandle(), "u_depth_att" );
  _attrib_xyzw_inx       = glGetAttribLocation(  (GLuint)_prog->ObjectHandle(), "attr_xyzw" );
  _attrib_y_inx          = glGetAttribLocation(  (GLuint)_prog->ObjectHandle(), "attr_y" );
}


/******************************************************************//**
* \brief Set the stroke color of the primitive. 
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
CPrimitiveOpenGL_2_00 & CPrimitiveOpenGL_2_00::SetColor( 
  const Render::TColor & color ) //!< in: new color
{ 
  _color = color; 

  if ( _successive_drawing )
    glUniform4fv( _color_loc, 1, _color.data() );
  
  return *this; 
}


/******************************************************************//**
* \brief Set the stroke color of the primitive. 
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
CPrimitiveOpenGL_2_00 & CPrimitiveOpenGL_2_00::SetColor( 
  const Render::TColor8 & color ) //!< in: new color
{ 
  _color = Render::toColor( color );

  if ( _successive_drawing )
    glUniform4fv( _color_loc, 1, _color.data() );
  
  return *this; 
}


/******************************************************************//**
* \brief Set depth attenuation.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
CPrimitiveOpenGL_2_00 CPrimitiveOpenGL_2_00::SetDeptAttenuation( 
  Render::t_fp depth_attenuation ) //!< depth attenuation
{
  _depth_attenuation = depth_attenuation;

  if ( _successive_drawing )
    glUniform1f( _depth_attenuation_loc, _depth_attenuation );

  return *this;
}


/******************************************************************//**
* \brief Notify the render that a sequence of successive primitives will
* follow, that is not interrupted by any other drawing operation.
* This allows the render to do some performance optimizations and to
* prepare for the primitive rendering.
* The render can keep states persistent from one primitive drawing to the
* other, without initializing and restoring them.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPrimitiveOpenGL_2_00::StartSuccessivePrimitiveDrawings( void )
{
  if ( _active_sequence )
  {
    assert( false );
    return false;
  }
  if ( _successive_drawing )
    return true;

  // activate the shader program
  _prog->Use();

  // initialize uniforms
  UpdateParameterUniforms();
  glUniform1i( _case_loc,  0 );

  // enable and disable vertex attributes
  glEnableVertexAttribArray( _attrib_xyzw_inx );
  glDisableVertexAttribArray( _attrib_y_inx );

  _attribute_case     = 0;
  _successive_drawing = true;
  return true;
}


/******************************************************************//**
* \brief Notify the renderer that a sequence of primitives has been
* finished, and that the internal states have to be restored.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPrimitiveOpenGL_2_00::FinishSuccessivePrimitiveDrawings( void )
{
  if ( _active_sequence )
  {
    assert( false );
    return false;
  }
  if ( _successive_drawing == false )
    return true;

  // disable vertex attributes
  glDisableVertexAttribArray( _attrib_xyzw_inx );
  glDisableVertexAttribArray( _attrib_y_inx );

  // activate the shader program 0
  glUseProgram( 0 );

  _attribute_case     = 0;
  _successive_drawing = false;
  return true;
}


/******************************************************************//**
* \brief Start a new primitive sequence.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CPrimitiveOpenGL_2_00::StartSequence( void ) 
{
  // A new sequence can't be started within an active sequence
  if ( _active_sequence )
  {
    ASSERT( false );
    return false;
  }
  _active_sequence = true;
  return true;
}


/******************************************************************//**
* \brief Start a new primitive sequence.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CPrimitiveOpenGL_2_00::EndSequence( void ) 
{
  // A new sequence can't be started within an active sequence
  if ( _active_sequence == false )
  {
    ASSERT( false );
    return false;
  }
  _active_sequence = false;
  return true;
}
 

/******************************************************************//**
* \brief Activate program, update uniforms and enable vertex attributes.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPrimitiveOpenGL_2_00::ActivateProgram( 
  bool x_y_case ) //! I - true: separate x and y coordinate attribute; false: one attribute fro x, y, z and w coordinate
{
  // A new sequence can't be started within an active sequence
  if ( _initilized == false )
  {
    ASSERT( false );
    return false;
  }

  // set attribute mode value
  int case_val = x_y_case ? 1 : 0;

  // activate program, update uniforms and enable vertex attributes
  bool set_case = false;
  if ( _successive_drawing == false )
  {
    _prog->Use();
   
    UpdateParameterUniforms();
    _attribute_case = case_val;
    set_case = true;
    glEnableVertexAttribArray( _attrib_xyzw_inx );
  }
  else if ( _attribute_case != case_val )
  {
    _attribute_case = case_val;
    set_case = true;
  }

  if ( set_case )
  {
    glUniform1i( _case_loc,  case_val );
    if ( _attribute_case == 1 )
      glEnableVertexAttribArray( _attrib_y_inx );
    else
      glDisableVertexAttribArray( _attrib_y_inx );
  }
}


/******************************************************************//**
* \brief Install the default program.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPrimitiveOpenGL_2_00::DeactivateProgram( void )
{
  if ( _successive_drawing )
    return false;
 
  // disable vertex attributes and activate program 0
  glDisableVertexAttribArray( _attrib_xyzw_inx );
  if ( _attribute_case == 1 )
    glDisableVertexAttribArray( _attrib_y_inx );
  glUseProgram( 0 );
  
  return true;
}
 

/******************************************************************//**
* \brief Install the program object as part of current rendering state  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
CPrimitiveOpenGL_2_00 & CPrimitiveOpenGL_2_00::Use( void )
{
  if ( _prog != nullptr )
    _prog->Use();
  return *this;
}


/******************************************************************//**
* \brief Set style and color parameter uniforms.  
* 
* \author  gernot
* \date    2018-09-21
* \version 1.0
**********************************************************************/
CPrimitiveOpenGL_2_00 & CPrimitiveOpenGL_2_00::UpdateParameterUniforms( void )
{
  glUniform4fv( _color_loc, 1, _color.data() );
  glUniform1f( _depth_attenuation_loc, _depth_attenuation );

  return *this;
}


} // OpenGL