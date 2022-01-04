/******************************************************************//**
* \brief   Generic interface for rendering context.
* 
* \author  gernot
* \date    2018-02-04
* \version 1.0
**********************************************************************/
#pragma once
#ifndef IRenderContext_h_INCLUDED
#define IRenderContext_h_INCLUDED


// includes


// includes



// interface declarations

/******************************************************************//**
* \brief namespace for generic interface of drawing operations
**********************************************************************/
namespace Render
{


class IContext;


class IViewport
{};


//*********************************************************************
// IContextManager
//*********************************************************************


/******************************************************************//**
* \brief   Generic interface for render context management.
* 
* \author  gernot
* \date    2018-02-09
* \version 1.0
**********************************************************************/
class IContextManager
{
public:

  IContextManager( void ) = default;
  virtual ~IContextManager( void ) = default;

  IContextManager( const IContextManager & ) = delete;
  IContextManager( IContextManager && ) = delete;

  IContextManager & operator =( const IContextManager & ) = delete;
  IContextManager & operator =( IContextManager & ) = delete;


  virtual IContext & GetGlobalContext( void ) = 0;
  virtual IContext && NewOffscreenContext( void ) = 0;
  virtual IContext && NewContext( const IViewport &viewport ) = 0;
};



//*********************************************************************
// IContext
//*********************************************************************


/******************************************************************//**
* \brief   Generic interface for a render context.
* 
* \author  gernot
* \date    2018-02-09
* \version 1.0
**********************************************************************/
class IContext
{
public:

  IContext( IContextManager &manager )
    : _manager( manager )
  {}

  IContext( const IContext & ) = delete;

  IContext( IContext &&src )
    : _manager( src._manager)
  {}

  virtual ~IContext() = default;

  IContext & operator =( const IContext & ) = delete;

  IContext & operator =( IContext && ) = default;

  virtual bool IsActive( void ) const = 0;
  virtual bool Activate( void ) = 0;
  virtual bool Release( void ) = 0;


private:

  IContextManager & _manager;
}; 


} // Draw


#endif // IRenderContext_h_INCLUDED

