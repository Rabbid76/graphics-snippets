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
#include <RenderUtil_MeshContainer.h>

#include <string>
#include <iostream>


// preprocessor definitions


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
* \brief   ctor
* 
* \author  gernot
* \date    2018-06-17
* \version 1.0
**********************************************************************/
template<typename T_DATA, typename T_INDEX>
CObjFileLoader<T_DATA, T_INDEX>::CObjFileLoader( 
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
template<typename T_DATA, typename T_INDEX>
CObjFileLoader<T_DATA, T_INDEX>::~CObjFileLoader()
{}

  
/******************************************************************//**
* \brief   load mesh data from file
* 
* \author  gernot
* \date    2018-06-17
* \version 1.0
**********************************************************************/
template<typename T_DATA, typename T_INDEX>
typename CObjFileLoader<T_DATA, T_INDEX>::TUniqueMesh CObjFileLoader<T_DATA, T_INDEX>::Load( void ) const 
{
  if ( _file_name.empty() )
    return nullptr;
  std::ifstream obj_stream( _file_name, std::ios::in );
  if( !obj_stream )
    return nullptr;

  std::unique_ptr<CMeshContainer<T_DATA, T_INDEX>> mesh_ptr = std::make_unique<CMeshContainer<T_DATA, T_INDEX>>();

  // parse the file, line by line
  static const std::string white_space = " \t\n\r";
  std::string token;
  std::string indices;
  std::string index;
  T_DATA value;
  CMeshContainer<T_DATA, T_INDEX> &m = *mesh_ptr.get();
  for( std::string line; std::getline( obj_stream, line ); )
  {
    // find first non whispce characterr in line
    size_t start = line.find_first_not_of( white_space );
    if ( start == std::string::npos )
      continue;

    // read the first token
    std::istringstream line_stream( line.substr(start) );
    line_stream.exceptions( 0 );
    line_stream >> token;
    
    // ignore comment lines
    if ( token[0] == '#' )
      continue;

    // read the line
    if ( token == "v" )
    {
      // read vertex coordinate
      while ( line_stream >> value )
        m._v._av.push_back( value );
      if ( m._v._tuple_size == 0 )
        m._v._tuple_size = (int)m._v._av.size();
    }
    else if ( token == "vt" )
    {
      // read normal_vectors 
      while ( line_stream >> value )
        m._vt._av.push_back( value );
      if ( m._vt._tuple_size == 0 )
        m._vt._tuple_size = (int)m._vt._av.size();
    }
    else if ( token == "vn" )
    {
      // read normal_vectors 
      while ( line_stream >> value )
        m._vn._av.push_back( value );
      if ( m._vn._tuple_size == 0 )
        m._vn._tuple_size = (int)m._vn._av.size();
    }
    else if ( token == "f" )
    {
      // read faces
      while( line_stream >> indices )
      {
        // TODO $$$ use `std::vector< std::vector<T_INDEX> > _f` and use `std::istringstream` and `std::getline` ???
        //
        //     std::istringstream index_stream( indices );
        //     for( int i=0; std::getline( obj_stream, index ); ++i )
        //         _f[i].push_back(index);

        // parse indices
        auto slash1 = indices.find("/");
        if( slash1 == std::string::npos )
        {
          m._f0._iv.push_back( std::stoi(indices.c_str(), nullptr, 10)-1 );
        }
        else
        {
          m._f0._iv.push_back( std::stoi(indices.substr(0, slash1), nullptr, 10)-1 );

          auto slash2 = indices.find("/", slash1+1 );
          if ( slash2 == std::string::npos ) 
          {
            if ( slash1+1 < indices.length() ) 
              m._f1._iv.push_back( std::stoi(indices.substr(slash1+1), nullptr, 10)-1 );
          }
          else
          {
            if ( slash2 > slash1+1 )
              m._f1._iv.push_back( std::stoi(indices.substr(slash1+1, slash2-slash1-1), nullptr, 10)-1 );
            if ( slash2+1 < indices.length() )
              m._f2._iv.push_back( std::stoi(indices.substr(slash2+1), nullptr, 10)-1 );
          }
        }
      }
      if ( m._face_size == 0 )
        m._face_size = (int)m._f0._iv.size();
    }
  }

  // clear identically index lists
  if ( m._f0._iv == m._f1._iv )
    m._f1._iv.clear();
  if ( m._f0._iv == m._f2._iv )
    m._f2._iv.clear();

  // set index pointers
  if ( m._vt._tuple_size > 0 )
  {
    m._f_vt = m._f1.empty() ? nullptr : &m._f1;
    if ( m._vn._tuple_size > 0 )
      m._f_vn = m._f2.empty() ? nullptr : &m._f2;
  }
  else if ( m._vn._tuple_size > 0 )
  {
    m._f_vn = m._f1.empty() ? nullptr : &m._f1;
  }
  
  return mesh_ptr;
}


} // Render

#endif // RenderUtil_ObjLoader_h_INCLUDED
