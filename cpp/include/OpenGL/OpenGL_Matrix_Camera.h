#pragma once

// glm

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// stl

#include <tuple>
#include <array>
#include <cmath>

#define _USE_MATH_DEFINES
#include <math.h> 


namespace OpenGL
{


float ToRad( float deg ) { return deg * (float)M_PI / 180.0f; }
float ToDeg( float rad ) { return rad * 180.0f / (float)M_PI; }

using TShaderInfo = std::tuple< std::string, int >;
using TVec2       = std::array< float, 2 >;
using TVec3       = std::array< float, 3 >;
using TVec4       = std::array< float, 4 >;
using TMat44      = std::array< TVec4, 4 >;
using TSize       = std::array< int, 2 >;

template< typename T_VEC >
TVec3 Cross( T_VEC a, T_VEC b ) { return { a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0] }; }

template< typename T_A, typename T_B >
float Dot( T_A a, T_B b ) { return a[0]*b[0] + a[1]*b[1] + a[2]*b[2]; }

template< typename T_VEC >
void Normalize( T_VEC & v )
{
  float len = sqrt( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] );
  v[0] /= len; v[1] /= len; v[2] /= len;
}

TMat44 Identity( void )
{
  return TMat44( { TVec4{ 1.0f, 0.0f, 0.0f, 0.0f },
                   TVec4{ 0.0f, 1.0f, 0.0f, 0.0f },
                   TVec4{ 0.0f, 0.0f, 1.0f, 0.0f },
                   TVec4{ 0.0f, 0.0f, 0.0f, 1.0f } } );
}

TMat44 Multiply( const TMat44 & matA, const TMat44 & matB )
{
  TMat44 matC;
  for ( int i0 = 0; i0 < 4; ++ i0 )
    for ( int i1 = 0; i1 < 4; ++ i1 )
      matC[i0][i1] = matB[i0][0] * matA[0][i1] + matB[i0][1] * matA[1][i1] + matB[i0][2] * matA[2][i1] + matB[i0][3] * matA[3][i1];
  return matC;
}

glm::mat4 ToGLM( const TMat44 &m )
{
  return glm::mat4(
    m[0][0], m[0][1], m[0][2], m[0][3],
    m[1][0], m[1][1], m[1][2], m[1][3],
    m[2][0], m[2][1], m[2][2], m[2][3],
    m[3][0], m[3][1], m[3][2], m[3][3]
  );
}

struct Camera
{
  TVec3 _pos    {0.0, -3.0, 0.0};
  TVec3 _target {0.0, 0.0, 0.0};
  TVec3 _up     {0.0, 0.0, 1.0};
  float _fov_y  {85.0};
  TSize _vp     {800, 600};
  float _near   {0.5};
  float _far    {100.0};

  TMat44 Perspective( void );
  TMat44 LookAt( void );

  static TMat44 Orthopraphic( float aspect, const std::array<float, 2> &depth_range );
  static TMat44 Orthopraphic( const std::array<float, 4> &view_rect, const std::array<float, 2> &depth_range );
  static TMat44 Orthopraphic( float l, float r, float b, float t, float n, float f );
};

TMat44 Camera::Perspective( void )
{
  float fn = _far + _near, f_n = _far - _near;
  float r = (float)_vp[0] / _vp[1], t = 1.0f / std::tan( ToRad( _fov_y ) / 2.0f );
  return TMat44{ TVec4{ t / r, 0.0f, 0.0f, 0.0f }, TVec4{ 0.0f, t, 0.0f, 0.0f }, TVec4{ 0.0f, 0.0f, -fn / f_n, -1.0 }, TVec4{ 0.0f, 0.0f, -2.0f*_far*_near / f_n, 0.0f } };
}

TMat44 Camera::LookAt( void )
{ 
  TVec3 mz = { _pos[0] - _target[0], _pos[1] - _target[1], _pos[2] - _target[2] };
  Normalize( mz );
  TVec3 my = { _up[0], _up[1], _up[2] };
  TVec3 mx = Cross( my, mz );
  Normalize( mx );
  my = Cross( mz, mx );
  
  TMat44 v{
    TVec4{ mx[0], my[0], mz[0], 0.0f },
    TVec4{ mx[1], my[1], mz[1], 0.0f },
    TVec4{ mx[2], my[2], mz[2], 0.0f },
    TVec4{ Dot(mx, _pos), Dot(my, _pos), -Dot(mz, _pos), 1.0f }
  };
  
  return v;
}

TMat44 Camera::Orthopraphic( float aspect, const std::array<float, 2> &depth_range )
{
  return Camera::Orthopraphic( -aspect, aspect, -1.0f, 1.0f, depth_range[0], depth_range[1] );
}

TMat44 Camera::Orthopraphic( const std::array<float, 4> &view_rect, const std::array<float, 2> &depth_range )
{
  return Camera::Orthopraphic( view_rect[0], view_rect[2], view_rect[1], view_rect[3], depth_range[0], depth_range[1] );
}

TMat44 Camera::Orthopraphic( float l, float r, float b, float t, float n, float f )
{
  return TMat44{ 
    TVec4{ 2.0f/(r-l),   0.0f,        0.0f,        0.0f },
    TVec4{ 0.0f,         2.0f/(t-b),  0.0f,        0.0f },
    TVec4{ 0.0f,         0.0f,        2.0f/(f-n),  0.0f },
    TVec4{ -(r+l)/(r-l), (t+b)/(t-b), (f+n)/(f-n), 1.0f } };
};


struct Mat44 : public TMat44
{
  Mat44( void ) : TMat44( Identity() ) {}
  Mat44( const Mat44 & ) = default;
  Mat44( const TMat44 & mat ) : TMat44( mat ) {}

  using pointer = float *;
  operator pointer(void) { return data()->data(); }

  Mat44 & Translate( const TVec3 & trans )
  {
    for ( int i = 0; i < 4; ++ i )
      (*this)[3][i] = (*this)[0][i] * trans[0] + (*this)[1][i] * trans[1] + (*this)[2][i] * trans[2] + (*this)[3][i];
    return *this;
  }
  
  Mat44 & Scale( const TVec3 & s )
  {
    for ( int i0 = 0; i0 < 3; ++ i0 )
      for ( int i1 = 0; i1 < 4; ++ i1 ) ( *this )[i0][i1] *= s[i0];
    return *this;
  }     

  Mat44 & RotateX( float angRad ) { return RotateHlp( angRad, 1, 2 ); }
  Mat44 & RotateY( float angRad ) { return RotateHlp( angRad, 2, 0 ); }
  Mat44 & RotateZ( float angRad ) { return RotateHlp( angRad, 0, 1 ); }
  Mat44 & RotateView( std::array< float, 3 > angRad )
  {
    RotateX( angRad[0] );
    RotateY( angRad[1] );
    RotateZ( angRad[2] );
    return *this;
  }

private:

  Mat44 & RotateHlp( float angRad, int a0, int a1)
  {
    TMat44 &matA = *this;
    TMat44 matB = matA;
    float sinAng = sin( angRad ), cosAng = cos( angRad );
    for ( int i = 0; i < 4; ++ i )
    {
      matB[a0][i] = matA[a0][i] * cosAng + matA[a1][i] * sinAng;
      matB[a1][i] = matA[a0][i] * -sinAng + matA[a1][i] * cosAng;
    }
    matA = matB;
    return *this;
  }
};


float Fract( float val ) { return val - trunc(val); }

float CalcAng( double currentTime, float intervall )
{ 
  return Fract( (float)( currentTime ) / intervall ) * 2.0f * (float)M_PI;
}

float CalcMove( double currentTime, float intervall, std::array< float, 2 > range )
{
  float pos = Fract( (float)( currentTime ) / intervall ) * 2.0f;
  pos = pos < 1.0 ? pos : 2.0f - pos;
  return range[0] + (range[1] - range[0]) * pos;
}

TVec3 EllipticalPosition( float a, float b, float angRag )
{
  float a_b = a * a - b * b;
  float ea = a_b > 0.0f ? sqrt( a_b ) : 0.0f;
  float eb = a_b < 0.0f ? sqrt( -a_b ) : 0.0f;
  return { a * std::sin( angRag ) - ea, b * std::cos( angRag ) - eb, 0.0f };
}

} // OpenGL 
