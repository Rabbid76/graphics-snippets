/******************************************************************//**
* @brief   Generic interface for a render program (shader).
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
#pragma once
#ifndef Render_IProram_h_INCLUDED
#define Render_IProram_h_INCLUDED


// includes

#include <Render_IDrawType.h>


// STL

#include <bitset>
#include <string>
#include <tuple>
#include <memory>



/******************************************************************//**
* @brief Namespace for renderer.
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
namespace Render
{


/******************************************************************//**
* \brief Namespace for shader programs and resources.  
* 
* \author  gernot
* \date    2018-07-31
* \version 1.0
**********************************************************************/
namespace Program
{

class IIntrospection;
using TIntrospectionPtr = std::shared_ptr<IIntrospection>;


//! type of a shader stage
enum class TShaderType
{
  vertex,          //!< vertex shader stage
  tess_control,    //!< tessellation control shader stage
  tess_evaluation, //!< tessellation evaluation shader stage
  geometry,        //!< geometry shader stage
  fragment,        //!< fragment shader stage
  compute          //!< compute shader
};


//! general type of a program
enum class TProgramType
{
  draw,              //!< shader program which renders to a framebuffer
  tranform_feedback, //!< shader program which generates geometry
  compute            //!< compute shader 
};


//! type of an active resource of a program 
enum class TResourceType
{
  attribute,            //!< attribute index
  fragment_data,        //!< fragment data location
  transform_feedback,   //!< transform feedback output (the type of a resource only, has no binding point index or location)
  uniform,              //!< uniform location
  uniform_block,        //!< uniform block index
  shader_storage_block, //!< shader storage block (SSBO)
  subroutine_uniform,   //!< shader subroutine uniform
  // ...
  NUMBER_OF,
};
using TResourceTypes = std::bitset<(int)TResourceType::NUMBER_OF>;

//! binding point location or index and its type
using TResourcePoint = std::tuple<size_t, TResourceType>;

//! name of a resource, binding point location or index and its type
using TResourceBinding = std::tuple<std::string, TResourceType, size_t>;


//! transform feedback buffer mode
enum class TTranformFeedbackMode
{
  NON,         //!< no transform feedback
  interleaved, //!< one buffer for all transform feedback outputs
  separate     //!< a separate buffer binding for each transform feedback output
};


//---------------------------------------------------------------------
// IShader
//---------------------------------------------------------------------


/******************************************************************//**
* \brief Generic interface representing a shader stage. 
* 
* The implementations of this interface should make use of 
* RAII (Resource acquisition is initialization) technique.
* [https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization]
*
* \author  gernot
* \date    2018-07-31
* \version 1.0
**********************************************************************/
class IShader
{
public:

  virtual ~IShader() = default;

  // return shader object handle 
  virtual size_t ObjectHandle( void ) const = 0;

  // return state type
  virtual TShaderType Type( void ) = 0;

  // append source code to the shader 
  virtual IShader & operator << ( const std::string & code ) = 0;
  virtual IShader & operator << ( std::string && code ) = 0;
  virtual IShader & ClearCode( void ) = 0;

  // Compile the shader stage, the function succeeds, even if the compilation fails, but it fails if the stage was not properly initialized.
  virtual bool Compile( void ) = 0;
  
  // Verifies the compilation result.
  virtual bool Verify( std::string &message ) = 0;
};
using TShaderPtr = std::shared_ptr<IShader>;


//---------------------------------------------------------------------
// IProgram
//---------------------------------------------------------------------


/******************************************************************//**
* \brief Generic interface representing a shader program.  
* 
* The implementations of this interface should make use of 
* RAII (Resource acquisition is initialization) technique.
* [https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization]
*
* \author  gernot
* \date    2018-07-31
* \version 1.0
**********************************************************************/
class IProgram
{
public:

  virtual ~IProgram() = default;

  // return shader object handle 
  virtual size_t ObjectHandle( void ) const = 0;

  // append a shader object
  virtual IProgram & operator << ( const TShaderPtr & shader ) = 0;

  // append resource specification (binding point, location or index)
  virtual IProgram & operator << ( const TResourceBinding & resource ) = 0;

  // set transform feedback mode
  virtual IProgram & operator << ( const TTranformFeedbackMode & mode ) = 0;

  // Link shader objects to a shader program, the function succeeds, even if the linking fails, but it fails if the program was not properly initialized.
  virtual bool Link( void ) = 0;

  // Activate a program.
  virtual bool Use( void ) = 0;
  
  // Verifies the linking result.
  virtual bool Verify( std::string &message ) = 0;

  // Ask for the resource information of the program.
  virtual TIntrospectionPtr Introspection( TResourceTypes resources, bool verbose ) = 0;
};
using TProgramPtr = std::shared_ptr<IProgram>;


//---------------------------------------------------------------------
// IIntrospection
//---------------------------------------------------------------------


/******************************************************************//**
* \brief Generic interface for the introspection of a program.
*
* \details Provides the active resource information of program.
* 
* \author  gernot
* \date    2018-07-31
* \version 1.0
**********************************************************************/
class IIntrospection
{
public:

  virtual ~IIntrospection() = default;

  // find a active program resource
  virtual bool FindResource( const std::string &name, size_t &handle, TResourceType &resource_type ) = 0;

  // bit set which resources have to be evaluated

  // resource maps : name -> location, index, binding point

  // return program type
  //virtual TProgramType Type( void ) = 0;

/*

map: name -> entry point, type 
     unorderd_map; if name would mean different entry point types the unorderd_multimap


*/
};



//---------------------------------------------------------------------
// IDataExchange
//---------------------------------------------------------------------


/******************************************************************//**
* \brief  Generic interface for the data exchange with a program.  
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/
class IDataExchange
{
public:

  virtual ~IDataExchange() = default;

  // associates a active program resource to binding point of data
  virtual bool SetBinding( size_t handle, TResourceType resource_type, size_t binding_point ) = 0;
  
  // TODO $$$ how to pass the type of a uniform ???
  // set data to the program
  virtual bool Set( size_t handle, TResourceType resource_type, void *data ) = 0;

};
using TDataExchangePtr = std::shared_ptr<IDataExchange>;



//---------------------------------------------------------------------
// IProvider
//---------------------------------------------------------------------


/******************************************************************//**
* \brief Generic provider for generic program objects.  
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/
class IProvider
{
public:

  //! create new shader object
  virtual TShaderPtr NewShader( TShaderType type ) const = 0;

  // create new program object
  virtual TProgramPtr NewProgram( void ) const = 0;

  // create new program introspection
  virtual TIntrospectionPtr NewIntrospection( TProgramPtr program, TResourceTypes resource_types ) const = 0;

  // create new data exchange 
  virtual TDataExchangePtr NewDataExchange( TProgramPtr program ) const = 0;
};



//---------------------------------------------------------------------
// CInstance
//---------------------------------------------------------------------


/******************************************************************//**
* \brief Instance of a single program.   
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/
class CInstance
{
public:

  // TODO $$$ CInstance( const IProvider & provider )

  virtual ~CInstance() = default;

  /*
  
  << [shader_type, sorce_code]

  */
};


} // Program

} // Render


#endif // Render_IProram_h_INCLUDED