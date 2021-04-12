#include <pch.h>
#include <wxutil/wx_opengl_canvas.h>

namespace wxutil
{
    open_gl_canvas::open_gl_canvas(wxFrame* parent, int* args) :
        wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
    {
        _context = new wxGLContext(this);


        // To avoid flashing on MSW
        SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    }

    open_gl_canvas::~open_gl_canvas()
    {
        delete _context;
    }

    void open_gl_canvas::resized(wxSizeEvent& evt)
    {
        //	wxGLCanvas::OnSize(evt);

        Refresh();
    }

    int open_gl_canvas::get_width()
    {
        return GetSize().x;
    }

    int open_gl_canvas::get_height()
    {
        return GetSize().y;
    }


    void open_gl_canvas::render(wxPaintEvent& evt)
    {
        if (!IsShown()) return;

        wxGLCanvas::SetCurrent(*_context);
        wxPaintDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event

        if (!_gl_initialized)
        {
            //if (glewInit() != GLEW_OK)
            //{
            //    std::cerr << "GLEW init failed" << std::endl;
            //}

            //OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
            //OpenGL::CContext context;
            //context.Init(debug_level);

            _gl_initialized = true;
        }

        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glFlush();
        SwapBuffers();
    }

    BEGIN_EVENT_TABLE(open_gl_canvas, wxGLCanvas)
    EVT_MOTION(open_gl_canvas::mouse_moved)
    EVT_LEFT_DOWN(open_gl_canvas::mouse_down)
    EVT_LEFT_UP(open_gl_canvas::mouse_released)
    EVT_RIGHT_DOWN(open_gl_canvas::right_click)
    EVT_LEAVE_WINDOW(open_gl_canvas::mouse_left_window)
    EVT_SIZE(open_gl_canvas::resized)
    EVT_KEY_DOWN(open_gl_canvas::key_pressed)
    EVT_KEY_UP(open_gl_canvas::key_released)
    EVT_MOUSEWHEEL(open_gl_canvas::mouse_wheel_moved)
    EVT_PAINT(open_gl_canvas::render)
    END_EVENT_TABLE()

    void open_gl_canvas::mouse_moved(wxMouseEvent& event)
    {}
    
    void open_gl_canvas::mouse_down(wxMouseEvent& event)
    {}
    
    void open_gl_canvas::mouse_wheel_moved(wxMouseEvent& event)
    {}
    
    void open_gl_canvas::mouse_released(wxMouseEvent& event)
    {}
    
    void open_gl_canvas::right_click(wxMouseEvent& event)
    {}
    
    void open_gl_canvas::mouse_left_window(wxMouseEvent& event)
    {}
    
    void open_gl_canvas::key_pressed(wxKeyEvent& event)
    {}
    
    void open_gl_canvas::key_released(wxKeyEvent& event)
    {}
}
