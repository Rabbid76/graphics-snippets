/******************************************************************//**
* \brief   Obj-File loader implementation.
*
* [https://de.wikipedia.org/wiki/Wavefront_OBJ]
* [https://en.wikipedia.org/wiki/Wavefront_.obj_file]
* [https://www.cs.utah.edu/~boulos/cs3505/obj_spec.pdf]
* 
* \author  gernot
* \date    2017-11-20
* \version 1.0
**********************************************************************/
#pragma once
#ifndef RenderUtil_ObjLoader_h_INCLUDED
#define RenderUtil_ObjLoader_h_INCLUDED


// includes

#include <Render_IMesh.h>


// preprocessor definitions

#ifndef DebugWarning
#define DebugWarning std::cout
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
* \brief   Implementation of obj-file loader.
* 
* \author  gernot
* \date    2018-05-29
* \version 1.0
**********************************************************************/
template<typename T_INDEX, typename T_DATA>
class CObjFileLoader
  : public IMeshResource<T_INDEX, T_DATA>
{
public:

  using TUniqueMesh = TMeshPtr<T_INDEX, T_DATA>;

  CObjFileLoader( const std::string &file_name );
  virtual ~CObjFileLoader();

  virtual TUniqueMesh Load( void ) const override; // read data from file a store data to mesh 

private:

  std::string _file_name; //!< name of the obj file
};


/******************************************************************//**
* \brief   Mesh which was read from obj file.
* 
* \author  gernot
* \date    2018-05-29
* \version 1.0
**********************************************************************/
template<typename T_DATA, typename T_INDEX>
class CObjFileMesh
  : public IMeshData<T_DATA, T_INDEX>
{                                
  friend class CObjFileLoader<T_INDEX, T_DATA>;

public: 

  using TFaces      = TIndexVectorN<T_INDEX>;
  using TAttributes = TAttributeVectorN<T_DATA>;
  using TAttributes = TAttributeVectorN<T_DATA>;
  using TAttributes = TAttributeVectorN<T_DATA>;

  virtual Render::TMeshFaceType  FaceType( void ) const override
  {
    if ( _face_size != 3 && _face_size != 4 )
      DebugWarning << "Illegal number of vertices for a face (primitive) size : " << _face_size;
    return _face_size == 4 ? Render::TMeshFaceType::quads : Render::TMeshFaceType::triangles; 
  }

  virtual Render::TMeshIndexKind IndexKind( void ) const override
  { 
    return (_f_vn == nullptr && _f_vt == nullptr) ? Render::TMeshIndexKind::common : Render::TMeshIndexKind::multiple; 
  }

  virtual const TAttributes & Vertices( void ) const override { return _v; } 
  virtual const TFaces      * Indices( void )  const override { return &_f0; }
  virtual T_INDEX             FaceSize( void ) const override { return _face_size; }

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
  virtual T_INDEX                     FaceRestart( void )       const override { return 0; }
  virtual const TAttributes         * FaceNormals( void )       const override { return nullptr; }
  virtual const TFaces              * FaceNormalIndices( void ) const override { return nullptr; }
  virtual const TAttributes         * Colors( void )            const override { return nullptr; }
  virtual const TFaces              * ColorIndices( void )      const override { return nullptr; }

private:

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



/******************************************************************//**
* \brief   ctor
* 
* \author  gernot
* \date    2018-06-17
* \version 1.0
**********************************************************************/
template<typename T_INDEX, typename T_DATA>
CObjFileLoader<T_INDEX, T_DATA>::CObjFileLoader( 
  const std::string &file_name ) //!< I - filename
  : _file_name( file_name )
{}
  
  
/******************************************************************//**
* \brief   dtor
* 
* \author  gernot
* \date    2018-06-17
* \version 1.0
**********************************************************************/
template<typename T_INDEX, typename T_DATA>
CObjFileLoader<T_INDEX, T_DATA>::~CObjFileLoader()
{}

  
/******************************************************************//**
* \brief   load mesh data from file
* 
* \author  gernot
* \date    2018-06-17
* \version 1.0
**********************************************************************/
template<typename T_INDEX, typename T_DATA>
typename CObjFileLoader<T_INDEX, T_DATA>::TUniqueMesh CObjFileLoader<T_INDEX, T_DATA>::Load( void ) const 
{
  if ( _file_name.empty() )
    return nullptr;

  std::unique_ptr<CObjFileMesh<T_INDEX, T_DATA>> mesh_ptr = std::make_unique<CObjFileMesh<T_INDEX, T_DATA>>();

  // TODO $$$

  return nullptr;
}


} // Render

#endif // RenderUtil_ObjLoader_h_INCLUDED
