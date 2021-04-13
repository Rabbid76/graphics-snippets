#include <pch.h>
#include <wxutil/wx_opengl_canvas.h>

namespace wxutil
{
    opengl_canvas::opengl_canvas(std::shared_ptr<view::view_interface> view, wxFrame* parent, int* args)
        : wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
        , _view(view)
        , _context{ std::make_unique<wxGLContext>(this) }
    {
        // To avoid flashing on MSW
        SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    }

    opengl_canvas::~opengl_canvas()
    {}

    void opengl_canvas::resized(wxSizeEvent& evt)
    {
        //	wxGLCanvas::OnSize(evt);
        _view->resize(*this);
        Refresh();
    }

    std::tuple<int, int> opengl_canvas::get_size(void) const
    {
        return std::make_tuple<int, int>(GetSize().x, GetSize().y);
    }

    void opengl_canvas::render(wxPaintEvent& evt)
    {
        if (!IsShown()) return;

        wxGLCanvas::SetCurrent(*_context);
        wxPaintDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event

        if (!_gl_initialized)
        {
            _view->init(*this);
            _gl_initialized = true;
        }

        _view->render(*this);
        glFlush();
        SwapBuffers();
    }

    BEGIN_EVENT_TABLE(opengl_canvas, wxGLCanvas)
    EVT_MOTION(opengl_canvas::mouse_moved)
    EVT_LEFT_DOWN(opengl_canvas::mouse_down)
    EVT_LEFT_UP(opengl_canvas::mouse_released)
    EVT_RIGHT_DOWN(opengl_canvas::right_click)
    EVT_LEAVE_WINDOW(opengl_canvas::mouse_left_window)
    EVT_SIZE(opengl_canvas::resized)
    EVT_KEY_DOWN(opengl_canvas::key_pressed)
    EVT_KEY_UP(opengl_canvas::key_released)
    EVT_MOUSEWHEEL(opengl_canvas::mouse_wheel_moved)
    EVT_PAINT(opengl_canvas::render)
    END_EVENT_TABLE()

    void opengl_canvas::mouse_moved(wxMouseEvent& event)
    {}
    
    void opengl_canvas::mouse_down(wxMouseEvent& event)
    {}
    
    void opengl_canvas::mouse_wheel_moved(wxMouseEvent& event)
    {}
    
    void opengl_canvas::mouse_released(wxMouseEvent& event)
    {}
    
    void opengl_canvas::right_click(wxMouseEvent& event)
    {}
    
    void opengl_canvas::mouse_left_window(wxMouseEvent& event)
    {}
    
    void opengl_canvas::key_pressed(wxKeyEvent& event)
    {}
    
    void opengl_canvas::key_released(wxKeyEvent& event)
    {}
}
