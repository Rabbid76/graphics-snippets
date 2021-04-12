#include <pch.h>

#include <GL/glew.h>
#ifdef __WXMAC__
#include "OpenGL/glu.h"
#include "OpenGL/gl.h"
#else
#include <GL/glu.h>
#include <GL/gl.h>
#endif

#include <wxutil/wx_opengl_canvas.h>

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
    auto gl_pane = new wxutil::open_gl_canvas((wxFrame*)this, args);

    sizer->Add(control_panel, 1, wxEXPAND | wxALL);
    sizer->Add(gl_pane, 1, wxEXPAND | wxALL);

    auto control_text = new wxStaticText(control_panel, wxID_ANY, wxString("controls"), wxPoint(10, 10));
}
