/******************************************************************//**
* \brief   Mesh implementation
* 
* \author  gernot
* \date    2017-11-20
* \version 1.0
**********************************************************************/
#pragma once
#ifndef RenderUtil_MeshContainer_h_INCLUDED
#define RenderUtil_MeshContainer_h_INCLUDED


// includes

#include <Render_IMesh.h>

#include <string>
#include <iostream>


// preprocessor definitions

#ifndef ASSERT
#include <assert.h>
#define ASSERT assert
#endif


/******************************************************************//**
* @brief   Namespace for renderer.
*
* @author  gernot
* @date    2018-03-17
* @version 1.0
**********************************************************************/
namespace Render
{


/******************************************************************//**
* \brief   Mesh which was read from obj file.
* 
* \author  gernot
* \date    2018-05-29
* \version 1.0
**********************************************************************/
template<typename T_DATA, typename T_INDEX>
class CMeshContainer
  : public IMeshData<T_DATA, T_INDEX>
{                                
public: 

  using TValue      = T_DATA;
  using TIndex      = T_INDEX;
  using TFaces      = TIndexVectorN<T_INDEX>;
  using TAttributes = TAttributeVectorN<T_DATA>;
  using TAttributes = TAttributeVectorN<T_DATA>;
  using TAttributes = TAttributeVectorN<T_DATA>;

  virtual Render::TMeshFaceType FaceType( void ) const override
  {
    ASSERT( _face_size == 3 || _face_size == 4 );
    return _face_size == 4 ? Render::TMeshFaceType::quads : Render::TMeshFaceType::triangles; 
  }

  virtual Render::TMeshIndexKind IndexKind( void ) const override
  { 
    return (_f_vn == nullptr && _f_vt == nullptr) ? Render::TMeshIndexKind::common : Render::TMeshIndexKind::multiple; 
  }

  virtual const TAttributes & Vertices( void ) const override { return _v; } 
  virtual const TFaces      * Indices( void )  const override { return &_f0; }
  virtual TIndex              FaceSize( void ) const override { return _face_size; }

  virtual Render::TMeshNormalKind NormalKind( void ) const override
  { 
    return _vn.empty() ? Render::TMeshNormalKind::non : Render::TMeshNormalKind::vertex;
  }

  virtual const TAttributes * Normals( void ) const override
  { 
    return _vn.empty() ? nullptr : &_vn;
  }

  virtual const TFaces * NormalIndices( void ) const override
  { 
    return _vn.empty() ? nullptr : _f_vn; 
  }

  virtual const TAttributes * TextureCoordinates( void ) const override
  { 
    return _vt.empty() ? nullptr : &_vt; 
  }

  virtual const TFaces * TextureCoordIndices( void ) const override
  { 
    return _vt.empty() ? nullptr : _f_vt;
  }

  virtual Render::TMeshFaceSizeKind   FaceSizeKind( void )      const override { return Render::TMeshFaceSizeKind::constant; } 
  virtual Render::TMeshAttributePack  Pack( void )              const override { return Render::TMeshAttributePack::separated_tightly; }
  virtual const TFaces              * FaceSizes( void )         const override { return nullptr; }
  virtual TIndex                      FaceRestart( void )       const override { return 0; }
  virtual const TAttributes         * FaceNormals( void )       const override { return nullptr; }
  virtual const TFaces              * FaceNormalIndices( void ) const override { return nullptr; }
  virtual const TAttributes         * Colors( void )            const override { return nullptr; }
  virtual const TFaces              * ColorIndices( void )      const override { return nullptr; }

  int         _face_size = 0;
  TFaces *    _f_vn = nullptr;
  TFaces *    _f_vt = nullptr;

  TAttributes _v;
  TAttributes _vn;
  TAttributes _vt;
  TFaces      _f0;
  TFaces      _f1;
  TFaces      _f2;
};

}

#endif // RenderUtil_MeshContainer_h_INCLUDED
