#ifndef __WX_OPENGL_CANVAS __
#define __WX_OPENGL_CANVAS __

#include "wx/glcanvas.h" 

namespace wxutil
{
    // [xGLCanvas](https://wiki.wxwidgets.org/WxGLCanvas)
    class open_gl_canvas : public wxGLCanvas
    {
    private:

        wxGLContext* _context;
        bool _gl_initialized = false;

    public:
        open_gl_canvas(wxFrame* parent, int* args);
        virtual ~open_gl_canvas();

        void resized(wxSizeEvent& evt);

        int get_width();
        int get_height();

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
