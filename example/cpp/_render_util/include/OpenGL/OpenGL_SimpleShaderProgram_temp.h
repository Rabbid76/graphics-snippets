#pragma once

// include

// OpenGL

#include <OpenGL_Matrix_Camera.h>
#include <OpenGLProgram.h>


// OpenGL wrapper

#include <GL/glew.h>
//#include <GL/gl.h> not necessary because of GLEW
#include <GL/glu.h>

// STL

#include <vector>
#include <map>
#include <tuple>
#include <fstream>
#include <iostream>
#include <string>
#include <deque>


// preprocessor definitions

//#define OpenGL_program_introspection_old


/******************************************************************//**
* \brief General namespace for OpenGL implementation.  
* 
* \author  gernot
* \date    2018-09-07
* \version 1.0
**********************************************************************/
namespace OpenGL
{


using TShaderInfo = std::tuple< std::string, int >;


class ShaderProgram
{
private:

  using TShaderPtr        = Render::Program::TShaderPtr;
  using TShaderList       = std::deque<TShaderPtr>; 
  using TProgramPtr       = Render::Program::TProgramPtr;
  using TIntrospectionPtr = Render::Program::TIntrospectionPtr;

  bool              _verbose = true;
  TShaderList       _shaders;
  TProgramPtr       _program;
  TIntrospectionPtr _introspection;
  
public:

  ShaderProgram( const std::vector< TShaderInfo > & shaderList )
  {
    Create( shaderList, {}, 0 );
  }

  ShaderProgram( const std::vector< TShaderInfo > & shaderList, const std::vector<std::string> &tf_varyings, unsigned int tf_mode )
  {
    Create( shaderList, tf_varyings, tf_mode );
  }

  virtual ~ShaderProgram() = default;
  
  GLuint Prog( void ) const { return _program != nullptr ? (GLuint)_program->ObjectHandle() : 0; }
  void Use( void ) const { if ( _program != nullptr ) _program->Use(); }
  void Release( void ) const { glUseProgram( 0 ); }

  int FindUniformLocation( const std::string &name ) const
  {
    if ( _introspection == nullptr )
      return -1;

    size_t handle;
    Render::Program::TResourceType resource_type;
    if ( _introspection->FindProgramResource( name, handle, resource_type ) == false )
      return -1;
    if ( resource_type != Render::Program::TResourceType::uniform )
      return -1;
   
    return (int)handle;
  }
  
  void SetUniformI1( const std::string &name, int val ) const
  { 
    int location = FindUniformLocation( name );
    if ( location >= 0 )
      glUniform1i( location, val );
  }

  void SetUniformF1( const std::string &name, float val ) const
  { 
    int location = FindUniformLocation( name );
    if ( location >= 0 )
      glUniform1f( location, val );
  }

  void SetUniformF2( const std::string &name, const TVec2 &vec ) const
  { 
    int location = FindUniformLocation( name );
    if ( location >= 0 )
      glUniform2fv( location, 1, vec.data() );
  }

  void SetUniformF3( const std::string &name, const TVec3 &vec ) const
  { 
    int location = FindUniformLocation( name );
    if ( location >= 0 )
      glUniform3fv( location, 1, vec.data() );
  }

  void SetUniformF4( const std::string &name, const TVec4 &vec ) const
  { 
    int location = FindUniformLocation( name );
    if ( location >= 0 )
      glUniform4fv( location, 1, vec.data() );
  }
  
  void SetUniformM44( const std::string &name, const float * mat ) const
  { 
    int location = FindUniformLocation( name );
    if ( location >= 0 )
      glUniformMatrix4fv( location, 1, GL_FALSE, mat );
  }
  void SetUniformM44( const std::string &name, const TMat44 & mat ) const
  { 
    int location = FindUniformLocation( name );
    if ( location >= 0 )
      glUniformMatrix4fv( location, 1, GL_FALSE, mat.data()->data() );
  }


  // Create program - compile and link
  void Create( 
    const std::vector< TShaderInfo > & shaderList,
    const std::vector<std::string>   & tf_varyings,
    unsigned int                       tf_mode )
  {
    if ( _verbose )
    {
      GLenum err = glGetError();
      if ( err )
        std::cout << "create shader program clear error: " << err << std::endl;
    }

    // create and compile the shader objects
    std::vector<int> shaderObjs;
    std::transform( shaderList.begin(), shaderList.end(), std::back_inserter( shaderObjs ), [&]( const TShaderInfo & sh_info ) -> int
    {
      return CompileShader( std::get<0>( sh_info ), std::get<1>( sh_info ) );
    } );

    if ( _verbose )
    {
      GLenum err = glGetError();
      if ( err )
        std::cout << "shader object error: " << err << std::endl;
    }

    // create the program object and attach the shader objects
    _program = std::make_shared<OpenGL::CShaderProgram>();
    for ( auto shader : _shaders )
    {
      if ( shader != nullptr )
        *_program << shader;
    }

    // int the transform feedback varyings
    if ( tf_varyings.size() > 0 &&
         ( tf_mode == GL_INTERLEAVED_ATTRIBS || tf_mode == GL_SEPARATE_ATTRIBS ) )
    {
      for ( auto &str : tf_varyings )  
        *_program << std::make_tuple( str, Render::Program::TResourceType::transform_feedback, 0 );
      *_program << (tf_mode == GL_INTERLEAVED_ATTRIBS ?
        Render::Program::TTranformFeedbackMode::interleaved : Render::Program::TTranformFeedbackMode::separate);
    }

    if ( _verbose )
    {
      GLenum err = glGetError();
      if ( err )
        std::cout << "transform feedback varying error: " << err << std::endl;
    }

    // link shader objects to shader program
    _program->Link();
    
    // verify linking
    std::string message;
    bool success = _program->Verify( message );
    if ( success == false )
      std::cout << message;

    if ( _verbose )
    {
      GLenum err = glGetError();
      if ( err )
        std::cout << "shader program error: " << err << std::endl;
    }

    // clean up
    _shaders.clear();

    // program introspection
    GetResources();

    if ( _verbose )
    {
      GLenum err = glGetError();
      if ( err )
        std::cout << "introspection error: " << err << std::endl;
    }
  }

  // read shader program and compile shader
  int CompileShader( const std::string & file, int shaderStage )
  {
    Render::Program::TShaderType type = CShaderObject::ShaderType( shaderStage );

    std::ifstream sourceFile( file, std::fstream::in );
    std::string sourceCode = file;
    if ( sourceFile.is_open() )
      sourceCode = std::string( std::istreambuf_iterator<char>( sourceFile ), std::istreambuf_iterator<char>() );
    std::string shader_type_name = CShaderObject::ShaderTypeName( type );

    if ( _verbose )
      std::cout << shader_type_name << " shader code:" << std::endl << sourceCode << std::endl << std::endl;
    
    // create shader object
    _shaders.emplace_back( std::make_shared<CShaderObject>( type ) );

    // append source code and compile
    *_shaders.back() << std::move(sourceCode);
    _shaders.back()->Compile();

    // verify compilation
    std::string message;
    bool success = _shaders.back()->Verify( message );
    if ( success == false )
      std::cout << message;
    
    // return shader object handle
    return (int)_shaders.back()->ObjectHandle();
  }

  // Get program resources
  void GetResources()
  {
    if ( _program == nullptr )
      return;

    if ( _verbose )
      std::cout << std::endl;
  
    _introspection = _program->Introspection( Render::Program::TResourceTypes().set(), _verbose );
    
    if ( _verbose )
      std::cout << std::endl;
  }
};

} // OpenGL 
