/******************************************************************//**
* \brief Implementation of OpenGL primitive renderer,
* with the use of "modern" OpenGL 4+ core and
* GLSL version 4.30 (`#version 430 core`) or
* OpenGL es 3(+) and GLSL ES version 3.00 (`#version 300 es`).
*  
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/

#include <stdafx.h>

// includes

#include "../../include/OpenGL/OpenGLPrimitive_core_and_es.h"
#include "../../include/OpenGL/OpenGLProgram.h"
#include "../../include/OpenGL/OpenGLVertexBuffer.h"


// OpenGL wrapper

#include "../../include/OpenGL/OpenGL_include.h"
#include "../../include/OpenGL/OpenGL_enumconst.h"


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

const size_t CPrimitiveOpenGL_core_and_es::_default_binding = 1;


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

// TODO $$$
const std::string CPrimitiveOpenGL_core_and_es::_vert_430 = R"(
#version 420

in vec4  attr_xyzw;
in float attr_y;

out vec4 v_color;

layout (std140, binding = 1) uniform UB_ViewData
{
    mat4  _model;      // model matrix
    mat4  _view;       // view matrix
    mat4  _projection; // projection matrix
    vec4  _vp_rect;    // viewport rectangle
    float _near;       // near plane 
    float _far;        // far plane 

} view_data;

uniform int   u_attr_case;
uniform vec4  u_color;
uniform float u_depth_att;

void main()
{
  vec4 vert_pos = u_attr_case == 0 ? attr_xyzw : vec4(attr_xyzw.x, attr_y, 0.0, 1.0);
  gl_Position   = view_data._projection * view_data._view * view_data._model * vert_pos;
  
  // TODO $$$
  //gl_ClipVertex = gl_Position;

  float depth   = 0.5 + 0.5 * gl_Position.z / gl_Position.w;
  v_color       = vec4( mix(u_color.rgb, vec3(0.0), depth * u_depth_att), u_color.a );
}
)";


// TODO $$$
const std::string CPrimitiveOpenGL_core_and_es::_frag_430 = R"(
#version 420

in  vec4 v_color;
out vec4 frag_color;

void main()
{
  frag_color = v_color;
}
)";


/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
CPrimitiveOpenGL_core_and_es::CPrimitiveOpenGL_core_and_es( void )
{}


/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-12-10
* \version 1.0
**********************************************************************/
CPrimitiveOpenGL_core_and_es::~CPrimitiveOpenGL_core_and_es()
{}


/******************************************************************//**
* \brief Trace error message.  
* 
* \author  gernot
* \date    2018-12-10
* \version 1.0
**********************************************************************/
void CPrimitiveOpenGL_core_and_es::Error( 
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
* \date    2018-12-10
* \version 1.0
**********************************************************************/
bool CPrimitiveOpenGL_core_and_es::Init( 
  size_t                   min_buffer_size, //!< I - minimum buffer size
  Render::TModelAndViewPtr mvp_data )       //!< I - model, view, projection and window data
{
  if ( _initilized )
    return true;
 
  TMVPBufferPtr mvp_buffer;
  if ( mvp_data != nullptr )
  {
    mvp_buffer = std::make_unique<CModelAndViewBuffer_std140>();
    _mvp_buffer->Init( _default_binding, mvp_data );
  }

  return Init( min_buffer_size, mvp_buffer );
}


/******************************************************************//**
* \brief Initialize the primitive renderer. 
*
* For the initialization a current and valid OpenGL context is required. 
* 
* \author  gernot
* \date    2018-12-10
* \version 1.0
**********************************************************************/
bool CPrimitiveOpenGL_core_and_es::Init( 
  size_t        min_buffer_size, //!< I - minimum buffer size
  TMVPBufferPtr mvp_buffer )     //!< I - model, view, projection and window data buffer 
{
  if ( _initilized )
    return true;
  _initilized = true;

  InitMVPBuffer( mvp_buffer );
  InitDrawBuffer( min_buffer_size );
  InitProgram();

  return true;
}


/******************************************************************//**
* \brief Initialize model, view, projection and window data buffer. 
* 
* \author  gernot
* \date    2018-12-10
* \version 1.0
**********************************************************************/
void CPrimitiveOpenGL_core_and_es::InitMVPBuffer( 
  TMVPBufferPtr mvp_buffer ) //!< I - model, view, projection and window data buffer 
{
  if ( _mvp_buffer != nullptr )
    return;
 
  _mvp_buffer = mvp_buffer != nullptr ? mvp_buffer : std::make_unique<CModelAndViewBuffer_std140>();
  _mvp_buffer->Init( _default_binding ); // `c_default_binding` is applied only, if the buffer was not initialized yet
}


/******************************************************************//**
* \brief Initialize vertex array object and vertex buffer.  
* 
* \author  gernot
* \date    2018-12-10
* \version 1.0
**********************************************************************/
void CPrimitiveOpenGL_core_and_es::InitDrawBuffer( 
  size_t min_buffer_size ) //! minimum buffer size
{
  if ( _mesh_buffer != nullptr )
    return;

  _mesh_buffer = std::make_unique<OpenGL::CDrawBuffer>( Render::TDrawBufferUsage::stream_draw, min_buffer_size );

  bool ret_xyz = _mesh_buffer->SpecifyVA( Render::b0_xyz );
  ASSERT( ret_xyz );

  bool ret_x_y = _mesh_buffer->SpecifyVA( Render::b0_x__b1_y );
  ASSERT( ret_x_y );
}


/******************************************************************//**
* \brief Initialize shader program. 
* 
* \author  gernot
* \date    2018-12-10
* \version 1.0
**********************************************************************/
void CPrimitiveOpenGL_core_and_es::InitProgram( void ) // shader program
{
  if ( _prog != nullptr )
    return;

  std::string msg;

  // compile shader objects

  auto vert_shader = std::make_shared<CShaderObject>( Render::Program::TShaderType::vertex );
  *vert_shader << _vert_430;
  vert_shader->Compile();
  if ( vert_shader->Verify( msg ) == false )
    Error( "compile error", msg );

  auto frag_shader = std::make_shared<CShaderObject>( Render::Program::TShaderType::fragment );
  *frag_shader << _frag_430;
  frag_shader->Compile();
  if ( frag_shader->Verify( msg ) == false )
    Error( "compile error", msg );


  // link shader programs

  _prog = std::make_shared<CShaderProgram>();
  *_prog << vert_shader << frag_shader;
  _prog->Link();
  if ( _prog->Verify( msg ) == false )
    Error( "link error", msg );
  
  _view_data_loc         = glGetUniformBlockIndex( (GLuint)_prog->ObjectHandle(), "UB_ViewData" );
  _color_loc             = glGetUniformLocation(   (GLuint)_prog->ObjectHandle(), "u_color" );
  _case_loc              = glGetUniformLocation(   (GLuint)_prog->ObjectHandle(), "u_attr_case" );
  _depth_attenuation_loc = glGetUniformLocation(   (GLuint)_prog->ObjectHandle(), "u_depth_att" );
  _attrib_xyzw_inx       = glGetAttribLocation(    (GLuint)_prog->ObjectHandle(), "attr_xyzw" );
  _attrib_y_inx          = glGetAttribLocation(    (GLuint)_prog->ObjectHandle(), "attr_y" );

  if ( _mvp_buffer != nullptr )
  {
    GLuint binding_point = (GLuint)_mvp_buffer->BufferBinding();
    GLuint progrma_obj = static_cast<GLuint>(_prog->ObjectHandle());
    glUniformBlockBinding( progrma_obj, _view_data_loc, binding_point );
  }
}


/******************************************************************//**
* \brief Set the stroke color of the primitive. 
* 
* \author  gernot
* \date    2018-12-10
* \version 1.0
**********************************************************************/
CPrimitiveOpenGL_core_and_es & CPrimitiveOpenGL_core_and_es::SetColor( 
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
* \date    2018-12-10
* \version 1.0
**********************************************************************/
CPrimitiveOpenGL_core_and_es & CPrimitiveOpenGL_core_and_es::SetColor( 
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
CPrimitiveOpenGL_core_and_es CPrimitiveOpenGL_core_and_es::SetDeptAttenuation( 
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
bool CPrimitiveOpenGL_core_and_es::StartSuccessivePrimitiveDrawings( void )
{
  if ( _active_sequence || _initilized == false )
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

  _attribute_case     = 0;
  _successive_drawing = true;
  return true;
}


/******************************************************************//**
* \brief Notify the renderer that a sequence of primitives has been
* finished, and that the internal states have to be restored.  
* 
* \author  gernot
* \date    2018-12-10
* \version 1.0
**********************************************************************/
bool CPrimitiveOpenGL_core_and_es::FinishSuccessivePrimitiveDrawings( void )
{
  if ( _active_sequence || _initilized == false )
  {
    assert( false );
    return false;
  }
  if ( _successive_drawing == false )
    return true;

  // disable vertex attributes
  _mesh_buffer->Release();

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
* \date    2018-12-10
* \version 1.0
**********************************************************************/
bool CPrimitiveOpenGL_core_and_es::StartSequence( void ) 
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
* \date    2018-12-10
* \version 1.0
**********************************************************************/
bool CPrimitiveOpenGL_core_and_es::EndSequence( void ) 
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
* \date    2018-12-10
* \version 1.0
**********************************************************************/
bool CPrimitiveOpenGL_core_and_es::ActivateProgram( 
  Render::TVA requested_va_type ) //! I - vertex array object specification
{
  // A new sequence can't be started within an active sequence
  if ( _initilized == false )
  {
    ASSERT( false );
    return false;
  }

  // activate program  
  bool set_case = false;
  if ( _successive_drawing == false )
    _prog->Use();

  // set attribute mode value: 1 : separated x y mode, 0: single attribute 
  int case_val = requested_va_type == Render::b0_x__b1_y || requested_va_type == Render::d__b0_x__b1_y ? 1 : 0;
  
  // update uniforms
  if ( _attribute_case != case_val )
  {
    UpdateParameterUniforms();
    glUniform1i( _case_loc,  case_val );
    _attribute_case = case_val;
  }

  // and enable vertex attributes
  if ( _va_type != requested_va_type )
  {
    // TODO $$$: get a mesh buffer from provider
    _mesh_buffer->SpecifyVA( requested_va_type );
    _mesh_buffer->Prepare();
    _va_type = requested_va_type;
  }
 
  return true;
}


/******************************************************************//**
* \brief Install the default program.  
* 
* \author  gernot
* \date    2018-12-10
* \version 1.0
**********************************************************************/
bool CPrimitiveOpenGL_core_and_es::DeactivateProgram( void )
{
  if ( _successive_drawing )
    return false;
 
  // disable vertex attributes and activate program 0
  _mesh_buffer->Release();
  glUseProgram( 0 );
  
  return true;
}
 

/******************************************************************//**
* \brief Install the program object as part of current rendering state  
* 
* \author  gernot
* \date    2018-12-10
* \version 1.0
**********************************************************************/
CPrimitiveOpenGL_core_and_es & CPrimitiveOpenGL_core_and_es::Use( void )
{
  if ( _prog != nullptr )
    _prog->Use();
  return *this;
}


/******************************************************************//**
* \brief Set style and color parameter uniforms.  
* 
* \author  gernot
* \date    2018-12-10
* \version 1.0
**********************************************************************/
CPrimitiveOpenGL_core_and_es & CPrimitiveOpenGL_core_and_es::UpdateParameterUniforms( void )
{
  // update properties
  glUniform4fv( _color_loc, 1, _color.data() );
  glUniform1f( _depth_attenuation_loc, _depth_attenuation );

  // update model, view and projection data
  _mvp_buffer->Update();

  return *this;
}


} // OpenGL