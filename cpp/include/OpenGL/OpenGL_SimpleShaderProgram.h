#pragma once

// include

#include <OpenGL_Matrix_Camera.h>

// OpenGL

#include <GL/glew.h>
//#include <GL/gl.h> not necessary because of glew 
#include <GL/glu.h>

// stl

#include <vector>
#include <map>
#include <tuple>
#include <fstream>
#include <iostream>
#include <string>


namespace OpenGL
{


using TShaderInfo = std::tuple< std::string, int >;


class ShaderProgram
{
private:

  GLuint                         _prog;
  std::map< std::string, GLint > _unifomLocation;

public:

  ShaderProgram( const std::vector< TShaderInfo > & shaderList, const std::vector< std::string > & uniformNames )
  {
    std::vector<int> shaderObjs;
    std::transform( shaderList.begin(), shaderList.end(), std::back_inserter( shaderObjs ), [&]( const TShaderInfo & sh_info ) -> int
    {
      return CompileShader( std::get<0>( sh_info ), std::get<1>( sh_info ) );
    } );
    LinkProgram( shaderObjs );
    for ( auto & name : uniformNames )
    {
      _unifomLocation[name] = glGetUniformLocation( _prog, name.data() );
      std::cout << "uniform " << name << " at loaction " << _unifomLocation[name] << std::endl;
    }
  }

  virtual ~ShaderProgram() { glDeleteProgram( _prog ); }
  
  GLuint Prog( void ) const { return _prog; }
  void Use( void ) const { glUseProgram( _prog ); }
  void Release( void ) const { glUseProgram( 0 ); }
  
  void SetUniformF1( const std::string &name, float val ) const
  { 
    auto it = _unifomLocation.find( name );
    if ( it != _unifomLocation.end() )
      glUniform1f( it->second, val );
  }

  void SetUniformF2( const std::string &name, const TVec2 &vec ) const
  { 
    auto it = _unifomLocation.find( name );
    if ( it != _unifomLocation.end() )
      glUniform2fv( it->second, 1, vec.data() );
  }

  void SetUniformF3( const std::string &name, const TVec3 &vec ) const
  { 
    auto it = _unifomLocation.find( name );
    if ( it != _unifomLocation.end() )
      glUniform3fv( it->second, 1, vec.data() );
  }

  void SetUniformF4( const std::string &name, const TVec4 &vec ) const
  { 
    auto it = _unifomLocation.find( name );
    if ( it != _unifomLocation.end() )
      glUniform4fv( it->second, 1, vec.data() );
  }
  
  void SetUniformM44( const std::string &name, const TMat44 & mat ) const
  { 
    auto it = _unifomLocation.find( name );
    if ( it != _unifomLocation.end() )
      glUniformMatrix4fv( it->second, 1, GL_FALSE, mat.data()->data() );
  }

  // read shader program and compile shader
  int CompileShader( const std::string & file, int shaderStage ) const
  {
    std::ifstream sourceFile( file, std::fstream::in );
    std::string sourceCode = file;
    if ( sourceFile.is_open() )
      sourceCode = std::string( (std::istreambuf_iterator<char>( sourceFile )), std::istreambuf_iterator<char>() );
    const std::map<int, std::string>nameMap{ { GL_VERTEX_SHADER,  "vertex" }, { GL_FRAGMENT_SHADER, "fragment" } };
    std::cout << nameMap.find(shaderStage)->second.data() << "shader code:" << std::endl << sourceCode << std::endl << std::endl;
    auto shaderObj = glCreateShader( shaderStage );
    const char *srcCodePtr = sourceCode.data();
    glShaderSource( shaderObj, 1, &srcCodePtr, nullptr );
    glCompileShader( shaderObj );
    GLint status = GL_TRUE;
	  glGetShaderiv( shaderObj, GL_COMPILE_STATUS, &status );
    if ( status == GL_FALSE )
    {
      GLint logLen;
			glGetShaderiv( shaderObj, GL_INFO_LOG_LENGTH, &logLen );
      std::vector< char >log( logLen+1 );
			GLsizei written;
			glGetShaderInfoLog( shaderObj, logLen, &written, log.data() );
			std::cout << "compile error:" << std::endl << log.data() << std::endl;
      throw std::runtime_error( "compile error" );
    }
    return shaderObj;
  }
    
  // linke shader objects to shader program
  void LinkProgram( const std::vector<int> & shaderObjs ) {
    _prog = glCreateProgram();
    for ( auto shObj : shaderObjs )
      glAttachShader( _prog, shObj );
    glLinkProgram( _prog );
    GLint status = GL_TRUE;
	  glGetProgramiv( _prog, GL_LINK_STATUS, &status );
    if ( status == GL_FALSE )
    {
      GLint logLen;
			glGetProgramiv( _prog, GL_INFO_LOG_LENGTH, &logLen );
      std::vector< char >log( logLen+1 );
			GLsizei written;
			glGetProgramInfoLog( _prog, logLen, &written, log.data() );
			std::cout  << "link error:" << std::endl << log.data() << std::endl;
      throw std::runtime_error( "link error" );
    }
  }
};


} // OpenGL 
