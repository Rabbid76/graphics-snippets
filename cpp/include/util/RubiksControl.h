#pragma once


// include

#include <Render_GLSL.h> 
#include <Render_GLM.h> 

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// STL
#include <vector>
#include <stdexcept>
#include <chrono>
#include <memory>
#include <cmath>

#define _USE_MATH_DEFINES
#include <math.h>


namespace Rubiks
{

static const int c_no_of_cubes = 27;
using TM44Cubes = std::array<glm::mat4, c_no_of_cubes>;
using TMapCubes = std::array<int, c_no_of_cubes>;


struct T_RUBIKS_DATA
{
  std::array<Render::GLSL::mat4, c_no_of_cubes> _model;
};

enum class TAxis { x, y, z };
enum class TRow { low, mid, high };
enum class TDirection { left, right };

struct TChangeOperation
{
  TAxis      _axis;      //!< rotation axis
  TRow       _row;       //!< rotation row along the rotation axis
  TDirection _direction; //!< direction of rotation
};


/******************************************************************//**
* \brief Representation of the positions and arrangement of the single
* sub cubes, of the Rubik's cube.
* 
* \author  gernot
* \date    2018-08-28
* \version 1.0
**********************************************************************/
class CCube
{
public:

  CCube( void ) { Init(); }
  virtual ~CCube() = default;

  const T_RUBIKS_DATA * Data( void ) const { return &_data; }

  CCube & Init( void );
  CCube & UpdateM44Cubes( void );
  CCube & Change( const TChangeOperation &op );

private:

  CCube & Rotate( const TChangeOperation &op );

  T_RUBIKS_DATA _data;    //!< final Rubik's cube data for rendering

  TMapCubes _cube_map;    //!< map the logical geometric position in the Rubik' cube to a corresponding sub cube 
  TM44Cubes _trans_scale; //!< translation and scale of the sub cubes
  TM44Cubes _current_pos; //!< current rotation of the sub cubes
  TM44Cubes _animation;   //!< additional animation transformation
};


/******************************************************************//**
* \brief Initializes the data structures.   
* 
* \author  gernot
* \date    2018-08-28
* \version 1.0
**********************************************************************/
CCube & CCube::Init( void )
{
  for ( int z=0; z<3; ++ z )
  {
    for ( int y=0; y<3; ++ y )
    {
      for ( int x=0; x<3; ++ x )
      {
        int i = z * 9 + y * 3 + x;
        _cube_map[i] = i;
        glm::mat4 part_scale = glm::scale( glm::mat4(1.0f), glm::vec3(1.0f/3.0f) );
        float offset = 1.1f * 2.0f / 3.0f;
        glm::vec3 trans_vec( (float)(x-1), (float)(y-1), (float)(z-1) );
        glm::mat4 part_trans = glm::translate( glm::mat4(1.0f), trans_vec * offset );
        _trans_scale[i] = part_trans * part_scale;
      }
    }
  }

  for ( int i=0; i<c_no_of_cubes; ++i )
  {
    _current_pos[i] = glm::mat4( 1.0f );
    _animation[i] = glm::mat4( 1.0f );
  }

  return UpdateM44Cubes();
}


/******************************************************************//**
* \brief Calculate the final model matrices of the sub cubes.    
* 
* \author  gernot
* \date    2018-08-28
* \version 1.0
**********************************************************************/
CCube & CCube::UpdateM44Cubes( void )
{
  for ( int i = 0; i < c_no_of_cubes; ++i )
    Render::GLM::CMat4(_data._model[i]) = _animation[i] * _current_pos[i] * _trans_scale[i];
  return *this;
}


/******************************************************************//**
* \brief Starts the rotation of a part of the Rubik's cube.
*
* Starts rotation animation
* 
* \author  gernot
* \date    2018-08-28
* \version 1.0
**********************************************************************/
CCube & CCube::Change( 
  const TChangeOperation &op ) //!< I - specifies the change operation
{
  return Rotate( op );
}


/******************************************************************//**
* \brief Calculate the rotation of a part of the Rubik's cube.
*
* Compute the new positions of the sub cubes and calculate the model
* matrices.
* 
* \author  gernot
* \date    2018-08-28
* \version 1.0
**********************************************************************/
CCube & CCube::Rotate( 
  const TChangeOperation &op ) //!< I - specifies the change operation
{
  static const std::unordered_map<TAxis, int> axis_map{ {TAxis::x, 0}, {TAxis::y, 1}, {TAxis::z, 2}};
  static const std::unordered_map<TRow, int> row_map{ {TRow::low, 0}, {TRow::mid, 1}, {TRow::high, 2}};
  int axis_i = axis_map.find(op._axis)->second;
  int row_i  = row_map.find(op._row)->second;

  // define the affected sub cubes
  int r_x[2]{ 0,2 };
  int r_y[2]{ 0,2 };
  int r_z[2]{ 0,2 };
  switch ( op._axis )
  {
    case TAxis::x: r_x[0] = r_x[1] = row_i; break;
    case TAxis::y: r_y[0] = r_y[1] = row_i; break;
    case TAxis::z: r_z[0] = r_z[1] = row_i; break;
  }

  // update the position model matrix of the affected sub cubes  
  static const std::array<glm::vec3, 3> axis_vec{ glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)};
  for ( int z = r_z[0]; z <= r_z[1]; ++ z )
  {
    for ( int y = r_y[0]; y <= r_y[1]; ++ y )
    {
      for ( int x = r_x[0]; x <= r_x[1]; ++ x )
      {
        int i = z * 9 + y * 3 + x;
        i = _cube_map[i];

        // TODO $$$ optimize this algorithm and ensure precision. Avoid floating point inaccuracy on continuous operations.
        // A rotation by 90 degrees can be performed by swapping 2 axis and inverting one of them,
        // dependent on the direction of the rotation left (-90) or right (90).  
        float angle = glm::radians( 90.0f ) * (op._direction == TDirection::left ? -1.0f : 1.0f);
        glm::mat4 rot_mat = glm::rotate(glm::mat4(1.0f), angle, axis_vec[axis_i] );
        _current_pos[i] = rot_mat * _current_pos[i];
      }
    }
  }

  // Recalculate the index map of the cubes
  static const std::array<std::array<int, 2>, 8> indices
  { 
    std::array<int, 2>{0, 0}, std::array<int, 2>{1, 0}, std::array<int, 2>{2, 0}, std::array<int, 2>{2, 1},
    std::array<int, 2>{2, 2}, std::array<int, 2>{1, 2}, std::array<int, 2>{0, 2}, std::array<int, 2>{0, 1} 
  };
  TMapCubes current_map = _cube_map;
  for ( int i = 0; i < 8; ++ i )
  {
    int j = ( op._direction == TDirection::left ? i + 6 : i + 2 ) % 8;
    
    std::array<int, 3> ao, an;
    ao[axis_i] = row_i;
    an[axis_i] = row_i;
    ao[(axis_i+1) % 3] = indices[i][0];
    an[(axis_i+1) % 3] = indices[j][0];
    ao[(axis_i+2) % 3] = indices[i][1];
    an[(axis_i+2) % 3] = indices[j][1];

    int io = ao[0] + ao[1] * 3 + ao[2] * 9;
    int in = an[0] + an[1] * 3 + an[2] * 9;

    _cube_map[in] = current_map[io];
  }

  // Update the final model matrices of the sub cubes
  UpdateM44Cubes();
  
  return *this;
}


}