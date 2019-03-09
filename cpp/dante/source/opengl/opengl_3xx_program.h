/***********************************************************************************************//**
* \brief Base class implementation for OpenGL ES 3.x and (desktop) OpenGL 3.2+ texture program.
*
* \author  Rabbid76    \date  2019-03-09
***************************************************************************************************/

#pragma once
#ifndef __OpenGL_3xx_program__h__
#define __OpenGL_3xx_prgram__h__


// include

#include <texture_itexture_types.h>


// STL

#include <string>
#include <vector>


namespace Program
{


namespace OpenGL
{


namespace GL3xx
{


using TStage = std::tuple<std::string, unsigned int>;
using TStages = std::vector<TStage>;


/***********************************************************************************************//**
* \brief Base class for shader program.  
*
* \author  gernot Rabbid76    \date  2019-02-24
***************************************************************************************************/
class CProgram
{
public:

    CProgram( void );
    virtual ~CProgram();

    unsigned int ProgramObject( void ) const { return _program_obj; }

    CProgram & Generate( const TStages & );

private:

    unsigned int GenerateShader( const std::string &, unsigned int ) const;
    unsigned int GenerateProgram( std::vector<unsigned int> & ) const;

    bool _verbose  = true;  //! true: activate shader source code logging
    bool _error_log = true; //! true: activate error logging

    std::vector<unsigned int> _shader_objs; //! shader stage objects
    unsigned int              _program_obj; //! program object
};


} // GL3xx


} // OpenGL


} // Program


#endif // __OpenGL_3xx_program__h__