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
#include <view/view_interface.h>

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
    wxPanel *_control_panel;
    wxutil::opengl_canvas*_view_panel;
};

class MyOpenGLView
    : public view::view_interface
{
private:

    const std::unique_ptr<OpenGL::CContext> _context;
    GLuint _program = 0;

public:

    MyOpenGLView();
    virtual ~MyOpenGLView();

    virtual void init(const view::canvas_interface& canvas) override;
    virtual void resize(const view::canvas_interface& canvas) override;
    virtual void render(const view::canvas_interface& canvas) override;
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
    auto view = std::make_shared< MyOpenGLView>();

    CreateStatusBar();
    SetStatusText("Status");

    wxFlexGridSizer* sizer = new wxFlexGridSizer(1, 2, 0, 0);
    SetSizer(sizer);
    SetAutoLayout(true);

    sizer->SetFlexibleDirection(wxBOTH);
    sizer->AddGrowableCol(1);
    sizer->AddGrowableRow(0);

    _control_panel = new wxPanel(this, wxID_ANY, wxPoint(0, 0), wxSize(100, -1));
    _control_panel->SetBackgroundColour(wxColour(255, 255, 128));

    int args[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0 };
    _view_panel = new wxutil::opengl_canvas(view, this, args);

    sizer->Add(_control_panel, 1, wxEXPAND | wxALL);
    sizer->Add(_view_panel, 1, wxEXPAND | wxALL);

    auto control_text = new wxStaticText(_control_panel, wxID_ANY, wxString("controls"), wxPoint(10, 10));
}


std::string phong_vert = R"(
#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_nv;
layout(location = 2) in vec3 a_uvw;

out vec3 v_pos;
out vec3 v_nv;
out vec3 v_uvw;

uniform mat4 u_proj;
uniform mat4 u_view;

layout(std430, binding = 1) buffer Draw
{
    mat4 model[];
} draw;

void main()
{
    mat4 model = draw.model[gl_DrawID];
    mat3 normal = transpose(inverse(mat3(model)));

    vec4 world_pos = model * vec4(a_pos.xyz, 1.0);

    v_pos = world_pos.xyz;
    v_nv = normal * a_nv;
    v_uvw = a_uvw;
    gl_Position = u_proj * u_view * world_pos;
}
)";

std::string phong_frag = R"(
#version 460 core

out vec4 frag_color;
in  vec3 v_pos;
in  vec3 v_nv;
in  vec3 v_uvw;
uniform mat4 u_view;

vec3 HUEtoRGB(in float H)
{
    float R = abs(H * 6.0 - 3.0) - 1.0;
    float G = 2.0 - abs(H * 6.0 - 2.0);
    float B = 2.0 - abs(H * 6.0 - 4.0);
    return clamp(vec3(R, G, B), 0.0, 1.0);
}

void main()
{
    vec4  color = vec4(HUEtoRGB(v_uvw.z), 1.0);
    vec3  L = normalize(vec3(1.0, -1.0, 1.0));
    vec3  eye = inverse(u_view)[3].xyz;
    vec3  V = normalize(eye - v_pos);
    float face = sign(dot(v_nv, V));
    vec3  N = normalize(v_nv) * face;
    vec3  H = normalize(V + L);
    float ka = 0.5;
    float kd = max(0.0, dot(N, L)) * 0.5;
    float NdotH = max(0.0, dot(N, H));
    float sh = 100.0;
    float ks = pow(NdotH, sh) * 0.1;
    frag_color = vec4(color.rgb * (ka + kd + ks), color.a);
}
)";


MyOpenGLView::MyOpenGLView()
    : _context{ std::make_unique<OpenGL::CContext>() }
{}

MyOpenGLView::~MyOpenGLView()
{}

void MyOpenGLView::init(const view::canvas_interface& canvas)
{
    if (glewInit() != GLEW_OK)
        throw std::runtime_error("GLEW initialization failed");

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
    _context->Init(debug_level);

    _program = OpenGL::CreateProgram(phong_vert, phong_frag);
}

void MyOpenGLView::resize(const view::canvas_interface& canvas)
{
    const auto [cx, cy] = canvas.get_size();
    glViewport(0, 0, cx, cy);
}

void MyOpenGLView::render(const view::canvas_interface& canvas)
{
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // [...]
}