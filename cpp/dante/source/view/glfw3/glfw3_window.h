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


//! GLFW window states
enum class TWindowState
{
    size_changed, //!< indicates that the size of the window has changed

    NO_OF         //!< number of window state flags
};

using TWindowStateSet = std::bitset<(int)TWindowState::NO_OF>;


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

    virtual bool  Init( const TViewSettings & ) noexcept(false) override;
    
    virtual bool  Dropped( void )      const noexcept(false) override;
    virtual bool  SizeChanged( bool )        noexcept(true)  override;
    virtual TSize Size( void )         const noexcept(true)  override;
    virtual void  HandleEvents( void ) const noexcept(false) override;
    virtual void  Activate( void )     const noexcept(false) override;
    virtual void  Flush( void )        const noexcept(false) override;

private:

    void Validate( void ) const noexcept(false);

    void Resize( TSize );
    void CursorEnter( bool );
    void CursorMove( TPointF );
    void CursorScroll( TSizeF );
    void CursorButton( bool, int, int );

    TWindowHandle   _handle;            //!< window object handle
    std::string     _name;              //!< name of the window
    TViewSettings   _paramters;         //!< initialization parameters
    TSize           _window_size{0, 0}; //!< current window size
    TSize           _buffer_size{0, 0}; //!< current frame buffer size
    TWindowStateSet _state;             //!< current states of the window
};


} // GLFW3


} // View


#endif // __View_GLFW3_Window__h__