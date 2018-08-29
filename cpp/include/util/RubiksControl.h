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
#include <deque>
#include <stdexcept>
#include <chrono>
#include <memory>
#include <cmath>
#include <ctime>

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
  int                                           _cube_hit = -1;
  int                                           _side_hit = 0;
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

using TChangeQueue = std::deque<TChangeOperation>;


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

  // TODO $$$ time control class ITimeing
  // - current time
  // - time spawn
  using TTime  = std::chrono::high_resolution_clock::time_point;

  CCube( float offset, float scale ) { Init( offset, scale ); }
  virtual ~CCube() = default;

  const T_RUBIKS_DATA * Data( void )   const     { return &_data; }
  T_RUBIKS_DATA *       Data( void )             { return &_data; }
  const TM44Cubes &     CubePosM44( void ) const { return _current_pos; } 
  
  bool  AnimationActive( void )  const { return _animation_is_active; }
  bool  AnimationPending( void ) const { return _animation_is_active || _pending_queue.empty() == false; }
  float Offset( void )           const { return _offset; }
  float Scale( void )            const { return _scale; }
  int   CubeIndex( int i )       const { return i < 0 || i >= c_no_of_cubes ? -1 : _cube_map[i]; }

  CCube & AnimationTime( double time_s ) { _animation_time_s = time_s; return *this; }

  CCube & Init( float offset, float scale );
  CCube & InitGeometry( float offset, float scale );
  CCube & Shuffle( int steps );
  CCube & Change( const TChangeOperation &op );
  CCube & Update( void );
  CCube & ResetHit( void );

  bool IntersectSidePlane( const glm::vec3 &r0_ray, const glm::vec3 &d_ray, int side_i, float &dist, glm::vec3 &pt );
  bool Intersect( const glm::vec3 &r0_ray, const glm::vec3 &d_ray, int &side_i, glm::vec3 &pt );
  bool IntersectedSubCube( int side_i, const glm::vec3 &pt, int &cube_i, int &mapped_cube_i );
  bool IntersectedSubCubeSide( int side_i, int cube_i, int &subcube_side_i );

private:

  CCube & UpdateM44Cubes( void );
  CCube & Rotate( const TChangeOperation &op );
  std::vector<int> SubCubeIndices( TAxis axis, TRow row );

  TTime TimeNow( void ) const
  { 
    return std::chrono::high_resolution_clock::now();
  }

  double DeltaTime_s( TTime time_start, TTime time_end ) const
  {
    auto   delta_time = time_end - time_start;
    double time_ms    = (double)std::chrono::duration_cast<std::chrono::milliseconds>(delta_time).count() / 1000.0;
    return time_ms;
  }

  int AxisIndex( TAxis axis ) const
  {
     static const std::unordered_map<TAxis, int> axis_map{ {TAxis::x, 0}, {TAxis::y, 1}, {TAxis::z, 2}};
     int axis_i = axis_map.find(axis)->second;
     return axis_i;
  }

  int RowIndex( TRow row ) const
  {
    static const std::unordered_map<TRow, int> row_map{ {TRow::low, 0}, {TRow::mid, 1}, {TRow::high, 2}};
    int row_i = row_map.find(row)->second;
    return row_i;
  }

  glm::vec3 Axis( int index ) const
  {
    static const std::array<glm::vec3, 3> axis_vec{ glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)};
    return axis_vec[index];
  }

  T_RUBIKS_DATA _data;                        //!< final Rubik's cube data for rendering

  float         _offset;                      //!< distance between 2 sub cubes (unscaled)
  float         _scale;                       //!< scale of the sub cube
  TMapCubes     _cube_map;                    //!< map the logical geometric position in the Rubik' cube to a corresponding sub cube 
  TM44Cubes     _trans_scale;                 //!< translation and scale of the sub cubes
  TM44Cubes     _current_pos;                 //!< current rotation of the sub cubes
  TM44Cubes     _animation;                   //!< additional animation transformation

  double        _animation_time_s{ 0 };       //!< time span for an animation
  TChangeQueue  _pending_queue;               //!< queue of pending change operations
  bool          _animation_is_active = false; //!< true: animation is active
  TTime         _current_time;                //!< current time
  TTime         _animation_start_time;        //!< start time of animation
};


/******************************************************************//**
* \brief Initializes the data structures.   
* 
* \author  gernot
* \date    2018-08-28
* \version 1.0
**********************************************************************/
CCube & CCube::Init( 
  float offset, //!< I - unscaled distance between 2 sub cubes
  float scale ) //!< I- scale of a single sub cube
{
  // initialize the size
  InitGeometry( offset, scale );

  // initialize animation and rotation matrices
  for ( int i=0; i<c_no_of_cubes; ++i )
  {
    _current_pos[i] = glm::mat4( 1.0f );
    _animation[i] = glm::mat4( 1.0f );
  }

  // Update the final model matrices of the sub cubes
  UpdateM44Cubes();

  return *this;
}


/******************************************************************//**
* \brief Initializes the size attributes and matrices.   
* 
* \author  gernot
* \date    2018-08-28
* \version 1.0
**********************************************************************/
CCube & CCube::InitGeometry( 
  float offset, //!< I - unscaled distance between 2 sub cubes
  float scale ) //!< I- scale of a single sub cube
{
  _offset = offset;
  _scale = scale;

  // calculate initial positions of sub cubes
  for ( int z=0; z<3; ++ z )
  {
    for ( int y=0; y<3; ++ y )
    {
      for ( int x=0; x<3; ++ x )
      {
        int i = z * 9 + y * 3 + x;
        _cube_map[i] = i;
        glm::mat4 part_scale = glm::scale( glm::mat4(1.0f), glm::vec3(_scale) );
        glm::vec3 trans_vec( (float)(x-1), (float)(y-1), (float)(z-1) );
        glm::mat4 part_trans = glm::translate( glm::mat4(1.0f), trans_vec * _offset );
        _trans_scale[i] = part_scale * part_trans;
      }
    }
  }

  return *this;
}


/******************************************************************//**
* \brief Shuffle the cube  
* 
* \author  gernot
* \date    2018-08-29
* \version 1.0
**********************************************************************/
CCube & CCube::Shuffle( 
  int steps ) //!< I - number of shuffles
{
  static const std::array<TAxis, 3> a_axis{TAxis::x, TAxis::y, TAxis::z};
  static const std::array<TRow, 3> a_row{TRow::low, TRow::mid, TRow::high};
  static const std::array<TDirection, 2> a_dir{TDirection::left, TDirection::right};

  std::srand( (unsigned int)std::time(nullptr) );

  // create random operations
  std::vector<TChangeOperation> shuffle_ops;
  for ( int i = 0; i < steps; ++ i )
  {
    TChangeOperation op;
    bool valid;
    do
    {
      op._axis      = a_axis[std::rand() % 3];
      op._row       = a_row[std::rand() % 3];
      op._direction = a_dir[std::rand() % 2];

      if ( shuffle_ops.empty() )
        break;

      // check if not inverse operation
      valid = shuffle_ops.back()._axis      != op._axis ||
              shuffle_ops.back()._row       != op._row ||
              shuffle_ops.back()._direction == op._direction;

      // check if not 3 equal operations in a row
      if ( valid && shuffle_ops.size() > 1 )
      {
        valid = shuffle_ops.front()._axis            != op._axis ||
                shuffle_ops.front()._row             != op._row ||
                shuffle_ops.front()._direction       != op._direction;
                (shuffle_ops.rbegin()+1)->_axis      != op._axis ||
                (shuffle_ops.rbegin()+1)->_row       != op._row ||
                (shuffle_ops.rbegin()+1)->_direction != op._direction; 
      }
    }
    while ( valid == false );

    shuffle_ops.push_back( op );
  }

  // add change operations to pending queue
  for ( auto & op : shuffle_ops )
     Change( op );

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
  _pending_queue.push_front( op );
  return *this;
}


/******************************************************************//**
* \brief Update animation and pending changes  
* 
* \author  gernot
* \date    2018-08-28
* \version 1.0
**********************************************************************/
CCube & CCube::Update( void )
{
  bool pending_changes = _pending_queue.empty() == false;
  _animation_is_active = _animation_is_active && pending_changes;

  _current_time = TimeNow();

  if ( pending_changes == false )
    return *this;
  TChangeOperation op = _pending_queue.back();

  if ( _animation_is_active )
  {
    double past_animation_time_s = DeltaTime_s( _animation_start_time, _current_time );
    if ( past_animation_time_s < _animation_time_s )
    {
      // get change information
      int              axis_i = AxisIndex( op._axis );
      int              row_i  = RowIndex( op._row );
      std::vector<int> cube_i = SubCubeIndices( op._axis, op._row );

      // update the position model matrix of the affected sub cubes 
      for ( auto i : cube_i )
      {
        double angle = glm::radians( 90.0f ) * (op._direction == TDirection::left ? -1.0f : 1.0f);
        angle *= past_animation_time_s / _animation_time_s;
        _animation[i] = glm::rotate( glm::mat4(1.0f), (float)angle, Axis(axis_i) );
      }

      // Update the final model matrices of the sub cubes
      UpdateM44Cubes();
      
      return *this;
    }
    _animation_is_active = false;
  }
  else if ( pending_changes )
  {
     _animation_is_active = true;
     _animation_start_time = TimeNow();
     return *this;
  }
 
  _pending_queue.pop_back();
  Rotate( op );

  // Update the final model matrices of the sub cubes
  UpdateM44Cubes();

  return *this;
}


/******************************************************************//**
* \brief Reset hit information  
* 
* \author  gernot
* \date    2018-08-29
* \version 1.0
**********************************************************************/
CCube & CCube::ResetHit( void )
{
  _data._cube_hit = -1;
  _data._side_hit = 0;

  return *this;
}


/******************************************************************//**
* \brief Intersect a plane on a side of the cube with a ray.  
* 
* \author  gernot
* \date    2018-08-29
* \version 1.0
**********************************************************************/
bool CCube::IntersectSidePlane( 
  const glm::vec3 &r0_ray, //! I - start point of ray
  const glm::vec3 &d_ray,  //! I - direction of the ray
  int              side_i, //! I - index of the side of the cube
  float           &dist,   //! O - distance to the intersection
  glm::vec3       &pt )    //! O - 
{
  // define the cube corner points and its faces

  static const std::array<glm::vec3, 8> cube_pts 
  {
    glm::vec3(-1.0f, -1.0f, -1.0f), // 0 : left  front bottom
    glm::vec3( 1.0f, -1.0f, -1.0f), // 1 : right front bottom
    glm::vec3(-1.0f,  1.0f, -1.0f), // 2 : left  back  bottom
    glm::vec3( 1.0f,  1.0f, -1.0f), // 3 : right back  bottom
    glm::vec3(-1.0f, -1.0f,  1.0f), // 4 : left  front top
    glm::vec3( 1.0f, -1.0f,  1.0f), // 5 : right front top
    glm::vec3(-1.0f,  1.0f,  1.0f), // 6 : left  back  top
    glm::vec3( 1.0f,  1.0f,  1.0f), // 7 : right back  top
  };
  static const std::array<std::array<int, 4>, 6> cube_faces
  {
    std::array<int, 4>{ 2, 0, 4, 6 }, // 0 : left
    std::array<int, 4>{ 1, 3, 7, 5 }, // 1 : right
    std::array<int, 4>{ 0, 1, 5, 4 }, // 2 : front
    std::array<int, 4>{ 2, 3, 7, 6 }, // 3 : back
    std::array<int, 4>{ 0, 1, 3, 2 }, // 4 : bottom
    std::array<int, 4>{ 4, 5, 7, 6 }  // 5 : top
  };

  float cube_sidelen_2 = (_offset + 1.0f) * _scale; // half side length of the entire cube
  
  // calculate the normal vector of the cube side
  const glm::vec3 &pa = cube_pts[cube_faces[side_i][0]];
  const glm::vec3 &pb = cube_pts[cube_faces[side_i][1]];
  const glm::vec3 &pc = cube_pts[cube_faces[side_i][3]];

  glm::vec3 dir      = glm::normalize( d_ray );
  glm::vec3 n_plane  = glm::normalize( glm::cross( pb - pa, pc - pa ) );
  glm::vec3 p0_plane = pa * cube_sidelen_2;

  if ( fabs( fabs( glm::dot( dir, n_plane ) ) - 1.0f ) < 0.0017f ) // 0.0017 < sin(1°)
    return false;

  // calculate the distance to the intersection with the plane defined by the side of the cube
  dist = glm::dot( pa - r0_ray, n_plane ) / glm::dot( dir, n_plane );
  
  // calculate the intersection point with the plane
  pt = r0_ray + dir * dist;

  return true;
}


/******************************************************************//**
* \brief  Intersect a ray with the cube and return the nearest point
* of intersection. 
* 
* \author  gernot
* \date    2018-08-29
* \version 1.0
**********************************************************************/
bool CCube::Intersect( 
  const glm::vec3 &r0_ray, //!< I - start point of the ray
  const glm::vec3 &d_ray,  //!< I - direction vector of the ray
  int             &side_i, //!< O - index of the intersected side 
  glm::vec3       &pt )    //!< O - point of intersection
{
  // find the nearest intersection of a side of the cube and the ray 

  int       isect_side = -1;
  float     isect_dist = std::numeric_limits<float>::max();
  glm::vec3 isect_pt;
  for ( int i = 0; i < 6; ++ i )
  {
    // calculate the intersection of the ray and a side of the cube map
    float dist;
    glm::vec3 x_pt;
    if ( IntersectSidePlane( r0_ray, d_ray, i, dist, x_pt ) == false )
      continue;

    // check if the intersection point is closer than the previous one
    if ( fabs(dist) > isect_dist )
      continue;

    // check if the intersection is on the side of the cube
    float cube_sidelen_2 = (_offset + 1.0f) * _scale; // half side length of the entire cube
    bool on_side = fabs( x_pt.x ) < cube_sidelen_2 + 0.001 &&
                    fabs( x_pt.y ) < cube_sidelen_2 + 0.001 &&
                    fabs( x_pt.z ) < cube_sidelen_2 + 0.001;
    if ( on_side == false )
      continue;

    isect_side = i;
    isect_dist = fabs(dist);
    isect_pt   = x_pt;
  }

  if ( isect_side < 0 )
    return false;

  side_i = isect_side;
  pt     = isect_pt;
  return true;
}


/******************************************************************//**
* \brief Get the index of the intersected sub cube.  
* 
* \author  gernot
* \date    2018-08-29
* \version 1.0
**********************************************************************/
bool CCube::IntersectedSubCube( 
  int              side_i,         //!< I - intersection side
  const glm::vec3 &pt,             //!< I - intersection point
  int             &cube_i,         //!< O - sub cube index
  int             &mapped_cube_i ) //!< O - mapped sub cube index
{
  if ( side_i < 0 )
    return false;

  // get intersected sub cube
  std::array<float, 3 > coords{ pt.x, pt.y, pt.z };
  std::vector<int> i_coord;
  bool hit_is_on = true;
  for ( float coord : coords )
  {
    int i = -1;
    if ( fabs( coord ) <= 1.0f * _scale )
      i = 1;
    else if ( coord <= -(_offset - 1.0f) * _scale)
      i = 0;
      else if ( coord >= (_offset - 1.0f) * _scale)
      i = 2;
    i_coord.push_back( i );
    hit_is_on = hit_is_on && i >= 0;
  }
  
  if ( hit_is_on )
  {
    int i = 9 * i_coord[2] + 3 * i_coord[1] + i_coord[0];
    cube_i        = i;
    mapped_cube_i = CubeIndex(i);
    return true;
  }
  return false;
}


/******************************************************************//**
* \brief Get the side index of the intersected sub cube.  
* 
* \author  gernot
* \date    2018-08-29
* \version 1.0
**********************************************************************/
bool CCube::IntersectedSubCubeSide( 
  int  side_i,          //!< I - index of intersected side
  int  cube_i,          //!< I - index of intersected sub cube
  int &subcube_side_i ) //!< O - index of intersected side of intersected sub cube 
{
  if ( side_i < 0 || cube_i < 0 )
    return false;
  
  // get the side on the intersected sub cube
  const glm::mat4 &cube_mat4 = CubePosM44()[cube_i];
  int axis_i = side_i / 2;
  float sign = side_i % 2 ? 1.0f : -1.0f;
  glm::vec3 test_vec = glm::vec3( cube_mat4[0][axis_i], cube_mat4[1][axis_i], cube_mat4[2][axis_i] ) * sign;

  int isect_i_side = -1;
  if ( test_vec.x < -0.5f )
    isect_i_side = 0;
  else if ( test_vec.x > 0.5f )
    isect_i_side = 1;
  else if ( test_vec.y < -0.5f )
    isect_i_side = 2;
  else if ( test_vec.y > 0.5f )
    isect_i_side = 3;
  else if ( test_vec.z < -0.5f )
    isect_i_side = 4;
  else if ( test_vec.z > 0.5f )
    isect_i_side = 5;

  if ( isect_i_side >= 0 )
  {
    subcube_side_i = isect_i_side;
    return true;
  }
  return false;
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
  // get change information
  int              axis_i = AxisIndex( op._axis );
  int              row_i  = RowIndex( op._row );
  std::vector<int> cube_i = SubCubeIndices( op._axis, op._row );

  // update the position model matrix of the affected sub cubes 
  for ( auto i : cube_i )
  {
    // TODO $$$ optimize this algorithm and ensure precision. Avoid floating point inaccuracy on continuous operations.
    // A rotation by 90 degrees can be performed by swapping 2 axis and inverting one of them,
    // dependent on the direction of the rotation left (-90) or right (90).  
    float angle = glm::radians( 90.0f ) * (op._direction == TDirection::left ? -1.0f : 1.0f);
    glm::mat4 rot_mat = glm::rotate( glm::mat4(1.0f), angle, Axis(axis_i) );
    _current_pos[i] = rot_mat * _current_pos[i];
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

  // reset animation matrices
  for ( int i=0; i<c_no_of_cubes; ++i )
    _animation[i] = glm::mat4( 1.0f );
  
  return *this;
}


/******************************************************************//**
* \brief Get all cubes in specific row of an specific axis  
* 
* \author  gernot
* \date    2018-08-29
* \version 1.0
**********************************************************************/
 std::vector<int> CCube::SubCubeIndices( 
   TAxis axis, //!< I - axis
   TRow  row ) //!< I - row
  {
    std::vector<int> indices;

    // define the affected sub cubes
    int row_i = RowIndex( row );
    int r_x[2]{ 0,2 };
    int r_y[2]{ 0,2 };
    int r_z[2]{ 0,2 };
    switch ( axis )
    {
      case TAxis::x: r_x[0] = r_x[1] = row_i; break;
      case TAxis::y: r_y[0] = r_y[1] = row_i; break;
      case TAxis::z: r_z[0] = r_z[1] = row_i; break;
    }

    // collect indices of collected sub cubes 
    for ( int z = r_z[0]; z <= r_z[1]; ++ z )
    {
      for ( int y = r_y[0]; y <= r_y[1]; ++ y )
      {
        for ( int x = r_x[0]; x <= r_x[1]; ++ x )
        {
          int i = z * 9 + y * 3 + x;
          indices.push_back( _cube_map[i] );
        }
      }
    }

    return indices;
  }      


}