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
#include <vector>
#include <numeric>
#include <tuple>

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
    wxutil::OpenGLCanvas*_view_panel;
};

class MyOpenGLView
    : public view::ViewInterface
{
private:

    const std::unique_ptr<OpenGL::CContext> _context;
    GLuint _program = 0;

public:

    MyOpenGLView();
    virtual ~MyOpenGLView();

    virtual void init(const view::CanvasInterface& canvas) override;
    virtual void resize(const view::CanvasInterface& canvas) override;
    virtual void render(const view::CanvasInterface& canvas) override;
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
    _view_panel = new wxutil::OpenGLCanvas(view, this, args);

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

namespace mesh
{
    template <class T_VERTEX = float>
    using VertexAttributes = std::tuple<size_t, const T_VERTEX*>;

    template <class T_INDEX = unsigned int>
    using Indices = std::tuple<size_t, const T_INDEX*>;

    template <class T_VERTEX = float, class T_INDEX = unsigned int>
    using VertexSpcification = std::vector<std::tuple<int, int>>;

    template <class T_VERTEX = float, class T_INDEX = int>
    class MeshDefinitionInterface
    {
    public:

        virtual const VertexAttributes<T_VERTEX> get_vertex_attributes(void) const = 0;
        virtual const Indices<T_INDEX> get_indices(void) const = 0;
        virtual const VertexSpcification<T_VERTEX, T_INDEX> get_specification(void) const = 0;
    };

    template <class T_VERTEX=float, class T_INDEX=int>
    class MeshDefinition
        : public MeshDefinitionInterface<T_VERTEX, T_INDEX>
    {
    private:

        std::vector<T_VERTEX> _vertex_attributes;
        std::vector<T_INDEX> _indices;
        std::vector<std::tuple<int, int>> _specification;

    public:

        virtual const VertexAttributes<T_VERTEX> get_vertex_attributes(void) const
        { 
            return std::make_tuple<size_t, const T_VERTEX*>(_vertex_attributes.size(), _vertex_attributes.data());
        }

        virtual const Indices<T_INDEX> get_indices(void) const
        {
            return std::make_tuple<size_t, const T_INDEX*>(_indices.size(), _indices.data());
        }

        virtual const VertexSpcification<T_VERTEX, T_INDEX> get_specification(void) const
        { 
            return _specification; 
        }
    };
}

namespace OpenGL::mesh
{
    class MeshInterface
    {
       // destroy
       // draw
    };

    class MultiMeshInterface
    {
        // darw_range
    };

    class SingleMesh 
        : public MeshInterface
    {
    private:

        GLuint _vertex_array_object;
        GLuint _vertex_buffer_object;
        GLuint _index_buffer_object;

    public:

        SingleMesh(const ::mesh::MeshDefinition<float, int>& specification);
    };

    class SingleMeshSeparateAttributeFormat
        : public MeshInterface
    {

    };
}

namespace OpenGL::mesh
{
    SingleMesh::SingleMesh(const ::mesh::MeshDefinition<float, int>& definiition)
    {
        auto [no_of_values, vertex_array] = definiition.get_vertex_attributes();
        auto [no_of_indices, index_array] = definiition.get_indices();
        auto specification = definiition.get_specification();
        
        glCreateVertexArrays(1, &_vertex_array_object);
        glBindVertexArray(_vertex_array_object);

        GLuint buffer_objects[2];
        glGenBuffers(2, buffer_objects);

        _index_buffer_object = buffer_objects[0];
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _index_buffer_object);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, no_of_indices * sizeof(GLuint), index_array, GL_STATIC_DRAW);

        _vertex_buffer_object = buffer_objects[1];
        glBindBuffer(GL_ARRAY_BUFFER, _vertex_buffer_object);
        glBufferData(GL_ARRAY_BUFFER, no_of_values*sizeof(GLfloat), vertex_array, GL_STATIC_DRAW);

        auto attribute_size = std::accumulate(specification.begin(), specification.end(), 0, [](auto &&a, const auto &b) -> int
            {
                return std::move(a) + std::get<1>(b);
            });
        for (const auto& [attribute_index, size] : specification)
        {
            glEnableVertexAttribArray(attribute_index);
            glVertexAttribPointer(attribute_index, size, GL_FLOAT, GL_FALSE, 
                attribute_size * sizeof(GLfloat), reinterpret_cast<const void*>(size * sizeof(GLfloat)));
        }
    }
}

MyOpenGLView::MyOpenGLView()
    : _context{ std::make_unique<OpenGL::CContext>() }
{}

MyOpenGLView::~MyOpenGLView()
{}

void MyOpenGLView::init(const view::CanvasInterface& canvas)
{
    if (glewInit() != GLEW_OK)
        throw std::runtime_error("GLEW initialization failed");

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
    _context->Init(debug_level);

    _program = OpenGL::CreateProgram(phong_vert, phong_frag);
}

void MyOpenGLView::resize(const view::CanvasInterface& canvas)
{
    const auto [cx, cy] = canvas.get_size();
    glViewport(0, 0, cx, cy);
}

void MyOpenGLView::render(const view::CanvasInterface& canvas)
{
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // [...]
}