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
#include <algorithm>

//---------------------------------------------------------------------
// suppress '#define' of 'min' and 'max'
//---------------------------------------------------------------------


#if defined( min )
#define cutil__define_min_pushed
#pragma push_macro( "min" )
#undef min
#endif

#if defined( max )
#define cutil__define_max_pushed
#pragma push_macro( "max" )
#undef max
#endif


/******************************************************************//**
* \brief namespace for generic interface of drawing operations
**********************************************************************/
namespace Render
{


/***************************************************************************//**
* @brief Threshold for maximum light distance
*
* When the light from a decreasing light source falls below this threshold, a fragment is defined as not illuminated
*
* @author  gernot76
* @date    2019-07-17
* @version 1.0
*******************************************************************************/
inline float LigthMinThreshold( void )
{ 
  static float th = 5.0f / 255.0f;
  return th;
}


/***************************************************************************//**
* @brief Set the constant attenuation dependent on the distance, linear and quadratic attenuation. 
*
* solve the equation :  min_threshold = maxI / ( x + max_dist * aLI + max_dist * max_dist * aQU )
*                   =>  aCO = maxI / min_threshold - max_dist * aLI - max_dist * max_dist * aQU
*
* @author  gernot76
* @date    2019-07-17
* @version 1.0
*******************************************************************************/
inline float LightConstantAttenuation(
  float maxI,      // I - maximum light intensity
  float aLI,       // I - linear attenuation
  float aQU,       // I - quadratic attenuation
  float max_dist ) // I - maximum distance
{
  if ( max_dist <= 0.0f )
    return 0.0f;

  float min_threshold = LigthMinThreshold();
  float aCO = maxI / min_threshold - max_dist * aLI - max_dist * max_dist * aQU;
  return aCO;
}


/***************************************************************************//**
* @brief Set the linear attenuation dependent on the distance, constant and quadratic attenuation. 
*
* solve the equation :  min_threshold = maxI / ( aCO + max_dist * x + max_dist * max_dist * aQU )
*                   =>  aLI = ( maxI / min_threshold - aCO - max_dist * max_dist * aQU ) / max_dist
*
* @author  gernot76
* @date    2019-07-17
* @version 1.0
*******************************************************************************/
inline float LightLinearAttenuation(
  float maxI,      // I - maximum light intensity
  float aCO,       // I - constant attenuation
  float aQU,       // I - quadratic attenuation
  float max_dist ) // I - maximum distance
{
  if ( max_dist <= 0.0f )
    return 0.0f;

  float min_threshold = LigthMinThreshold();
  float aLI = (maxI / min_threshold - aCO - max_dist * max_dist * aQU) / max_dist;
  return aLI;
}


/***************************************************************************//**
* @brief Set the quadratic attenuation dependent on the distance, constant and linear attenuation. 
*
* solve the equation :  min_threshold = maxI / ( aCO + max_dist * aLI + max_dist * max_dist * x )
*                   =>  aQU = (maxI / min_threshold - aCO - max_dist * aLI) / ( max_dist * max_dist )
*
* @author  gernot76
* @date    2019-07-17
* @version 1.0
*******************************************************************************/
inline float LightQuadraticAttenuation(
  float maxI,      // I - maximum light intensity
  float aCO,       // I - constant attenuation
  float aLI,       // I - linear attenuation
  float max_dist ) // I - maximum distance
{
  if ( max_dist <= 0.0f )
    return 0.0f;

  float min_threshold = LigthMinThreshold();
  float aQU = (maxI / min_threshold - aCO - max_dist * aLI) / ( max_dist * max_dist );
  return aQU;
}


/***************************************************************************//**
* @brief Calculate the maximum distance of a light source
*
* solve the equation :  min_threshold = maxI / ( aCO + x * aLI + x * x * aQA )  
*     if   aQU != 0.0    =>      x = ( -aLI +/- sqrt( aLI^2 - 4 * aQU * ( aCO - maxI / min_threshold ) ) ) / ( 2 * aQU )
*     else                       x = ( - aCO + maxI / min_threshold ) / aLI
*
* @author  gernot76
* @date    2019-07-17
* @version 1.0
*******************************************************************************/
inline float LightMaxDistance(
  float maxI, // I - maximum light intensity
  float aCO,  // I - constant attenuation
  float aLI,  // I - linear attenuation
  float aQU ) // I - quadratic attenuation
{
  float min_threshold = LigthMinThreshold();
  float i_th = maxI / min_threshold;
  float max_dist = 1.0e38f;
  if ( aQU <= 0.0f )
  {
    if ( aLI > 0.0f )
      max_dist = abs( ( -aCO + i_th ) / aLI );
  }
  else
  {
    float sqrt_term = sqrt( aLI*aLI - 4.0f * aQU * (aCO - i_th) );
    max_dist = std::max( abs(-aLI + sqrt_term) / (2.0f*aQU), abs(-aLI - sqrt_term) / (2.0f*aQU) );
  }
  return max_dist;
}


/***************************************************************************//**
* @brief Interpret the value of the light range as the absolute 
*        distance in meters and solve the equation :
*   
*        c_min_threshold = 1 / ( C + distance * L + distance * distance * Q )
*
* @author  gernot76
* @date    2019-07-17
* @version 1.0
*******************************************************************************/
inline TVec3 CalculateLightAttenuationByDistance( 
  t_fp attenuation_level, //!< I - level of the attenuation
  t_fp max_distance )     //!< I - maximum distance
{
  const int c_CA = 0;
  const int c_LA = 1;
  const int c_QA = 2;
  static int map_mode = c_QA;
  static std::array< float, 3 > c_att{ 0.0f, 0.0f, 0.0f };
  if (max_distance <= 0.0f)
    return c_att;
  
  std::array< float, 3 > att = c_att;
  float maxI = 1.0;
  switch ( map_mode )
  {
    case c_CA: att[c_CA] = LightConstantAttenuation(  maxI, att[c_LA], att[c_QA], max_distance ); break;
    case c_LA: att[c_LA] = LightLinearAttenuation(    maxI, att[c_CA], att[c_QA], max_distance ); break;
    
    case c_QA: 
      att[c_LA] = (attenuation_level - 1.0f) / LigthMinThreshold() / max_distance;
      att[c_QA] = LightQuadraticAttenuation( maxI, att[c_CA], att[c_LA], max_distance ); 
      break;
  }

  return att;
}


//! Additional properties that specify the behavior of the light source
enum class TLightProperty
{
  enabled,                    //!< the light source is active
  camera_relative,            //!< the light source is not relative to the world, but it is relative to the camera
  cast_shadow,                //!< the light source casts shadows
  radius_cutoff,              //!< the light source has a cut off at maximum influence radius (only for finite light sources)
  auto_radius,                //!< the maximum influence radius cut off is automatically calculated by the light intensity and the attenuation
  sun,                        //!< the light is a sun light
  auto_attenuation_by_radius, //!< the attenuation is automatically determined by the radius of the light source
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

  t_fp LinearConeAttenuation( void ) const { return std::min( 1.0f, _cone_attenuation / 100.0f ); }
  t_fp InnerConeAngle( void )        const { return _light_cone - _light_cone * LinearConeAttenuation() * 0.5f; }
  t_fp OuterConeAngle( void )        const { return _light_cone + _light_cone * LinearConeAttenuation() * 0.5f; }


  TColor8          _ambient;           //!< ambient light color; the brightness of the light is encoded to the alpha channel
  TColor8          _diffuse;           //!< diffuse light color; the brightness of the light is encoded to the alpha channel
  TColor8          _specular;          //!< specular light color; the brightness of the light is encoded to the alpha channel
  TPoint4          _position;          //!< position of the light source (homogeneous coordinate); can be infinite (w=0.0)
  t_fp             _attenuation_value; //!< a value in the range [0, 1], which represents the attenuation of a light source
  TVec3            _attenuation;       //!< constant, linear and quadratic attenuation and of the light source by its distance
  t_fp             _cone_attenuation;  //!< light cone attenuation (attenuation from the center of the light cone to its borders)
  TVec3            _direction;         //!< direction of a spot light
  t_fp             _light_cone;        //!< full light cone angle of a spot light in radians [0, PI]
  t_fp             _maximum_radius;    //!< the maximum influence radius for a finite light source (in meter) (except the distance is calculated automatically `auto_distance`)
  t_fp             _cut_off_weight;    //!< radius cut off weight [0.0, 1.0]; soft (= 0.0) or hard (= 1.0)
  TLightProperties _properties;        //!< `TLightProperty` properties 
};
using TLightTable = std::vector<TLight>;


//! Additional properties that specify the behavior of the ambient source
enum class TAmbientLightProperty
{
  add_to_sun,                 //!< add the ambient light to the sun light
  add_to_global_illumination, //!< add the ambient light to the global illumination
  // ...
  NO_OF
};
using TAmbientLightProperties = std::bitset<static_cast<size_t>(TAmbientLightProperty::NO_OF)>;


/******************************************************************//**
* \brief   General parameters for an ambient light source.
* 
* \author  gernot
* \date    2018-02-20
* \version 1.0
**********************************************************************/
struct TAmbientLight
{
  TColor8                 _color;      //!< the ambient light color
  TAmbientLightProperties _properties; //!< `TLightProperty` properties 
};
using TAmbientLightTable = std::vector<TAmbientLight>;


} // Draw


// restore preprocessor definitions

#if defined( cutil__define_min_pushed )
#pragma pop_macro( "min" )
#endif

#if defined( cutil__define_max_pushed )
#pragma pop_macro( "max" )
#endif

#endif // Render_ILight_h_INCLUDED
