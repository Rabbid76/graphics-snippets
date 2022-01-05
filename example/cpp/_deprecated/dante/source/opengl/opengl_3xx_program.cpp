/***********************************************************************************************//**
* \brief Base class implementation for OpenGL ES 3.x and (desktop) OpenGL 3.2+ texture program.
*
* \author  Rabbid76    \date  2019-03-09
***************************************************************************************************/

#include <stdafx.h>


// include

#include <OpenGL_3xx_program.h>
#include <OpenGL_include.h>

// STL

#include <map>
#include <iostream>


namespace Program
{


namespace OpenGL
{


namespace GL3xx
{


/***********************************************************************************************//**
* \brief ctor  
*
* \author  gernot Rabbid76    \date  2019-03-09
***************************************************************************************************/
CProgram::CProgram( void )
{}


/***********************************************************************************************//**
* \brief dtor  
*
* \author  gernot Rabbid76    \date  2019-03-09
***************************************************************************************************/
CProgram::~CProgram()
{}


/***********************************************************************************************//**
* \brief Generate the program  
*
* \author  gernot Rabbid76    \date  2019-03-09
***************************************************************************************************/
CProgram & CProgram::Generate( 
    const TStages &stages ) //! in: shader stages source code  
{
    std::transform( stages.begin(), stages.end(), std::back_inserter( _shader_objs ), [&]( const TStage & stage ) -> int
    {
      return GenerateShader( std::get<0>( stage ), std::get<1>( stage ) );
    } );
    _program_obj = GenerateProgram( _shader_objs );

    // prepare deletion of shader objects
    for (auto shader_obj : _shader_objs)
        glDeleteShader( shader_obj );

    return *this;
}


/***********************************************************************************************//**
* \brief Create a shader object, attach the program code and compile.   
*
* \author  gernot Rabbid76    \date  2019-03-09
***************************************************************************************************/
unsigned int CProgram::GenerateShader( 
    const std::string & source, //! in: shader source code 
    unsigned int        stage   //! in: OpenGL stage enumerator constant
    ) const
{
    // stage for stage evaluation and logging
    static const std::map<int, std::string> stages
    { 
          { GL_VERTEX_SHADER,          "vertex" },
          { GL_TESS_CONTROL_SHADER,    "tessellation control" },
          { GL_TESS_EVALUATION_SHADER, "tessellation evaluation" },
          { GL_GEOMETRY_SHADER,        "geometry" },
          { GL_FRAGMENT_SHADER,        "fragment" },
          { GL_COMPUTE_SHADER,         "compute" }
    };

    auto it = stages.find( stage );
    if (it == stages.end())
        return 0;

    // source code logging
    if ( _verbose )
      std::cout << it->second.data() << "shader code:" << std::endl << source << std::endl << std::endl;

    // create object attach source code and compile
    const char * source_ptr = source.c_str();
    auto shader_obj = glCreateShader( stage );
    glShaderSource( shader_obj, 1, &source_ptr, nullptr );
    glCompileShader( shader_obj );

    // evaluate compile result
    GLint status; 
    glGetShaderiv( shader_obj, GL_COMPILE_STATUS, &status );
    if ( status )
        return shader_obj;

    // error logging
    if ( _error_log )
    {
        GLint max_len;
	    glGetShaderiv( shader_obj, GL_INFO_LOG_LENGTH, &max_len );
        std::vector< char >log( max_len );
		GLsizei len;
		glGetShaderInfoLog( shader_obj, max_len, &len, log.data() );
		std::cout << "compile error:" << std::endl << log.data() << std::endl;
    }
    
    // delete invalid object
    glDeleteShader( shader_obj );

    return 0;
}


/***********************************************************************************************//**
* \brief Generate program object attach shader objects and link program.    
*
* \author  gernot Rabbid76    \date  2019-03-09
***************************************************************************************************/
unsigned int CProgram::GenerateProgram(
    std::vector<unsigned int> & shader_objs //! in: shader stage objects
    ) const
{
   //  create program object attach shader objects and link program
    GLuint program_obj = glCreateProgram();
    for ( auto sh_obj : shader_objs )
        glAttachShader( program_obj, sh_obj );
    glLinkProgram( program_obj );

    // evaluate link result
    GLint status;
    glGetProgramiv( program_obj, GL_LINK_STATUS, &status );
    if ( status )
        return program_obj;
    
    // error logging
    if (_error_log)
    {
        GLint max_len;
        glGetProgramiv( program_obj, GL_INFO_LOG_LENGTH, &max_len );
        std::vector< char >log( max_len );
        GLsizei len;
        glGetProgramInfoLog( program_obj, max_len, &len, log.data() );
        std::cout << "link error:" << std::endl << log.data() << std::endl;
    }

    // delete invalid object
    glDeleteProgram( program_obj );

    return 0;
}


} // GL3xx


} // OpenGL


} // Program