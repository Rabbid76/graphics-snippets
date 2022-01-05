/***********************************************************************************************//**
* \brief GLFW-3 window implementation.
*
* [GLFW](https://www.glfw.org/)
*
* [Introduction to the API](https://www.glfw.org/docs/latest/intro_guide.html#event_order)
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/

#include <stdafx.h>

// include

#include <glfw3_window.h>


// GLFW-3

#include <GLFW/glfw3.h>


// STL

#include <cassert>


namespace View
{


namespace GLFW3 
{


/***********************************************************************************************//**
* \brief GLFW exception object  
*
* \author  Rabbid76    \date  2019-02-02
***************************************************************************************************/
class GLFWError
    : public View::Error
{
public:

    GLFWError( 
        const std::string &message ) //! error message
        : View::Error( "GLFW-3: " + message )
    {}
};


/***********************************************************************************************//**
* \brief GLFW-3 window pointer  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
class CWindowHandle
{
public:
    GLFWwindow *_window{ nullptr };

    static CWindow * WindowPtr( GLFWwindow *window )
    {
        void *ptr = glfwGetWindowUserPointer( window );
        return static_cast<CWindow*>(ptr);
    }

    TSize WindowSize( void ) const
    {
        //! [`glfwGetWindowSize`]https://www.glfw.org/docs/latest/group__window.html#gaeea7cbc03373a41fb51cfbf9f2a5d4c6)

        int cx, cy;
        glfwGetWindowSize(_window, &cx, &cy);
        return { (TScale)cx, (TScale)cy };
    }

    TSize FramebufferSize( void ) const
    {
        //! [`glfwGetFramebufferSize`](https://www.glfw.org/docs/latest/group__window.html#ga0e2637a4161afb283f5300c7f94785c9)

        int cx, cy;
        glfwGetFramebufferSize(_window, &cx, &cy);
        return { (TScale)cx, (TScale)cy };
    }

    TPointF CursorPosition( void ) const
    {
        //! [`glfwGetCursorPos`](https://www.glfw.org/docs/latest/group__input.html#ga01d37b6c40133676b9cea60ca1d7c0cc)

        double px, py;
        glfwGetCursorPos(_window, &px, &py);
        return { (TScaleF)px, (TScaleF)py };
    }

    void CursorPosition( TPointF position )
    {
        //! [`glfwSetCursorPos`](https://www.glfw.org/docs/latest/group__input.html#ga04b03af936d906ca123c8f4ee08b39e7)

        glfwSetCursorPos(_window, (double)position[0], (double)position[1]);
    }

    bool LastButton( int button )
    {
        //! [`glfwGetMouseButton`](https://www.glfw.org/docs/latest/group__input.html#gac1473feacb5996c01a7a5a33b5066704);

        int return_state = glfwGetMouseButton(_window, button);
         
        VIEW_ASSERT(return_state == GLFW_PRESS || return_state == GLFW_RELEASE);
        return return_state == GLFW_PRESS;
    }
};


/***********************************************************************************************//**
* \brief ctor  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
CWindow::CWindow( 
    const std::string &name ) //!< name of the window
    : _name( name )
{
    if ( Valid() )
        glfwDestroyWindow( _handle->_window  );
}


/***********************************************************************************************//**
* \brief dtor  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
CWindow::~CWindow()
{}


/***********************************************************************************************//**
* \brief Validate window handle and throw an exception if the validation fails.  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
void CWindow::Validate( void ) const
{
    if ( Valid() == false )
        throw GLFWError( "invalid window handle" );
}


/***********************************************************************************************//**
* \brief Initialize the window  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
bool CWindow::Init(
    const TViewSettings &parameters ) //!< initialization parameter
{
    class CInstance
    {
    public:
        CInstance( void )
        {
            if ( glfwInit() == GLFW_FALSE )
                throw GLFWError("error initializing");
        }

        virtual ~CInstance()
        {
            glfwTerminate();
        }
    };

    // initialize GLFW-3
    static CInstance instance;

    // check if window was initialized already 
    if ( Valid() )
        return true;

    _paramters = parameters;

    // [GLFW Window guide; Window creation hints](http://www.glfw.org/docs/latest/window_guide.html#window_hints_values)

    glfwWindowHint( GLFW_DEPTH_BITS, 24 );
    glfwWindowHint( GLFW_STENCIL_BITS, 8 ); 

    glfwWindowHint( GLFW_SAMPLES, _paramters._samples );
    glfwWindowHint( GLFW_DOUBLEBUFFER, _paramters.Test<TCapability::doublebuffer>() ? GLFW_TRUE : GLFW_FALSE );

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
    //glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );

    glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, _paramters.Test<TCapability::debug>() ? GLFW_TRUE : GLFW_FALSE );

    auto window = glfwCreateWindow( _paramters._size[0], _paramters._size[1], _name.c_str(), nullptr, nullptr );
    if ( window == nullptr )
    {
        glfwTerminate();
        throw GLFWError( "error initializing window" );
    }

    glfwSetWindowUserPointer( window, this );

    //! [`glfwSetWindowSizeCallback`](https://www.glfw.org/docs/latest/group__window.html#gaa40cd24840daa8c62f36cafc847c72b6)
    glfwSetWindowSizeCallback( window, [](GLFWwindow* window, int cx, int cy)
    {
        if ( CWindow *wndPtr = CWindowHandle::WindowPtr(window) )
            wndPtr->Resize( { (TScale)cx, (TScale)cy } );
    } );

    //! [`glfwSetCursorEnterCallback`](https://www.glfw.org/docs/latest/group__input.html#gaa299c41dd0a3d171d166354e01279e04)
    glfwSetCursorEnterCallback( window, [](GLFWwindow* window, int enter_or_leave)
    {
        if ( CWindow *wndPtr = CWindowHandle::WindowPtr(window) )
        {
            VIEW_ASSERT(enter_or_leave == GLFW_TRUE || enter_or_leave == GLFW_FALSE);

            bool entered = enter_or_leave == GLFW_TRUE;
            wndPtr->CursorEnter( entered );
        }
    } );

    //! [`glfwSetCursorPosCallback`](https://www.glfw.org/docs/latest/group__input.html#ga7dad39486f2c7591af7fb25134a2501d)
    glfwSetCursorPosCallback( window, [](GLFWwindow* window, double xpos, double ypos)
    {
        if ( CWindow *wndPtr = CWindowHandle::WindowPtr(window) )
            wndPtr->CursorMove( { (TScaleF)xpos, (TScaleF)ypos } );
    } );

    //! [`glfwSetScrollCallback`](https://www.glfw.org/docs/latest/group__input.html#gacf02eb10504352f16efda4593c3ce60e)
    glfwSetScrollCallback( window, [](GLFWwindow* window, double xscroll, double yscroll)
    {
        if ( CWindow *wndPtr = CWindowHandle::WindowPtr(window) )
            wndPtr->CursorScroll( { (TScaleF)xscroll, (TScaleF)yscroll } );
    } );

    //! [`glfwSetMouseButtonCallback`](https://www.glfw.org/docs/latest/group__input.html#gaef49b72d84d615bca0a6ed65485e035d)
    glfwSetMouseButtonCallback( window, [](GLFWwindow* window, int button, int action, int mode)
    {
        if ( CWindow *wndPtr = CWindowHandle::WindowPtr(window) )
        {
            VIEW_ASSERT(action == GLFW_PRESS || action == GLFW_RELEASE);
            
            bool pressed = action == GLFW_PRESS;
            
            //! [Mouse buttons](https://www.glfw.org/docs/latest/group__buttons.html)
            int cursor_button = button;

            wndPtr->CursorButton( pressed, cursor_button, mode );
        }
    } );

    /*
    
    std::array< int, 2 > _wndPos         {0, 0};
    std::array< int, 2 > _wndSize        {0, 0};
    std::array< int, 2 > _vpSize         {0, 0};
    bool                 _updateViewport = true;
    bool                 _doubleBuffer   = true;
    GLFWwindow *         _wnd            = nullptr;
    GLFWmonitor *        _monitor        = nullptr;

    */

    /*
   
    void CWindow_Glfw::Resize( int cx, int cy )
    {
        _updateViewport = true;
    }
    */

    //_monitor =  glfwGetPrimaryMonitor();
    //glfwGetWindowSize( window, &_wndSize[0], &_wndSize[1] );
    //glfwGetWindowPos( window, &_wndPos[0], &_wndPos[1] );

    _handle = std::make_shared<CWindowHandle>( CWindowHandle( { window } ) );
    _window_size = _handle->WindowSize();
    _buffer_size = _handle->FramebufferSize();

    // ...

    return Valid();
}


/***********************************************************************************************//**
* \brief Resize notification 
*
* \author  Rabbid76    \date  2019-02-02
***************************************************************************************************/
void CWindow::Resize( 
    TSize ) //!< new window size - not this may be different to framebuffer size
{
    _window_size = _handle->WindowSize();
    _buffer_size = _handle->FramebufferSize();
    _state.set( (int)TWindowState::size_changed, true );
}


/***********************************************************************************************//**
* \brief Cursor enter or leave notification.  
*
* \author  gernot Rabbid76    \date  2019-02-10
***************************************************************************************************/
void CWindow::CursorEnter( 
    bool entered ) //!< true: the window was entered; false: the window was left
{
    TCursorEventData event_data;
    event_data._view_size = _window_size;
    event_data._kind      = entered ? TCursorEventKind::enter : TCursorEventKind::leave;
    event_data._position  = _handle->CursorPosition();

    PerformCursorEvents( event_data );
}


/***********************************************************************************************//**
* \brief Cursor move or leave notification.  
*
* \author  gernot Rabbid76    \date  2019-02-10
***************************************************************************************************/
void CWindow::CursorMove( 
    TPointF positon ) //!< current cursor position
{
    TCursorEventData event_data;
    event_data._view_size = _window_size;
    event_data._kind      = TCursorEventKind::move;
    event_data._position  = positon;

    PerformCursorEvents( event_data );
}


/***********************************************************************************************//**
* \brief Cursor scroll notification.  
*
* \author  gernot Rabbid76    \date  2019-02-10
***************************************************************************************************/
void CWindow::CursorScroll( 
    TSizeF scroll ) //! scroll width
{
    TCursorEventData event_data;
    event_data._view_size = _window_size;
    event_data._kind      = TCursorEventKind::scroll;
    event_data._position  = _handle->CursorPosition();
    event_data._distance  = scroll;

    PerformCursorEvents( event_data );
}


/***********************************************************************************************//**
* \brief Cursor button notification.  
*
* \author  gernot Rabbid76    \date  2019-02-10
***************************************************************************************************/
void CWindow::CursorButton( 
    bool pressed, //!< true: button was pressed; false: button was released 
    int  button,  //!< the button identifier
    int  mode )   //!< additional information (e.g. key modifiers)
{
    TCursorEventData event_data;
    event_data._view_size = _window_size;
    event_data._kind      = pressed ? TCursorEventKind::pressed : TCursorEventKind::released;
    event_data._position  = _handle->CursorPosition();
    event_data._button    = (TCursorButton)button;
    event_data._info      = mode;

    PerformCursorEvents( event_data );
}


/***********************************************************************************************//**
* \brief Returns `true` if the widow has received the close notification.
* Otherwise `false` is returned.
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
bool CWindow::Dropped( void ) const
{
    Validate();
    return glfwWindowShouldClose( _handle->_window ) != GLFW_FALSE;
}


/***********************************************************************************************//**
* \brief Evaluate if the size of the window has changed.  
*
* \author  Rabbid76    \date  2019-02-02
***************************************************************************************************/
bool CWindow::SizeChanged( 
    bool reset_changed ) //!< true: rest the size changed state
    noexcept(true)
{
    bool changed = _state.test( (int)TWindowState::size_changed );
    if ( reset_changed )
        _state.reset( (int)TWindowState::size_changed );
    return changed;
}


/***********************************************************************************************//**
* \brief Get the current window size.  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
TSize CWindow::Size( void ) const noexcept(true)
{
    return _buffer_size;
}


/***********************************************************************************************//**
* \brief Handle pending events.  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
void CWindow::HandleEvents( void ) const
{
    Validate();
    glfwPollEvents();
}


/***********************************************************************************************//**
* \brief Make context current.  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
void CWindow::Activate( void ) const
{
    Validate();
    glfwMakeContextCurrent( _handle->_window );
}


/***********************************************************************************************//**
* \brief Flush or swap the display buffers.  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
void CWindow::Flush( void ) const
{
    Validate();
    glfwSwapBuffers( _handle->_window );
}


} // GLFW3


} // View