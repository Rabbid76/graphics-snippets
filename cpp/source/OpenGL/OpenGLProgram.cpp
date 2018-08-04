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

#include <GL/glew.h>
//#include <GL/gl.h> not necessary because of glew 
#include <GL/glu.h>


// stl

#include <algorithm>
#include <unordered_map>
#include <cassert>
#include <sstream>


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
* \brief dotr   
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
  CShaderObject && source_objet ) //!< soure object
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
* \brief Get the OpenGL enum constant for the shader type. 
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
* \brief Get the shader type from the OpenGL enum constant. 
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
* \brief Get the a user friendl name for the shader type. 
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
  
  // create the sahder object
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

  assert( false );
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
* \brief dotr   
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
  CShaderProgram && source_objet ) //!< soure object
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

  // set addtional resource bindings
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
Render::Program::IIntrospection && CShaderProgram::Introspection( 
  Render::Program::TResourceTypes resources ) //! I - set of resource types
{
  // TODO $$$
  return std::move( Render::Program::IIntrospection() );
}


} // OpenGL

