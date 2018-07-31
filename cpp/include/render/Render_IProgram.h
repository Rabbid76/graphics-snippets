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


// stl

#include <bitset>
#include <string>
#include <tuple>



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
* \brief Namspece for shader progrmas and resources.  
* 
* \author  gernot
* \date    2018-07-31
* \version 1.0
**********************************************************************/
namespace Program
{

class IIntrospection;


enum class TResourceType
{
  // ...
  NUMBER_OF,
};
using TResourceTypes = std::bitset<(int)TResourceType::NUMBER_OF>;



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

  // Compile the shader stage, the function succeeds, even if the compilation fails, but it fails if the stage was not properly initialized.
  virtual bool Compile( void ) = 0;
  
  // Verifies the compilation result.
  virtual bool Verify( std::string &message ) = 0;
};


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

  // Link the program, the function succeeds, even if the linking fails, but it fails if the program was not properly initialized.
  virtual bool Link( void ) = 0;
  
  // Verifies the linking result.
  virtual bool Verify( std::string &message ) = 0;

  // Ask for the resource information of the program.
  virtual IIntrospection && Introspection( TResourceTypes resources ) = 0;
};


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
  virtual bool FindResource( const std::string &name, size_t handle, TResourceType &resource_type ) = 0;

  // bit set which resources have to be evaluated

  // resource maps : name -> location, index, binding point

/*

map: name -> entry point, type 
     unorderd_map; if name would mean differnt entryp point types the unorderd_multimap


*/
};


} // Program

} // Render


#endif // Render_IProram_h_INCLUDED