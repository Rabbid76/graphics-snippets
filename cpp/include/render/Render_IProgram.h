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



/******************************************************************//**
* @brief   Namespace for renderer.
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
namespace Render
{

class IProgram
{

};

class IProgramIntrospection
{

  // bit set which resources have to be evaluated

  // resource maps : name -> location, index, binding point

/*

map: name -> entry point, type 
     unorderd_map; if name would mean differnt entryp point types the unorderd_multimap


*/
};


}; // Render


#endif // Render_IProram_h_INCLUDED