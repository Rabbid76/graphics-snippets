/******************************************************************//**
* \brief Implementation of OpenGL shader programs.
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/
#pragma once
#ifndef OpenGLProgram_h_INCLUDED
#define OpenGLProgram_h_INCLUDED

// includes

#include "../render/Render_IProgram.h"


// STL

#include <vector>
#include <deque>
#include <tuple> 
#include <string> 
#include <unordered_map> 


// class definitions


/******************************************************************//**
* \brief General namespace for OpenGL implementation.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
namespace OpenGL
{


//*********************************************************************
// CShaderObject
//*********************************************************************


//! TODO $$$ [Standard, Portable Intermediate Representation - V (SPIR-V) shader


/******************************************************************//**
* \brief OpenGL implementation of a shader stage.  
* 
* \author  gernot
* \date    2018-08-03
* \version 1.0
**********************************************************************/
class CShaderObject
  : public Render::Program::IShader
{
public:

  CShaderObject( Render::Program::TShaderType type );
  CShaderObject( CShaderObject && source_objet );
  
  virtual ~CShaderObject();

  virtual CShaderObject & operator =( CShaderObject && source_objet );
  
  static size_t                       ShaderEnum( Render::Program::TShaderType type );
  static Render::Program::TShaderType ShaderType( size_t stage );
  static const char *                 ShaderTypeName( Render::Program::TShaderType type );

  // return shader object handle 
  virtual size_t ObjectHandle( void ) const override { return _object; }

  // return state type
  virtual Render::Program::TShaderType Type( void ) override { return _type; }
  
  // append source code to the shader 
  virtual CShaderObject & operator << ( const std::string & code ) override;
  virtual CShaderObject & operator << ( std::string && code ) override;
  virtual CShaderObject & ClearCode( void ) override;
  
  // Compile the shader stage, the function succeeds, even if the compilation fails, but it fails if the stage was not properly initialized.
  virtual bool Compile( void ) override;
  
  // Verifies the compilation result.
  virtual bool Verify( std::string &message ) override;

private: 

  static const std::vector<std::tuple<Render::Program::TShaderType, size_t>> c_type_map;

  std::deque<std::string>      _code;       //!< shader source code
  Render::Program::TShaderType _type;       //!< type the shader (stage)
  unsigned int                 _object = 0; //!< named shader object (GPU)
};


//*********************************************************************
// CShaderProgram
//*********************************************************************

/******************************************************************//**
* \brief OpenGL implementation of a shader program.  
* 
* \author  gernot
* \date    2018-08-03
* \version 1.0
**********************************************************************/
class CShaderProgram
  : public Render::Program::IProgram
{
public:

  using TShaderList           = std::deque<Render::Program::TShaderPtr>;
  using TResourceLinkList     = std::deque<Render::Program::TResourceBinding>;
  using TTranformFeedbackMode = Render::Program::TTranformFeedbackMode;

  CShaderProgram( void );
  CShaderProgram( CShaderProgram && source_objet );

  virtual ~CShaderProgram();

  virtual CShaderProgram & operator =( CShaderProgram && source_objet );

  // return shader object handle 
  virtual size_t ObjectHandle( void ) const override { return _object; }

  // append a shader object
  virtual Render::Program::IProgram & operator << ( const Render::Program::TShaderPtr & shader ) override
  {
    _shaders.push_back( shader );
    return *this;
  }

  // append resource specification (binding point, location or index)
  virtual IProgram & operator << ( const Render::Program::TResourceBinding & resource ) override
  {
    _resource_binding.push_back( resource );
    return *this;
  }

  // set transform feedback mode
  virtual IProgram & operator << ( const Render::Program::TTranformFeedbackMode & mode ) override
  {
    _transform_feedback_mode = mode;
    return *this;
  }

  // Link shader objects to a shader program, the function succeeds, even if the linking fails, but it fails if the program was not properly initialized.
  virtual bool Link( void ) override;

  // Verifies the linking result.
  virtual bool Verify( std::string &message ) override;

  // validates if the shader program was verified successfully
  virtual bool Valid( void ) override { return _object_status != 0; }

  // Activate a program.
  virtual bool Use( void ) override;

  // Ask for the resource information of the program.
  virtual Render::Program::TIntrospectionPtr Introspection( Render::Program::TResourceTypes resources, bool verbose ) override;

private:

  TShaderList           _shaders;                                              //!< shader objects linked to a program
  TResourceLinkList     _resource_binding;                                     //!< additional resource bindings for linking the program
  TTranformFeedbackMode _transform_feedback_mode = TTranformFeedbackMode::NON; //!< buffer mode for transform feedback shader (required for linking)
  unsigned int          _object                  = 0;                          //!< named program object (GPU)
  int                   _object_status           = 0;                          //!< status of the verified program object
};


/******************************************************************//**
* \brief Implementation of the introspection of an OpenGL shader program.  
* 
* \author  gernot
* \date    2018-08-05
* \version 1.0
**********************************************************************/
class CIntrospection
  : public Render::Program::IIntrospection
{
public:

  using IProgram = Render::Program::IProgram;
  using TResourceType  = Render::Program::TResourceType;
  using TResourceTypes = Render::Program::TResourceTypes;
  using TResourcePoint = Render::Program::TResourcePoint;

  CIntrospection( const IProgram & program );
  virtual ~CIntrospection();

  bool Verbose( void ) const { return _verbose; }
  CIntrospection & Verbose( bool verbose ) { _verbose = verbose; return *this; }

  // observe the program 
  virtual CIntrospection & Observe( Render::Program::TResourceTypes resources );

  // find a active program resource
  virtual bool FindProgramResource( const std::string &name, size_t &handle, TResourceType &resource_type ) override
  {
    auto it = _resources.find( name );
    if ( it == _resources.end() )
      return false;

    handle        = std::get<0>( it->second );
    resource_type = std::get<1>( it->second );
    return true;
  }

private:

  enum class TResourceKind { NON, location };
  using TResourceMap     = std::unordered_map< std::string, size_t >;
  using TResourceTypeMap = std::unordered_map< std::string, TResourcePoint >;

  void GetInterfaceResources( unsigned int prog, unsigned int prog_interface, TResourceMap &resources, TResourceKind kind ) const;
  void AddResources( const TResourceMap &resources, TResourceType type );

  void GetAttributes( void );
  void GetTransformFeedbackVaryings( void );
  void GetFragmentData( void );
  void GetUniforms( void );           
  void GetUniformBlocks( void );
  void GetSubroutines( void );

  const IProgram & _program;                   //!< program object
  TResourceTypes   _resource_types;            //!< observed resource types

  bool             _verbose = false;           //!< trace introspection result
  
  TResourceTypeMap _resources;                 //!< common resource map
  TResourceMap     _attributeIndices;          //!< map of active attributes and its indices
  TResourceMap     _transformFeedbackVaryings; //!< map of transformed feedback varyings
  TResourceMap     _fragDataLocation;          //!< map of fragment outputs and its locations
  TResourceMap     _unifomLocation;            //!< map of uniforms and its locations
};

} // OpenGL


#endif // OpenGLFramebuffer_h_INCLUDED
