#include <pch.h>

#include <GL/glew.h>
#ifdef __WXMAC__
#include "OpenGL/glu.h"
#include "OpenGL/gl.h"
#else
#include <GL/glu.h>
#include <GL/gl.h>
#endif

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#define _CRT_SECURE_NO_WARNINGS
#include <wx/wx.h>
#undef _CRT_SECURE_NO_WARNINGS
#endif
#include "wx/glcanvas.h" 

#include <memory>

// project includes
#include <gl/gl_debug.h>
#include <gl/gl_shader.h>

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

class MyFrame : public wxFrame
{
public:
    MyFrame();

private:

    // wxWindows don't need to be deleted. See [Window Deletion](https://docs.wxwidgets.org/3.0/overview_windowdeletion.html)
    wxPanel* control_panel;
    wxPanel* view_panel;
};


// [xGLCanvas](https://wiki.wxwidgets.org/WxGLCanvas)
// [Hello Triangle : OpenGL with wxWidgets](https://computingonplains.wordpress.com/hello-triangle-opengl-with-wxwidgets/)
class OpenGLCanvas : public wxGLCanvas
{
private:

    wxGLContext* _context;
    bool _gl_initialized = false;

public:
    OpenGLCanvas(wxFrame* parent, int* args);
    virtual ~OpenGLCanvas();

    void resized(wxSizeEvent& evt);

    int getWidth();
    int getHeight();

    void render(wxPaintEvent& evt);
    void prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
    void prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);

    // events
    void mouseMoved(wxMouseEvent& event);
    void mouseDown(wxMouseEvent& event);
    void mouseWheelMoved(wxMouseEvent& event);
    void mouseReleased(wxMouseEvent& event);
    void rightClick(wxMouseEvent& event);
    void mouseLeftWindow(wxMouseEvent& event);
    void keyPressed(wxKeyEvent& event);
    void keyReleased(wxKeyEvent& event);

    DECLARE_EVENT_TABLE()
};

// SubSystem Windows (/SUBSYSTEM:WINDOWS)
wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    MyFrame* frame = new MyFrame();
    frame->Show(true);
    return true;
}

MyFrame::MyFrame()
    : wxFrame(NULL, wxID_ANY, "OpenGL view", wxDefaultPosition, wxSize(600, 400))
{
    CreateStatusBar();
    SetStatusText("Status");

    wxFlexGridSizer* sizer = new wxFlexGridSizer(1, 2, 0, 0);
    SetSizer(sizer);
    SetAutoLayout(true);

    sizer->SetFlexibleDirection(wxBOTH);
    sizer->AddGrowableCol(1);
    sizer->AddGrowableRow(0);

    control_panel = new wxPanel(this, wxID_ANY, wxPoint(0, 0), wxSize(100, -1));
    control_panel->SetBackgroundColour(wxColour(255, 255, 128));

    int args[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0 };
    auto gl_pane = new OpenGLCanvas((wxFrame*)this, args);

    sizer->Add(control_panel, 1, wxEXPAND | wxALL);
    sizer->Add(gl_pane, 1, wxEXPAND | wxALL);

    auto control_text = new wxStaticText(control_panel, wxID_ANY, wxString("controls"), wxPoint(10, 10));
}


BEGIN_EVENT_TABLE(OpenGLCanvas, wxGLCanvas)
EVT_MOTION(OpenGLCanvas::mouseMoved)
EVT_LEFT_DOWN(OpenGLCanvas::mouseDown)
EVT_LEFT_UP(OpenGLCanvas::mouseReleased)
EVT_RIGHT_DOWN(OpenGLCanvas::rightClick)
EVT_LEAVE_WINDOW(OpenGLCanvas::mouseLeftWindow)
EVT_SIZE(OpenGLCanvas::resized)
EVT_KEY_DOWN(OpenGLCanvas::keyPressed)
EVT_KEY_UP(OpenGLCanvas::keyReleased)
EVT_MOUSEWHEEL(OpenGLCanvas::mouseWheelMoved)
EVT_PAINT(OpenGLCanvas::render)
END_EVENT_TABLE()


// some useful events to use
void OpenGLCanvas::mouseMoved(wxMouseEvent& event) {}
void OpenGLCanvas::mouseDown(wxMouseEvent& event) {}
void OpenGLCanvas::mouseWheelMoved(wxMouseEvent& event) {}
void OpenGLCanvas::mouseReleased(wxMouseEvent& event) {}
void OpenGLCanvas::rightClick(wxMouseEvent& event) {}
void OpenGLCanvas::mouseLeftWindow(wxMouseEvent& event) {}
void OpenGLCanvas::keyPressed(wxKeyEvent& event) {}
void OpenGLCanvas::keyReleased(wxKeyEvent& event) {}

// Vertices and faces of a simple cube to demonstrate 3D render
// source: http://www.opengl.org/resources/code/samples/glut_examples/examples/cube.c
GLfloat v[8][3];
GLint faces[6][4] = {  /* Vertex indices for the 6 faces of a cube. */
    {0, 1, 2, 3}, {3, 2, 6, 7}, {7, 6, 5, 4},
    {4, 5, 1, 0}, {5, 6, 2, 1}, {7, 4, 0, 3} };



OpenGLCanvas::OpenGLCanvas(wxFrame* parent, int* args) :
    wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
    _context = new wxGLContext(this);
    // prepare a simple cube to demonstrate 3D render
    // source: http://www.opengl.org/resources/code/samples/glut_examples/examples/cube.c
    v[0][0] = v[1][0] = v[2][0] = v[3][0] = -1;
    v[4][0] = v[5][0] = v[6][0] = v[7][0] = 1;
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = -1;
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = 1;
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = 1;
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = -1;

    // To avoid flashing on MSW
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

OpenGLCanvas::~OpenGLCanvas()
{
    delete _context;
}

void OpenGLCanvas::resized(wxSizeEvent& evt)
{
    //	wxGLCanvas::OnSize(evt);

    Refresh();
}

/** Inits the OpenGL viewport for drawing in 3D. */
void OpenGLCanvas::prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glClearDepth(1.0f);	// Depth Buffer Setup
    glEnable(GL_DEPTH_TEST); // Enables Depth Testing
    glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glEnable(GL_COLOR_MATERIAL);

    glViewport(topleft_x, topleft_y, bottomrigth_x - topleft_x, bottomrigth_y - topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float ratio_w_h = (float)(bottomrigth_x - topleft_x) / (float)(bottomrigth_y - topleft_y);
    gluPerspective(45 /*view angle*/, ratio_w_h, 0.1 /*clip close*/, 200 /*clip far*/);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

/** Inits the OpenGL viewport for drawing in 2D. */
void OpenGLCanvas::prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glEnable(GL_TEXTURE_2D);   // textures
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(topleft_x, topleft_y, bottomrigth_x - topleft_x, bottomrigth_y - topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(topleft_x, bottomrigth_x, bottomrigth_y, topleft_y);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int OpenGLCanvas::getWidth()
{
    return GetSize().x;
}

int OpenGLCanvas::getHeight()
{
    return GetSize().y;
}


void OpenGLCanvas::render(wxPaintEvent& evt)
{
    if (!IsShown()) return;

    wxGLCanvas::SetCurrent(*_context);
    wxPaintDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event

    if (!_gl_initialized)
    {
        if (glewInit() != GLEW_OK)
        {
            std::cerr << "GLEW init failed" << std::endl;
        }

        OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
        OpenGL::CContext context;
        context.Init(debug_level);

        _gl_initialized = true;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ------------- draw some 2D ----------------
    prepare2DViewport(0, 0, getWidth() / 2, getHeight());
    glLoadIdentity();

    // white background
    glColor4f(1, 1, 1, 1);
    glBegin(GL_QUADS);
    glVertex3f(0, 0, 0);
    glVertex3f(getWidth(), 0, 0);
    glVertex3f(getWidth(), getHeight(), 0);
    glVertex3f(0, getHeight(), 0);
    glEnd();

    // red square
    glColor4f(1, 0, 0, 1);
    glBegin(GL_QUADS);
    glVertex3f(getWidth() / 8, getHeight() / 3, 0);
    glVertex3f(getWidth() * 3 / 8, getHeight() / 3, 0);
    glVertex3f(getWidth() * 3 / 8, getHeight() * 2 / 3, 0);
    glVertex3f(getWidth() / 8, getHeight() * 2 / 3, 0);
    glEnd();

    // ------------- draw some 3D ----------------
    prepare3DViewport(getWidth() / 2, 0, getWidth(), getHeight());
    glLoadIdentity();

    glColor4f(0, 0, 1, 1);
    glTranslatef(0, 0, -5);
    glRotatef(50.0f, 0.0f, 1.0f, 0.0f);

    glColor4f(1, 0, 0, 1);
    for (int i = 0; i < 6; i++)
    {
        glBegin(GL_LINE_STRIP);
        glVertex3fv(&v[faces[i][0]][0]);
        glVertex3fv(&v[faces[i][1]][0]);
        glVertex3fv(&v[faces[i][2]][0]);
        glVertex3fv(&v[faces[i][3]][0]);
        glVertex3fv(&v[faces[i][0]][0]);
        glEnd();
    }

    glFlush();
    SwapBuffers();
}