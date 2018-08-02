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
  virtual size_t                       ObjectHandle( void ) override { return _object; }
  virtual Render::Program::TShaderType Type( void )         override { return _type; }
  
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
  unsigned int                 _object = 0; //!< named sahder object (GPU)
};

} // OpenGL


#endif // OpenGLFramebuffer_h_INCLUDED
