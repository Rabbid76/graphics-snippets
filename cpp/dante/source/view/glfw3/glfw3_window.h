/***********************************************************************************************//**
* \brief GLFW-3 window implementation.
*
* [GLFW](https://www.glfw.org/)
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/

#pragma once
#ifndef __View_GLFW3_Window__h__
#define __View_GLFW3_Window__h__


// includes

#include <view_iview.h>


// STL

#include <string>
#include <memory>


namespace View
{


namespace GLFW3 
{


class CWindowHandle;
using TWindowHandle = std::shared_ptr<CWindowHandle>;


/***********************************************************************************************//**
* \brief GLFW-3 window implementation  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
class CWindow
    : public IView
{
public:

    CWindow( const std::string & );
    virtual ~CWindow() override;

    bool Valid( void ) const noexcept(true) { return _handle != nullptr; }

    virtual bool  Init( const TInitialize & ) noexcept(false) override;
    
    virtual bool  Dropped( void )      const noexcept(false) override;
    virtual TSize Size( void )         const noexcept(false) override;
    virtual void  HandleEvents( void ) const noexcept(false) override;
    virtual void  Activate( void )     const noexcept(false) override;
    virtual void  Flush( void )        const noexcept(false) override;

private:

    void Validate( void ) const noexcept(false);

    TWindowHandle _handle;    //!< window object handle
    std::string   _name;      //!< name of the window
    TInitialize   _paramters; //!< initialization parameters
    TSize         _size;      //!< current window size
};


} // GLFW3


} // View


#endif // __View_GLFW3_Window__h__