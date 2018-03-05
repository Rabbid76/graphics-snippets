/*XEOMETRIC********************************************************//**
* @brief   Generic interface for render data like camera, light, mesh
*          and material.
*
* @author  gernot76
* @date    2018-02-06
* @version 1.0
**********************************************************************/
#pragma once
#ifndef IRenderData_h_INCLUDED
#define IRenderData_h_INCLUDED


// includes

#include <IDrawType.h>
#include <ILight.h>

#include <vector>
#include <array>



// type declarations


/*XEOMETRIC********************************************************//**
* @brief   Namespace for render tables.
*
* @author  gernot76
* @date    2018-02-06
* @version 1.0
**********************************************************************/
namespace Render
{

using TVertexData = float;         //!< type of the vetex array elements
using TIndexData  = unsigned long; //!< type of the indfex array elements

using TMat44 = std::array<std::array<TVertexData, 4>, 4>;

const size_t iVertexSize = 3; //!< size of the vertex coordinates
const size_t iNoramSize  = 3; //!< size of the normal vector
const size_t iUVSize     = 2; //!< size of the UV coordinates
const size_t iRSTSize    = 2; //!< size of the RST coordinates



//---------------------------------------------------------------------
// Vertices
//---------------------------------------------------------------------


/*XEOMETRIC********************************************************//**
* @brief   Vertex attributes of a definition or surface and the model
*          matrix.
*
* @author  gernot76
* @date    2018-02-06
* @version 1.0
**********************************************************************/
struct TVertexAttributes
{
  TIndexData         _no_of_attrib; //!< number of vertex attribute data sets (number of points)
  const TVertexData *_vertex;       //!< pointer to the array of vertex coordinates
  const TVertexData *_normal;       //!< pointer to the array of normal vectors
  const TVertexData *_uv;           //!< pointer to the array of texture coordiantes (optional)
  const TVertexData *_rst;          //!< pointer to the array of texture coordiantes (optional)
  TMat44             _model_mat44;  //!< model matrix for the vertices
};

using TVertexTable = std::vector<TVertexAttributes>; //!< table of the vertex attrivutes of a scene


//---------------------------------------------------------------------
// Surfaces
//---------------------------------------------------------------------


/*XEOMETRIC********************************************************//**
* @brief   Additional properties that specify the behavior of the
*          surface.
*
* @author  gernot76
* @date    2018-02-14
* @version 1.0
**********************************************************************/
enum class TSurfaceProperty
{
  simple_material, //!< the maaterial data reference refers to a simple material 
  // ...
  NO_OF
};
using TSurfaceProperties = std::bitset<static_cast<size_t>(TSurfaceProperty::NO_OF)>;

/*XEOMETRIC********************************************************//**
* @brief   Primitives, representation properties and reference to the
*          representation parameters of a surface.
*
* @author  gernot76
* @date    2018-02-06
* @version 1.0
**********************************************************************/
struct TSurface
{
  TIndexData          _no_of_elements; //!< number of the indices (3 * number of triangles)
  const TIndexData   *_elements;       //!< array of indices
  TIndexData          _vertex_ref;     //!< reference to the vertex attributes (index in the vertex attribute table)
  TIndexData          _material_ref;   //!< reference to either 
  TSurfaceProperties  _properties;     //!< additional surface properties
};

using TSurfaceTable = std::vector<TSurface>; //!< table of the vertex attributes of a scene



//---------------------------------------------------------------------
// Simple material
//---------------------------------------------------------------------

/*XEOMETRIC********************************************************//**
* @brief   Render material data.
*
* @author  gernot76
* @date    2018-02-06
* @version 1.0
**********************************************************************/
struct TMaterialSimple
{
  TColor8 _diffuse;  //!< diffuse material color
};

using TMaterialSimpleTable = std::vector<TMaterialSimple>; //!< table of the simple materials



//---------------------------------------------------------------------
// Standard material
//---------------------------------------------------------------------


/*XEOMETRIC********************************************************//**
* @brief   Render material data.
*
* @author  gernot76
* @date    2018-02-06
* @version 1.0
**********************************************************************/
struct TMaterial 
  : public TMaterialSimple
{
  TColor8 _specular; //!< specular material color
  
  // TODO $$$
  // ...
};

using TMaterialTable = std::vector<TMaterial>; //!< table of the standard materials



//---------------------------------------------------------------------
// Camera
//---------------------------------------------------------------------


/*XEOMETRIC********************************************************//**
* @brief   Additional properties that specify the behavior of the
*          camera.
*
* @author  gernot76
* @date    2018-02-14
* @version 1.0
**********************************************************************/
enum class TCameraProperty
{
  orthographic, //!< the camera is not perspecive but orthographic
  test,
  // ...
  NO_OF
};
using TCameraProperties = std::bitset<static_cast<size_t>(TCameraProperty::NO_OF)>;


/*XEOMETRIC********************************************************//**
* @brief   Camera specification for a scene.
*
* @author  gernot76
* @date    2018-02-14
* @version 1.0
**********************************************************************/
struct TCamera
{
  using TSize = std::array<size_t, 2>;

  bool IsOrthographic( void ) const { return _properties.test( (int)Render::TCameraProperty::orthographic ); }

  TVec3             _position;   //!< camera position
  TVec3             _target;     //!< camera target
  TVec3             _up;         //!< up vecor of the camera
  t_fp              _fov_y;      //!< vertical filed of view angle in radians (for a perspective camera)
  TVec4             _viewRect;   //!< left, right, bottom and top on the near plane
  TVec2             _depthRange; //!< distance to the near and the fare plane
  TCameraProperties _properties; //!< `TCameraProperties` properties 
};



//---------------------------------------------------------------------
// Scene
//---------------------------------------------------------------------


/*XEOMETRIC********************************************************//**
* @brief   Complete render data of a scene.
*
* @author  gernot76
* @date    2018-02-14
* @version 1.0
**********************************************************************/
struct TSceneData
{
  TVec3 _min; //!< minimum of AABB of the scene
  TVec3 _max; //!< maximum of AABB of the scene
};


/*XEOMETRIC********************************************************//**
* @brief   Complete render data of a scene.
*
* @author  gernot76
* @date    2018-02-14
* @version 1.0
**********************************************************************/
struct TRenderData
{
  TSceneData           _sceneData;      //!< general scene data
  TCamera              _camera;         //!< the camera
  TAmbientLightTable   _ambientLights;  //!< the ambient light data
  TLightTable          _lightSources;   //!< light source data
  TVertexTable         _vertices;       //!< verterx data
  TSurfaceTable        _surfaces;       //!< index and surface 
  TMaterialSimpleTable _materialSimple; //!< simple material data
  TMaterialTable       _material;       //!< material data
};

}; // Render


#endif // IRenderData_h_INCLUDED