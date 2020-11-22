#pragma once

// includes

#include <Render_IMesh.h>

#include <array>
#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <cmath>
#include <cassert>

// preprocessor definitions

#define SIN_120  0.86602540378443864676
#define SIN_120f 0.86602540378443864676f 
#define MATH_PI  3.141592653589793238463    

// class definitions

namespace MeshDef
{


enum TUVunwrapping
{
  eUV_NON = 0,
  eUV_DEFAULT = 1,
  eUV_XY = 2,
  eUV_XZ = 3,
  eUV_YZ = 4,
  eUV_ANGLE_XY_ZY = 5,
  eUV_SPHERE = 6
};


//---------------------------------------------------------------------
// class TMeshData
//---------------------------------------------------------------------


template < class DATA_TYPE, class INDEX_TYPE >
class TMeshData
  : public Render::IMeshData<DATA_TYPE, INDEX_TYPE>
{
public:
  TMeshData( void ) {}
  virtual ~TMeshData() {}

  virtual void Reserve( INDEX_TYPE count ) = 0;
  virtual void ReserveFaces( INDEX_TYPE count ) = 0;

  virtual void Add( const DATA_TYPE *pt, const DATA_TYPE *tex )
  { Add( pt[0], pt[1], pt[2], tex[0], tex[1] );	}
  virtual void Add( DATA_TYPE ptX, DATA_TYPE ptY, DATA_TYPE ptZ, DATA_TYPE tU, DATA_TYPE tV )
  { assert( false ); }

  virtual void Add( const DATA_TYPE *pt, const DATA_TYPE *nv, bool normalized )
  { Add( pt[0], pt[1], pt[2], nv[0], nv[1], nv[2], normalized );	}
  virtual void Add( DATA_TYPE ptX, DATA_TYPE ptY, DATA_TYPE ptZ, DATA_TYPE nvX, DATA_TYPE nvY, DATA_TYPE nvZ, bool normalized )
  { assert( false ); }

  virtual void Add( const DATA_TYPE *pt, const DATA_TYPE *nv, bool normalized, const DATA_TYPE *tex )
  { Add( pt[0], pt[1], pt[2], nv[0], nv[1], nv[2], normalized, tex[0], tex[1] );	}
  virtual void Add( DATA_TYPE ptX, DATA_TYPE ptY, DATA_TYPE ptZ, DATA_TYPE nvX, DATA_TYPE nvY, DATA_TYPE nvZ, bool normalized, DATA_TYPE tU, DATA_TYPE tV )
  { assert( false ); }

  virtual void Add( DATA_TYPE ptX, DATA_TYPE ptY, DATA_TYPE ptZ, bool normalized, DATA_TYPE tU, DATA_TYPE tV )
  { Add( ptX, ptY, ptZ, ptX, ptY, ptZ, normalized, tU, tV ); }

  virtual void Add( const DATA_TYPE *pt, const DATA_TYPE *nv, bool normalized, const DATA_TYPE *tang, const DATA_TYPE *tex )
  { Add( pt[0], pt[1], pt[2], nv[0], nv[1], nv[2], normalized, tang[0], tang[1], tang[2], tang[3], tex[0], tex[1] );	}
  virtual void Add( DATA_TYPE ptX, DATA_TYPE ptY, DATA_TYPE ptZ, DATA_TYPE nvX, DATA_TYPE nvY, DATA_TYPE nvZ, bool normalized, DATA_TYPE tangX, DATA_TYPE tangY, DATA_TYPE tangZ, DATA_TYPE tangW, DATA_TYPE tU, DATA_TYPE tV )
  { assert( false ); }

  virtual void Add( 
    DATA_TYPE ptX, DATA_TYPE ptY, DATA_TYPE ptZ,
    DATA_TYPE nvX, DATA_TYPE nvY, DATA_TYPE nvZ, bool normalized,
    DATA_TYPE tU, DATA_TYPE tV,
    DATA_TYPE i0, DATA_TYPE i1, DATA_TYPE i2, DATA_TYPE i3,
    DATA_TYPE w0, DATA_TYPE w1, DATA_TYPE w2, DATA_TYPE w3 )
  { return Add( ptX, ptY, ptZ, nvX, nvY, nvZ, normalized, tU, tV ); }

  virtual void AddFace( INDEX_TYPE i0, INDEX_TYPE i1, INDEX_TYPE i2 ) = 0;
  virtual void AddFace( INDEX_TYPE i0, INDEX_TYPE i1, INDEX_TYPE i2, INDEX_TYPE i3 )
  {
    AddFace( i0, i1, i2 );
    AddFace( i0, i2, i3 );
  }
};


//---------------------------------------------------------------------
// class TTrianglesAdjacency
//---------------------------------------------------------------------


template < typename TINDEX >
struct TTrianglesAdjacency
{
  typedef std::vector< TINDEX > TIndexList;
  typedef std::vector< std::map< TINDEX, std::array< TINDEX, 2 > > > TAdjacencyMapSimple;
  typedef std::vector< std::map< TINDEX, std::vector< TINDEX > > > TAdjacencyMapComplex;

  virtual size_t NoOfFaceIndices( void ) const = 0;
  virtual TINDEX Face( size_t faceIndex ) const = 0;
  virtual void ReserveAdjacencies( size_t noOfIndices ) {}
  virtual void AddAdjacency( TINDEX adjacency ) = 0;

  void MapAdjacencyies( TAdjacencyMapSimple &adj, TINDEX p1, TINDEX p2, TINDEX a )
  {
    TINDEX tp1 = ( p1 < p2 ) ? p1 : p2;
    TINDEX tp2 = ( p1 < p2 ) ? p2 : p1;

    auto findIt = adj[tp1].find( tp2 );
    int aInx = findIt == adj[tp1].end() ? 0 : 1;
    for ( int inx = aInx; inx < 2; inx ++ )
      adj[tp1][tp2][inx] = a;
  }

  void MapAdjacencyies( TAdjacencyMapComplex &adj, TINDEX p1, TINDEX p2, TINDEX a )
  {
    TINDEX tp1 = ( p1 < p2 ) ? p1 : p2;
    TINDEX tp2 = ( p1 < p2 ) ? p2 : p1;
    adj[tp1][tp2].push_back( a );
  }

  void AddAdjacency( TAdjacencyMapSimple &adj, TINDEX p1, TINDEX p2, TINDEX p3 )
  {
    TINDEX tp1 = ( p1 < p2 ) ? p1 : p2;
    TINDEX tp2 = ( p1 < p2 ) ? p2 : p1;

    auto findIt = adj[tp1].find( tp2 );
    if ( findIt == adj[tp1].end() )
    {
      AddAdjacency( p3 );
      m_closed = false;
    }
    else
    {
      const std::array< TINDEX, 2 > &a = adj[tp1][tp2];
      AddAdjacency( a[0] == p3 ? a[1] : a[0] );
    }
  }

  void AddAdjacency( TAdjacencyMapSimple &adj, TIndexList &reverseInxMap, TINDEX p1, TINDEX p2, TINDEX p3 )
  {
    TINDEX tp1 = ( p1 < p2 ) ? p1 : p2;
    TINDEX tp2 = ( p1 < p2 ) ? p2 : p1;

    auto findIt = adj[tp1].find( tp2 );
    if ( findIt == adj[tp1].end() )
    {
      AddAdjacency( reverseInxMap[p3] );
      m_closed = false;
    }
    else
    {
      const std::array< TINDEX, 2 > &a = adj[tp1][tp2];
      AddAdjacency( reverseInxMap[ (a[0] == p3 ? a[1] : a[0]) ] );
    }
  }

  void AddAdjacency( TAdjacencyMapComplex &adj, TIndexList &reverseInxMap, TINDEX p1, TINDEX p2, TINDEX p3 )
  {
    TINDEX tp1 = ( p1 < p2 ) ? p1 : p2;
    TINDEX tp2 = ( p1 < p2 ) ? p2 : p1;

    auto findIt = adj[tp1].find( tp2 );
    if ( findIt == adj[tp1].end() )
    {
      AddAdjacency( reverseInxMap[ p3 ] );
      m_closed = false;
    }
    else
    {
      const std::vector< TINDEX > &a = findIt->second;
      if ( a.size() < 2 )
      {
        AddAdjacency( reverseInxMap[p3] );
        m_closed = false;
      }
      else if ( a.size() > 2 )
      {
        // TODO $$$ optimization : find adjacency pairs
        AddAdjacency( reverseInxMap[p3] );
        m_closed = false;
      }
      else
        AddAdjacency( reverseInxMap[ (a[0] == p3 ? a[1] : a[0]) ] );
    }
  }

  void CreateAdjacencyies( size_t maxIndex )
  {
    size_t noOfFaceIndices = NoOfFaceIndices();

    TAdjacencyMapSimple adj( maxIndex );
    for ( size_t faceInx = 0; faceInx < noOfFaceIndices-2; faceInx += 3 )
    {
      TINDEX faceI[3] = { Face( faceInx ), Face( faceInx+1 ), Face( faceInx+2 ) };
      if ( faceI[0] == faceI[1] || faceI[1] == faceI[2] || faceI[2] == faceI[0] )
        continue;

      MapAdjacencyies( adj, faceI[0], faceI[1], faceI[2] );
      MapAdjacencyies( adj, faceI[1], faceI[2], faceI[0] );
      MapAdjacencyies( adj, faceI[2], faceI[0], faceI[1] );
    }

    ReserveAdjacencies( noOfFaceIndices * 2 ); // 3 index face -> 6 index  adjacency
    for ( size_t faceInx = 0; faceInx < noOfFaceIndices-2; faceInx += 3 )
    {
      TINDEX faceI[3] = { Face( faceInx ), Face( faceInx+1 ), Face( faceInx+2 ) };
      if ( faceI[0] == faceI[1] || faceI[1] == faceI[2] || faceI[2] == faceI[0] )
        continue;

      AddAdjacency( faceI[0] );
      AddAdjacency( adj, faceI[0], faceI[1], faceI[2] );
      AddAdjacency( faceI[1] );
      AddAdjacency( adj, faceI[1], faceI[2], faceI[0] );
      AddAdjacency( faceI[2] );
      AddAdjacency( adj, faceI[2], faceI[0], faceI[1] );
    }
  }

  template< typename TSCALAR >
  static long long Floor( TSCALAR val, TSCALAR epsi )
  {
    return (long long)std::floor( val / epsi );
  }

  template < typename TPOINT >
  void CreateAdjacencyiesSimple( size_t noOfPoints, const TPOINT *points, bool( *f_ptequal )( const TPOINT &, const TPOINT & ) )
  {
    CreateAdjacencyies< TPOINT, TAdjacencyMapSimple >( noOfPoints, points, f_ptequal );
  }

  template < typename TPOINT >
  void CreateAdjacencyiesComplex( size_t noOfPoints, const TPOINT *points, bool( *f_ptequal )( const TPOINT &, const TPOINT & ) )
  {
    CreateAdjacencyies< TPOINT, TAdjacencyMapComplex >( noOfPoints, points, f_ptequal );
  }

  template < typename TPOINT, typename TADJACENCIESMAP >
  void CreateAdjacencyies( size_t noOfPoints, const TPOINT *points, bool(*f_ptequal)(const TPOINT &,const TPOINT &) )
  {
    size_t noOfFaceIndices = NoOfFaceIndices();

    TINDEX nextMapInx = 0;
    std::map< TINDEX, TINDEX > indexMap;
    TIndexList reverseIndexMap;
    std::map< TPOINT, TINDEX, bool(*)(const TPOINT &,const TPOINT &) > pointMap( f_ptequal );
    TADJACENCIESMAP adj( noOfPoints );
    for ( size_t faceInx = 0; faceInx < noOfFaceIndices-2; faceInx += 3 )
    {
      TINDEX faceI[3] = { Face( faceInx ), Face( faceInx+1 ), Face( faceInx+2 ) };
      TINDEX mappedI[3];
      for ( int triInx = 0; triInx < 3; triInx ++ )
      {
        TINDEX srcI = faceI[triInx];
        auto findInxIt = indexMap.find( srcI );
        if ( findInxIt != indexMap.end() )
        {
          mappedI[triInx] = findInxIt->second;
          continue;
        }

        const TPOINT &pt = points[srcI];
        auto findPtIt = pointMap.find( pt );
        if ( findPtIt != pointMap.end() )
        {
          mappedI[triInx] = indexMap[srcI] = findPtIt->second;
          continue;
        }

        TINDEX nextMapInx = (TINDEX)reverseIndexMap.size();
        mappedI[triInx] = indexMap[srcI] = pointMap[pt] = nextMapInx;
        reverseIndexMap.push_back( srcI );
      }

      if ( mappedI[0] == mappedI[1] || mappedI[1] == mappedI[2] || mappedI[2] == mappedI[0] )
        continue;

      MapAdjacencyies( adj, mappedI[0], mappedI[1], mappedI[2] );
      MapAdjacencyies( adj, mappedI[1], mappedI[2], mappedI[0] );
      MapAdjacencyies( adj, mappedI[2], mappedI[0], mappedI[1] );
    }

    ReserveAdjacencies( noOfFaceIndices * 2 ); // 3 index face -> 6 index  adjacency
    for ( size_t faceInx = 0; faceInx < noOfFaceIndices-2; faceInx += 3 )
    {
      TINDEX mappedI[3] = { indexMap[Face( faceInx )], indexMap[Face( faceInx+1 )], indexMap[Face( faceInx+2 )] };
      if ( mappedI[0] == mappedI[1] || mappedI[1] == mappedI[2] || mappedI[2] == mappedI[0] )
        continue;

      AddAdjacency( reverseIndexMap[ mappedI[0] ] );
      AddAdjacency( adj, reverseIndexMap, mappedI[0], mappedI[1], mappedI[2] );
      AddAdjacency( reverseIndexMap[ mappedI[1] ] );
      AddAdjacency( adj, reverseIndexMap, mappedI[1], mappedI[2], mappedI[0] );
      AddAdjacency( reverseIndexMap[ mappedI[2] ] );
      AddAdjacency( adj, reverseIndexMap, mappedI[2], mappedI[0], mappedI[1] );
    }
  }

  bool m_closed = true;
};


//---------------------------------------------------------------------
// class TTrianglesAdjacencyVectorRef
//---------------------------------------------------------------------



template < typename TINDEX >
struct TTrianglesAdjacencyVectorRef  
  : public TTrianglesAdjacency< TINDEX >
{
  TTrianglesAdjacencyVectorRef(
    std::vector< TINDEX >       &adjacencies,
    const std::vector< TINDEX > &faces )
    : m_faces( faces )
    , m_adjacencies( adjacencies )
  {}

  virtual size_t NoOfFaceIndices( void )  const override { m_faces.size(); }
  virtual TINDEX Face( size_t faceIndex ) const override { return m_faces[faceIndex]; }
  virtual void ReserveAdjacencies( size_t noOfIndices ) override { m_adjacencies.clear(); m_adjacencies.reserve( noOfIndices ); }
  virtual void AddAdjacency( TINDEX adjacency )         override { m_adjacencies.push_back( adjacency ); }

  const std::vector< TINDEX > & m_faces;
  std::vector< TINDEX >       & m_adjacencies;
};


//---------------------------------------------------------------------
// class TDefBase
//---------------------------------------------------------------------


template < class DATA_TYPE, class INDEX_TYPE >
class TDefBase
{
public: // public types

  typedef std::array< DATA_TYPE, 2 > CTexCoord;
  typedef std::array< DATA_TYPE, 3 > CVertex;
  typedef std::array< DATA_TYPE, 3 > CNormal;
  typedef std::array< INDEX_TYPE, 3 > CPrimitive;

  static CVertex Normalize(const CVertex &v)
  {
    DATA_TYPE len = std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    if (len == (DATA_TYPE)0)
        return v;
    return CVertex{v[0] / len, v[1] / len, v[2] / len};
  }

  static CVertex Cross(const CVertex &a, const CVertex &b)
  {
    return CVertex{ a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0] };
  } 
  
  TDefBase( void ) {}
  virtual ~TDefBase() {}
  virtual bool CreateMesh( TMeshData< DATA_TYPE, INDEX_TYPE > &def ) = 0;

  static CVertex RelativeToBox( CVertex &pt, CVertex &minBox, CVertex &maxBox );
  static CTexCoord CalcUV( TUVunwrapping uvMode, int axisU, int axisV, const CVertex &dir );
  static void CalculateFacesNV( bool separatePointsForFaces, std::vector< CVertex > &pts, std::vector< CPrimitive > &primitives, std::vector< CVertex > &nv );

  static void CalculateTetrahedron( bool separatePointsForFaces, std::vector< CVertex > &pts, std::vector< CTexCoord > &texCoord, std::vector< CPrimitive > &primitives );
  static void CalculateIcosahedron( bool separatePointsForFaces, std::vector< CVertex > &pts, std::vector< CTexCoord > &texCoord, std::vector< CPrimitive > &primitives );

  template < size_t VERTEX_COUNT, size_t FACE_COUNT >
  static void CreateVerticesAndPrimitives( 
    bool                                                              separatePointsForFaces, // I -
    const std::array< std::pair< CVertex, CTexCoord >, VERTEX_COUNT > &vertices,              // I -
    const std::array< CPrimitive, FACE_COUNT >                        &faces,                 // I -
    std::vector< CVertex >                                            &pts,                   // O -
    std::vector< CTexCoord >                                          &texCoord,              // O -
    std::vector< CPrimitive >                                         &primitives )           // O -
  {
    if ( separatePointsForFaces == false )
    {
      for ( auto & vertex : vertices )
      {
        pts.push_back( vertex.first );
        texCoord.push_back( vertex.second );
      }
      for ( auto & face : faces )
        primitives.push_back( face );
    }
    else
    {
      for ( auto & face : faces )
      {
        size_t ptSize = pts.size();
        primitives.push_back( CPrimitive{ (INDEX_TYPE)ptSize, (INDEX_TYPE)(ptSize + 1), INDEX_TYPE(ptSize + 2) } );

        for ( int triInx = 0; triInx < 3; triInx ++ )
        {
          pts.push_back( vertices[face[triInx]].first );
          texCoord.push_back( vertices[face[triInx]].second );
        }
      }
    }
  }

public: // public attributes 

  const DATA_TYPE c_sin120 = (DATA_TYPE)0.8660254;
  const DATA_TYPE c_1_sqrt3 = (DATA_TYPE)(1.0/1.7320508075);
};


template < class DATA_TYPE, class INDEX_TYPE >
typename TDefBase< DATA_TYPE, INDEX_TYPE >::CVertex TDefBase< DATA_TYPE, INDEX_TYPE >::RelativeToBox( 
  CVertex &pt,      // I -
  CVertex &minBox,  // I -
  CVertex &maxBox ) // I -
{
  CVertex lenV{ maxBox[0] - minBox[0], maxBox[1] - minBox[1], maxBox[2] - minBox[2] };
  return CVertex
  {
    lenV[0] <= (DATA_TYPE)0.00001 ? (DATA_TYPE)0.0 : ( ( pt[0] - minBox[0] ) / lenV[0] ),
    lenV[1] <= (DATA_TYPE)0.00001 ? (DATA_TYPE)0.0 : ( ( pt[1] - minBox[1] ) / lenV[1] ),
    lenV[2] <= (DATA_TYPE)0.00001 ? (DATA_TYPE)0.0 : ( ( pt[2] - minBox[2] ) / lenV[2] ),
  };
}


template < class DATA_TYPE, class INDEX_TYPE >
typename TDefBase< DATA_TYPE, INDEX_TYPE >::CTexCoord TDefBase< DATA_TYPE, INDEX_TYPE >::CalcUV( 
  TUVunwrapping  uvMode, // I -
  int            axisU,  // I -
  int            axisV,  // I -
  const CVertex &relV )  // I -
{
  DATA_TYPE uu = (DATA_TYPE)0.0;
  DATA_TYPE vv = (DATA_TYPE)0.0;
  switch ( uvMode )
  {
    default:
    case eUV_NON:
    case eUV_DEFAULT:
    case eUV_XY:
    case eUV_XZ:
    case eUV_YZ:
      uu = relV[axisU];
      vv = relV[axisV];
      break;
    case eUV_ANGLE_XY_ZY:
      {
        DATA_TYPE x = (DATA_TYPE)(( relV[0] - 0.5 ) * 2.0);
        DATA_TYPE y = (DATA_TYPE)(( relV[1] - 0.5 ) * 2.0);
        DATA_TYPE z = (DATA_TYPE)(( relV[2] - 0.5 ) * 2.0);
        uu = (DATA_TYPE)(fabs( atan2( y, -x ) ) / MATH_PI);
        while ( uu > (DATA_TYPE)1.0 ) uu -= (DATA_TYPE)1.0;
        vv = (DATA_TYPE)(fabs( atan2( y, -z ) ) / MATH_PI);
        while ( vv > (DATA_TYPE)1.0 ) vv -= (DATA_TYPE)1.0;
      }
      break;
    case eUV_SPHERE:
      {
        DATA_TYPE x = (DATA_TYPE)(( relV[0] - 0.5 ) * 2.0);
        DATA_TYPE y = (DATA_TYPE)(( relV[1] - 0.5 ) * 2.0);
        DATA_TYPE z = (DATA_TYPE)(( relV[2] - 0.5 ) * 2.0);
        uu = (DATA_TYPE)(fabs( atan2( y, -x ) ) / MATH_PI) + (DATA_TYPE)0.5;
        while ( uu > (DATA_TYPE)1.0 ) uu -= (DATA_TYPE)1.0;
        vv = (DATA_TYPE)(fabs( atan2( y, -z ) ) / MATH_PI) + (DATA_TYPE)0.5;
        while ( vv > (DATA_TYPE)1.0 ) vv -= (DATA_TYPE)1.0;
      }
      break;
  }
  return CTexCoord{ uu, (DATA_TYPE)1.0 - vv };
}


template < class DATA_TYPE, class INDEX_TYPE >
void TDefBase< DATA_TYPE, INDEX_TYPE >::CalculateFacesNV(
  bool                       separatePointsForFaces, // I -
  std::vector< CVertex >    &pts,                    // I -
  std::vector< CPrimitive > &primitives,             // I -
  std::vector< CNormal >    &nv )                    // O -
{
  nv = std::vector< CNormal >( pts.size() );

  if ( separatePointsForFaces )
  {
    for ( auto &primitive : primitives )
    {
      CVertex vec1{ pts[primitive[1]][0] - pts[primitive[0]][0], pts[primitive[1]][1] - pts[primitive[0]][1], pts[primitive[1]][2] - pts[primitive[0]][2] };
      CVertex vec2{ pts[primitive[2]][0] - pts[primitive[0]][0], pts[primitive[2]][1] - pts[primitive[0]][1], pts[primitive[2]][2] - pts[primitive[0]][2] };
      CVertex n = Normalize(Cross(vec1, vec2));
      nv[primitive[1]] = nv[primitive[2]] = nv[primitive[0]] = n;
    }
  }
  else
  {
    for ( size_t inx = 0; inx < pts.size(); inx ++ )
    {
      nv[inx] = Normalize(pts[inx]);
    }
  }
}


template < class DATA_TYPE, class INDEX_TYPE >
void TDefBase< DATA_TYPE, INDEX_TYPE >::CalculateTetrahedron( 
  bool                       separatePointsForFaces, // I -
  std::vector< CVertex >    &pts,                    // O -
  std::vector< CTexCoord >  &texCoord,               // O -
  std::vector< CPrimitive > &primitives )            // O -
{
  const DATA_TYPE s_8_9 = ( DATA_TYPE )std::sqrt(0.8/0.9);
  const DATA_TYPE s_2_9 = ( DATA_TYPE )std::sqrt(0.2/0.9);
  const DATA_TYPE s_2_3 = ( DATA_TYPE )std::sqrt(0.2/0.3);

  static std::array< std::pair< CVertex, CTexCoord >, 4 > vertices
  {
    std::pair< CVertex, CTexCoord >( CVertex{  0.0f,   0.0f,   1.0f     }, CTexCoord{ 1.0f, 1.0f } ),
    std::pair< CVertex, CTexCoord >( CVertex{  0.0f,  -s_8_9, -1.0f/3.0f }, CTexCoord{ 0.0f, 0.0f } ),
    std::pair< CVertex, CTexCoord >( CVertex{  s_2_3,  s_2_9, -1.0f/3.0f }, CTexCoord{ 1.0f, 0.0f } ),
    std::pair< CVertex, CTexCoord >( CVertex{ -s_2_3,  s_2_9, -1.0f/3.0f }, CTexCoord{ 0.0f, 1.0f } )
  };

  static std::array< CPrimitive, 4 > faces
  {
    CPrimitive{ 0, 1, 2 }, 
    CPrimitive{ 0, 2, 3 }, 
    CPrimitive{ 0, 3, 1 }, 
    CPrimitive{ 1, 3, 2 } 
  };

  CreateVerticesAndPrimitives< 4, 4 >( separatePointsForFaces, vertices, faces, pts, texCoord, primitives );
}


template < class DATA_TYPE, class INDEX_TYPE >
void TDefBase< DATA_TYPE, INDEX_TYPE >::CalculateIcosahedron( 
  bool                       separatePointsForFaces, // I -
  std::vector< CVertex >    &pts,                    // O -
  std::vector< CTexCoord >  &texCoord,               // O -
  std::vector< CPrimitive > &primitives )            // O -
{
  const DATA_TYPE c_sin120 = ( DATA_TYPE )0.8660254;

  static std::array< std::pair< CVertex, CTexCoord >, 12 > vertices
  {
    std::pair< CVertex, CTexCoord >( CVertex{  0.000f,  0.000f,  1.000f }, CTexCoord{ 0.0f, 0.0f } ),
    std::pair< CVertex, CTexCoord >( CVertex{  0.894f,  0.000f,  0.447f }, CTexCoord{ 0.0f, 0.0f } ),
    std::pair< CVertex, CTexCoord >( CVertex{  0.276f,  0.851f,  0.447f }, CTexCoord{ 0.0f, 0.0f } ),
    std::pair< CVertex, CTexCoord >( CVertex{ -0.724f,  0.526f,  0.447f }, CTexCoord{ 0.0f, 0.0f } ),
    std::pair< CVertex, CTexCoord >( CVertex{ -0.724f, -0.526f,  0.447f }, CTexCoord{ 0.0f, 0.0f } ),
    std::pair< CVertex, CTexCoord >( CVertex{  0.276f, -0.851f,  0.447f }, CTexCoord{ 0.0f, 0.0f } ),
    std::pair< CVertex, CTexCoord >( CVertex{  0.724f,  0.526f, -0.447f }, CTexCoord{ 0.0f, 0.0f } ),
    std::pair< CVertex, CTexCoord >( CVertex{ -0.276f,  0.851f, -0.447f }, CTexCoord{ 0.0f, 0.0f } ),
    std::pair< CVertex, CTexCoord >( CVertex{ -0.894f,  0.000f, -0.447f }, CTexCoord{ 0.0f, 0.0f } ),
    std::pair< CVertex, CTexCoord >( CVertex{ -0.276f, -0.851f, -0.447f }, CTexCoord{ 0.0f, 0.0f } ),
    std::pair< CVertex, CTexCoord >( CVertex{  0.724f, -0.526f, -0.447f }, CTexCoord{ 0.0f, 0.0f } ),
    std::pair< CVertex, CTexCoord >( CVertex{  0.000f,  0.000f, -1.000f }, CTexCoord{ 0.0f, 0.0f } )
  };

  static bool texCoordsInitialzed = false;
  if ( texCoordsInitialzed == false )
  {
    for ( auto & vertex : vertices )
      vertex.second = CalcUV( eUV_ANGLE_XY_ZY, 0, 2, vertex.first );
  }

  static std::array< CPrimitive, 20 > faces
  {
    CPrimitive{  2,  0,  1 },
    CPrimitive{  3,  0,  2 },
    CPrimitive{  4,  0,  3 },
    CPrimitive{  5,  0,  4 },
    CPrimitive{  1,  0,  5 },
    CPrimitive{ 11,  7,  6 },
    CPrimitive{ 11,  8,  7 },
    CPrimitive{ 11,  9,  8 },
    CPrimitive{ 11, 10,  9 },
    CPrimitive{ 11,  6, 10 },
    CPrimitive{  1,  6,  2 },
    CPrimitive{  2,  7,  3 },
    CPrimitive{  3,  8,  4 },
    CPrimitive{  4,  9,  5 },
    CPrimitive{  5, 10,  1 },
    CPrimitive{  2,  6,  7 },
    CPrimitive{  3,  7,  8 },
    CPrimitive{  4,  8,  9 },
    CPrimitive{  5,  9, 10 },
    CPrimitive{  1, 10,  6 }
  };

  CreateVerticesAndPrimitives< 12, 20 >( separatePointsForFaces, vertices, faces, pts, texCoord, primitives );
}


//---------------------------------------------------------------------
// class TDef
//---------------------------------------------------------------------


template < class DATA_TYPE, class INDEX_TYPE >
class TDef 
  : public MeshDef::TMeshData< DATA_TYPE, INDEX_TYPE >
{
public: // public types

  using TAttribute2 = Render::TAttributeVector<DATA_TYPE, 2, 0, 0>;
  using TAttribute3 = Render::TAttributeVector<DATA_TYPE, 3, 0, 0>;
  using TAttribute4 = Render::TAttributeVector<DATA_TYPE, 4, 0, 0>;
  using TFaces      = Render::TIndexVector<INDEX_TYPE, 3>;

  using TAttributeContainer = Render::IAttributeData<DATA_TYPE>;
  using TIndexContainer     = Render::IIndexData<INDEX_TYPE>;

  typedef std::array< DATA_TYPE, 2 > TVec2;
  typedef std::array< DATA_TYPE, 3 > TVec3;
  typedef std::array< DATA_TYPE, 4 > TVec4;
  typedef std::array< INDEX_TYPE, 3 > TFace;

  TVec3 Normalize(const TVec3 &v)
  {
    DATA_TYPE len = std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    if (len == (DATA_TYPE)0)
        return v;
    return TVec3{v[0] / len, v[1] / len, v[2] / len};
  }
  
  typedef std::vector< INDEX_TYPE > TAdjacencies;

  struct TTrianglesAdjacencyFactory  
    : public TTrianglesAdjacency< INDEX_TYPE >
  {
    TTrianglesAdjacencyFactory(
      TAdjacencies                         &adjacencies,
      const Render::IIndexData<INDEX_TYPE> &faces )
      : m_faces( faces )
      , m_adjacencies( adjacencies )
    {}

    virtual size_t NoOfFaceIndices( void )      const override { return m_faces.size(); }
    virtual INDEX_TYPE Face( size_t faceIndex ) const override { return m_faces.data()[faceIndex]; }
    virtual void ReserveAdjacencies( size_t noOfIndices ) override { m_adjacencies.clear(); m_adjacencies.reserve( noOfIndices ); }
    virtual void AddAdjacency( INDEX_TYPE adjacency )     override { m_adjacencies.push_back( adjacency ); }

    const Render::IIndexData<INDEX_TYPE> & m_faces;
    TAdjacencies                         & m_adjacencies;
  };

public: // public operations

  TDef( void ) : MeshDef::TMeshData< DATA_TYPE, INDEX_TYPE >() {}
  virtual ~TDef() {}

  virtual Render::TMeshFaceType       FaceType( void )            const override { return Render::TMeshFaceType::triangles; }
  virtual Render::TMeshFaceSizeKind   FaceSizeKind( void )        const override { return Render::TMeshFaceSizeKind::constant; } 
  virtual Render::TMeshIndexKind      IndexKind( void )           const override { return Render::TMeshIndexKind::common; }
  virtual Render::TMeshAttributePack  Pack( void )                const override { return Render::TMeshAttributePack::separated_tightly; }
  virtual const TAttributeContainer & Vertices( void )            const override { return m_pt; } 
  virtual const TIndexContainer     * Indices( void )             const override { return &m_faces; }
  virtual INDEX_TYPE                  FaceSize( void )            const override { return 3; }
  virtual const TIndexContainer     * FaceSizes( void )           const override { return nullptr; }
  virtual INDEX_TYPE                  FaceRestart( void )         const override { return 0; }
  virtual Render::TMeshNormalKind     NormalKind( void )          const override { return Render::TMeshNormalKind::vertex; }
  virtual const TAttributeContainer * Normals( void )             const override { return &m_nv;}
  virtual const TIndexContainer     * NormalIndices( void )       const override { return Indices(); }
  virtual const TAttributeContainer * FaceNormals( void )         const override { return nullptr; }
  virtual const TIndexContainer     * FaceNormalIndices( void )   const override { return nullptr; }
  virtual const TAttributeContainer * TextureCoordinates( void )  const override { return &m_tex; }
  virtual const TIndexContainer     * TextureCoordIndices( void ) const override { return Indices(); }
  virtual const TAttributeContainer * Colors( void )              const override { return nullptr; }
  virtual const TIndexContainer     * ColorIndices( void )        const override { return nullptr; }

  const std::vector< TVec3 > & Pt( void )    const { return m_pt._av; }
  const std::vector< TVec3 > & NV( void )    const { return m_nv._av; }
  const std::vector< TVec2 > & Tex( void )   const { return m_tex._av; }
  const std::vector< TFace > & Faces( void ) const { return m_faces._iv; }
  const std::vector< TVec4 > & Tang( void )  const { return m_tang._av; }
  TAdjacencies & Adjacencies( void ) { return m_adjacencies; }

  size_t FaceBufferSize( void ) const { return m_faces._iv.size(); }
  const unsigned char* FaceBuffer( void ) const { return (unsigned char*)m_faces._iv.data(); }

  bool Closed( void ) const { return m_closed; }

  virtual void Reserve( INDEX_TYPE count ) override
  {
    m_pt._av.reserve( (size_t)count );
    m_nv._av.reserve( (size_t)count );
    m_tang._av.reserve( (size_t)count );
    m_tex._av.reserve( (size_t)count );
  }

  virtual void ReserveFaces( INDEX_TYPE count ) override
  {
    m_faces._iv.reserve( (size_t)count );
  }

  virtual void Add( const DATA_TYPE *pt, const DATA_TYPE *tex ) override
  { 
    m_pt._av.push_back( TVec3{ pt[0], pt[1], pt[2] } );
    m_tex._av.push_back( TVec2{ tex[0], tex[1] } );
  }
  virtual void Add( DATA_TYPE ptX, DATA_TYPE ptY, DATA_TYPE ptZ, DATA_TYPE tU, DATA_TYPE tV ) override
  { 
    m_pt._av.push_back( TVec3{ ptX, ptY, ptZ } );
    m_tex._av.push_back( TVec2{ tU, tV } );
  }

  virtual void Add( const DATA_TYPE *pt, const DATA_TYPE *nv, bool normalized ) override
  { 
    m_pt._av.push_back( TVec3{ pt[0], pt[1], pt[2] } );
    m_nv._av.push_back( TVec3{ nv[0], nv[1], nv[2] } );
    if ( !normalized )
      m_nv._av.back() = Normalize( m_nv._av.back() );
  }
  virtual void Add( DATA_TYPE ptX, DATA_TYPE ptY, DATA_TYPE ptZ, DATA_TYPE nvX, DATA_TYPE nvY, DATA_TYPE nvZ, bool normalized ) override
  { 
    m_pt._av.push_back( TVec3{ ptX, ptY, ptZ } );
    m_nv._av.push_back( TVec3{ nvX, nvY, nvZ } );
    if ( !normalized )
      m_nv._av.back() = Normalize( m_nv._av.back() );
  }

  virtual void Add( const DATA_TYPE *pt, const DATA_TYPE *nv, bool normalized, const DATA_TYPE *tex ) override
  { 
    m_pt._av.push_back( TVec3{ pt[0], pt[1], pt[2] } );
    m_nv._av.push_back( TVec3{ nv[0], nv[1], nv[2] } );
    if ( !normalized )
      m_nv._av.back() = Normalize( m_nv._av.back() );
    m_tex._av.push_back( TVec2{ tex[0], tex[1] } );
  }
  virtual void Add( DATA_TYPE ptX, DATA_TYPE ptY, DATA_TYPE ptZ, DATA_TYPE nvX, DATA_TYPE nvY, DATA_TYPE nvZ, bool normalized, DATA_TYPE tU, DATA_TYPE tV ) override
  { 
    m_pt._av.push_back( TVec3{ ptX, ptY, ptZ } );
    m_nv._av.push_back( TVec3{ nvX, nvY, nvZ } );
    if ( !normalized )
      m_nv._av.back() = Normalize( m_nv._av.back() );
    m_tex._av.push_back( TVec2{ tU, tV } );
  }

  virtual void Add( const DATA_TYPE *pt, const DATA_TYPE *nv, bool normalized, const DATA_TYPE *tang, const DATA_TYPE *tex ) override
  { 
    m_pt._av.push_back( TVec3{ pt[0], pt[1], pt[2] } );
    m_nv._av.push_back( TVec3{ nv[0], nv[1], nv[2] } );
    if ( !normalized )
      m_nv._av.back() = Normalize( m_nv._av.back() );
    m_tang._av.push_back( TVec4{ tang[0], tang[1], tang[2], tang[3] } );
    m_tex._av.push_back( TVec2{ tex[0], tex[1] } );
  }
  virtual void Add( DATA_TYPE ptX, DATA_TYPE ptY, DATA_TYPE ptZ, DATA_TYPE nvX, DATA_TYPE nvY, DATA_TYPE nvZ, bool normalized, DATA_TYPE tangX, DATA_TYPE tangY, DATA_TYPE tangZ, DATA_TYPE tangW, DATA_TYPE tU, DATA_TYPE tV ) override
  { 
    m_pt._av.push_back( TVec3{ ptX, ptY, ptZ } );
    m_nv._av.push_back( TVec3{ nvX, nvY, nvZ } );
    if ( !normalized )
      m_nv._av.back() = Normalize( m_nv._av.back() );
    m_tang._av.push_back( TVec4{ tangX, tangY, tangZ, tangW } );
    m_tex._av.push_back( TVec2{ tU, tV } );
  }

  virtual void AddFace( INDEX_TYPE i0, INDEX_TYPE i1, INDEX_TYPE i2 ) override
  {
    m_faces._iv.push_back( TFace{ i0, i1, i2 } );
  }

  bool CreateAdjacencies( void )
  {
    if ( m_adjacencies.empty() == false )
      return true;
    if ( Faces().empty() )
      return false;

    TTrianglesAdjacencyFactory adjacencyFactory( m_adjacencies, m_faces );

    adjacencyFactory.CreateAdjacencyiesComplex( Pt().size(), Pt().data(), []( const TVec3 & A, const TVec3 & B ) -> bool
    {
      double epsi = 0.001;                                     

      long long sA = TTrianglesAdjacencyFactory::Floor( A[0], epsi );
      long long sB = TTrianglesAdjacencyFactory::Floor( B[0], epsi );
      if ( sA != sB ) return sA < sB;

      sA = TTrianglesAdjacencyFactory::Floor( A[1], epsi );
      sB = TTrianglesAdjacencyFactory::Floor( B[1], epsi );
      if ( sA != sB ) return sA < sB;

      sA = TTrianglesAdjacencyFactory::Floor( A[2], epsi );
      sB = TTrianglesAdjacencyFactory::Floor( B[2], epsi );
      return sA < sB; 
    } );

    m_closed = adjacencyFactory.m_closed;

    return true;
  }

  void Smooth( bool smooth ) { m_smooth = smooth; }
  virtual bool Smooth( void ) const override { return m_smooth; }

protected: // protected attributes

  
  TAttribute3  m_pt;
  TAttribute3  m_nv;
  TAttribute4  m_tang;
  TAttribute2  m_tex;
  TFaces       m_faces;
  TAdjacencies m_adjacencies;
  bool         m_closed = false;
  bool         m_smooth = false;
};


//---------------------------------------------------------------------
// class TPlaneDef
//---------------------------------------------------------------------


template < class DATA_TYPE, class INDEX_TYPE >
class TPlaneDef
  : public TDefBase< DATA_TYPE, INDEX_TYPE > 
{
public:
  TPlaneDef( DATA_TYPE xSize, DATA_TYPE ySize, INDEX_TYPE xDivs, INDEX_TYPE yDivs )
    : m_xSize( xSize ), m_ySize( ySize ), m_xDivs( xDivs ), m_yDivs( yDivs ) {}
  virtual ~TPlaneDef() {}
  virtual bool CreateMesh( TMeshData< DATA_TYPE, INDEX_TYPE > &def ) override;
private:
  DATA_TYPE m_xSize;
  DATA_TYPE m_ySize;
  INDEX_TYPE m_xDivs;
  INDEX_TYPE m_yDivs;
};

template < class DATA_TYPE, class INDEX_TYPE >
bool TPlaneDef< DATA_TYPE, INDEX_TYPE >::CreateMesh( TMeshData< DATA_TYPE, INDEX_TYPE > &def )
{
  bool ret = true;

  float x2 = m_xSize / 2.0f;
  float z2 = m_ySize / 2.0f;
  float iFactor = (float)m_ySize / m_yDivs;
  float jFactor = (float)m_xSize / m_xDivs;
  float texi = 1.0f / m_yDivs;
  float texj = 1.0f / m_xDivs;
  float x, z;
  int vidx = 0, tidx = 0;
  for( int i = 0; i <= (int)m_yDivs; i++ ) {
    z = iFactor * i - z2;
    for( int j = 0; j <= (int)m_xDivs; j++ ) {
      x = jFactor * j - x2;
      def.Add( x, z, 0.0f, 0.0f, 0.0f, 1.0f, false, j * texj, i * texi );
    }
  }

  unsigned int rowStart, nextRowStart;
  int idx = 0;
  for( int i = 0; i < (int)m_yDivs; i++ )
  {
    rowStart = i * (m_xDivs+1);
    nextRowStart = (i+1) * (m_xDivs+1);
    for( int j = 0; j < (int)m_xDivs; j++ )
    {
      def.AddFace( rowStart + j, rowStart + j + 1, nextRowStart + j );
      def.AddFace( rowStart + j + 1, nextRowStart + j + 1, nextRowStart + j );
      //def.AddFace( rowStart + j, nextRowStart + j, nextRowStart + j + 1 );
      //def.AddFace( rowStart + j, nextRowStart + j + 1,  );
    }
  }

  return ret;
}


//---------------------------------------------------------------------
// class TCubeDef
//---------------------------------------------------------------------


template < class DATA_TYPE, class INDEX_TYPE >
class TCubeDef
  : public TDefBase< DATA_TYPE, INDEX_TYPE > 
{
public:
  enum TCubeType : int
  {
    eCT_UV_per_side,
    eCT_UV_around
  };
public:
  TCubeDef( DATA_TYPE length, DATA_TYPE width, DATA_TYPE height, TCubeType type )
    : m_length( length ), m_width( width ), m_height( height ), m_type( type ) {}
  virtual ~TCubeDef() {}
  virtual bool CreateMesh( TMeshData< DATA_TYPE, INDEX_TYPE > &def ) override;
private:
  DATA_TYPE m_length;
  DATA_TYPE m_width;
  DATA_TYPE m_height;
  TCubeType m_type;
};

template < class DATA_TYPE, class INDEX_TYPE >
bool TCubeDef< DATA_TYPE, INDEX_TYPE >::CreateMesh( TMeshData< DATA_TYPE, INDEX_TYPE > &def )
{
  bool ret = true;

  const INDEX_TYPE sides = 6;

  DATA_TYPE lenX = m_length / 2.0f;
  DATA_TYPE lenY = m_width / 2.0f;
  DATA_TYPE lenZ = m_height / 2.0f;

  switch ( m_type )
  {
    default:
    case eCT_UV_per_side:
      def.Reserve( 4 * sides );
      def.ReserveFaces( 2 * sides );

      def.Add( -lenX,  lenY, -lenZ, (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)-1.0, true, (DATA_TYPE)0.0, (DATA_TYPE)0.0 );
      def.Add(  lenX,  lenY, -lenZ, (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)-1.0, true, (DATA_TYPE)1.0, (DATA_TYPE)0.0 );
      def.Add(  lenX, -lenY, -lenZ, (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)-1.0, true, (DATA_TYPE)1.0, (DATA_TYPE)1.0 );
      def.Add( -lenX, -lenY, -lenZ, (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)-1.0, true, (DATA_TYPE)0.0, (DATA_TYPE)1.0 );

      def.Add( -lenX, -lenY, lenZ, (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)1.0, true, (DATA_TYPE)0.0, (DATA_TYPE)0.0 );
      def.Add(  lenX, -lenY, lenZ, (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)1.0, true, (DATA_TYPE)1.0, (DATA_TYPE)0.0 );
      def.Add(  lenX,  lenY, lenZ, (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)1.0, true, (DATA_TYPE)1.0, (DATA_TYPE)1.0 );
      def.Add( -lenX,  lenY, lenZ, (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)1.0, true, (DATA_TYPE)0.0, (DATA_TYPE)1.0 );

      def.Add( -lenX, -lenY, -lenZ, (DATA_TYPE)0.0, (DATA_TYPE)-1.0, (DATA_TYPE)0.0, true, (DATA_TYPE)0.0, (DATA_TYPE)0.0 );
      def.Add(  lenX, -lenY, -lenZ, (DATA_TYPE)0.0, (DATA_TYPE)-1.0, (DATA_TYPE)0.0, true, (DATA_TYPE)1.0, (DATA_TYPE)0.0 );
      def.Add(  lenX, -lenY,  lenZ, (DATA_TYPE)0.0, (DATA_TYPE)-1.0, (DATA_TYPE)0.0, true, (DATA_TYPE)1.0, (DATA_TYPE)1.0 );
      def.Add( -lenX, -lenY,  lenZ, (DATA_TYPE)0.0, (DATA_TYPE)-1.0, (DATA_TYPE)0.0, true, (DATA_TYPE)0.0, (DATA_TYPE)1.0 );

      def.Add(  lenX,  lenY, -lenZ, (DATA_TYPE)0.0, (DATA_TYPE)1.0, (DATA_TYPE)0.0, true, (DATA_TYPE)0.0, (DATA_TYPE)0.0 );
      def.Add( -lenX,  lenY, -lenZ, (DATA_TYPE)0.0, (DATA_TYPE)1.0, (DATA_TYPE)0.0, true, (DATA_TYPE)1.0, (DATA_TYPE)0.0 );
      def.Add( -lenX,  lenY,  lenZ, (DATA_TYPE)0.0, (DATA_TYPE)1.0, (DATA_TYPE)0.0, true, (DATA_TYPE)1.0, (DATA_TYPE)1.0 );
      def.Add(  lenX,  lenY,  lenZ, (DATA_TYPE)0.0, (DATA_TYPE)1.0, (DATA_TYPE)0.0, true, (DATA_TYPE)0.0, (DATA_TYPE)1.0 );

      def.Add( -lenX,  lenY, -lenZ, (DATA_TYPE)-1.0, (DATA_TYPE)0.0, (DATA_TYPE)0.0, true, (DATA_TYPE)0.0, (DATA_TYPE)0.0 );
      def.Add( -lenX, -lenY, -lenZ, (DATA_TYPE)-1.0, (DATA_TYPE)0.0, (DATA_TYPE)0.0, true, (DATA_TYPE)1.0, (DATA_TYPE)0.0 );
      def.Add( -lenX, -lenY,  lenZ, (DATA_TYPE)-1.0, (DATA_TYPE)0.0, (DATA_TYPE)0.0, true, (DATA_TYPE)1.0, (DATA_TYPE)1.0 );
      def.Add( -lenX,  lenY,  lenZ, (DATA_TYPE)-1.0, (DATA_TYPE)0.0, (DATA_TYPE)0.0, true, (DATA_TYPE)0.0, (DATA_TYPE)1.0 );

      def.Add(  lenX, -lenY, -lenZ, (DATA_TYPE)1.0, (DATA_TYPE)0.0, (DATA_TYPE)0.0, true, (DATA_TYPE)0.0, (DATA_TYPE)0.0 );
      def.Add(  lenX,  lenY, -lenZ, (DATA_TYPE)1.0, (DATA_TYPE)0.0, (DATA_TYPE)0.0, true, (DATA_TYPE)1.0, (DATA_TYPE)0.0 );
      def.Add(  lenX,  lenY,  lenZ, (DATA_TYPE)1.0, (DATA_TYPE)0.0, (DATA_TYPE)0.0, true, (DATA_TYPE)1.0, (DATA_TYPE)1.0 );
      def.Add(  lenX, -lenY,  lenZ, (DATA_TYPE)1.0, (DATA_TYPE)0.0, (DATA_TYPE)0.0, true, (DATA_TYPE)0.0, (DATA_TYPE)1.0 );

      def.AddFace(  0,  1,  2,  3 );
      def.AddFace(  4,  5,  6,  7 );
      def.AddFace(  8,  9, 10, 11 );
      def.AddFace( 12, 13, 14, 15 );
      def.AddFace( 16, 17, 18, 19 );
      def.AddFace( 20, 21, 22, 23 );
      break;

    case eCT_UV_around:
      def.Reserve( 14 );
      def.ReserveFaces( 2 * sides );

      def.Add( -lenX, -lenY, lenZ, false, (DATA_TYPE)(1.0/3.0), (DATA_TYPE)0.75 );
      def.Add(  lenX, -lenY, lenZ, false, (DATA_TYPE)(2.0/3.0), (DATA_TYPE)0.75 );
      def.Add(  lenX,  lenY, lenZ, false, (DATA_TYPE)(2.0/3.0), (DATA_TYPE)1.0 );
      def.Add( -lenX,  lenY, lenZ, false, (DATA_TYPE)(1.0/2.0), (DATA_TYPE)1.0 );

      def.Add( -lenX, -lenY, -lenZ, false, (DATA_TYPE)(1.0/3.0), (DATA_TYPE)0.5 );
      def.Add(  lenX, -lenY, -lenZ, false, (DATA_TYPE)(2.0/3.0), (DATA_TYPE)0.5 );

      def.Add( -lenX, lenY, -lenZ, false, (DATA_TYPE)(1.0/3.0), (DATA_TYPE)0.25 );
      def.Add(  lenX, lenY, -lenZ, false, (DATA_TYPE)(2.0/3.0), (DATA_TYPE)0.25 );

      def.Add( -lenX, lenY, lenZ, false, (DATA_TYPE)(1.0/3.0), (DATA_TYPE)0.0 );
      def.Add(  lenX, lenY, lenZ, false, (DATA_TYPE)(2.0/3.0), (DATA_TYPE)0.0 );

      def.Add( -lenX, lenY, -lenZ, false, (DATA_TYPE)0.0, (DATA_TYPE)0.5 );
      def.Add( -lenX, lenY,  lenZ, false, (DATA_TYPE)0.0, (DATA_TYPE)0.75 );

      def.Add( lenX, lenY, -lenZ, false, (DATA_TYPE)1.0, (DATA_TYPE)0.5 );
      def.Add( lenX, lenY,  lenZ, false, (DATA_TYPE)1.0, (DATA_TYPE)0.75 );

      def.AddFace(  0,  1,  2,  3 );
      def.AddFace(  4,  5,  1,  0 );
      def.AddFace(  6,  7,  5,  4 );
      def.AddFace(  8,  9,  7,  6 );
      def.AddFace( 10,  4,  0, 11 );
      def.AddFace(  5, 12, 13,  1 );

      break;
  }

  return ret;
}


//---------------------------------------------------------------------
// class TTetrahedronDef
//---------------------------------------------------------------------


template < class DATA_TYPE, class INDEX_TYPE >
class TTetrahedronDef
  : public TDefBase< DATA_TYPE, INDEX_TYPE > 
{
public: // public operations

  typedef std::array< DATA_TYPE, 2 > CTexCoord;
  typedef std::array< DATA_TYPE, 3 > CVertex;
  typedef std::array< DATA_TYPE, 3 > CNormal;
  typedef std::array< INDEX_TYPE, 3 > CPrimitive;

  TTetrahedronDef( DATA_TYPE radius )
    : m_radius( radius ) {}
  virtual ~TTetrahedronDef() {}
  virtual bool CreateMesh( TMeshData< DATA_TYPE, INDEX_TYPE > &def ) override;

protected: // protected attributes 

  DATA_TYPE m_radius;
};


template < class DATA_TYPE, class INDEX_TYPE >
bool TTetrahedronDef< DATA_TYPE, INDEX_TYPE >::CreateMesh( 
  TMeshData< DATA_TYPE, INDEX_TYPE > &def ) // I -
{
  bool separatePointsForFaces = true;

  std::vector< CVertex > pts;
  std::vector< CTexCoord > texCoord;
  std::vector< CPrimitive > primitives;
  TDefBase< DATA_TYPE, INDEX_TYPE >::CalculateTetrahedron( separatePointsForFaces, pts, texCoord, primitives );

  std::vector< CNormal > nv;
  TDefBase< DATA_TYPE, INDEX_TYPE >::CalculateFacesNV( separatePointsForFaces, pts, primitives, nv );

  def.Reserve( (INDEX_TYPE)pts.size() );
  for ( size_t inx = 0; inx < pts.size(); inx ++ )
    def.Add( pts[inx][0] * m_radius, pts[inx][1] * m_radius, pts[inx][2] * m_radius, nv[inx][0], nv[inx][1], nv[inx][2], true, texCoord[inx][0], texCoord[inx][1] );
  def.ReserveFaces( (INDEX_TYPE)primitives.size() );
  for ( auto &primitive : primitives )
    def.AddFace( primitive[0], primitive[1], primitive[2] );

  return true;
}


//---------------------------------------------------------------------
// class TIcosahedronDef
//---------------------------------------------------------------------


template < class DATA_TYPE, class INDEX_TYPE >
class TIcosahedronDef
  : public TDefBase< DATA_TYPE, INDEX_TYPE > 
{
public: // public operations

  typedef std::array< DATA_TYPE, 2 > CTexCoord;
  typedef std::array< DATA_TYPE, 3 > CVertex;
  typedef std::array< DATA_TYPE, 3 > CNormal;
  typedef std::array< INDEX_TYPE, 3 > CPrimitive;

  TIcosahedronDef( DATA_TYPE radius )
    : m_radius( radius ) {}
  virtual ~TIcosahedronDef() {}
  virtual bool CreateMesh( TMeshData< DATA_TYPE, INDEX_TYPE > &def ) override;

protected: // protected attributes 

  DATA_TYPE m_radius;
};


template < class DATA_TYPE, class INDEX_TYPE >
bool TIcosahedronDef< DATA_TYPE, INDEX_TYPE >::CreateMesh( 
  TMeshData< DATA_TYPE, INDEX_TYPE > &def ) // I -
{
  bool separatePointsForFaces = true;

  std::vector< CVertex > pts;
  std::vector< CTexCoord > texCoord;
  std::vector< CPrimitive > primitives;
  TDefBase< DATA_TYPE, INDEX_TYPE >::CalculateIcosahedron( separatePointsForFaces, pts, texCoord, primitives );

  std::vector< CNormal > nv;
  TDefBase< DATA_TYPE, INDEX_TYPE >::CalculateFacesNV( separatePointsForFaces, pts, primitives, nv );

  def.Reserve( (INDEX_TYPE)pts.size() );
  for ( size_t inx = 0; inx < pts.size(); inx ++ )
    def.Add( pts[inx][0] * m_radius, pts[inx][1] * m_radius, pts[inx][2] * m_radius, nv[inx][0], nv[inx][1], nv[inx][2], true, texCoord[inx][0], texCoord[inx][1] );
  def.ReserveFaces( (INDEX_TYPE)primitives.size() );
  for ( auto &primitive : primitives )
    def.AddFace( primitive[0], primitive[1], primitive[2] );

  return true;
}


//---------------------------------------------------------------------
// class TTriangleSphereDef
//---------------------------------------------------------------------


template < class DATA_TYPE, class INDEX_TYPE >
class TTriangleSphereDef
  : public TTetrahedronDef< DATA_TYPE, INDEX_TYPE > 
{
public: // public operations

  typedef std::array< DATA_TYPE, 2 > CTexCoord;
  typedef std::array< DATA_TYPE, 3 > CVertex;
  typedef std::array< DATA_TYPE, 3 > CNormal;
  typedef std::array< INDEX_TYPE, 3 > CPrimitive;

  TTriangleSphereDef( INDEX_TYPE minNoOfPts, DATA_TYPE radius, bool tirangleTexCoords )
    : TTetrahedronDef< DATA_TYPE, INDEX_TYPE >( radius ), m_minNoOfPts( minNoOfPts ), m_tirangleTexCoords( tirangleTexCoords ) {}
  
  virtual ~TTriangleSphereDef() {}
  virtual bool CreateMesh( TMeshData< DATA_TYPE, INDEX_TYPE > &def ) override;

  static INDEX_TYPE GetSphereMidPointOnLine( std::vector< CVertex > &pts, std::vector< CTexCoord > &texCoord, std::map< INDEX_TYPE, std::map< INDEX_TYPE, INDEX_TYPE > > &midPtMap, INDEX_TYPE pt1Inx, INDEX_TYPE pt2Inx );
  static void SplitSpherPrimitive( std::vector< CVertex > &pts, std::vector< CTexCoord > &texCoord, std::map< INDEX_TYPE, std::map< INDEX_TYPE, INDEX_TYPE > > &midPtMap, const CPrimitive &primToSplit, std::vector< CPrimitive > &newPrimitives );
  static void SplitSphere( std::vector< CVertex > &pts, std::vector< CTexCoord > &texCoord, std::vector< CPrimitive > &primitives );
  static void CalculateSphere( INDEX_TYPE minPts, bool tirangleTexCoords, std::vector< CVertex > &pts, std::vector< CTexCoord > &texCoord, std::vector< CPrimitive > &primitives );

protected: // protected attributes 

  INDEX_TYPE m_minNoOfPts;
  bool       m_tirangleTexCoords;
};

template < class DATA_TYPE, class INDEX_TYPE >
INDEX_TYPE TTriangleSphereDef< DATA_TYPE, INDEX_TYPE >::GetSphereMidPointOnLine( 
  std::vector< CVertex >                                     &pts,      // O -
  std::vector< CTexCoord >                                   &texCoord,  // U -
  std::map< INDEX_TYPE, std::map< INDEX_TYPE, INDEX_TYPE > > &midPtMap, // O -
  INDEX_TYPE                                                  pt1Inx,   // I -
  INDEX_TYPE                                                  pt2Inx )  // I -
{
  INDEX_TYPE minPt = pt1Inx < pt2Inx ? pt1Inx : pt2Inx;
  INDEX_TYPE maxPt = pt1Inx < pt2Inx ? pt2Inx : pt1Inx;

  auto &innerMap = midPtMap[minPt];
  auto findIt = innerMap.find( maxPt );
  if ( findIt != innerMap.end() )
    return findIt->second;

  INDEX_TYPE midPtInx = innerMap[maxPt] = (INDEX_TYPE)pts.size();
  CVertex newPt{ pts[minPt][0] + pts[maxPt][0], pts[minPt][1] + pts[maxPt][1], pts[minPt][2] + pts[maxPt][2] };
  newPt.Normalize();
  pts.push_back( { newPt[0], newPt[1], newPt[2] } );

  if ( texCoord.empty() == false )
  {
    DATA_TYPE u = ( texCoord[minPt][0] + texCoord[maxPt][0] ) / ( DATA_TYPE )2.0;
    DATA_TYPE v = ( texCoord[minPt][1] + texCoord[maxPt][1] ) / ( DATA_TYPE )2.0;
    texCoord.push_back( { u, v } );
  }
  
  return midPtInx;
}

template < class DATA_TYPE, class INDEX_TYPE >
void TTriangleSphereDef< DATA_TYPE, INDEX_TYPE >::SplitSpherPrimitive( 
  std::vector< CVertex >                                     &pts,            // U -
  std::vector< CTexCoord >                                   &texCoord,       // U -
  std::map< INDEX_TYPE, std::map< INDEX_TYPE, INDEX_TYPE > > &midPtMap,       // U -
  const CPrimitive                                           &primToSplit,    // U -
  std::vector< CPrimitive >                                  &newPrimitives ) // U -
{
  std::array< INDEX_TYPE, 3 > splitPtInx
  {
    GetSphereMidPointOnLine( pts, texCoord, midPtMap, primToSplit[0], primToSplit[1] ),
    GetSphereMidPointOnLine( pts, texCoord, midPtMap, primToSplit[1], primToSplit[2] ),
    GetSphereMidPointOnLine( pts, texCoord, midPtMap, primToSplit[2], primToSplit[0] ),
  };

  newPrimitives.push_back( { primToSplit[0], splitPtInx[0], splitPtInx[2] } );
  newPrimitives.push_back( { primToSplit[1], splitPtInx[1], splitPtInx[0] } );
  newPrimitives.push_back( { primToSplit[2], splitPtInx[2], splitPtInx[1] } );
  newPrimitives.push_back( { splitPtInx[0],  splitPtInx[1], splitPtInx[2] } );
}

template < class DATA_TYPE, class INDEX_TYPE >
void TTriangleSphereDef< DATA_TYPE, INDEX_TYPE >::SplitSphere( 
  std::vector< CVertex >    &pts,         // U -
  std::vector< CTexCoord >  &texCoord,    // U -
  std::vector< CPrimitive > &primitives ) // U -
{
  std::map< INDEX_TYPE, std::map< INDEX_TYPE, INDEX_TYPE > > midPtMap;
  std::vector< CPrimitive > newPrimitives;
  for ( auto &primitive : primitives )
    SplitSpherPrimitive( pts, texCoord, midPtMap, primitive, newPrimitives );
  primitives.swap( newPrimitives );
}

template < class DATA_TYPE, class INDEX_TYPE >
void TTriangleSphereDef< DATA_TYPE, INDEX_TYPE >::CalculateSphere( 
  INDEX_TYPE                 minPts,            // I -
  bool                       tirangleTexCoords, // I -
  std::vector< CVertex >    &pts,               // O -
  std::vector< CTexCoord >  &texCoord,          // O -
  std::vector< CPrimitive > &primitives )       // O -
{
  CalculateTetrahedron( false, pts, texCoord, primitives );
  if ( tirangleTexCoords == false )
    texCoord.clear();
  while ( (INDEX_TYPE)pts.size() < minPts )
    SplitSphere( pts, texCoord, primitives );
}

template < class DATA_TYPE, class INDEX_TYPE >
bool TTriangleSphereDef< DATA_TYPE, INDEX_TYPE >::CreateMesh( 
  TMeshData< DATA_TYPE, INDEX_TYPE > &def ) // I -
{
  std::vector< CVertex >    pts;
  std::vector< CTexCoord >  texCoord;
  std::vector< CPrimitive > primitives;
  CalculateSphere( m_minNoOfPts, m_tirangleTexCoords, pts, texCoord, primitives );
  
  TUVunwrapping uvMode = eUV_ANGLE_XY_ZY;

  def.Reserve( (INDEX_TYPE)pts.size() );
  auto radius = TTetrahedronDef< DATA_TYPE, INDEX_TYPE >::m_radius;
  if ( texCoord.size() == pts.size() )
  {
    for ( size_t inx = 0; inx < pts.size(); inx ++  )
      def.Add( pts[inx][0] * radius, pts[inx][1] * radius, pts[inx][2] * radius, pts[inx][0], pts[inx][1], pts[inx][2], false, texCoord[inx][0], texCoord[inx][1] );
  }
  else
  {
    for ( auto &pt : pts )
    {
      int axisU = 0;
      int axisV = 2;
      CVertex relV = RelativeToBox( pt, CVertex{ (DATA_TYPE)-1.0, (DATA_TYPE)-1.0, (DATA_TYPE)-1.0 }, CVertex{ (DATA_TYPE)1.0, (DATA_TYPE)1.0, (DATA_TYPE)1.0 } );
      CTexCoord uv = CalcUV( uvMode, axisU, axisV, relV );

      def.Add( pt[0] * radius, pt[1] * radius, pt[2] * radius, pt[0],  pt[1], pt[2], false, uv[0], uv[1] );
    }
  }

  def.ReserveFaces( (INDEX_TYPE)primitives.size() );
  for ( auto &primitive : primitives )
    def.AddFace( primitive[0], primitive[1], primitive[2] );

  return true;
}


//---------------------------------------------------------------------
// class TCylinderDef
//---------------------------------------------------------------------


template < class DATA_TYPE, class INDEX_TYPE >
class TCylinderDef
  : public TDefBase< DATA_TYPE, INDEX_TYPE > 
{
public: // public operations

  typedef std::array< DATA_TYPE, 2 > CTexCoord;
  typedef std::array< DATA_TYPE, 3 > CVertex;
  typedef std::array< DATA_TYPE, 3 > CNormal;
  typedef std::array< INDEX_TYPE, 3 > CPrimitive;

  TCylinderDef( DATA_TYPE height, DATA_TYPE radius, DATA_TYPE circumferenceTile, DATA_TYPE topToBottomeTile )
    : TCylinderDef( height, radius, circumferenceTile, topToBottomeTile, false, false )
  {}
  TCylinderDef( DATA_TYPE height, DATA_TYPE radius, DATA_TYPE circumferenceTile, DATA_TYPE topToBottomeTile, bool openAtBottom, bool openAtTop )
    : m_height( height )
    , m_radius( radius )
    , m_circumferenceTile( circumferenceTile )
    , m_topToBottomeTile( topToBottomeTile )
    , m_openAtBottom( openAtBottom )
    , m_openAtTop( openAtTop )
  {
    DATA_TYPE texScale = radius * 2.0f / height;
    //DATA_TYPE texScale = radius * static_cast<DATA_TYPE>(M_PI) / height;
    m_texScale[0] = texScale < 1.0f ? 1.0f : texScale;
    m_texScale[1] = texScale < 1.0f ? 1.0f/texScale : 1.0f;
  }
  virtual ~TCylinderDef() {}
  virtual bool CreateMesh( TMeshData< DATA_TYPE, INDEX_TYPE > &def ) override;

protected: // protected attributes 

  DATA_TYPE m_height;
  DATA_TYPE m_radius;
  DATA_TYPE m_circumferenceTile;
  DATA_TYPE m_topToBottomeTile;
  DATA_TYPE m_texScale[2];
  bool      m_openAtBottom;  //!< pipe
  bool      m_openAtTop;     //!< pipe
};


template < class DATA_TYPE, class INDEX_TYPE >
bool TCylinderDef< DATA_TYPE, INDEX_TYPE >::CreateMesh( 
  TMeshData< DATA_TYPE, INDEX_TYPE > &def ) // I -
{
  const DATA_TYPE CONST_PI = 3.14159265358979323846264338327950288f;
  const DATA_TYPE CONST_2PI = 6.28318530717958647692528676655900576f;

  INDEX_TYPE circumferenceCount = (INDEX_TYPE)( m_circumferenceTile + 0.5f );
  if ( circumferenceCount < 4 ) circumferenceCount = 4;
  INDEX_TYPE circumferenceCount_2 = circumferenceCount / 2;
  INDEX_TYPE topToBottomCount = (INDEX_TYPE)( m_topToBottomeTile + 0.5f );
  if ( topToBottomCount < 2 ) topToBottomCount = 2;
  DATA_TYPE height_2 = m_height / 2.0f;

  def.Add( (DATA_TYPE)0.0, (DATA_TYPE)0.0, height_2, (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)1.0, true, (DATA_TYPE)0.5f, 0.5f );
  for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount; cInx ++ )
  {
    DATA_TYPE angle = (DATA_TYPE)( CONST_2PI * cInx / circumferenceCount );
    DATA_TYPE x = (DATA_TYPE)cos( angle );
    DATA_TYPE y = (DATA_TYPE)sin( angle );
    DATA_TYPE u = (DATA_TYPE)((cos( angle ) + 1.0) / 2.0);
    DATA_TYPE v = (DATA_TYPE)((sin( angle ) + 1.0) / 2.0);
    def.Add( (DATA_TYPE)(x * m_radius), (DATA_TYPE)(y * m_radius), (DATA_TYPE)height_2, (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)1.0, true, u, v );
  }
  for ( INDEX_TYPE tbInx = 0; tbInx <= topToBottomCount; tbInx ++ )
  {
    DATA_TYPE v = (DATA_TYPE)( 1.0 - (DATA_TYPE)tbInx / topToBottomCount );
    DATA_TYPE heightFac = (DATA_TYPE)( 1.0 - 2.0 * tbInx / topToBottomCount  );
    for ( INDEX_TYPE cInx = 0; cInx <= circumferenceCount_2; cInx ++ )
    {
      DATA_TYPE u = (DATA_TYPE)cInx / (DATA_TYPE)circumferenceCount_2;
      DATA_TYPE angle = (DATA_TYPE)( CONST_PI * u );
      DATA_TYPE x = (DATA_TYPE)cos( angle );
      DATA_TYPE y = (DATA_TYPE)sin( angle );
      def.Add( (DATA_TYPE)(x * m_radius), (DATA_TYPE)(y * m_radius), (DATA_TYPE)(heightFac * height_2), x, y, (DATA_TYPE)0.0, true, u*m_texScale[0], v*m_texScale[1] );
    }
    for ( INDEX_TYPE cInx = 0; cInx <= circumferenceCount_2; cInx ++ )
    {
      DATA_TYPE u = (DATA_TYPE)cInx / (DATA_TYPE)circumferenceCount_2;
      DATA_TYPE angle = (DATA_TYPE)( CONST_PI * u + CONST_PI );
      DATA_TYPE x = (DATA_TYPE)cos( angle );
      DATA_TYPE y = (DATA_TYPE)sin( angle );
      def.Add( (DATA_TYPE)(x * m_radius), (DATA_TYPE)(y * m_radius), (DATA_TYPE)(heightFac * height_2), x, y, (DATA_TYPE)0.0, true, u*m_texScale[0], v*m_texScale[1] );
    }
  }
  for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount; cInx ++ )
  {
    DATA_TYPE angle = (DATA_TYPE)( CONST_2PI * cInx / circumferenceCount );
    DATA_TYPE x = (DATA_TYPE)cos( angle );
    DATA_TYPE y = (DATA_TYPE)sin( angle );
    DATA_TYPE u = (DATA_TYPE)((cos( angle ) + 1.0) / 2.0);
    DATA_TYPE v = (DATA_TYPE)((sin( angle ) + 1.0) / 2.0);
    def.Add( (DATA_TYPE)(x * m_radius), (DATA_TYPE)(y * m_radius), (DATA_TYPE)-height_2, (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)-1.0, true, u, v );
  }
  def.Add( (DATA_TYPE)0.0, (DATA_TYPE)0.0, -height_2, (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)-1.0, true, (DATA_TYPE)0.5f, 0.5f );

  if ( m_openAtTop == false )
  {
    INDEX_TYPE start = 1;
    for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount; cInx ++ )
    {
      INDEX_TYPE cInxNext = (cInx + 1) % circumferenceCount;
      def.AddFace( start + cInx, start + cInxNext, 0 );
    }
  }
  INDEX_TYPE diskPtCount         = 1 + circumferenceCount;
  INDEX_TYPE circumferenceSize_2 = circumferenceCount_2 + 1;
  INDEX_TYPE circumferenceSize   = circumferenceSize_2 * 2;
  for ( INDEX_TYPE tbInx = 1; tbInx <= topToBottomCount; tbInx ++ )
  {
    INDEX_TYPE ringStart     = diskPtCount + (tbInx-1) * circumferenceSize;
    INDEX_TYPE nextRingStart = diskPtCount + tbInx     * circumferenceSize;
    for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount_2; cInx ++ )
      def.AddFace( ringStart + cInx, nextRingStart + cInx, nextRingStart + cInx + 1, ringStart + cInx + 1 );
    ringStart += circumferenceSize_2;
    nextRingStart += circumferenceSize_2;
    for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount_2; cInx ++ )
      def.AddFace( ringStart + cInx, nextRingStart + cInx, nextRingStart + cInx + 1, ringStart + cInx + 1 );
  }
  INDEX_TYPE shaftPtCount = ( topToBottomCount + 1 ) * circumferenceSize;
  if ( m_openAtBottom == false )
  {
    INDEX_TYPE start = shaftPtCount + diskPtCount;
    for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount; cInx ++ )
    {
      INDEX_TYPE cInxNext = (cInx + 1) % circumferenceCount;
      def.AddFace( start + cInxNext, start + cInx, start + circumferenceCount );
    }
  }

  return true;
}


//---------------------------------------------------------------------
// class TConeDef
//---------------------------------------------------------------------


template < class DATA_TYPE, class INDEX_TYPE >
class TConeDef
  : public TDefBase< DATA_TYPE, INDEX_TYPE > 
{
public: // public operations

  typedef std::array< DATA_TYPE, 2 > CTexCoord;
  typedef std::array< DATA_TYPE, 3 > CVertex;
  typedef std::array< DATA_TYPE, 3 > CNormal;
  typedef std::array< INDEX_TYPE, 3 > CPrimitive;

  TConeDef( DATA_TYPE heightBottom, DATA_TYPE heightTop, DATA_TYPE radius, DATA_TYPE circumferenceTile, DATA_TYPE topToBottomeTile )
    : TConeDef( heightBottom, heightTop, radius, circumferenceTile, topToBottomeTile, false )
  {}
  TConeDef( DATA_TYPE heightBottom, DATA_TYPE heightTop, DATA_TYPE radius, DATA_TYPE circumferenceTile, DATA_TYPE topToBottomeTile, bool openAtBottom )
    : m_heightBottom( heightBottom )
    , m_heightTop( heightTop )
    , m_radius( radius )
    , m_circumferenceTile( circumferenceTile )
    , m_topToBottomeTile( topToBottomeTile )
    , m_openAtBottom( openAtBottom )
  {
    DATA_TYPE texScale = (DATA_TYPE)(radius * 2.0 / (heightTop - heightBottom));
    //DATA_TYPE texScale = radius * static_cast<DATA_TYPE>(M_PI) / height;
    m_texScale[0] = (DATA_TYPE)(texScale < 1.0 ? 1.0 : texScale);
    m_texScale[1] = (DATA_TYPE)(texScale < 1.0 ? 1.0/texScale : 1.0);
  }
  virtual ~TConeDef() {}
  virtual bool CreateMesh( TMeshData< DATA_TYPE, INDEX_TYPE > &def ) override;

protected: // protected attributes 

  DATA_TYPE m_heightBottom;
  DATA_TYPE m_heightTop;
  DATA_TYPE m_radius;
  DATA_TYPE m_circumferenceTile;
  DATA_TYPE m_topToBottomeTile;
  DATA_TYPE m_texScale[2];
  bool      m_openAtBottom;
};


template < class DATA_TYPE, class INDEX_TYPE >
bool TConeDef< DATA_TYPE, INDEX_TYPE >::CreateMesh( 
  TMeshData< DATA_TYPE, INDEX_TYPE > &def ) // I -
{
  const DATA_TYPE CONST_PI = 3.14159265358979323846264338327950288f;
  const DATA_TYPE CONST_2PI = 6.28318530717958647692528676655900576f;

  INDEX_TYPE circumferenceCount = (INDEX_TYPE)( m_circumferenceTile + 0.5f );
  if ( circumferenceCount < 4 ) circumferenceCount = 4;
  INDEX_TYPE circumferenceCount_2 = circumferenceCount / 2;
  INDEX_TYPE topToBottomCount = (INDEX_TYPE)( m_topToBottomeTile + 0.5f );
  if ( topToBottomCount < 2 ) topToBottomCount = 2;

  for ( INDEX_TYPE tbInx = 0; tbInx <= topToBottomCount; tbInx ++ )
  {
    DATA_TYPE v = (DATA_TYPE)( 1.0 - (DATA_TYPE)tbInx / topToBottomCount );
    DATA_TYPE height_weight = (DATA_TYPE)( 1.0 * tbInx / topToBottomCount  );
    DATA_TYPE ring_height = (DATA_TYPE)(m_heightTop * height_weight + (1.0 - height_weight) * m_heightBottom);
    DATA_TYPE radius = (DATA_TYPE)(m_radius * (1.0 - height_weight));
    for ( INDEX_TYPE cInx = 0; cInx <= circumferenceCount_2; cInx ++ )
    {
      DATA_TYPE u = (DATA_TYPE)cInx / (DATA_TYPE)circumferenceCount_2;
      DATA_TYPE angle = (DATA_TYPE)( CONST_PI * u );
      DATA_TYPE x = (DATA_TYPE)cos( angle );
      DATA_TYPE y = (DATA_TYPE)sin( angle );
      def.Add( (DATA_TYPE)(x * radius), (DATA_TYPE)(y * radius), ring_height, x, y, (DATA_TYPE)0.0, true, u*m_texScale[0], v*m_texScale[1] );
    }
    for ( INDEX_TYPE cInx = 0; cInx <= circumferenceCount_2; cInx ++ )
    {
      DATA_TYPE u = (DATA_TYPE)cInx / (DATA_TYPE)circumferenceCount_2;
      DATA_TYPE angle = (DATA_TYPE)( CONST_PI * u + CONST_PI );
      DATA_TYPE x = (DATA_TYPE)cos( angle );
      DATA_TYPE y = (DATA_TYPE)sin( angle );
      def.Add( (DATA_TYPE)(x * radius), (DATA_TYPE)(y * radius), ring_height, x, y, (DATA_TYPE)0.0, true, u*m_texScale[0], v*m_texScale[1] );
    }
  }
  for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount; cInx ++ )
  {
    DATA_TYPE angle = (DATA_TYPE)( CONST_2PI * cInx / circumferenceCount );
    DATA_TYPE x = (DATA_TYPE)cos( angle );
    DATA_TYPE y = (DATA_TYPE)sin( angle );
    DATA_TYPE u = (DATA_TYPE)((cos( angle ) + 1.0) / 2.0);
    DATA_TYPE v = (DATA_TYPE)((sin( angle ) + 1.0) / 2.0);
    def.Add( (DATA_TYPE)(x * m_radius), (DATA_TYPE)(y * m_radius), m_heightBottom, (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)-1.0, true, u, v );
  }
  def.Add( (DATA_TYPE)0.0, (DATA_TYPE)0.0, m_heightBottom, (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)-1.0, true, (DATA_TYPE)0.5f, 0.5f );

  INDEX_TYPE circumferenceSize_2 = circumferenceCount_2 + 1;
  INDEX_TYPE circumferenceSize   = circumferenceSize_2 * 2;
  for ( INDEX_TYPE tbInx = 1; tbInx <= topToBottomCount; tbInx ++ )
  {
    INDEX_TYPE ringStart     = (tbInx-1) * circumferenceSize;
    INDEX_TYPE nextRingStart = tbInx     * circumferenceSize;
    for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount_2; cInx ++ )
      def.AddFace( nextRingStart + cInx, ringStart + cInx, ringStart + cInx + 1, nextRingStart + cInx + 1);
    ringStart += circumferenceSize_2;
    nextRingStart += circumferenceSize_2;
    for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount_2; cInx ++ )
      def.AddFace( nextRingStart + cInx, ringStart + cInx, ringStart + cInx + 1,  nextRingStart + cInx + 1 );
  }
  INDEX_TYPE shaftPtCount = ( topToBottomCount + 1 ) * circumferenceSize;
  if ( m_openAtBottom == false )
  {
    INDEX_TYPE start = shaftPtCount;
    for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount; cInx ++ )
    {
      INDEX_TYPE cInxNext = (cInx + 1) % circumferenceCount;
      def.AddFace( start + cInxNext, start + cInx, start + circumferenceCount );
    }
  }

  return true;
}


//---------------------------------------------------------------------
// class TSphereDef
//---------------------------------------------------------------------


template < class DATA_TYPE, class INDEX_TYPE >
class TSphereDef
  : public TDefBase< DATA_TYPE, INDEX_TYPE > 
{
public: // public operations

  typedef std::array< DATA_TYPE, 2 > CTexCoord;
  typedef std::array< DATA_TYPE, 3 > CVertex;
  typedef std::array< DATA_TYPE, 3 > CNormal;
  typedef std::array< INDEX_TYPE, 3 > CPrimitive;

  TSphereDef( DATA_TYPE radius, DATA_TYPE circumferenceTile, DATA_TYPE topToBottomeTile )
    : m_radius( radius )
    , m_circumferenceTile( circumferenceTile )
    , m_topToBottomeTile( topToBottomeTile )
  {}
  virtual ~TSphereDef() {}
  virtual bool CreateMesh( TMeshData< DATA_TYPE, INDEX_TYPE > &def ) override;

protected: // protected attributes 

  DATA_TYPE m_radius;
  DATA_TYPE m_circumferenceTile;
  DATA_TYPE m_topToBottomeTile;
};


template < class DATA_TYPE, class INDEX_TYPE >
bool TSphereDef< DATA_TYPE, INDEX_TYPE >::CreateMesh( 
  TMeshData< DATA_TYPE, INDEX_TYPE > &def ) // I -
{
  const DATA_TYPE CONST_PI_2 = (DATA_TYPE)1.5707963267948966192313216916398;
  const DATA_TYPE CONST_PI   = (DATA_TYPE)3.14159265358979323846264338327950288;
  const DATA_TYPE CONST_2PI  = (DATA_TYPE)6.28318530717958647692528676655900576;

  INDEX_TYPE circumferenceCount = (INDEX_TYPE)( m_circumferenceTile + 0.5f );
  if ( circumferenceCount < 4 ) circumferenceCount = 4;
  INDEX_TYPE circumferenceCount_2 = circumferenceCount / 2;
  INDEX_TYPE topToBottomCount = (INDEX_TYPE)( m_topToBottomeTile + 0.5f );
  if ( topToBottomCount < 2 ) topToBottomCount = 2;

  def.Add( (DATA_TYPE)0.0, (DATA_TYPE)0.0, m_radius, (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)1.0, true, (DATA_TYPE)0.5f, 1.0f );
  for ( INDEX_TYPE tbInx = 1; tbInx < topToBottomCount; tbInx ++ )
  {
    DATA_TYPE v = (DATA_TYPE)( 1.0 - (DATA_TYPE)tbInx / topToBottomCount );
    DATA_TYPE heightFac = (DATA_TYPE)sin( ( 1.0 - 2.0 * tbInx / topToBottomCount ) * CONST_PI_2 );
    DATA_TYPE cosUp = (DATA_TYPE)sqrt( 1.0 - heightFac * heightFac );
    DATA_TYPE z = (DATA_TYPE)heightFac;
    for ( INDEX_TYPE cInx = 0; cInx <= circumferenceCount_2; cInx ++ )
    {
      DATA_TYPE u = (DATA_TYPE)cInx / (DATA_TYPE)circumferenceCount_2;
      DATA_TYPE angle = (DATA_TYPE)( CONST_PI * u );
      DATA_TYPE x = (DATA_TYPE)cos( angle ) * cosUp;
      DATA_TYPE y = (DATA_TYPE)sin( angle ) * cosUp;
      def.Add( (DATA_TYPE)(x * m_radius), (DATA_TYPE)(y * m_radius), (DATA_TYPE)(z * m_radius), x, y, z, true, u, v );
    }
    for ( INDEX_TYPE cInx = 0; cInx <= circumferenceCount_2; cInx ++ )
    {
      DATA_TYPE u = (DATA_TYPE)cInx / (DATA_TYPE)circumferenceCount_2;
      DATA_TYPE angle = (DATA_TYPE)( CONST_PI * u + CONST_PI );
      DATA_TYPE x = (DATA_TYPE)cos( angle ) * cosUp;
      DATA_TYPE y = (DATA_TYPE)sin( angle ) * cosUp;
      def.Add( (DATA_TYPE)(x * m_radius), (DATA_TYPE)(y * m_radius), (DATA_TYPE)(z * m_radius), x, y, z, true, u, v );
    }
  }
  def.Add( (DATA_TYPE)0.0, (DATA_TYPE)0.0, -m_radius, (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)-1.0, true, (DATA_TYPE)0.5f, 0.0f );

  INDEX_TYPE circumferenceSize_2 = circumferenceCount_2 + 1;
  INDEX_TYPE circumferenceSize = circumferenceSize_2 * 2;
  for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount_2; cInx ++ )
    def.AddFace( cInx + 1, cInx + 2, 0 );
  for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount_2; cInx ++ )
    def.AddFace( circumferenceSize_2 + cInx + 1, circumferenceSize_2 + cInx + 2, 0 );
  for ( INDEX_TYPE tbInx = 1; tbInx < topToBottomCount - 1; tbInx ++ )
  {
    INDEX_TYPE ringStart = 1 + (tbInx - 1) * circumferenceSize;
    INDEX_TYPE nextRingStart = 1 + tbInx * circumferenceSize;
    for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount_2; cInx ++ )
      def.AddFace( ringStart + cInx, nextRingStart + cInx, nextRingStart + cInx + 1, ringStart + cInx + 1 );
    ringStart += circumferenceSize_2;
    nextRingStart += circumferenceSize_2;
    for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount_2; cInx ++ )
      def.AddFace( ringStart + cInx, nextRingStart + cInx, nextRingStart + cInx + 1, ringStart + cInx + 1 );
  }
  INDEX_TYPE ringPtCount = ( topToBottomCount - 1 ) * circumferenceSize;
  INDEX_TYPE start = 1 + ringPtCount - circumferenceSize;
  for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount_2; cInx ++ )
    def.AddFace( start + cInx + 1, start + cInx, 1 + ringPtCount );
  start += circumferenceSize_2;
  for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount_2; cInx ++ )
    def.AddFace( start + cInx + 1, start + cInx, 1 + ringPtCount );

  return true;
}


//---------------------------------------------------------------------
// class TTorusDef
//---------------------------------------------------------------------


template < class DATA_TYPE, class INDEX_TYPE >
class TTorusDef
  : public TDefBase< DATA_TYPE, INDEX_TYPE > 
{
public:

  typedef std::array< DATA_TYPE, 2 > CTexCoord;
  typedef std::array< DATA_TYPE, 3 > CVertex;
  typedef std::array< DATA_TYPE, 3 > CNormal;
  typedef std::array< INDEX_TYPE, 3 > CPrimitive;

  TTorusDef( DATA_TYPE outerRadius, DATA_TYPE innerRadius, INDEX_TYPE nSides, INDEX_TYPE nRings )
    : m_outerRadius( outerRadius ), m_innerRadius( innerRadius ), m_nSides( nSides ), m_nRings( nRings ) {}
  virtual ~TTorusDef() {}
  virtual bool CreateMesh( TMeshData< DATA_TYPE, INDEX_TYPE > &def ) override;

private:

  DATA_TYPE m_outerRadius;
  DATA_TYPE m_innerRadius;
  INDEX_TYPE m_nSides;
  INDEX_TYPE m_nRings;
};

template < class DATA_TYPE, class INDEX_TYPE >
bool TTorusDef< DATA_TYPE, INDEX_TYPE >::CreateMesh( TMeshData< DATA_TYPE, INDEX_TYPE > &def )
{
  const DATA_TYPE CONST_PI = 3.14159265358979323846264338327950288f;
  const DATA_TYPE CONST_2PI = 6.28318530717958647692528676655900576f;

  INDEX_TYPE faces = m_nSides * m_nRings;
  INDEX_TYPE nVerts  = m_nSides * (m_nRings+1);   // One extra ring to duplicate first ring

  def.Reserve( nVerts );
  def.ReserveFaces( 2 * faces );

  DATA_TYPE ringFactor  = (DATA_TYPE)(CONST_2PI / m_nRings);
  DATA_TYPE sideFactor = (DATA_TYPE)(CONST_2PI / m_nSides);
  INDEX_TYPE idx = 0, tidx = 0;
  for ( INDEX_TYPE ring = 0; ring <= m_nRings; ring++ )
  {
    DATA_TYPE u = (DATA_TYPE)ring * ringFactor;
    DATA_TYPE cu = cos(u);
    DATA_TYPE su = sin(u);
    for ( INDEX_TYPE side = 0; side <= m_nSides; side++ )
    {
      DATA_TYPE v = (DATA_TYPE)side * sideFactor;
      DATA_TYPE cv = -cos(v);
      DATA_TYPE sv = -sin(v);
      DATA_TYPE r = (m_outerRadius + m_innerRadius * cv);
      def.Add( r * cu, r * su, m_innerRadius * sv, cv * cu * r, cv * su * r, sv * r, false, (DATA_TYPE)(u / CONST_2PI), (DATA_TYPE)(v / CONST_2PI) );
    }
  }

  idx = 0;
  for ( INDEX_TYPE ring = 0; ring < m_nRings; ring++ )
  {
    INDEX_TYPE ringStart = ring * (m_nSides + 1);
    INDEX_TYPE nextRingStart = ringStart + m_nSides + 1;
    for ( INDEX_TYPE side = 0; side < m_nSides; side++ )
    {
      INDEX_TYPE nextSide = side + 1;
      def.AddFace( ringStart + side, nextRingStart + side, nextRingStart + nextSide, ringStart + nextSide );
    }
  }

  return true;
}


//---------------------------------------------------------------------
// class TTrefoilKnotDef
//---------------------------------------------------------------------


template < class DATA_TYPE, class INDEX_TYPE >
class TTrefoilKnotDef
  : public TDefBase< DATA_TYPE, INDEX_TYPE > 
{
public:

  typedef std::array< DATA_TYPE, 2 > CTexCoord;
  typedef std::array< DATA_TYPE, 3 > CVertex;
  typedef std::array< DATA_TYPE, 3 > CNormal;
  typedef std::array< INDEX_TYPE, 3 > CPrimitive;

  CVertex Normalize(const CVertex &v)
  {
    DATA_TYPE len = std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    if (len == (DATA_TYPE)0)
        return v;
    return CVertex{v[0] / len, v[1] / len, v[2] / len};
  }

  CVertex Cross(const CVertex &a, const CVertex &b)
  {
    return CVertex{ a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0] };
  } 

  TTrefoilKnotDef( DATA_TYPE scale, INDEX_TYPE nSlices, INDEX_TYPE nStacks )
    : m_ra( scale * (DATA_TYPE)0.6 ), m_rb( scale * (DATA_TYPE)0.2 ), m_rc( scale * (DATA_TYPE)0.4 ), m_rd( scale * (DATA_TYPE)0.175 ), m_nSlices( nSlices ), m_nStacks( nStacks ) {}
  TTrefoilKnotDef( DATA_TYPE ra, DATA_TYPE rb, DATA_TYPE rc, DATA_TYPE rd, INDEX_TYPE nSlices, INDEX_TYPE nStacks )
    : m_ra( ra ), m_rb( rb ), m_rc( rc ), m_nSlices( nSlices ),  m_nStacks( nStacks ) {}
  virtual ~TTrefoilKnotDef() {}
  virtual bool CreateMesh( TMeshData< DATA_TYPE, INDEX_TYPE > &def ) override;

private:

  CVertex EvaluateTrefoilKnot( DATA_TYPE s, DATA_TYPE t );

  DATA_TYPE  m_ra = (DATA_TYPE)0.6;   // general radius
  DATA_TYPE  m_rb = (DATA_TYPE)0.2;   // curvature
  DATA_TYPE  m_rc = (DATA_TYPE)0.4;   // depth
  DATA_TYPE  m_rd = (DATA_TYPE)0.175; // tube radius
  INDEX_TYPE m_nSlices;
  INDEX_TYPE m_nStacks;
};

template < class DATA_TYPE, class INDEX_TYPE >
typename TTrefoilKnotDef< DATA_TYPE, INDEX_TYPE >::CVertex TTrefoilKnotDef< DATA_TYPE, INDEX_TYPE >::EvaluateTrefoilKnot( DATA_TYPE s, DATA_TYPE t )
{
  const DATA_TYPE CONST_PI  = static_cast<const DATA_TYPE>( 3.14159265358979323846264338327950288 );
  const DATA_TYPE CONST_2PI = static_cast<const DATA_TYPE>( 6.28318530717958647692528676655900576 );

  DATA_TYPE a = m_ra;
  DATA_TYPE b = m_rb;
  DATA_TYPE c = m_rc;
  DATA_TYPE d = m_rd;
  DATA_TYPE u = ( 1.0f - s ) * 2.0f * CONST_2PI;
  DATA_TYPE v = t * CONST_2PI;
  DATA_TYPE r = a + b * cos( 1.5f * u );
  DATA_TYPE x = r * cos( u );
  DATA_TYPE y = r * sin( u );
  DATA_TYPE z = c * sin( 1.5f * u );

  CVertex dv;
  dv[0] = -1.5f * b * sin( 1.5f * u ) * cos( u ) - ( a + b * cos( 1.5f * u ) ) * sin( u );
  dv[1] = -1.5f * b * sin( 1.5f * u ) * sin( u ) + ( a + b * cos( 1.5f * u ) ) * cos( u );
  dv[2] = 1.5f * c * cos( 1.5f * u ); 

  CVertex q = Normalize( dv );
  CVertex qvn = Normalize( CVertex{ q[1], -q[0], 0.0f } );
  CVertex ww = Cross( q, qvn );
  
  CVertex range;
  range[0] = x + d * ( qvn[0] * cos( v ) + ww[0] * sin( v ) );
  range[1] = y + d * ( qvn[1] * cos( v ) + ww[1] * sin( v ) );
  range[2] = z + d * ww[2] * sin( v );
    
  return range;
}

template < class DATA_TYPE, class INDEX_TYPE >
bool TTrefoilKnotDef< DATA_TYPE, INDEX_TYPE >::CreateMesh( TMeshData< DATA_TYPE, INDEX_TYPE > &def )
{
  DATA_TYPE ds = 1.0f / (DATA_TYPE)m_nSlices;
  DATA_TYPE dt = 1.0f / (DATA_TYPE)m_nStacks;

  for ( DATA_TYPE s = 0.0f; s < 1.0f + ds / 2.0f; s += ds)
  {
    for ( DATA_TYPE t = 0.0f; t < 1.0f + dt / 2.0f; t += dt )
    {
      DATA_TYPE E = 0.01f;
      
      CVertex n{ 0.0f, 0.0f, 1.0f};
      CVertex p = EvaluateTrefoilKnot( s, t );
      CVertex u = EvaluateTrefoilKnot( s - E, t );
      u[0] -= p[0]; u[1] -= p[1]; u[2] -= p[2]; 
      CVertex v = EvaluateTrefoilKnot( s, t + E );
      v[0] -= p[0]; v[1] -= p[1]; v[2] -= p[2];
      CVertex nv = Normalize(Cross(u, v));
      def.Add( p[0], p[1], p[2], nv[0], nv[1], nv[2], false, s * 18.0f, t );
    }
  }
 
  INDEX_TYPE vertexCount = (INDEX_TYPE)def.NoOfVertices();
  INDEX_TYPE n = 0;
  INDEX_TYPE circum_no = m_nStacks + 1;
  for ( INDEX_TYPE i = 0; i < m_nSlices; ++ i )
  {
    for ( INDEX_TYPE j = 0; j < m_nStacks; ++ j )
    {
      def.AddFace( n + j, (n + j + circum_no) % vertexCount, n + (j + 1) % circum_no );
      def.AddFace( (n + j + circum_no) % vertexCount, (n + (j + 1) % circum_no + circum_no) % vertexCount, (n + (j + 1) % circum_no) % vertexCount );
    }
    n += circum_no;
  }

  return true;
}


//---------------------------------------------------------------------
// class TTorusKnotDef
//---------------------------------------------------------------------


template < class DATA_TYPE, class INDEX_TYPE >
class TTorusKnotDef
  : public TDefBase< DATA_TYPE, INDEX_TYPE > 
{
public:
  
  typedef std::array< DATA_TYPE, 2 > CTexCoord;
  typedef std::array< DATA_TYPE, 3 > CVertex;
  typedef std::array< DATA_TYPE, 3 > CNormal;
  typedef std::array< INDEX_TYPE, 3 > CPrimitive;

  CVertex Normalize(const CVertex &v)
  {
    DATA_TYPE len = std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    if (len == (DATA_TYPE)0)
        return v;
    return CVertex{v[0] / len, v[1] / len, v[2] / len};
  }

  CVertex Cross(const CVertex &a, const CVertex &b)
  {
    return CVertex{ a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0] };
  } 

  TTorusKnotDef( INDEX_TYPE p, INDEX_TYPE q, DATA_TYPE outerScale, DATA_TYPE innerRadius, INDEX_TYPE nSlices, INDEX_TYPE nStacks )
    : m_p( p ), m_q( q ), m_outerScale( outerScale ), m_tubeRadius( innerRadius ), m_nSlices( nSlices ), m_nStacks( nStacks ) {}
  virtual ~TTorusKnotDef() {}
  virtual bool CreateMesh( TMeshData< DATA_TYPE, INDEX_TYPE > &def ) override;

private:

  typename TDefBase< DATA_TYPE, INDEX_TYPE >::CVertex EvaluateTorusKnot( DATA_TYPE s, DATA_TYPE t );

  INDEX_TYPE m_p;
  INDEX_TYPE m_q;
  DATA_TYPE  m_outerScale;
  DATA_TYPE  m_tubeRadius;
  INDEX_TYPE m_nSlices;
  INDEX_TYPE m_nStacks;
};

template < class DATA_TYPE, class INDEX_TYPE >
typename TDefBase< DATA_TYPE, INDEX_TYPE >::CVertex TTorusKnotDef< DATA_TYPE, INDEX_TYPE >::EvaluateTorusKnot( DATA_TYPE s, DATA_TYPE t )
{
  const DATA_TYPE CONST_PI  = static_cast<const DATA_TYPE>( 3.14159265358979323846264338327950288 );
  const DATA_TYPE CONST_2PI = static_cast<const DATA_TYPE>( 6.28318530717958647692528676655900576 );

  DATA_TYPE phi = s * CONST_2PI;
  DATA_TYPE r = cos( m_q * phi ) + 2.0f;
  DATA_TYPE dir_x = cos( m_p * phi );
  DATA_TYPE dir_y = sin( m_p * phi );
  DATA_TYPE x = r * dir_x;
  DATA_TYPE y = r * dir_y;
  DATA_TYPE z = - sin( m_q * phi );
  
  DATA_TYPE beta = t * CONST_PI * 2.0f;
  DATA_TYPE tube_x = sin( beta );
  DATA_TYPE tube_y = cos( beta );

  CVertex dir_circle = Normalize( CVertex{ -y, x, 0.0f } );
  CVertex dir_up = Normalize( { 0, cos( m_q * phi ), 1.0f } );
  
  CVertex vertex;
  vertex[0] = x * m_outerScale + m_tubeRadius * tube_x * dir_x + m_tubeRadius * tube_y * dir_up[1] * dir_circle[0]; 
  vertex[1] = y * m_outerScale + m_tubeRadius * tube_x * dir_y + m_tubeRadius * tube_y * dir_up[1] * dir_circle[1]; 
  vertex[2] = z * m_outerScale + m_tubeRadius * tube_y * dir_up[2]; 

  return vertex;
}

template < class DATA_TYPE, class INDEX_TYPE >
bool TTorusKnotDef< DATA_TYPE, INDEX_TYPE >::CreateMesh( TMeshData< DATA_TYPE, INDEX_TYPE > &def )
{
  DATA_TYPE ds = 1.0f / (DATA_TYPE)m_nSlices;
  DATA_TYPE dt = 1.0f / (DATA_TYPE)m_nStacks;

  for ( DATA_TYPE s = 0.0f; s < 1.0f + ds / 2.0f; s += ds)
  {
    for ( DATA_TYPE t = 0.0f; t < 1.0f + dt / 2.0f; t += dt )
    {
      DATA_TYPE E = 0.001f;
      
      CVertex p = EvaluateTrefoilKnot( s, t );
      CVertex u = EvaluateTrefoilKnot( s - E, t );
      u[0] -= p[0]; u[1] -= p[1]; u[2] -= p[2]; 
      CVertex v = EvaluateTrefoilKnot( s, t + E );
      v[0] -= p[0]; v[1] -= p[1]; v[2] -= p[2]; 
      CVertex n = Normalize(Cross(u, v));
      def.Add(  p[0], p[1], p[2], n[0], n[1], n[2], false, s * 3.0f * (DATA_TYPE)m_p * (DATA_TYPE)m_q, t );
    }
  }
 
  INDEX_TYPE vertexCount = (INDEX_TYPE)def.Vertices().NoOfAttributes();
  INDEX_TYPE n = 0;
  INDEX_TYPE circum_no = m_nStacks + 1;
  for ( INDEX_TYPE i = 0; i < m_nSlices; ++ i )
  {
    for ( INDEX_TYPE j = 0; j < m_nStacks; ++ j )
    {
      def.AddFace( n + j, n + (j + 1) % circum_no, (n + j + circum_no) % vertexCount );
      def.AddFace( (n + j + circum_no) % vertexCount, (n + (j + 1) % circum_no) % vertexCount, (n + (j + 1) % circum_no + circum_no) % vertexCount );
    }
    n += circum_no;
  }

  return true;
}


//---------------------------------------------------------------------
// class TTest1Def
//---------------------------------------------------------------------


template < class DATA_TYPE, class INDEX_TYPE >
class TTest1Def
  : public TTetrahedronDef< DATA_TYPE, INDEX_TYPE > 
{
public: // public operations

  typedef std::array< DATA_TYPE, 2 > CTexCoord;
  typedef std::array< DATA_TYPE, 3 > CVertex;
  typedef std::array< DATA_TYPE, 3 > CNormal;
  typedef std::array< INDEX_TYPE, 3 > CPrimitive;

  TTest1Def( size_t iterations, DATA_TYPE radius, bool tirangleTexCoords )
    : TTetrahedronDef< DATA_TYPE, INDEX_TYPE > ( radius ), m_iterations( iterations ), m_tirangleTexCoords( tirangleTexCoords ) {}
  virtual ~TTest1Def() {}
  virtual bool CreateMesh( TMeshData< DATA_TYPE, INDEX_TYPE > &def ) override;

  static INDEX_TYPE GetCenterPoint( std::vector< CVertex > &pts, std::vector< CTexCoord > &texCoord, const CPrimitive &primToSplit );
  static void SplitPrimitive(bool separatePointsForFaces, std::vector< CVertex > &pts, std::vector< CTexCoord > &texCoord, const CPrimitive &primToSplit, std::vector< CPrimitive > &newPrimitives );
  static void Split( bool separatePointsForFaces, std::vector< CVertex > &pts, std::vector< CTexCoord > &texCoord, std::vector< CPrimitive > &primitives );
  static void CalculateDef( bool separatePointsForFaces, size_t iterations, bool tirangleTexCoords, std::vector< CVertex > &pts, std::vector< CTexCoord > &texCoord, std::vector< CPrimitive > &primitives );

protected: // protected attributes 

  size_t m_iterations;
  bool   m_tirangleTexCoords;
};

template < class DATA_TYPE, class INDEX_TYPE >
INDEX_TYPE TTest1Def< DATA_TYPE, INDEX_TYPE >::GetCenterPoint( 
  std::vector< CVertex >   &pts,            // O -
  std::vector< CTexCoord > &texCoord,       // U -
  const CPrimitive         &primToSplit  )  // I -
{
  INDEX_TYPE midPtInx = (INDEX_TYPE)pts.size();
  CVertex newPt
  {
      pts[primToSplit[0]][0] + pts[primToSplit[1]][0] + pts[primToSplit[2]][0],
      pts[primToSplit[0]][1] + pts[primToSplit[1]][1] + pts[primToSplit[2]][1],
      pts[primToSplit[0]][2] + pts[primToSplit[1]][2] + pts[primToSplit[2]][2]
  };
  newPt = Normalize(newPt);
  pts.push_back( { newPt[0], newPt[1], newPt[2] } );

  if ( texCoord.empty() == false )
  {
    DATA_TYPE u = ( texCoord[primToSplit[0]][0] + texCoord[primToSplit[1]][0] + texCoord[primToSplit[2]][0] ) / ( DATA_TYPE )3.0;
    DATA_TYPE v = ( texCoord[primToSplit[0]][1] + texCoord[primToSplit[1]][1] + texCoord[primToSplit[2]][1] ) / ( DATA_TYPE )3.0;
    texCoord.push_back( { u, v } );
  }

  return midPtInx;
}

template < class DATA_TYPE, class INDEX_TYPE >
void TTest1Def< DATA_TYPE, INDEX_TYPE >::SplitPrimitive( 
  bool                       separatePointsForFaces, // I -
  std::vector< CVertex >    &pts,                    // U -
  std::vector< CTexCoord >  &texCoord,               // U -
  const CPrimitive          &primToSplit,            // I -
  std::vector< CPrimitive > &newPrimitives )         // U -
{
  INDEX_TYPE splitIndex = GetCenterPoint( pts, texCoord, primToSplit );
  newPrimitives.push_back( { primToSplit[0], primToSplit[1], splitIndex } );
  
  if ( separatePointsForFaces )
  {
    INDEX_TYPE newSplitIdex  = (INDEX_TYPE)pts.size();
    pts.push_back( pts[splitIndex] );
    texCoord.push_back( texCoord[splitIndex] );
    splitIndex = newSplitIdex;
  }
  newPrimitives.push_back( { primToSplit[1], primToSplit[2], splitIndex } );
  
  if ( separatePointsForFaces )
  {
    INDEX_TYPE newSplitIdex  = (INDEX_TYPE)pts.size();
    pts.push_back( pts[splitIndex] );
    texCoord.push_back( texCoord[splitIndex] );
    splitIndex = newSplitIdex;
  }
  newPrimitives.push_back( { primToSplit[2], primToSplit[0], splitIndex } );
}

template < class DATA_TYPE, class INDEX_TYPE >
void TTest1Def< DATA_TYPE, INDEX_TYPE >::Split( 
  bool                       separatePointsForFaces, // I -
  std::vector< CVertex >    &pts,                    // U -
  std::vector< CTexCoord >  &texCoord,               // U -
  std::vector< CPrimitive > &primitives )            // U -
{
  std::vector< CPrimitive > newPrimitives;
  for ( auto &primitive : primitives )
    SplitPrimitive( separatePointsForFaces, pts, texCoord, primitive, newPrimitives );
  primitives.swap( newPrimitives );
}

template < class DATA_TYPE, class INDEX_TYPE >
void TTest1Def< DATA_TYPE, INDEX_TYPE >::CalculateDef( 
  bool                       separatePointsForFaces, // I -
  size_t                     iterations,             // I -
  bool                       tirangleTexCoords,      // I -
  std::vector< CVertex >    &pts,                    // O -
  std::vector< CTexCoord >  &texCoord,               // O -
  std::vector< CPrimitive > &primitives )            // O -
{
  CalculateTetrahedron( separatePointsForFaces, pts, texCoord, primitives );
  if ( tirangleTexCoords == false )
    texCoord.clear();
  for ( size_t iteration = 0; iteration < iterations; iteration ++ )
    Split( true, pts, texCoord, primitives );
}

template < class DATA_TYPE, class INDEX_TYPE >
bool TTest1Def< DATA_TYPE, INDEX_TYPE >::CreateMesh( 
  TMeshData< DATA_TYPE, INDEX_TYPE > &def ) // I -
{
  bool separatePointsForFaces = true;

  std::vector< CVertex >    pts;
  std::vector< CTexCoord >  texCoord;
  std::vector< CPrimitive > primitives;
  CalculateDef( separatePointsForFaces, m_iterations, m_tirangleTexCoords, pts, texCoord, primitives );

  std::vector< CNormal > nv;
  CalculateFacesNV( separatePointsForFaces, pts, primitives, nv );

  TUVunwrapping uvMode = eUV_ANGLE_XY_ZY;

  def.Reserve( (INDEX_TYPE)pts.size() );
  DATA_TYPE radius = TTetrahedronDef< DATA_TYPE, INDEX_TYPE >::m_radius;
  if ( texCoord.size() == pts.size() )
  {
    for ( size_t inx = 0; inx < pts.size(); inx ++  )
      def.Add( pts[inx][0] * radius, pts[inx][1] * radius, pts[inx][2] * radius, pts[inx][0], pts[inx][1], pts[inx][2], true, texCoord[inx][0], texCoord[inx][1] );
  }
  else
  {
    for ( auto &pt : pts )
    {
      int axisU = 0;
      int axisV = 2;
      CVertex relV = RelativeToBox( pt, CVertex{ (DATA_TYPE)-1.0, (DATA_TYPE)-1.0, (DATA_TYPE)-1.0 }, CVertex{ (DATA_TYPE)1.0, (DATA_TYPE)1.0, (DATA_TYPE)1.0 } );
      CTexCoord uv = CalcUV( uvMode, axisU, axisV, relV );

      def.Add( pt[0] * radius, pt[1] * radius, pt[2] * radius, pt[0],  pt[1], pt[2], true, uv[0], uv[1] );
    }
  }

  def.ReserveFaces( (INDEX_TYPE)primitives.size() );
  for ( auto &primitive : primitives )
    def.AddFace( primitive[0], primitive[1], primitive[2] );

  return true;
}


//---------------------------------------------------------------------
// class TArrowDef
//---------------------------------------------------------------------


template < class DATA_TYPE, class INDEX_TYPE >
class TArrowDef
  : public TDefBase< DATA_TYPE, INDEX_TYPE > 
{
public: // public operations

  typedef std::array< DATA_TYPE, 2 > CTexCoord;
  typedef std::array< DATA_TYPE, 3 > CVertex;
  typedef std::array< DATA_TYPE, 3 > CNormal;
  typedef std::array< INDEX_TYPE, 3 > CPrimitive;

  TArrowDef( DATA_TYPE height, DATA_TYPE radius, DATA_TYPE peakLen, DATA_TYPE peakRad, DATA_TYPE circumferenceTile, DATA_TYPE indices )
    : TArrowDef( height, radius, peakLen, peakRad, circumferenceTile, indices, false, true )
  {}
  TArrowDef( DATA_TYPE height, DATA_TYPE radius, DATA_TYPE peakLen, DATA_TYPE peakRad, DATA_TYPE circumferenceTile, DATA_TYPE indices, bool peakAtBottom, bool peakAtTop )
    : m_height( height )
    , m_radius( radius )
    , m_peakLen( peakLen )
    , m_peakRad( peakRad )
    , m_circumferenceTile( circumferenceTile )
    , m_indices( indices )
    , m_peakAtBottom( peakAtBottom )
    , m_peakAtTop( peakAtTop )
  {
    DATA_TYPE texScale = radius * static_cast<DATA_TYPE>(MATH_PI) / height;
    m_texScale[0] = texScale < 1.0f ? 1.0f : texScale;
    m_texScale[1] = texScale < 1.0f ? 1.0f/texScale : 1.0f;
  }
  virtual ~TArrowDef() {}
  virtual bool CreateMesh( TMeshData< DATA_TYPE, INDEX_TYPE > &def ) override;

protected: // protected attributes 

  DATA_TYPE m_height;
  DATA_TYPE m_radius;
  DATA_TYPE m_peakLen;
  DATA_TYPE m_peakRad;
  DATA_TYPE m_circumferenceTile;
  DATA_TYPE m_indices;
  DATA_TYPE m_texScale[2];
  bool      m_peakAtBottom;
  bool      m_peakAtTop;
};


template < class DATA_TYPE, class INDEX_TYPE >
bool TArrowDef< DATA_TYPE, INDEX_TYPE >::CreateMesh( 
  TMeshData< DATA_TYPE, INDEX_TYPE > &def ) // I -
{
  const DATA_TYPE CONST_PI = 3.14159265358979323846264338327950288f;
  const DATA_TYPE CONST_2PI = 6.28318530717958647692528676655900576f;

  INDEX_TYPE topToBottomeTile = static_cast<INDEX_TYPE>(m_indices) - 1 - (m_peakAtBottom ? 1 : 0) - (m_peakAtTop ? 1 : 0);
  DATA_TYPE  peakSkI_bottom  = m_indices-1.0f;
  DATA_TYPE  peakSkI_top     = 0.0f;
  DATA_TYPE  shaftSkI_bottom = peakSkI_bottom - (m_peakAtBottom ? 1.0f : 0.0f);
  DATA_TYPE  shaftSkI_top    = peakSkI_top    + (m_peakAtTop ? 1.0f : 0.0f);

  INDEX_TYPE circumferenceCount = (INDEX_TYPE)( m_circumferenceTile + 0.5f );
  if ( circumferenceCount < 4 ) circumferenceCount = 4;
  INDEX_TYPE circumferenceCount_2 = circumferenceCount / 2;
  INDEX_TYPE topToBottomCount = (INDEX_TYPE)( topToBottomeTile + 0.5f );
  if ( topToBottomCount < 2 ) topToBottomCount = 2;
  DATA_TYPE height_2        = m_height / 2.0f;
  DATA_TYPE texIScaleTop    = m_peakAtTop    ? (m_peakRad/m_radius) : 1.0f;
  DATA_TYPE texIScaleBottom = m_peakAtBottom ? (m_peakRad/m_radius) : 1.0f;

  // top peak points
  std::array<DATA_TYPE, 4> sk_i = { peakSkI_top, peakSkI_top, peakSkI_top, peakSkI_top };
  std::array<DATA_TYPE, 4> sw_i = { 1.0f, 0.0f, 0.0f, 0.0f };
  def.Add(
    (DATA_TYPE)0.0, (DATA_TYPE)0.0, height_2+m_peakLen,
    (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)1.0, true,
    (DATA_TYPE)0.5f, (DATA_TYPE)0.5f,
    sk_i[0], sk_i[1], sk_i[2], sk_i[3],
    sw_i[0], sw_i[1], sw_i[2], sw_i[3] );
  // top outer peak points
  sk_i = { shaftSkI_top, shaftSkI_top, shaftSkI_top, shaftSkI_top };
  sw_i = { 1.0f, 0.0f, 0.0f, 0.0f };
  for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount; cInx ++ )
  {
    DATA_TYPE angle = (DATA_TYPE)( CONST_2PI * cInx / circumferenceCount );
    DATA_TYPE x = (DATA_TYPE)cos( angle );
    DATA_TYPE y = (DATA_TYPE)sin( angle );
    DATA_TYPE u = (DATA_TYPE)((cos( angle ) + 1.0) / 2.0);
    DATA_TYPE v = (DATA_TYPE)((sin( angle ) + 1.0) / 2.0);
    def.Add( 
      (DATA_TYPE)(x * m_peakRad), (DATA_TYPE)(y * m_peakRad), (DATA_TYPE)height_2,
      x, y, (DATA_TYPE)0.0, true,
      u, v,
      sk_i[0], sk_i[1], sk_i[2], sk_i[3],
      sw_i[0], sw_i[1], sw_i[2], sw_i[3] );
  }
  
  // top cylinder point
  def.Add(
    (DATA_TYPE)0.0, (DATA_TYPE)0.0, height_2,
    (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)1.0, true,
    (DATA_TYPE)0.5f, (DATA_TYPE)0.5f,
    sk_i[0], sk_i[1], sk_i[2], sk_i[3],
    sw_i[0], sw_i[1], sw_i[2], sw_i[3] );
  // top outer cylinder points
  for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount; cInx ++ )
  {
    DATA_TYPE angle = (DATA_TYPE)( CONST_2PI * cInx / circumferenceCount );
    DATA_TYPE x = (DATA_TYPE)cos( angle );
    DATA_TYPE y = (DATA_TYPE)sin( angle );
    DATA_TYPE u = (DATA_TYPE)((cos( angle ) + 1.0) / 2.0) * texIScaleTop;
    DATA_TYPE v = (DATA_TYPE)((sin( angle ) + 1.0) / 2.0) * texIScaleTop;
    def.Add(
      (DATA_TYPE)(x * m_radius), (DATA_TYPE)(y * m_radius), (DATA_TYPE)height_2,
      (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)1.0, true,
      u, v,
      sk_i[0], sk_i[1], sk_i[2], sk_i[3],
      sw_i[0], sw_i[1], sw_i[2], sw_i[3] );
  }

  // shaft points
  for ( INDEX_TYPE tbInx = 0; tbInx <= topToBottomCount; tbInx ++ )
  {
    DATA_TYPE skPart_I = shaftSkI_top + static_cast<DATA_TYPE>( tbInx );
    sk_i = { skPart_I, skPart_I, skPart_I, skPart_I };
    sw_i = { 1.0f, 0.0f, 0.0f, 0.0f };
    DATA_TYPE v = (DATA_TYPE)( 1.0 - (DATA_TYPE)tbInx / topToBottomCount );
    DATA_TYPE heightFac = (DATA_TYPE)( 1.0 - 2.0 * tbInx / topToBottomCount  );
    for ( INDEX_TYPE cInx = 0; cInx <= circumferenceCount_2; cInx ++ )
    {
      DATA_TYPE u = (DATA_TYPE)cInx / (DATA_TYPE)circumferenceCount_2;
      DATA_TYPE angle = (DATA_TYPE)( CONST_PI * u );
      DATA_TYPE x = (DATA_TYPE)cos( angle );
      DATA_TYPE y = (DATA_TYPE)sin( angle );
      def.Add(
        (DATA_TYPE)(x * m_radius), (DATA_TYPE)(y * m_radius), (DATA_TYPE)(heightFac * height_2),
        x, y, (DATA_TYPE)0.0, true,
        v*m_texScale[1], u*m_texScale[0],
        sk_i[0], sk_i[1], sk_i[2], sk_i[3],
        sw_i[0], sw_i[1], sw_i[2], sw_i[3] );
    }
    for ( INDEX_TYPE cInx = 0; cInx <= circumferenceCount_2; cInx ++ )
    {
      DATA_TYPE u = (DATA_TYPE)cInx / (DATA_TYPE)circumferenceCount_2;
      DATA_TYPE angle = (DATA_TYPE)( CONST_PI * u + CONST_PI );
      DATA_TYPE x = (DATA_TYPE)cos( angle );
      DATA_TYPE y = (DATA_TYPE)sin( angle );
      def.Add( (DATA_TYPE)(x * m_radius), (DATA_TYPE)(y * m_radius), (DATA_TYPE)(heightFac * height_2),
        x, y, (DATA_TYPE)0.0, true,
        v*m_texScale[1], u*m_texScale[0],
        sk_i[0], sk_i[1], sk_i[2], sk_i[3],
        sw_i[0], sw_i[1], sw_i[2], sw_i[3] );
    }
  }

  // bottom outer cylinder points
  sk_i = { shaftSkI_bottom, shaftSkI_bottom, shaftSkI_bottom, shaftSkI_bottom };
  sw_i = { 1.0f, 0.0f, 0.0f, 0.0f };
  for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount; cInx ++ )
  {
    DATA_TYPE angle = (DATA_TYPE)( CONST_2PI * cInx / circumferenceCount );
    DATA_TYPE x = (DATA_TYPE)cos( angle );
    DATA_TYPE y = (DATA_TYPE)sin( angle );
    DATA_TYPE u = (DATA_TYPE)((cos( angle ) + 1.0) / 2.0) * texIScaleBottom;
    DATA_TYPE v = (DATA_TYPE)((sin( angle ) + 1.0) / 2.0) * texIScaleBottom;
    def.Add(
      (DATA_TYPE)(x * m_radius), (DATA_TYPE)(y * m_radius), (DATA_TYPE)-height_2,
      (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)-1.0, true,
      u, v,
      sk_i[0], sk_i[1], sk_i[2], sk_i[3],
      sw_i[0], sw_i[1], sw_i[2], sw_i[3] );
  }
  // bottom cylinder point
  def.Add( 
    (DATA_TYPE)0.0, (DATA_TYPE)0.0, -height_2,
    (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)-1.0, true,
    (DATA_TYPE)0.5f, (DATA_TYPE)0.5f,
    sk_i[0], sk_i[1], sk_i[2], sk_i[3],
    sw_i[0], sw_i[1], sw_i[2], sw_i[3] );
  
  // bottom outer peak points
  for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount; cInx ++ )
  {
    DATA_TYPE angle = (DATA_TYPE)( CONST_2PI * cInx / circumferenceCount );
    DATA_TYPE x = (DATA_TYPE)cos( angle );
    DATA_TYPE y = (DATA_TYPE)sin( angle );
    DATA_TYPE u = (DATA_TYPE)((cos( angle ) + 1.0) / 2.0);
    DATA_TYPE v = (DATA_TYPE)((sin( angle ) + 1.0) / 2.0);
    def.Add(
      (DATA_TYPE)(x * m_peakRad), (DATA_TYPE)(y * m_peakRad), (DATA_TYPE)-height_2,
      x, y, (DATA_TYPE)0.0, true,
      u, v,
      sk_i[0], sk_i[1], sk_i[2], sk_i[3],
      sw_i[0], sw_i[1], sw_i[2], sw_i[3] );
  }
  // bottom peak points
  sk_i = { peakSkI_bottom, peakSkI_bottom, peakSkI_bottom, peakSkI_bottom };
  sw_i = { 1.0f, 0.0f, 0.0f, 0.0f };
  def.Add(
    (DATA_TYPE)0.0, (DATA_TYPE)0.0, -(height_2+m_peakLen),
    (DATA_TYPE)0.0, (DATA_TYPE)0.0, (DATA_TYPE)-1.0, true,
    (DATA_TYPE)0.5f, (DATA_TYPE)0.5f,
    sk_i[0], sk_i[1], sk_i[2], sk_i[3],
    sw_i[0], sw_i[1], sw_i[2], sw_i[3] );

  if ( m_peakAtTop )
  {
    // top peak triangles
    INDEX_TYPE startI = 2 + circumferenceCount;
    INDEX_TYPE startO = 1;
    for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount; cInx ++ )
    {
      INDEX_TYPE cInxNext = (cInx + 1) % circumferenceCount;
      def.AddFace( startO + cInx, startO + cInxNext, 0 );
    }
    for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount; cInx ++ )
    {
      INDEX_TYPE cInxNext = (cInx + 1) % circumferenceCount;
      def.AddFace( startI + cInx, startI + cInxNext, startO + cInxNext, startO + cInx );
    }
  }
  else
  {
    // top disc triangles 
    INDEX_TYPE start = 2 + circumferenceCount;
    for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount; cInx ++ )
    {
      INDEX_TYPE cInxNext = (cInx + 1) % circumferenceCount;
      def.AddFace( start + cInx, start + cInxNext, circumferenceCount+1 );
    }
  }

  // shaft quads (2 trianlges)
  INDEX_TYPE peakPtCount         = 2 + 2 * circumferenceCount;
  INDEX_TYPE circumferenceSize_2 = circumferenceCount_2 + 1;
  INDEX_TYPE circumferenceSize   = circumferenceSize_2 * 2;
  for ( INDEX_TYPE tbInx = 1; tbInx <= topToBottomCount; tbInx ++ )
  {
    INDEX_TYPE ringStart     = peakPtCount + (tbInx-1) * circumferenceSize;
    INDEX_TYPE nextRingStart = peakPtCount + tbInx     * circumferenceSize;
    for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount_2; cInx ++ )
      def.AddFace( ringStart + cInx, nextRingStart + cInx, nextRingStart + cInx + 1, ringStart + cInx + 1 );
    ringStart     += circumferenceSize_2;
    nextRingStart += circumferenceSize_2;
    for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount_2; cInx ++ )
      def.AddFace( ringStart + cInx, nextRingStart + cInx, nextRingStart + cInx + 1, ringStart + cInx + 1 );
  }

  INDEX_TYPE shaftPtCount = ( topToBottomCount + 1 ) * circumferenceSize;
  if ( m_peakAtBottom )
  {
    // bottom peak triangles
    INDEX_TYPE startI = peakPtCount + shaftPtCount;
    INDEX_TYPE startO = peakPtCount + shaftPtCount + 1 + circumferenceCount;
    for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount; cInx ++ )
    {
      INDEX_TYPE cInxNext = (cInx + 1) % circumferenceCount;
      def.AddFace( startI + cInxNext, startI + cInx, startO + cInx, startO + cInxNext );
    }
    for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount; cInx ++ )
    {
      INDEX_TYPE cInxNext = (cInx + 1) % circumferenceCount;
      def.AddFace( startO + cInxNext, startO + cInx, startO + circumferenceCount );
    }
  }
  else
  {
    // bottom peak triangles
    INDEX_TYPE start = peakPtCount + shaftPtCount;
    for ( INDEX_TYPE cInx = 0; cInx < circumferenceCount; cInx ++ )
    {
      INDEX_TYPE cInxNext = (cInx + 1) % circumferenceCount;
      def.AddFace( start + cInxNext, start + cInx, start + circumferenceCount );
    }
  }


  return true;
}


} // namespace MeshDef
