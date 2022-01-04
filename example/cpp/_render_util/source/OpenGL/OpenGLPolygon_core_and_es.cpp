/******************************************************************//**
* \brief Implementation of OpenGL polygon renderer,
* with the use of "modern" OpenGL 4+ core and
* GLSL version 4.30 (`#version 430 core`) or
* OpenGL es 3(+) and GLSL ES version 3.00 (`#version 300 es`).
* 
* \author  gernot
* \date    2018-12-05
* \version 1.0
**********************************************************************/

#include <stdafx.h>

// includes

#include <OpenGLPolygon_core_and_es.h>
#include <OpenGLPrimitive_core_and_es.h>


// OpenGL wrapper

#include <OpenGL_include.h>
#include <OpenGL_enumconst.h>


// STL

#include <iostream>
#include <algorithm>


// class definitions


/******************************************************************//**
* \brief General namespace for OpenGL implementation.  
* 
* \author  gernot
* \date    2018-12-05
* \version 1.0
**********************************************************************/
namespace OpenGL
{


/******************************************************************//**
* \brief Namespace for drawing polygons with the use of OpenGL.  
* 
* \author  gernot
* \date    2018-12-05
* \version 1.0
**********************************************************************/
namespace Polygon
{


/******************************************************************//**
* \brief ctor.  
* 
* \author  gernot
* \date    2018-12-05
* \version 1.0
**********************************************************************/
CPolygon_base_OpenGL4_OpenGLES3::CPolygon_base_OpenGL4_OpenGLES3( 
  size_t min_cache_elems ) //! I - size of the element cache
  : _min_buffer_size( min_cache_elems )
  , _vertex_cache( min_cache_elems )
{}
  

/******************************************************************//**
* \brief dtor.  
* 
* \author  gernot
* \date    2018-12-05
* \version 1.0
**********************************************************************/
CPolygon_base_OpenGL4_OpenGLES3::~CPolygon_base_OpenGL4_OpenGLES3()
{}


/******************************************************************//**
* \brief Initialize the polygon renderer.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPolygon_base_OpenGL4_OpenGLES3::Init( void )
{
  if ( _initialized )
    return true;

  TProgramPtr prpgram;
  return Init( prpgram );
}


/******************************************************************//**
* \brief Initialize the line renderer. 
*
* For the initialization a current and valid OpenGL context is required. 
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CPolygon_base_OpenGL4_OpenGLES3::Init( 
  Render::TModelAndViewPtr mvp_data ) //!< I - model, view, projection and window data buffer 
{
  if ( _initialized )
    return true;
 
  TMVPBufferPtr mvp_buffer;
  if ( mvp_data != nullptr )
  {
    static const size_t c_default_binding = 1; 
    mvp_buffer = std::make_unique<CModelAndViewBuffer_std140>();
    mvp_buffer->Init( c_default_binding, mvp_data );
  }

  return Init( mvp_buffer );
}


/******************************************************************//**
* \brief Initialize the line renderer. 
*
* For the initialization a current and valid OpenGL context is required. 
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CPolygon_base_OpenGL4_OpenGLES3::Init( 
  TMVPBufferPtr mvp_buffer ) //!< I - model, view, projection and window data buffer 
{
  if ( _initialized )
    return true;
 
  TProgramPtr prpgram;
  if ( mvp_buffer != nullptr )
  {
    prpgram = std::make_unique<CPrimitiveOpenGL_core_and_es>();
    prpgram->Init( _min_buffer_size, mvp_buffer );
  }

  return Init( prpgram );
}


/******************************************************************//**
* \brief Initialize the line renderer. 
*
* For the initialization a current and valid OpenGL context is required. 
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
bool CPolygon_base_OpenGL4_OpenGLES3::Init( 
  TProgramPtr program ) //!< I - shader program
{
  if ( _initialized )
    return true;
  _initialized = true;

  _primitive_prog = program != nullptr ?  program : std::make_unique<CPrimitiveOpenGL_core_and_es>();
  TMVPBufferPtr mvp_buffer;
  return _primitive_prog->Init( _min_buffer_size, mvp_buffer );
}


/******************************************************************//**
* \brief Notify the render that a sequence of successive polygons will
* follow, that is not interrupted by any other drawing operation.
* This allows the render to do some performance optimizations and to
* prepare for the polygon rendering.
* The render can keep states persistent from one polygon drawing to the
* other, without initializing and restoring them.  
* 
* \author  gernot
* \date    2018-09-12
* \version 1.0
**********************************************************************/
bool CPolygon_base_OpenGL4_OpenGLES3::StartSuccessivePolygonDrawings( void )
{
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() )
  {
    ASSERT( false );
    return false;
  }

  if ( _successive_draw_started )
    return false;

  if ( _primitive_prog->SuccessiveDrawing() == false )
  {
    _primitive_prog->StartSuccessivePrimitiveDrawings(); 
    _successive_draw_started = true;
  }
  
  return true;
}


/******************************************************************//**
* \brief Notify the renderer that a sequence of polygons has been
* finished, and that the internal states have to be restored.  
* 
* \author  gernot
* \date    2018-09-12
* \version 1.0
**********************************************************************/
bool CPolygon_base_OpenGL4_OpenGLES3::FinishSuccessivePolygonDrawings( void )
{
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() )
  {
    ASSERT( false );
    return false;
  }
  
  if ( _successive_draw_started )
  {
    _primitive_prog->FinishSuccessivePrimitiveDrawings();
    _successive_draw_started = false;
  }

  return true;
}


/******************************************************************//**
* \brief Set the stroke color of the polygon.
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
Render::Polygon::IRender & CPolygon_base_OpenGL4_OpenGLES3::SetColor(
  const Render::TColor & color ) //!< I - the new color
{
  if ( _primitive_prog != nullptr )
    _primitive_prog->SetColor( color );
  return *this; 
}


/******************************************************************//**
* \brief Set the stroke color of the polygon.
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
Render::Polygon::IRender & CPolygon_base_OpenGL4_OpenGLES3::SetColor( 
  const Render::TColor8 & color ) //!< I - the new color
{
  if ( _primitive_prog != nullptr )
    _primitive_prog->SetColor( color );
  return *this; 
}


/******************************************************************//**
* \brief Set the polygon style.
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
Render::Polygon::IRender & CPolygon_base_OpenGL4_OpenGLES3::SetStyle( 
  const Render::Polygon::TStyle & style )
{
  //! This is impossible, while an drawing sequence is active.  
  //! The only possible operations within a `glBegin`/`glEnd` sequence are those operations,
  //! which directly change fixed function attributes or specify a new vertex coordinate.  
  //! See [`glBegin`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glBegin.xml)
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() )
  {
    assert( false );
    return *this;
  }
  _primitive_prog->SetDeptAttenuation( _polygon_style._depth_attenuation );

  _polygon_style = style;

  // ...
  
  return *this;
}


/******************************************************************//**
* \brief Draw a single polygon sequence.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPolygon_base_OpenGL4_OpenGLES3::Draw( 
  Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - triangles, triangle strip, triangle fan, triangle adjacency or triangle strip adjacency
  unsigned int       tuple_size,     //!< in: kind of the coordinates - 2: 2D (x, y), 3: 3D (x, y, z), 4: homogeneous (x, y, z, w)   
  size_t             coords_size,    //!< in: number of elements (size) of the coordinate array - `coords_size` = `tuple_size` * "number of coordinates" 
  const float       *coords )        //!< in: pointer to an array of the vertex coordinates
{ 
  // A new sequence can't be started within an active sequence
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() )
  {
    ASSERT( false );
    return false;
  }
  ASSERT( Render::BasePrimitive(primitive_type) == Render::TBasePrimitive::polygon );
  auto &prog = *_primitive_prog;

  // activate program, update uniforms and enable vertex attributes
  Render::TVA va_type = tuple_size == 4 ? Render::b0_xyzw : (tuple_size == 2 ? Render::b0_xy : Render::b0_xyz);
  prog.ActivateProgram( va_type );

  // set vertex attribute pointer and draw the line
  if ( auto *buffer = prog.DrawBuffer() )
    buffer->UpdateVB( 0, tuple_size*sizeof(float), coords_size, coords );
  
  // TODO $$$ append to cache and delays the drawing until it is absolutely necessary.
  //          dependent on VAO specification and `_successive_draw_started`
  glDrawArrays( OpenGL::Primitive( primitive_type ), 0, (GLsizei)(coords_size / tuple_size) );
  
  // disable vertex attributes and activate program 0
  prog.DeactivateProgram();

  return true;
}
 

/******************************************************************//**
* \brief Draw a single polygon sequence.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPolygon_base_OpenGL4_OpenGLES3::Draw( 
  Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - triangles, triangle strip, triangle fan, triangle adjacency or triangle strip adjacency
  unsigned int       tuple_size,     //!< in: kind of the coordinates - 2: 2D (x, y), 3: 3D (x, y, z), 4: homogeneous (x, y, z, w)   
  size_t             coords_size,    //!< in: number of elements (size) of the coordinate array - `coords_size` = `tuple_size` * "number of coordinates" 
  const double      *coords )        //!< in: pointer to an array of the vertex coordinates
{
  // A new sequence can't be started within an active sequence
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() )
  {
    ASSERT( false );
    return false;
  }
  ASSERT( Render::BasePrimitive(primitive_type) == Render::TBasePrimitive::polygon );
  auto &prog = *_primitive_prog;

  // activate program, update uniforms and enable vertex attributes
  Render::TVA va_type = tuple_size == 4 ? Render::d__b0_xyzw : (tuple_size == 2 ? Render::d__b0_xy : Render::d__b0_xyz);
  prog.ActivateProgram( va_type );

  // set vertex attribute pointer and draw the line
  if ( auto *buffer = prog.DrawBuffer() )
    buffer->UpdateVB( 0, 3, coords_size*sizeof(double), coords );

  // TODO $$$ append to cache and delays the drawing until it is absolutely necessary.
  //          dependent on VAO specification and `_successive_draw_started`
  glDrawArrays( OpenGL::Primitive( primitive_type ), 0, (GLsizei)(coords_size / tuple_size) );
 
  // disable vertex attributes and activate program 0
  prog.DeactivateProgram();

  return true;
}


/******************************************************************//**
* \brief Draw a single polygon sequence.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPolygon_base_OpenGL4_OpenGLES3::Draw( 
  Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - triangles, triangle strip, triangle fan, triangle adjacency or triangle strip adjacency
  size_t             no_of_coords,   //!< in: number of coordinates and number of elements (size) of the coordinate array
  const float       *x_coords,       //!< int pointer to an array of the x coordinates
  const float       *y_coords )      //!< int pointer to an array of the y coordinates
{
  // A new sequence can't be started within an active sequence
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() )
  {
    ASSERT( false );
    return false;
  }
  ASSERT( Render::BasePrimitive(primitive_type) == Render::TBasePrimitive::polygon );
  auto &prog = *_primitive_prog;

  // activate program, update uniforms and enable vertex attributes
  prog.ActivateProgram( Render::b0_x__b1_y );

  // set vertex attribute pointers and draw the line
  if ( auto *buffer = prog.DrawBuffer() )
  {
    buffer->UpdateVB( 0, sizeof(float), no_of_coords, x_coords );
    buffer->UpdateVB( 1, sizeof(float), no_of_coords, y_coords );
  }

  // TODO $$$ append to cache and delays the drawing until it is absolutely necessary.
  //          dependent on VAO specification and `_successive_draw_started`
  glDrawArrays( OpenGL::Primitive( primitive_type ), 0, (GLsizei)no_of_coords );
   
  // disable vertex attributes and activate program 0
  prog.DeactivateProgram();

  return true;
}


/******************************************************************//**
* \brief Draw a single polygon sequence.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPolygon_base_OpenGL4_OpenGLES3::Draw( 
  Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - triangles, triangle strip, triangle fan, triangle adjacency or triangle strip adjacency
  size_t             no_of_coords,   //!< in: number of coordinates and number of elements (size) of the coordinate array
  const double      *x_coords,       //!< int pointer to an array of the x coordinates
  const double      *y_coords )      //!< int pointer to an array of the y coordinates
{
  // A new sequence can't be started within an active sequence
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() )
  {
    ASSERT( false );
    return false;
  }
  ASSERT( Render::BasePrimitive(primitive_type) == Render::TBasePrimitive::polygon );
  auto &prog = *_primitive_prog;

  // activate program, update uniforms and enable vertex attributes
  prog.ActivateProgram( Render::d__b0_x__b1_y );

  // set vertex attribute pointers and draw the line
  if ( auto *buffer = prog.DrawBuffer() )
  {
    buffer->UpdateVB( 0, sizeof(double), no_of_coords, x_coords );
    buffer->UpdateVB( 1, sizeof(double), no_of_coords, y_coords );
  }

  // TODO $$$ append to cache and delays the drawing until it is absolutely necessary.
  //          dependent on VAO specification and `_successive_draw_started`
  glDrawArrays( OpenGL::Primitive( primitive_type ), 0, (GLsizei)no_of_coords );

  // disable vertex attributes and activate program 0
  prog.DeactivateProgram();

  return true;
}


/******************************************************************//**
* \brief Start a new polygon sequence.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPolygon_base_OpenGL4_OpenGLES3::StartSequence( 
  Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - triangles, triangle strip, triangle fan, triangle adjacency or triangle strip adjacency
  unsigned int       tuple_size )    //!< in: kind of the coordinates - 2: 2D (x, y), 3: 3D (x, y, z), 4: homogeneous (x, y, z, w)   
{
  // A new sequence can't be started within an active sequence
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() )
  {
    ASSERT( false );
    return false;
  }
  ASSERT( Render::BasePrimitive(primitive_type) == Render::TBasePrimitive::polygon );
  auto &prog = *_primitive_prog;
  
  prog.StartSequence();
  _vertex_cache.TupleSize( tuple_size );
  _squence_type = primitive_type;

  return true;
}
  

/******************************************************************//**
* \brief Complete an active polygon sequence.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPolygon_base_OpenGL4_OpenGLES3::EndSequence( void )
{
  // A sequence can't be completed if there is no active sequence
  if ( _primitive_prog == nullptr || _primitive_prog->EndSequence() == false )
  {
    ASSERT( false );
    return false;
  }
  auto &prog = *_primitive_prog;

  // draw the line

  // activate program, update uniforms and enable vertex attributes
  GLsizei tuple_size = (GLsizei)_vertex_cache.TupleSize();
  Render::TVA va_type = tuple_size == 4 ? Render::b0_xyzw : (tuple_size == 2 ? Render::b0_xy : Render::b0_xyz);
  prog.ActivateProgram( va_type );

  //  set vertex attribute pointer and draw the line
  if ( auto *buffer = prog.DrawBuffer() )
    buffer->UpdateVB( 0, tuple_size*sizeof(float), _vertex_cache.SequenceSize(),_vertex_cache.VertexData() );

  // TODO $$$ append to cache and delays the drawing until it is absolutely necessary.
  //          dependent on VAO specification and `_successive_draw_started`
  glDrawArrays( OpenGL::Primitive( _squence_type ), 0, (GLsizei)(_vertex_cache.SequenceSize() / tuple_size) );
  
  // disable vertex attributes and activate program 0
  prog.DeactivateProgram();
  _vertex_cache.Reset();
  
  return true;
}


/******************************************************************//**
* \brief Specify a new vertex coordinate in an active polygon sequence.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPolygon_base_OpenGL4_OpenGLES3::DrawSequence( 
  float x,  //!< in: x coordinate
  float y,  //!< in: y coordinate
  float z ) //!< in: z coordinate
{
  // A sequence has to be active, to specify a new vertex coordinate
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() == false )
  {
    ASSERT( false );
    return false;
  }

  // add vertex coordinate to cache
  _vertex_cache.Add( x, y, z );
  return true;
}


/******************************************************************//**
* \brief Specify a new vertex coordinate in an active polygon sequence.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPolygon_base_OpenGL4_OpenGLES3::DrawSequence( 
  double x,  //!< in: x coordinate
  double y,  //!< in: y coordinate
  double z ) //!< in: z coordinate
{
  // A sequence has to be active, to specify a new vertex coordinate
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() == false )
  {
    ASSERT( false );
    return false;
  }

  // add vertex coordinate to cache
  _vertex_cache.Add( x, y, z );
  return true;
}
  

/******************************************************************//**
* \brief Specify a sequence of new vertex coordinates in an active
* polygon sequence.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPolygon_base_OpenGL4_OpenGLES3::DrawSequence( 
  size_t             coords_size, //!< in: number of elements (size) of the coordinate array - `coords_size` = `tuple_size` * "number of coordinates" 
  const float       *coords )     //!< in: pointer to an array of the vertex coordinates
{
  // A sequence has to be active, to specify new vertex coordinates
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() == false )
  {
    ASSERT( false );
    return false;
  }

  // add the vertex coordinates to the cache
  _vertex_cache.Add( coords_size, coords );
  return true;
}


/******************************************************************//**
* \brief Specify a sequence of new vertex coordinates in an active
* polygon sequence.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPolygon_base_OpenGL4_OpenGLES3::DrawSequence( 
  size_t             coords_size, //!< in: number of elements (size) of the coordinate array - `coords_size` = `tuple_size` * "number of coordinates" 
  const double      *coords )     //!< in: pointer to an array of the vertex coordinates
{
  // A sequence has to be active, to specify new vertex coordinates
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() == false )
  {
    ASSERT( false );
    return false;
  }
 
  // add the vertex coordinates to the cache
  _vertex_cache.Add( coords_size, coords );
  return true;
}


} // Polygon

} // OpenGL