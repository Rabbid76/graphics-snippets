/******************************************************************//**
* \brief Implementation of OpenGL shader programs.
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/#

#include <stdafx.h>


// includes

#include <OpenGLError.h>
#include <OpenGLProgram.h>


// OpenGL wrapper

#include <OpenGL_include.h>


// STL

#include <algorithm>
#include <unordered_map>
#include <cassert>
#include <sstream>
#include <iostream>


// class implementations


namespace OpenGL
{


//*********************************************************************
// CShaderObject
//*********************************************************************


const std::vector<std::tuple<Render::Program::TShaderType, size_t>> CShaderObject::c_type_map
{
  { Render::Program::TShaderType::vertex,          GL_VERTEX_SHADER },
  { Render::Program::TShaderType::tess_control,    GL_TESS_CONTROL_SHADER },
  { Render::Program::TShaderType::tess_evaluation, GL_TESS_EVALUATION_SHADER },
  { Render::Program::TShaderType::geometry,        GL_GEOMETRY_SHADER },
  { Render::Program::TShaderType::fragment,        GL_FRAGMENT_SHADER },
  { Render::Program::TShaderType::compute,         GL_COMPUTE_SHADER }
};


/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-08-02
* \version 1.0
**********************************************************************/
CShaderObject::CShaderObject( 
  Render::Program::TShaderType type ) //!< I - shader type
  : _type( type )
{}
  

/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-08-02
* \version 1.0
**********************************************************************/
CShaderObject::CShaderObject(
  CShaderObject && source_objet ) //!< I - source object
{
  *this = std::move( source_objet );
}
 

/******************************************************************//**
* \brief dtor   
* 
* \author  gernot
* \date    2018-08-02
* \version 1.0
**********************************************************************/
CShaderObject::~CShaderObject()
{
  if ( _object != 0 )
    glDeleteShader( (GLuint)_object );
}


/******************************************************************//**
* \brief Move operator  
* 
* \author  gernot
* \date    2018-08-02
* \version 1.0
**********************************************************************/
CShaderObject & CShaderObject::operator =( 
  CShaderObject && source_objet ) //!< source object
{
  _type   = source_objet._type;
  _code   = std::move( source_objet._code );
  _object = source_objet._object;
  source_objet._object = 0;

  return *this;
}


/******************************************************************//**
* \brief Append source code to the shader.    
* 
* \author  gernot
* \date    2018-08-02
* \version 1.0
**********************************************************************/
CShaderObject & CShaderObject::operator << ( 
    const std::string & code ) //!< I - source code
{
  _code.emplace_back( code );
  return *this;
}


/******************************************************************//**
* \brief Append source code to the shader.    
* 
* \author  gernot
* \date    2018-08-02
* \version 1.0
**********************************************************************/
CShaderObject & CShaderObject::operator << ( 
  std::string && code ) //!< I - source code
{
  _code.emplace_back( std::move( code ) );
  return *this;
}


/******************************************************************//**
* \brief Clear internal source code.    
* 
* \author  gernot
* \date    2018-08-02
* \version 1.0
**********************************************************************/
CShaderObject & CShaderObject::ClearCode( void )
{
  _code.clear();
  return *this;
}
  

/******************************************************************//**
* \brief Get the OpenGL enumerator constant for the shader type. 
* 
* \author  gernot
* \date    2018-08-02
* \version 1.0
**********************************************************************/
size_t CShaderObject::ShaderEnum( 
  Render::Program::TShaderType type )  //!< I - shader type
{
  auto it = std::find_if( c_type_map.begin(), c_type_map.end(), [type]( auto &t ) -> bool
  {
    return std::get<0>(t) == type;
  } );
  assert( it != c_type_map.end() );
  return it != c_type_map.end() ? std::get<1>(*it ) : GL_VERTEX_SHADER;
}


/******************************************************************//**
* \brief Get the shader type from the OpenGL enumerator constant. 
* 
* \author  gernot
* \date    2018-08-02
* \version 1.0
**********************************************************************/
Render::Program::TShaderType CShaderObject::ShaderType( 
  size_t stage )  //!< I - shader stage
{
  auto it = std::find_if( c_type_map.begin(), c_type_map.end(), [stage]( auto &t ) -> bool
  {
    return std::get<1>(t) == stage;
  } );
  assert( it != c_type_map.end() );
  return it != c_type_map.end() ? std::get<0>(*it ) : Render::Program::TShaderType::vertex;
}


/******************************************************************//**
* \brief Get the a user friendly name for the shader type. 
* 
* \author  gernot
* \date    2018-08-02
* \version 1.0
**********************************************************************/
const char *CShaderObject:: ShaderTypeName( 
  Render::Program::TShaderType type )   //!< I - shader type
{
  const std::unordered_map<Render::Program::TShaderType, const char*> c_name_map
  {
    { Render::Program::TShaderType::vertex,          "vertex" },
    { Render::Program::TShaderType::tess_control,    "tessellation control" },
    { Render::Program::TShaderType::tess_evaluation, "tessellation evaluation" },
    { Render::Program::TShaderType::geometry,        "geometry" },
    { Render::Program::TShaderType::fragment,        "fragment" },
    { Render::Program::TShaderType::compute,         "compute" }
  };

  auto it = c_name_map.find( type );
  assert( it != c_name_map.end() );
  return it != c_name_map.end() ? it->second : "";
}


/******************************************************************//**
* \brief Compile the shader stage, the function succeeds, even
* if the compilation fails, but it fails if the stage was not properly
* initialized.
* 
* \author  gernot
* \date    2018-08-02
* \version 1.0
**********************************************************************/
bool CShaderObject::Compile( void )
{
  if ( _object != 0 )
    glDeleteShader( (GLuint)_object );
  
  // create the shader object
  GLenum stage = (GLenum)ShaderEnum( _type );
  _object = glCreateShader( stage );
  
  // append the source code to the shader object
  std::vector<const char *> code_ptr( _code.size() );
  std::transform( _code.begin(), _code.end(), code_ptr.begin(), []( auto &str ) -> const char * { return str.c_str(); } );
  glShaderSource( (GLuint)_object, (GLsizei)code_ptr.size(), code_ptr.data(), nullptr );
  
  // compile the shader
  glCompileShader( (GLuint)_object );

  return true;
}

  
/******************************************************************//**
* \brief Verifies the compilation result.    
* 
* \author  gernot
* \date    2018-08-02
* \version 1.0
**********************************************************************/
bool CShaderObject::Verify( 
  std::string &message ) //! O - error messages
{
  message.clear();
  if ( _object == 0 )
    return false;

  GLint status = GL_TRUE;
  glGetShaderiv( _object, GL_COMPILE_STATUS, &status );
  if ( status != GL_FALSE )
    return true;
  
  GLint maxLen;
	glGetShaderiv( _object, GL_INFO_LOG_LENGTH, &maxLen );
  std::vector< char >log( maxLen );
	GLsizei len;
	glGetShaderInfoLog( _object, maxLen, &len, log.data() );
  
  std::stringstream str_stream;
  str_stream << "compile error:" << std::endl << log.data() << std::endl;
  message = str_stream.str();

  //assert( false );
  return false;
}


//*********************************************************************
// CShaderProgram
//*********************************************************************


/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-08-03
* \version 1.0
**********************************************************************/
CShaderProgram::CShaderProgram( void )
{}


/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-08-03
* \version 1.0
**********************************************************************/
CShaderProgram::CShaderProgram(
  CShaderProgram && source_objet ) //!< I - source object
{
  *this = std::move( source_objet );
}


/******************************************************************//**
* \brief dtor   
* 
* \author  gernot
* \date    2018-08-03
* \version 1.0
**********************************************************************/
CShaderProgram::~CShaderProgram()
{
  if ( _object != 0 )
    glDeleteProgram( (GLuint)_object );
}


/******************************************************************//**
* \brief Move operator  
* 
* \author  gernot
* \date    2018-08-03
* \version 1.0
**********************************************************************/
CShaderProgram & CShaderProgram::operator =( 
  CShaderProgram && source_objet ) //!< source object
{
  _shaders = std::move( source_objet._shaders );
  _object  = source_objet._object;
  source_objet._object = 0;

  return *this;
}


/******************************************************************//**
* \brief Link the program, the function succeeds, even
* if the linking fails, but it fails if the program was not 
* properly initialized.  
* 
* \author  gernot
* \date    2018-08-03
* \version 1.0
**********************************************************************/
bool CShaderProgram::Link( void )
{
  if ( _object != 0 )
    glDeleteProgram( (GLuint)_object );

  // create the program object
  _object = glCreateProgram();
  GLuint obj = (GLuint)_object;
  
  // attach shader objects 
  for ( auto shader : _shaders )
  {
    size_t shader_objbect = shader != nullptr ? shader->ObjectHandle() : 0;
    if ( shader_objbect == 0 )
      continue;
    glAttachShader( obj, (GLuint)shader_objbect );
  }

  // set additional resource bindings
  std::vector<const char*> tf_varying_ptrs;
  for ( auto & resource : _resource_binding )
  {
    std::string                    &name    = std::get<0>( resource );
    Render::Program::TResourceType &type    = std::get<1>( resource );
    size_t                         &binding = std::get<2>( resource );
    
    switch( type )
    {
      default: assert( false ); break;  
      case Render::Program::TResourceType::attribute:          glBindAttribLocation( obj, (GLuint)binding, name.c_str() ); break;
      case Render::Program::TResourceType::fragment_data:      glBindFragDataLocation( obj, (GLuint)binding, name.c_str() ); break;
      case Render::Program::TResourceType::transform_feedback: tf_varying_ptrs.push_back( name.c_str() ); break;
    }
  }
  if ( _transform_feedback_mode != TTranformFeedbackMode::NON )
  {
    GLenum mode = _transform_feedback_mode == TTranformFeedbackMode::interleaved ? GL_INTERLEAVED_ATTRIBS : GL_SEPARATE_ATTRIBS;
    glTransformFeedbackVaryings( obj, (GLsizei)tf_varying_ptrs.size(), tf_varying_ptrs.data(), mode );
  }


  // link the program
  glLinkProgram( obj );
  return true;
}
  

/******************************************************************//**
* \brief Verifies the linking result.  
* 
* \author  gernot
* \date    2018-08-03
* \version 1.0
**********************************************************************/
bool CShaderProgram::CShaderProgram::Verify( 
  std::string &message ) //!< O - error messages
{
  message.clear();
  if ( _object == 0 )
    return false;

  GLint status = GL_TRUE;
  glGetProgramiv( _object, GL_LINK_STATUS, &status );
  if ( status != GL_FALSE )
    return true;
  
  GLint maxLen;
	glGetProgramiv( _object, GL_INFO_LOG_LENGTH, &maxLen );
  std::vector< char >log( maxLen );
	GLsizei len;
	glGetProgramInfoLog( _object, maxLen, &len, log.data() );
  
  std::stringstream str_stream;
  str_stream << "link error:" << std::endl << log.data() << std::endl;
  message = str_stream.str();

  assert( false );
  return false;
}


/******************************************************************//**
* \brief Activate a program.  
* 
* \author  gernot
* \date    2018-08-03
* \version 1.0
**********************************************************************/
bool CShaderProgram::Use( void )
{
  if ( _object == 0 )
    return false;
  glUseProgram( _object );
  return true;
}

 
/******************************************************************//**
* \brief Ask for the resource information of the program.  
* 
* \author  gernot
* \date    2018-08-03
* \version 1.0
**********************************************************************/
Render::Program::TIntrospectionPtr CShaderProgram::Introspection( 
  Render::Program::TResourceTypes resources, //!< I - set of resource types
  bool                            verbose )  //!< I - trace active resources
{
  auto introspection = std::make_shared<CIntrospection>( *this );
  introspection->Verbose( verbose ).Observe( resources );
  return introspection;
}


//*********************************************************************
// CShaderProgram
//*********************************************************************


/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-08-05
* \version 1.0
**********************************************************************/
CIntrospection::CIntrospection( 
  const IProgram & program ) //!< program object
  : _program( program )
{}
  

/******************************************************************//**
* \brief dtor  
* 
* \author  gernot
* \date    2018-08-05
* \version 1.0
**********************************************************************/
CIntrospection::~CIntrospection()
{}

  
/******************************************************************//**
* \brief Observe the program.   
* 
* \author  gernot
* \date    2018-08-05
* \version 1.0
**********************************************************************/
CIntrospection & CIntrospection::Observe(
  Render::Program::TResourceTypes resources ) //!< resource types to be observed
{
  // Program Introspection [https://www.khronos.org/opengl/wiki/Program_Introspection]

  // glGetProgramResourceiv            [https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetProgramResource.xhtml]
  // glGetProgramResourceName          [https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetProgramResourceName.xhtml]
  // glGetProgramResourceIndex         [https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetProgramResourceIndex.xhtml]
  // glGetProgramResourceLocation      [https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetProgramResourceLocation.xhtml]
  // glGetProgramResourceLocationIndex [https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetProgramResourceLocationIndex.xhtml]

  // progSize = glGetProgramiv( self.__prog, GL_PROGRAM_BINARY_LENGTH )

  // TODO $$$ interpret Render::Program::TProgramType

  // get active attributes
  if ( resources.test((int)TResourceType::attribute) )
  {
    GetAttributes();
    AddResources( _attributeIndices, TResourceType::attribute );
  }

  // get active transform feedback varyings
  if ( resources.test((int)TResourceType::transform_feedback) )
  {
    GetTransformFeedbackVaryings();
    AddResources( _transformFeedbackVaryings, TResourceType::transform_feedback );
  }
  
  // get active fragment data
  if ( resources.test((int)TResourceType::fragment_data) )
  {
    GetFragmentData();
    AddResources( _fragDataLocation, TResourceType::fragment_data );
  }  

  // get active uniforms
  if ( resources.test((int)TResourceType::uniform) )
  {
    GetUniforms();
    AddResources( _unifomLocation, TResourceType::uniform );
  }
  
  // get active uniform blocks
  if ( resources.test((int)TResourceType::uniform_block) )
    GetUniformBlocks();

  // TODO $$$ SSBO
  
  // get active subroutine uniforms
  if ( resources.test((int)TResourceType::subroutine_uniform) )
    GetSubroutines();


  // TODO $$$

  return *this;
}


/******************************************************************//**
* \brief Get program interface resources.  
* 
* \author  gernot
* \date    2018-08-05
* \version 1.0
**********************************************************************/
void CIntrospection::GetInterfaceResources( 
  unsigned int  prog,           //!< I - program object
  unsigned int  prog_interface, //!< I - interface name
  TResourceMap &resources,      //!< O - resource map
  TResourceKind kind            //!< I - kind of binding (location or NON)
) const
{
  if ( prog == 0 )
    return;

  GLint no_of, max_len;
	glGetProgramInterfaceiv( prog, prog_interface, GL_ACTIVE_RESOURCES, &no_of ); // OpenGL 4.3
  glGetProgramInterfaceiv( prog, prog_interface, GL_MAX_NAME_LENGTH, &max_len ); // OpenGL 4.3

	std::vector< GLchar >name( max_len );
	for( int i_resource = 0; i_resource < no_of; i_resource++ )
	{
    GLsizei strLength;
    glGetProgramResourceName( prog, prog_interface, i_resource, max_len, &strLength, name.data() );
    switch( kind )
    {
    default:
    case TResourceKind::NON:
      resources[name.data()] = -1;
      break;

    case TResourceKind::location: 
			resources[name.data()] = glGetProgramResourceLocation( prog, prog_interface, name.data() ); // OpenGL 4.3
      break;
    }
    //resources[name.data()] = glGetProgramResourceIndex( prog, prog_interface, name.data() ); // OpenGL 4.3
    //resources[name.data()] = glGetProgramResourceLocationIndex( prog, prog_interface, name.data() ); // OpenGL 4.3
	}
}


/******************************************************************//**
* \brief Add new resources to common resource map.   
* 
* \author  gernot
* \date    2018-08-05
* \version 1.0
**********************************************************************/
void CIntrospection::AddResources( 
  const TResourceMap &resources, //! I - new resources
  TResourceType       type )     //! I - type of resources
{
  for ( auto & attribute : resources )
  {
    auto it = _resources.find( attribute.first );
    if ( it != _resources.end() )
    {
      bool ok = std::get<1>(it->second) == Render::Program::TResourceType::transform_feedback &&
                type == Render::Program::TResourceType::fragment_data;
      assert( ok );
      continue;
    }
    _resources[attribute.first] = std::make_tuple( attribute.second, type );
  }
}


/******************************************************************//**
* \brief Get program attribute indices.  
* 
* \author  gernot
* \date    2018-08-05
* \version 1.0
**********************************************************************/
void CIntrospection::GetAttributes( void )
{
  size_t program_hdl = _program.ObjectHandle();
  if ( program_hdl == 0 )
    return;
  GLuint prog = (GLuint)program_hdl;

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
  GetInterfaceResources( prog, GL_PROGRAM_INPUT, _attributeIndices, TResourceKind::location );
#endif

  if ( _verbose == false )
    return;
  for ( auto & resource : _attributeIndices )
    std::cout << "attribute index " << resource.second << ": " << resource.first << std::endl;
}


/******************************************************************//**
* \brief Get transform feedback varyings.  
* 
* \author  gernot
* \date    2018-08-05
* \version 1.0
**********************************************************************/
void CIntrospection::GetTransformFeedbackVaryings(void)
{
  size_t program_hdl = _program.ObjectHandle();
  if ( program_hdl == 0 )
    return;
  GLuint prog = (GLuint)program_hdl;

  GetInterfaceResources( prog, GL_TRANSFORM_FEEDBACK_VARYING, _transformFeedbackVaryings, TResourceKind::NON );

  if ( _verbose == false )
    return;
  for ( auto & resource : _transformFeedbackVaryings )
    std::cout << "transform feedback varyings " << resource.second << ": " << resource.first << std::endl;
}


/******************************************************************//**
* \brief Get fragment shader outputs.  
* 
* \author  gernot
* \date    2018-08-05
* \version 1.0
**********************************************************************/
void CIntrospection::GetFragmentData( void )
{
  size_t program_hdl = _program.ObjectHandle();
  if ( program_hdl == 0 )
    return;
  GLuint prog = (GLuint)program_hdl;

  // Fragment Outputs [https://www.khronos.org/opengl/wiki/Program_Introspection#Fragment_Outputs]
#if defined(OpenGL_program_introspection_old)
  // int GetFragDataLocation( uint program, const char *name )
  // int GetFragDataIndex( uint program, const char *name )
#else
  GetInterfaceResources( prog, GL_PROGRAM_OUTPUT, _fragDataLocation, TResourceKind::location );
#endif

  if ( _verbose == false )
    return;
  for ( auto & resource : _fragDataLocation )
    std::cout << "fragment output location " << resource.second << ": " << resource.first << std::endl;
}


/******************************************************************//**
* \brief Get uniform locations.  
* 
* \author  gernot
* \date    2018-08-05
* \version 1.0
**********************************************************************/
void CIntrospection::GetUniforms( void )
{
  size_t program_hdl = _program.ObjectHandle();
  if ( program_hdl == 0 )
    return;
  GLuint prog = (GLuint)program_hdl;

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
  GetInterfaceResources( prog, GL_UNIFORM, _unifomLocation, TResourceKind::location );
#endif
    
  if ( _verbose == false )
    return;
  for ( auto & resource : _unifomLocation )
    std::cout << "uniform location " << resource.second << ": " << resource.first << std::endl;
}


/******************************************************************//**
* \brief Get active program uniform block binding points.  
* 
* \author  gernot
* \date    2018-08-05
* \version 1.0
**********************************************************************/
void CIntrospection::GetUniformBlocks( void )
{
  size_t program_hdl = _program.ObjectHandle();
  if ( program_hdl == 0 )
    return;
  GLuint prog = (GLuint)program_hdl;

  GLint nUniformBlocks;
  glGetProgramiv( prog, GL_ACTIVE_UNIFORM_BLOCKS, &nUniformBlocks );
		
  if ( nUniformBlocks > 0 )
  {					  
		GLint bufferSize = 0;
		glGetProgramiv( prog, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &bufferSize );
    bufferSize =std:: max( bufferSize, 256 );

    std::vector< GLchar >name( bufferSize );
    for ( int sourceInx = 0; sourceInx < nUniformBlocks; sourceInx++ )
    {
      GLsizei size;
      glGetActiveUniformBlockName( prog, sourceInx, bufferSize, &size, name.data() );
      GLuint uniformBlockInx = glGetUniformBlockIndex( prog, name.data() );
        
			// do not change inactive uniform block (just in case)
			if (uniformBlockInx == GL_INVALID_INDEX)
				continue;
        
      GLint m_bufferSize;
      glGetActiveUniformBlockiv( prog, uniformBlockInx, GL_UNIFORM_BLOCK_DATA_SIZE, &m_bufferSize );

      GLint numberOfUniforms;
      glGetActiveUniformBlockiv( prog, uniformBlockInx, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numberOfUniforms );
      std::vector< GLuint > indices( numberOfUniforms );
      glGetActiveUniformBlockiv( prog, uniformBlockInx, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, (GLint*)indices.data() );
      std::vector< GLuint > offsets( numberOfUniforms );
      glGetActiveUniformsiv( prog, numberOfUniforms, indices.data(), GL_UNIFORM_OFFSET, (GLint*)offsets.data() );
        
        
      std::vector<std::string> names;
      for ( size_t inx = 0; inx < indices.size(); inx ++ )
      {
        std::vector< GLchar >name( bufferSize );
        GLsizei written;
        GLenum type;
        glGetActiveUniform( prog, indices[inx], bufferSize, &written, &size, &type, name.data() );
        name[bufferSize-1] = 0;
        names.push_back( name.data() );
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
    }
  }  
}


/******************************************************************//**
* \brief Get program subroutine uniforms.  
* 
* \author  gernot
* \date    2018-08-05
* \version 1.0
**********************************************************************/
void CIntrospection::GetSubroutines( void )
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
				GLsizei length;
				glGetActiveSubroutineName( Object(), shaderType, subroutineInx, maxSubroutineLen, &length, name.data() );
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

} // OpenGL

