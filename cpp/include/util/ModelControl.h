#pragma once

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// STL
#include <vector>
#include <array>
#include <stdexcept>
#include <chrono>
#include <memory>
#include <cmath>

#define _USE_MATH_DEFINES
#include <math.h>


// Controls model tracking
class CModelControl
{
public:

  using TM44         = glm::mat4;
  using TV3          = glm::vec3;
  using TPos         = std::array<int, 2>; 
  using TSize        = std::array<int, 2>;
  using TTime        = std::chrono::high_resolution_clock::time_point;
  using TAttenuation = std::array<float, 3>;

  const TM44 _ident_mat = glm::mat4( 1.0f );

  CModelControl( void ) { Init(); }
  virtual ~CModelControl() = default;

  TM44 OrbitMatrix( void )
  {
    return (_mouse_drag || (_auto_rotate && _auto_spin )) ? _current_orbit_mat * _orbit_mat : _orbit_mat;
  }

  TM44 AutoModelMatrix( void )
  {
    return _auto_rotate ? _current_model_mat * _model_mat : _model_mat;
  }

  CModelControl & VpSize( TSize vp_size ) { _vp_size = vp_size; return *this; }

  //! Set constant, linear and quadratic attenuation
  //! \f$ f(x) \; = \; \frac{ x }{ a[0] \,+\, a[1]*x \,+\, a[2]*x*x } \f$
  //! `attenuation = {1.0, 0.0, 0.0 }` is a linear uniform motion
  //! TODO function plotter "x/dot(vec3(1.0,x,x*x),vec3(1.0,0.2,0.2))"
  CModelControl & Attenuation( const TAttenuation &attenuation ) { _attenuation = attenuation; return *this; }  


  CModelControl & Init( void );
  CModelControl & Update( void );
  CModelControl & UpdatePosition( TPos pos );

  CModelControl & StartRotate( TPos pos )
  {
    _mouse_start = pos;
    return ChangeMotionMode( true, false, false );
  }

  CModelControl & FinishRotate( TPos pos )
  {
    UpdatePosition( pos );
    return ChangeMotionMode( false, true, true );
  }

  CModelControl & ToogleRotate( void )
  {
    return ChangeMotionMode( false, false, !_auto_rotate );
  }

private:

  TM44 RotateMat( float angle_rad, const TV3 &axis ) const 
  {
    return glm::rotate( glm::mat4( 1.0f ), angle_rad, axis );
  }

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

  CModelControl & ChangeMotionMode( bool drag, bool spin, bool automatically );

  TM44         _orbit_mat{ _ident_mat };             // persistent orbit orientation matrix
  TM44         _current_orbit_mat{ _ident_mat };     // additional orbit orientation while dragging
  bool         _mouse_drag{ false };                 // dragging on or off
  TV3          _mouse_drag_axis{ 1.0f, 0.0f, 0.0f }; // current drag axis
  float        _mouse_drag_angle{ 0.0f };            // current drag distance
  double       _mouse_drag_time_s{ 0 };              // current drag time in seconds
  TPos         _mouse_start{ 0, 0 };                 // start of mouse dragging operation
  bool         _auto_rotate{ true };                 // auto rotate on or of
  bool         _auto_spin{ false };                  // auto spin
  TM44         _model_mat{ _ident_mat };             // persistent model matrix
  TM44         _current_model_mat{ _ident_mat };     // current model matrix
  TTime        _currentTime;                         // current time
  TTime        _rotateStartTime;                     // start time of rotation
  TTime        _dragStartTime;                       // start time of dragging
  TSize        _vp_size;                             // viewport size
  TAttenuation _attenuation{ 1.0f, 0.0, 0.0 };       // attenuation of the automatic spin
};


CModelControl & CModelControl::Init( void )
{
  _currentTime     = TimeNow();  
  _rotateStartTime = _currentTime;
  _dragStartTime   = _currentTime;
  return *this;
}


CModelControl & CModelControl::Update( void )
{
  _currentTime       = TimeNow();
  _current_model_mat = _ident_mat;

  double delta_time_s = DeltaTime_s( _rotateStartTime, _currentTime );

  if ( _mouse_drag ) 
  {
    _current_orbit_mat = RotateMat( _mouse_drag_angle, _mouse_drag_axis );
  }
  else if ( _auto_rotate && _auto_spin )
  {
    if ( _mouse_drag_time_s > 0.0 )
    {
      double angle = _mouse_drag_angle * delta_time_s / _mouse_drag_time_s;
      if ( fabs( _attenuation[0] ) > 0.001 )
        angle /= _attenuation[0] + _attenuation[1] * angle + _attenuation[2] * angle*angle;
      _current_orbit_mat = RotateMat( (float)angle, _mouse_drag_axis );
    }
  }
  else if ( _auto_rotate && _auto_spin == false )
  {
    double dummy;
    double auto_angle_x = std::modf( delta_time_s / 13.0, &dummy ) * 2.0 * M_PI;
    double auto_angle_y = std::modf( delta_time_s / 17.0, &dummy ) * 2.0 * M_PI;
    _current_model_mat = glm::rotate( _current_model_mat, (float)auto_angle_x, glm::vec3( 1.0f, 0.0f, 0.0f ) );
    _current_model_mat = glm::rotate( _current_model_mat, (float)auto_angle_y, glm::vec3( 0.0f, 1.0f, 0.0f ) );
  }

  return *this;
}


CModelControl & CModelControl::UpdatePosition( TPos pos )
{
  if ( _mouse_drag == false )
    return *this;

  double dx = (double)(pos[0] - _mouse_start[0]) / (double)_vp_size[0];
  double dy = (double)(pos[1] - _mouse_start[1]) / (double)_vp_size[1];

  double len = std::sqrt(dx*dx + dy*dy);
  if ( len <= 0.000001 )
    return *this;
  
  _mouse_drag_angle  = (float)(M_PI * len);
  _mouse_drag_axis   = glm::vec3((float)(dy/len), 0.0f, (float)(dx/len));
  _mouse_drag_time_s = DeltaTime_s( _dragStartTime, TimeNow() );

  return *this;
}


CModelControl & CModelControl::ChangeMotionMode( bool drag, bool spin, bool automatically )
{
  bool new_drag = drag;
  bool new_auto = new_drag == false && automatically;
  bool new_spin = new_auto && spin;
  bool change   = _mouse_drag != new_drag || _auto_rotate != new_auto || _auto_spin != new_spin; 
  if ( change == false )
    return *this;

  if ( new_drag && _mouse_drag == false )
  {
    _dragStartTime     = TimeNow();
    _mouse_drag_angle  = 0.0f;
    _mouse_drag_time_s = 0.0;
  }
  if ( new_auto && _auto_rotate == false )
  {
    _rotateStartTime = TimeNow();
  }

  _mouse_drag  = new_drag;
  _auto_rotate = new_auto;
  _auto_spin   = new_spin;

  _orbit_mat         = _current_orbit_mat * _orbit_mat;
  _current_orbit_mat = _ident_mat;
  _model_mat         = _current_model_mat * _model_mat;
  _current_model_mat =_ident_mat;

  return *this;
}
