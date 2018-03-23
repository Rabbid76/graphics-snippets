#pragma once

// include

// OpenGL

#include <OpenGL_Matrix_Camera.h>


// OpenGL wrapper

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


// preprcessor definitions

//#define OpenGL_program_introspection_old

namespace OpenGL
{


using TShaderInfo = std::tuple< std::string, int >;


class ShaderProgram
{
private:

  using TResourceMap = std::map< std::string, GLint >;

  bool         _verbose = true;
  GLuint       _prog    = 0;
  TResourceMap _attributeIndices;
  TResourceMap _transformFeedbackVaryings;
  TResourceMap _fragOutputLocation;
  TResourceMap _unifomLocation;
  
public:

  ShaderProgram( const std::vector< TShaderInfo > & shaderList )
  {
    Create( shaderList, {}, 0 );
  }

  ShaderProgram( const std::vector< TShaderInfo > & shaderList, const std::vector<std::string> &tf_varyings, unsigned int tf_mode )
  {
    Create( shaderList, tf_varyings, tf_mode );
  }

  virtual ~ShaderProgram() { glDeleteProgram( _prog ); }
  
  GLuint Prog( void ) const { return _prog; }
  void Use( void ) const { glUseProgram( _prog ); }
  void Release( void ) const { glUseProgram( 0 ); }
  
  void SetUniformI1( const std::string &name, int val ) const
  { 
    auto it = _unifomLocation.find( name );
    if ( it != _unifomLocation.end() )
      glUniform1i( it->second, val );
  }

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
  
  void SetUniformM44( const std::string &name, const float * mat ) const
  { 
    auto it = _unifomLocation.find( name );
    if ( it != _unifomLocation.end() )
      glUniformMatrix4fv( it->second, 1, GL_FALSE, mat );
  }
  void SetUniformM44( const std::string &name, const TMat44 & mat ) const
  { 
    auto it = _unifomLocation.find( name );
    if ( it != _unifomLocation.end() )
      glUniformMatrix4fv( it->second, 1, GL_FALSE, mat.data()->data() );
  }


  // Create program - compile and link
  void Create( 
    const std::vector< TShaderInfo > & shaderList,
    const std::vector<std::string>   & tf_varyings,
    unsigned int                       tf_mode )
  {
    // create and compile the shader objects
    std::vector<int> shaderObjs;
    std::transform( shaderList.begin(), shaderList.end(), std::back_inserter( shaderObjs ), [&]( const TShaderInfo & sh_info ) -> int
    {
      return CompileShader( std::get<0>( sh_info ), std::get<1>( sh_info ) );
    } );

    // create the program object and attach the shader objects
    _prog = glCreateProgram();
    for ( auto shObj : shaderObjs )
      glAttachShader( Prog(), shObj );

    if ( _verbose )
    {
      GLenum err = glGetError();
      if ( err )
        std::cout << "shader object error: " << err << std::endl;
    }

    // int the transform feedback varyings
    if ( tf_varyings.size() > 0 &&
         ( tf_mode == GL_INTERLEAVED_ATTRIBS || tf_mode == GL_SEPARATE_ATTRIBS ) )
    {
      // When no `xfb_buffer`, `xfb_offset`, or `xfb_stride` layout qualifiers are specified, the set of variables to record is specified with the command
      //[`glTransformFeedbackVaryings`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTransformFeedbackVaryings.xhtml)
      std::vector<const char *>varying_ptrs;
      for ( auto &str : tf_varyings )
        varying_ptrs.emplace_back( str.c_str() );
      glTransformFeedbackVaryings(	Prog(), (GLsizei)varying_ptrs.size(), varying_ptrs.data(), (GLenum)tf_mode );
    }

    if ( _verbose )
    {
      GLenum err = glGetError();
      if ( err )
        std::cout << "transform feedback varying error: " << err << std::endl;
    }

    // link the program 
    LinkProgram( shaderObjs );

    if ( _verbose )
    {
      GLenum err = glGetError();
      if ( err )
        std::cout << "shader program error: " << err << std::endl;
    }

    // clean up
    for ( auto shObj : shaderObjs )
      glDeleteShader( shObj );

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
  int CompileShader( const std::string & file, int shaderStage ) const
  {
    std::ifstream sourceFile( file, std::fstream::in );
    std::string sourceCode = file;
    if ( sourceFile.is_open() )
      sourceCode = std::string( (std::istreambuf_iterator<char>( sourceFile )), std::istreambuf_iterator<char>() );
    const std::map<int, std::string>nameMap{ { GL_VERTEX_SHADER,  "vertex" }, { GL_FRAGMENT_SHADER, "fragment" } };

    if ( _verbose )
      std::cout << nameMap.find(shaderStage)->second.data() << "shader code:" << std::endl << sourceCode << std::endl << std::endl;
    
    auto shaderObj = glCreateShader( shaderStage );
    const char *srcCodePtr = sourceCode.data();
    glShaderSource( shaderObj, 1, &srcCodePtr, nullptr );
    glCompileShader( shaderObj );
    GLint status;
    glGetShaderiv( shaderObj, GL_COMPILE_STATUS, &status );
    if ( status == GL_FALSE )
    {
        GLint maxLen;
	      glGetShaderiv( shaderObj, GL_INFO_LOG_LENGTH, &maxLen );
        std::vector< char >log( maxLen );
		    GLsizei len;
		    glGetShaderInfoLog( shaderObj, maxLen, &len, log.data() );
		    std::cout << "compile error:" << std::endl << log.data() << std::endl;
        throw std::runtime_error( "compile error" );
    }
    return shaderObj;
  }
    
  // linke shader objects to shader program
  void LinkProgram( const std::vector<int> & shaderObjs ) {
    glLinkProgram( Prog() );
    GLint status;
    glGetProgramiv( Prog(), GL_LINK_STATUS, &status );
    if ( status == GL_FALSE )
    {
        GLint maxLen;
	      glGetProgramiv( Prog(), GL_INFO_LOG_LENGTH, &maxLen );
        std::vector< char >log( maxLen );
		    GLsizei len;
		    glGetProgramInfoLog( Prog(), maxLen, &len, log.data() );
		    std::cout  << "link error:" << std::endl << log.data() << std::endl;
        throw std::runtime_error( "link error" );
    }
  }

  // Get program resources
  void GetResources()
  {
    // Program Introspection [https://www.khronos.org/opengl/wiki/Program_Introspection]

    // glGetProgramResourceiv            [https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetProgramResource.xhtml]
    // glGetProgramResourceName          [https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetProgramResourceName.xhtml]
    // glGetProgramResourceIndex         [https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetProgramResourceIndex.xhtml]
    // glGetProgramResourceLocation      [https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetProgramResourceLocation.xhtml]
    // glGetProgramResourceLocationIndex [https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetProgramResourceLocationIndex.xhtml]

    if ( _verbose )
      std::cout << std::endl;

    GetAttributes();
    GetTransformFeedbackVaryings();
    GetFragmentOutputs();
    GetUniforms();
    GetUniformBlocks();
    GetSubroutines();

    // progSize = glGetProgramiv( self.__prog, GL_PROGRAM_BINARY_LENGTH )

    if ( _verbose )
      std::cout << std::endl;
  }

  enum class TResourceKind { NON, location };
  // Get program interface resources
  void GetInterfaceResources( GLenum prog_interface, TResourceMap &resources, TResourceKind kind ) const
  {
    GLint no_of, max_len;
		glGetProgramInterfaceiv( Prog(), prog_interface, GL_ACTIVE_RESOURCES, &no_of ); // OpenGL 4.3
    glGetProgramInterfaceiv( Prog(), prog_interface, GL_MAX_NAME_LENGTH, &max_len ); // OpenGL 4.3

		std::vector< GLchar >name( max_len );
		for( int i_resource = 0; i_resource < no_of; i_resource++ )
		{
      GLsizei strLength;
      glGetProgramResourceName( Prog(), prog_interface, i_resource, max_len, &strLength, name.data() );
      switch( kind )
      {
      default:
      case TResourceKind::NON:
        resources[name.data()] = -1;
        break;

      case TResourceKind::location: 
			  resources[name.data()] = glGetProgramResourceLocation( Prog(), prog_interface, name.data() ); // OpenGL 4.3
        break;
      }
      //resources[name.data()] = glGetProgramResourceIndex( Prog(), prog_interface, name.data() ); // OpenGL 4.3
      //resources[name.data()] = glGetProgramResourceLocationIndex( Prog(), prog_interface, name.data() ); // OpenGL 4.3
		}
  }

  // Get program attribute indices
  void GetAttributes( void )
  {
#if defined(OpenGL_program_introspection_old)
    GLint maxAttribLen, nAttribs;
		glGetProgramiv( Prog(), GL_ACTIVE_ATTRIBUTES, &nAttribs );
		glGetProgramiv( Prog(), GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxAttribLen );

		std::vector< GLchar >name( maxAttribLen );
		for( int attribInx = 0; attribInx < nAttribs; attribInx++ )
		{
      GLint written, size;
		  GLenum type;
			glGetActiveAttrib( Prog(), attribInx, maxAttribLen, &written, &size, &type, name.data() );
			_attributeIndices[name.data()] = glGetAttribLocation( Prog(), name.data() );
		}
#else
    GetInterfaceResources( GL_PROGRAM_INPUT, _attributeIndices, TResourceKind::location );
#endif

    if ( _verbose == false )
      return;
    for ( auto & resource : _attributeIndices )
      std::cout << "attribute index " << resource.second << ": " << resource.first << std::endl;
  }

  // Get transform feedvback varyings
  void GetTransformFeedbackVaryings(void)
  {
    GetInterfaceResources( GL_TRANSFORM_FEEDBACK_VARYING, _transformFeedbackVaryings, TResourceKind::NON );

    if ( _verbose == false )
      return;
    for ( auto & resource : _transformFeedbackVaryings )
      std::cout << "transform feedback varyings " << resource.second << ": " << resource.first << std::endl;
  }

  // Get fragmetn outputs
  void GetFragmentOutputs( void )
  {
    // Fragment Outputs [https://www.khronos.org/opengl/wiki/Program_Introspection#Fragment_Outputs]
#if defined(OpenGL_program_introspection_old)
    // int GetFragDataLocation( uint program, const char *name )
    // int GetFragDataIndex( uint program, const char *name )
#else
    GetInterfaceResources( GL_PROGRAM_OUTPUT, _fragOutputLocation, TResourceKind::location );
#endif

    if ( _verbose == false )
      return;
    for ( auto & resource : _fragOutputLocation )
      std::cout << "fragment output location " << resource.second << ": " << resource.first << std::endl;
  }

  //Get unifrom locations
  void GetUniforms( void )
  {
#if defined(OpenGL_program_introspection_old)
    GLint maxUniformLen, nUniforms;
		glGetProgramiv( Prog(), GL_ACTIVE_UNIFORMS, &nUniforms );
		glGetProgramiv( Prog(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformLen );

    std::vector< GLchar >name( maxUniformLen );
    for( int uniformInx = 0; uniformInx < nUniforms; uniformInx++ )
    {
      GLint written, size;
		  GLenum type;
      glGetActiveUniform( Prog(), uniformInx, maxUniformLen, &written, &size, &type, name.data() );
      _unifomLocation[name.data()] = glGetUniformLocation( Prog(), name.data() ); 
    }
#else
    GetInterfaceResources( GL_UNIFORM, _unifomLocation, TResourceKind::location );
#endif
    
    if ( _verbose == false )
      return;
    for ( auto & resource : _unifomLocation )
      std::cout << "uniform location " << resource.second << ": " << resource.first << std::endl;
  }

  void GetUniformBlocks( void )
  {
    /*
    GLint nUniformBlocks;
    glGetProgramiv( Object(), GL_ACTIVE_UNIFORM_BLOCKS, &nUniformBlocks );
		
    if ( nUniformBlocks > 0 )
    {					  
			GLint bufferSize = 0;
			glGetProgramiv( Object(), GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &bufferSize );
      bufferSize = cutil::Max( bufferSize, 256 );

      std::vector< GLchar >name( bufferSize );
      for ( int sourceInx = 0; sourceInx < nUniformBlocks; sourceInx++ )
      {
        glGetActiveUniformBlockName( Object(), sourceInx, bufferSize, &size, name.data() );
        GLuint uniformBlockInx = glGetUniformBlockIndex( Object(), name.data() );
        ASSERT( sourceInx == (int)uniformBlockInx );

				// do not change inactive uniform block (just in case)
				if (uniformBlockInx == GL_INVALID_INDEX)
					continue;
        
        TUniformBlockOffsets &offsets = m_uniformBlockMap[name.data()];
        offsets.m_index             = sourceInx;
        offsets.m_uniformBlockIndex = uniformBlockInx;
        glGetActiveUniformBlockiv( Object(), uniformBlockInx, GL_UNIFORM_BLOCK_DATA_SIZE, &offsets.m_bufferSize );

        GLib().EvaluateGlError( s_subComponentName );

        GLint numberOfUniforms;
        glGetActiveUniformBlockiv( Object(), uniformBlockInx, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numberOfUniforms );
        offsets.m_indices.swap( std::vector< GLuint >( numberOfUniforms, 0 ) );
        glGetActiveUniformBlockiv( Object(), uniformBlockInx, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, (GLint*)offsets.m_indices.data() );
        offsets.m_offsets.swap( std::vector< GLint >( numberOfUniforms, 0 ) );
        glGetActiveUniformsiv( Object(), numberOfUniforms, offsets.m_indices.data(), GL_UNIFORM_OFFSET, offsets.m_offsets.data() );
          
        for ( size_t inx = 0; inx < offsets.m_indices.size(); inx ++ )
        {
          std::vector< GLchar >name( maxUniformLen+1 );
          glGetActiveUniform( Object(), offsets.m_indices[inx], maxUniformLen, &written, &size, &type, name.data() );
          name[maxUniformLen] = 0;
          offsets.m_nameMap[name.data()] = inx;
        }

        // glGetActiveUniformBlockiv
        // GL_UNIFORM_BLOCK_BINDING
        // GL_UNIFORM_BLOCK_DATA_SIZE
        // GL_UNIFORM_BLOCK_NAME_LENGTH
        // GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER
        // GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER
        // GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER
        // GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER
        // GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER
        // GL_UNIFORM_BLOCK_REFERENCED_BY_COMPUTE_SHADER

        int bindingPoint = TUniformBlockLinkHelper().LinkLoop( name.data(), uniformBlockInx, m_usedUniformBlocks, m_ubIndizes, offsets );
        if ( bindingPoint >= 0 )
          LinkUniformBlock( uniformBlockInx, bindingPoint );
      }
    }
    */
  }

  void GetSubroutines( void )
  {
    /*
    for ( int typeInx = 0; typeInx < eShader_NO_OF_TYPES; typeInx ++ )
		{
      if ( _gl.IsShaderTypeValid( (TShaderType)typeInx ) == false )
        continue;

			GLenum shaderType = CShaderObject::MapShaderType( (TShaderType)typeInx );
			
			GLint nSubroutineUniforms, maxSubroutineUniformLen;
			glGetProgramStageiv( Object(), shaderType, GL_ACTIVE_SUBROUTINE_UNIFORMS, &nSubroutineUniforms );
			glGetProgramStageiv( Object(), shaderType, GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS, &m_locationCount[typeInx] );
			glGetProgramStageiv( Object(), shaderType, GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH, &maxSubroutineUniformLen );
			GLib().EvaluateGlError( s_subComponentName );

			if ( maxSubroutineUniformLen > 0 &&  nSubroutineUniforms > 0 )
			{
				std::vector< GLchar >name( maxSubroutineUniformLen+1 );
				for( int subroutineUniformInx = 0; subroutineUniformInx < nSubroutineUniforms; subroutineUniformInx++ )
				{
					TSubroutineUniform subroutine;
					GLint nCompatible;
					glGetActiveSubroutineUniformiv( Object(), shaderType, subroutineUniformInx, GL_NUM_COMPATIBLE_SUBROUTINES, &nCompatible );
					subroutine.m_compatible.swap( std::vector< GLint >( nCompatible ) );
					glGetActiveSubroutineUniformiv( Object(), shaderType, subroutineUniformInx, GL_COMPATIBLE_SUBROUTINES, subroutine.m_compatible.data() );
					glGetActiveSubroutineUniformiv( Object(), shaderType, subroutineUniformInx, GL_UNIFORM_SIZE, &subroutine.m_arraySize );
					GLint nameLen;
					glGetActiveSubroutineUniformiv( Object(), shaderType, subroutineUniformInx, GL_UNIFORM_NAME_LENGTH, &nameLen );
					glGetActiveSubroutineUniformName( Object(), shaderType, subroutineUniformInx, maxSubroutineUniformLen + 1, &nameLen, name.data() );
					subroutine.m_location = glGetSubroutineUniformLocation( Object(), shaderType, name.data() );
					subroutine.m_name = name.data();
					m_subroutineUniform[typeInx][subroutine.m_location] = subroutine;
				}
			}

      GLib().EvaluateGlError( s_subComponentName );

			GLint nSubroutine, maxSubroutineLen;
			glGetProgramStageiv( Object(), shaderType, GL_ACTIVE_SUBROUTINES, &nSubroutine );
			glGetProgramStageiv( Object(), shaderType, GL_ACTIVE_SUBROUTINE_MAX_LENGTH, &maxSubroutineLen );
			
			m_subroutineName[typeInx].clear();
			if ( maxSubroutineLen > 0 && nSubroutine > 0)
			{
				TNameList subroutineList;
				std::vector< GLchar >name( maxSubroutineLen+1 );
				for ( int subroutineInx = 0; subroutineInx < nSubroutine; subroutineInx++ )
				{
					GLsizei lenght;
					glGetActiveSubroutineName( Object(), shaderType, subroutineInx, maxSubroutineLen, &lenght, name.data() );
					m_subroutineName[typeInx].push_back( name.data() );
				}
			}
		}

    glGetProgramResourceLocation
    GL_VERTEX_SUBROUTINE_UNIFORM,
    GL_TESS_CONTROL_SUBROUTINE_UNIFORM,
    GL_TESS_EVALUATION_SUBROUTINE_UNIFORM,
    GL_GEOMETRY_SUBROUTINE_UNIFORM,
    GL_FRAGMENT_SUBROUTINE_UNIFORM,
    GL_COMPUTE_SUBROUTINE_UNIFORM
    */
  }

};

} // OpenGL 
