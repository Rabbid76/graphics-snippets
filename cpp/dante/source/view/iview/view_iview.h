/***********************************************************************************************//**
* \brief Interfaces related to view, window and display.
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/

#pragma once
#ifndef __View_IView__h__
#define __View_IView__h__


// includes

#include <view_iview_types.h>


// STL



namespace View
{


/***********************************************************************************************//**
* \brief Basic interface for view (e.g. window)  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
class IView
{
public:
 
    IView() = default;
    IView( const IView & ) = delete;
    IView & operator = ( const IView & ) = default;
    
    virtual ~IView() {};

    virtual bool  Init( const TViewSettings & ) = 0;
    virtual bool  Dropped( void ) const = 0;
    virtual bool  SizeChanged( bool ) = 0;
    virtual TSize Size( void ) const = 0;
    virtual void  HandleEvents( void ) const = 0;
    virtual void  Activate( void ) const = 0;
    virtual void  Flush( void ) const = 0;
};



} // View

#endif // __View_IView__h__