#ifndef __WX_OPENGL_CANVAS__
#define __WX_OPENGL_CANVAS__

#include "wx/glcanvas.h" 
#include <view/canvas_interface.h>
#include <view/view_interface.h>

#include <memory>


namespace wxutil
{
    // [xGLCanvas](https://wiki.wxwidgets.org/WxGLCanvas)
    class OpenGLCanvas 
        : public wxGLCanvas, view::CanvasInterface
    {
    private:

        const std::shared_ptr<view::ViewInterface> _view;
        const std::unique_ptr<wxGLContext> _context;
        bool _gl_initialized = false;

    public:

        OpenGLCanvas(std::shared_ptr<view::ViewInterface> view, wxFrame* parent, int* args);
        virtual ~OpenGLCanvas();

        virtual std::tuple<int, int> get_size(void) const override;
        virtual void activate(void) const override;
        void resized(wxSizeEvent& evt);
        void render(wxPaintEvent& evt);

        // events
        void mouse_moved(wxMouseEvent& event);
        void mouse_down(wxMouseEvent& event);
        void mouse_wheel_moved(wxMouseEvent& event);
        void mouse_released(wxMouseEvent& event);
        void right_click(wxMouseEvent& event);
        void mouse_left_window(wxMouseEvent& event);
        void key_pressed(wxKeyEvent& event);
        void key_released(wxKeyEvent& event);

        DECLARE_EVENT_TABLE()
    };
}

#endif
