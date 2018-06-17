/******************************************************************//**
* \brief   General interface for GPU light parameters.
* 
* \author  gernot
* \date    2017-11-20
* \version 1.0
**********************************************************************/
#pragma once
#ifndef Render_ILight_h_INCLUDED
#define Render_ILight_h_INCLUDED


// includes

#include <Render_IDrawType.h>
 
#include <vector>


/******************************************************************//**
* \brief namespace for generic interface of drawing operations
**********************************************************************/
namespace Render
{


/*XEOMETRIC********************************************************//**
* @brief   Additional properties that specify the behavior of the
*          light source.
*
* @author  gernot76
* @date    2018-02-14
* @version 1.0
**********************************************************************/
enum class TLightProperty
{
  enabled,         //!< the light source is active
  camera_relative, //!< the light source is not relative to the worled, but it is relative to the camera
  cast_shadow,     //!< the light source casts shadows
  radius_cutoff,   //!< the light source has a cut off at maximum influence radius (only for finite light sources)
  auto_radius,     //!< the maximum influence radius cut off is automatically calculated by the light intensity and the attenuation
  // ...
  NO_OF
};
using TLightProperties = std::bitset<static_cast<size_t>(TLightProperty::NO_OF)>;


/******************************************************************//**
* \brief   General parameters for a light source.
* 
* \author  gernot
* \date    2018-01-21
* \version 1.0
**********************************************************************/
struct TLight
{
  bool operator == ( const TLight &l ) const
  {
    return std::tie( _ambient, _diffuse, _specular, _position, _attenuation, _cone_attenuation, _direction, _light_cone, _maximum_radius, _properties ) ==
      std::tie( l._ambient, l._diffuse, l._specular, l._position, l._attenuation, l._cone_attenuation, l._direction, l._light_cone, l._maximum_radius, l._properties );
  }

  bool operator != ( const TLight &l ) const
  {
    return std::tie( _ambient, _diffuse, _specular, _position, _attenuation, _cone_attenuation, _direction, _light_cone, _maximum_radius, _properties ) !=
      std::tie( l._ambient, l._diffuse, l._specular, l._position, l._attenuation, l._cone_attenuation, l._direction, l._light_cone, l._maximum_radius, l._properties );
  }

  TColor8          _ambient;           //!< ambinet light color
  TColor8          _diffuse;           //!< diffuse light color
  TColor8          _specular;          //!< specular light color
  TPoint4          _position;          //!< position of the light source (homoguenus coordinate); can be infinite (w=0.0)
  //t_fp             _attenuation_value; //!< a value in the range [0, 1], which represents the attenuation of a light source
  TVec3            _attenuation;       //!< constant, linear and quadratic attenuation of the light source by its distance
  t_fp             _cone_attenuation;  //!< light cone attenuation (attenuation from the center of the light cone to its borders)
  TVec3            _direction;         //!< direction of a spot light
  t_fp             _light_cone;        //!< ligtht cone angle of a spot light in radians
  t_fp             _maximum_radius;    //!< the maximum influence radius for a finite light source (except the distance is calculated automatically `auto_distance`)
  t_fp             _cut_off_weight;    //!< radius cut off weight [0.0, 1.0]; soft (= 0.0) or hard (= 1.0)
  TLightProperties _properties;        //!< `TLightProperty` properties 
};
using TLightTable = std::vector<TLight>;


/******************************************************************//**
* \brief   General parameters for an ambient light source.
* 
* \author  gernot
* \date    2018-02-20
* \version 1.0
**********************************************************************/
struct TAmbientLight
{
  TColor8 _color;  //!< the ambient light color
};
using TAmbientLightTable = std::vector<TAmbientLight>;


} // Draw

#endif // Render_ILight_h_INCLUDED
