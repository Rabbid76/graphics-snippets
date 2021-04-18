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
#include <string>
#include <utility>

// project includes
#include <gl/gl_debug.h>
#include <gl/gl_shader.h>
#include <view/view_interface.h>
#include <mesh/mesh_data_interface.h>
#include <mesh/mesh_data_container.h>

// glm
# define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

        SingleMesh(const ::mesh::MeshDataInterface<float, int>& specification);
    };

    class SingleMeshSeparateAttributeFormat
        : public MeshInterface
    {

    };
}

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
    GLuint _shader_storag_buffer_object;
    std::unique_ptr<OpenGL::mesh::MeshInterface> _mesh;

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


namespace OpenGL::mesh
{
    SingleMesh::SingleMesh(const ::mesh::MeshDataInterface<float, int>& definiition)
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
            
        size_t offset = 0;
        for (const auto& [attribute_index, size] : specification)
        {
            glEnableVertexAttribArray(attribute_index);
            glVertexAttribPointer(attribute_index, size, GL_FLOAT, GL_FALSE, 
                attribute_size * sizeof(GLfloat), reinterpret_cast<const void*>(offset * sizeof(GLfloat)));
            offset += size;
        }
    }
}

std::string phong_vert = R"(
#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_nv;
layout(location = 2) in vec3 a_uvw;

out vec3 v_pos;
out vec3 v_nv;
out vec3 v_uvw;

layout(location = 0) uniform mat4 u_proj;
layout(location = 1) uniform mat4 u_view;

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
    gl_Position = u_proj * world_pos;
}
)";

std::string phong_frag = R"(
#version 460 core

out vec4 frag_color;
in  vec3 v_pos;
in  vec3 v_nv;
in  vec3 v_uvw;
layout(location = 1) uniform mat4 u_view;

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

void MyOpenGLView::init(const view::CanvasInterface& canvas)
{
    if (glewInit() != GLEW_OK)
        throw std::runtime_error("GLEW initialization failed");

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
    _context->Init(debug_level);

    _program = OpenGL::CreateProgram(phong_vert, phong_frag);

    std::vector<float> vertices{
                    -0.707f, -0.75f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                     0.707f, -0.75f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f / 6.0f,
                     0.0f,    0.75f, 0.0f, 0.0f, -1.0f, 1.0f, 0.5f, 2.0f / 3.0f,
    };

    std::vector<unsigned int> indices{ 0, 1, 2 };

    std::vector<std::tuple<int, int>> specification{ std::tuple<int, int>(0, 3), std::tuple<int, int>(1, 3), std::tuple<int, int>(2, 3) };

    _mesh = std::make_unique<OpenGL::mesh::SingleMesh>(
        mesh::MeshDataContainer<float, int>::new_mesh(
            {
                -0.866f, -0.75f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                 0.866f, -0.75f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
                 0.0f,    0.75f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.5f, 0.5f,
            },
            { 0, 1, 2 },
            { {0, 3}, {1, 3}, {2, 3} }));
        

    glGenBuffers(1, &_shader_storag_buffer_object);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _shader_storag_buffer_object);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _shader_storag_buffer_object);

    glUseProgram(_program);

    glm::mat4 view(1.0f);
    glProgramUniformMatrix4fv(_program, 1, 1, false, glm::value_ptr(view));

    glm::mat4 model(1.0f);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(model));

    resize(canvas);
}

void MyOpenGLView::resize(const view::CanvasInterface& canvas)
{
    const auto [cx, cy] = canvas.get_size();
    glViewport(0, 0, cx, cy);

    if (_program == 0)
        return;
    
    float aspect = static_cast<float>(cx) / static_cast<float>(cy);
    glm::mat4 projection = aspect < 1.0f
        ? glm::ortho(-1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect, -1.0f, 1.0f)
        : glm::ortho(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);
    glProgramUniformMatrix4fv(_program, 0, 1, false, glm::value_ptr(projection));
}

void MyOpenGLView::render(const view::CanvasInterface& canvas)
{
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 3);
}