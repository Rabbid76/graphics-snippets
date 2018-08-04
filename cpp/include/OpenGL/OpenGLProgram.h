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

#include <Render_IProgram.h>


// stl

#include <vector>
#include <deque>
#include <tuple> 
#include <string> 


// class definitions

namespace OpenGL
{


//*********************************************************************
// CShaderObject
//*********************************************************************


//! TODO $$$ [Standard, Portable Intermediate Representation - V (SPIR-V) shaders


/******************************************************************//**
* \brief OpenGL implementation of a sahder stage.  
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
  virtual size_t ObjectHandle( void ) override { return _object; }

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
  virtual size_t ObjectHandle( void ) override { return _object; }

  // append a shader obeject
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

  // Activate a program.
  virtual bool Use( void ) override;

  // Ask for the resource information of the program.
  virtual Render::Program::IIntrospection && Introspection( Render::Program::TResourceTypes resources ) override;

private:

  TShaderList           _shaders;                                              //!< shader objects linked to a program
  TResourceLinkList     _resource_binding;                                     //!< additional resource bindings for linking the program
  TTranformFeedbackMode _transform_feedback_mode = TTranformFeedbackMode::NON; //!< buffer mode for transform feedback sahders (required for linking)
  unsigned int          _object                  = 0;                          //!< named program object (GPU)
};


} // OpenGL


#endif // OpenGLFramebuffer_h_INCLUDED
