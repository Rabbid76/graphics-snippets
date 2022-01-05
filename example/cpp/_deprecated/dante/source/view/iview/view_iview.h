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

#include <map>
#include <list>
#include <vector>
#include <functional> 


namespace View
{

class IView;


//! kind of the cursor event
enum class TCursorEventKind
{
    unknown,  //!< unknown event - used for initialization
    move,     //!< cursor was moved
    scroll,   //!< "wheel" scroll 
    pressed,  //!< button pressed
    released, //!< button released
    enter,    //!< cursor entered the window
    leave     //!< cursor has left the window 
};


//! name of the button
enum TCursorButton : int
{
    eCursorBtn_NON    = -1, //!< no button
    eCursorBtn_left   =  0, //!< left button 
    eCursorBtn_middle =  1, //!< middle button 
    eCursorBtn_right  =  2, //!< right button 
};


//! data for the cursor event
class TCursorEventData
{
public:

    TCursorEventKind _kind      { TCursorEventKind::unknown }; //!< kind of the mouse event
    TCursorButton    _button    { eCursorBtn_NON };            //!< pressed or released button
    TPointF          _position  { 0, 0 };                      //!< position information for the event (e.g. mouse position)
    TSizeF           _distance  { 0, 0 };                      //!< distance information for the event (e.g. scroll mouse wheel)
    TSize            _view_size { 0, 0 };                      //!< size of the view   
    int              _info      { 0 };                         //!< additional information like button modifiers 
};


//! call back type for cursor event 
using TCursorEvent    = std::function<void(IView &, const TCursorEventData &)>;
using TCursorEventSet = std::map<size_t, std::list<TCursorEvent>>;


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

    //! add an event to the set of events
    void AddCursorEvent( size_t id, TCursorEvent &callback_object )
    { 
        _cursor_events[id].emplace_back( callback_object ); 
    }

    //! remove all events associated to the `id` 
    void RemoveCursorEvent( size_t id )
    {
        _cursor_events.erase( id );
    }

protected:

    //! send the cursor event
    void PerformCursorEvents( const TCursorEventData &data )
    {
        for (auto &event_queue : _cursor_events)
        {
            for (auto & event_cb : event_queue.second)
                event_cb( *this, data );
        }
    }

private:

    TCursorEventSet _cursor_events; //! event objects for cursor event
};



} // View

#endif // __View_IView__h__