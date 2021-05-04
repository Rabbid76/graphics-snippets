#ifndef __WX_OPENGL_CANVAS__
#define __WX_OPENGL_CANVAS__

#include <wxutil/wx_include.h>
#include <view/canvas_interface.h>
#include <view/view_interface.h>
#include <view/mouse_event_interface.h>

#include <memory>


namespace wxutil
{
    // [xGLCanvas](https://wiki.wxwidgets.org/WxGLCanvas)
    class OpenGLCanvas 
        : public wxGLCanvas, view::CanvasInterface
    {
    private:

        static const int _timer_id = 1;
        const std::shared_ptr<view::ViewInterface> _view;
        std::vector<const view::MouseEventInterface*> _mouse_event_clients;
        const std::unique_ptr<wxGLContext> _context;
        bool _gl_initialized = false;
        std::unique_ptr<wxTimer> _timer;

    public:

        static OpenGLCanvas* new_gl_canvas(std::shared_ptr<view::ViewInterface> view, wxFrame* parent, 
            int depth_bits, int stencil_bits, int samples);
        static OpenGLCanvas* new_gl_canvas(std::shared_ptr<view::ViewInterface> view, wxFrame* parent, 
            int depth_bits, int stencil_bits, int samples, int major, int minor, bool core, bool forward_compatibility, bool debug);
        OpenGLCanvas(std::shared_ptr<view::ViewInterface> view, wxFrame* parent, int* args);
        virtual ~OpenGLCanvas();

        void add_mouse_event_client(const view::MouseEventInterface* mouse_event_client)
        {
            _mouse_event_clients.emplace_back(mouse_event_client);
        }

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

    private:

        void refresh_timer(wxTimerEvent& event_id);
    };
}

#endif
