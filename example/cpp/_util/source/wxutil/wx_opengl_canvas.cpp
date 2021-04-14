#include <pch.h>
#include <wxutil/wx_opengl_canvas.h>

namespace wxutil
{
    OpenGLCanvas::OpenGLCanvas(std::shared_ptr<view::ViewInterface> view, wxFrame* parent, int* args)
        : wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
        , _view(view)
        , _context{ std::make_unique<wxGLContext>(this) }
    {
        // To avoid flashing on MSW
        SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    }

    OpenGLCanvas::~OpenGLCanvas()
    {}

    std::tuple<int, int> OpenGLCanvas::get_size(void) const
    {
        return std::make_tuple<int, int>(GetSize().x, GetSize().y);
    }

    void OpenGLCanvas::activate(void) const
    {
        wxGLCanvas::SetCurrent(*_context);
    }

    void OpenGLCanvas::resized(wxSizeEvent& evt)
    {
        //	wxGLCanvas::OnSize(evt);
        _view->resize(*this);
        Refresh();
    }

    void OpenGLCanvas::render(wxPaintEvent& evt)
    {
        if (!IsShown()) 
            return;

        wxPaintDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event

        if (!_gl_initialized)
        {
            activate();
            _view->init(*this);
            _gl_initialized = true;
        }

        _view->render(*this);
        glFlush();
        SwapBuffers();
    }

    BEGIN_EVENT_TABLE(OpenGLCanvas, wxGLCanvas)
    EVT_MOTION(OpenGLCanvas::mouse_moved)
    EVT_LEFT_DOWN(OpenGLCanvas::mouse_down)
    EVT_LEFT_UP(OpenGLCanvas::mouse_released)
    EVT_RIGHT_DOWN(OpenGLCanvas::right_click)
    EVT_LEAVE_WINDOW(OpenGLCanvas::mouse_left_window)
    EVT_SIZE(OpenGLCanvas::resized)
    EVT_KEY_DOWN(OpenGLCanvas::key_pressed)
    EVT_KEY_UP(OpenGLCanvas::key_released)
    EVT_MOUSEWHEEL(OpenGLCanvas::mouse_wheel_moved)
    EVT_PAINT(OpenGLCanvas::render)
    END_EVENT_TABLE()

    void OpenGLCanvas::mouse_moved(wxMouseEvent& event)
    {}
    
    void OpenGLCanvas::mouse_down(wxMouseEvent& event)
    {}
    
    void OpenGLCanvas::mouse_wheel_moved(wxMouseEvent& event)
    {}
    
    void OpenGLCanvas::mouse_released(wxMouseEvent& event)
    {}
    
    void OpenGLCanvas::right_click(wxMouseEvent& event)
    {}
    
    void OpenGLCanvas::mouse_left_window(wxMouseEvent& event)
    {}
    
    void OpenGLCanvas::key_pressed(wxKeyEvent& event)
    {}
    
    void OpenGLCanvas::key_released(wxKeyEvent& event)
    {}
}
