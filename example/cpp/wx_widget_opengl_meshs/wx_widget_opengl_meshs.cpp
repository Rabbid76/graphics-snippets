#include <pch.h>

#include <wxutil/wx_include.h>
#include <wxutil/wx_opengl_canvas.h>
#include <wxutil/wx_control_factory.h>
#include <gl/opengl_include.h>
#include <gl/gl_debug.h>
#include <gl/gl_shader.h>
#include <view/view_interface.h>
#include <mesh/mesh_include.h>
#include <gl/opengl_mesh_interface.h>
#include <gl/opengl_mesh_vector.h>
#include <gl/opengl_mesh_single.h>
#include <gl/opengl_mesh_single_separate_attribute.h>
#include <controls/spinning_controls.h>
#include <math/glm_include.h>

#include <memory>
#include <vector>
#include <numeric>
#include <tuple>
#include <string>
#include <utility>

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

class MyOpenGLView;

class MyFrame : public wxFrame
{
public:
    MyFrame();

    void shape_changed(wxCommandEvent& event);
    void polygon_mode_changed(wxCommandEvent& event);
    void face_culling_changed(wxCommandEvent& event);

private:

    // wxWindows don't need to be deleted. See [Window Deletion](https://docs.wxwidgets.org/3.0/overview_windowdeletion.html)
    wxPanel *_control_panel;
    wxutil::OpenGLCanvas*_view_panel;
    std::shared_ptr<MyOpenGLView> _view;
};


class MyOpenGLView
    : public view::ViewInterface
    , public controls::ControlsViewInterface
    , public view::MouseEventInterface
{
private:

    const std::unique_ptr<OpenGL::CContext> _context;
    GLuint _program = 0;
    GLuint _shader_storag_buffer_object = 0;
    OpenGL::mesh::MeshVector _meshs;
    GLfloat _angle1 = 0.0f;
    GLfloat _angle2 = 0.0f;
    int _selected_shape = 0;
    int _selected_polygon_mode = 2;
    int _selected_culling_mode = 0;
    controls::TViewportRectangle _viewport_rectangle{ 0, 0, 0, 0 };
    std::unique_ptr<controls::ControlsInterface> _controls;
    std::chrono::high_resolution_clock::time_point _start_time;
    glm::mat4 _view_matrix{ glm::mat4(1.0f) };
    glm::mat4 _projection_matrix{ glm::mat4(1.0f) };

public:

    MyOpenGLView();
    virtual ~MyOpenGLView();

    virtual void init(const view::CanvasInterface& canvas) override;
    virtual void resize(const view::CanvasInterface& canvas) override;
    virtual void render(const view::CanvasInterface& canvas) override;

    virtual const controls::TViewportRectangle& get_viewport_rectangle(void) const override
    {
        return _viewport_rectangle;
    }

    virtual const double get_time(void) const override
    {
        auto current_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = current_time - _start_time;
        return diff.count();
    }

    virtual const glm::mat4 get_view_matrix(void) const override
    {
        return _view_matrix;
    };

    virtual const glm::mat4 get_inverse_view_matrix(void) const override
    {
        return glm::inverse(_view_matrix);
    };

    virtual const glm::mat4 get_projection_matrix(void) const override
    {
        return _projection_matrix;
    };
    virtual const glm::mat4 get_inverse_projection_matrix(void) const override
    {
        return glm::inverse(_projection_matrix);
    };

    virtual void mouse_motion(int x, int y) const override
    {
        glm::vec2 position(static_cast<float>(x), static_cast<float>(y));
        _controls->drag(position);
    }

    virtual void mouse_action(int x, int y, view::MouseButton button, view::MouseAction action) const override
    {
        glm::vec2 position(static_cast<float>(x), static_cast<float>(y));

        switch (button)
        {
            case view::MouseButton::LEFT:
                switch (action)
                {
                    case view::MouseAction::PRESS:
                        _controls->start_drag(position);
                        break;
                    case view::MouseAction::RELEASE:
                        _controls->end_drag(position);
                        break;
                }
                break;
        }
    }

    void select_shape(int shape)
    {
        _selected_shape = shape;
    }

    void select_polygon_mode(int polygon_mode)
    {
        _selected_polygon_mode = polygon_mode;
    }

    void select_culling_mode(int culling_mode)
    {
        _selected_culling_mode = culling_mode;
    }
};

// SubSystem Windows (/SUBSYSTEM:WINDOWS)
wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    MyFrame* frame = new MyFrame();
    frame->Show(true);
    return true;
}

// Platonic solid https://en.wikipedia.org/wiki/Platonic_solid
MyFrame::MyFrame()
    : wxFrame(NULL, wxID_ANY, "OpenGL view", wxDefaultPosition, wxSize(600, 400))
{
    _view = std::make_shared< MyOpenGLView>();

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

    _view_panel = wxutil::OpenGLCanvas::new_gl_canvas(_view, this, 24, 8, 8, 4, 6, true, true, true);

    sizer->Add(_control_panel, 1, wxEXPAND | wxALL);
    sizer->Add(_view_panel, 1, wxEXPAND | wxALL);

    auto controls_sizer = new wxBoxSizer(wxVERTICAL);
    _control_panel->SetSizer(controls_sizer);
    _control_panel->SetAutoLayout(true);

    auto shape_text = wx_utility::new_static_text(_control_panel, wxID_ANY, controls_sizer, wxT("Shape"));
    auto shape_names = std::vector<std::wstring>
    {
       wxT("Tetrahedron"),
       wxT("Hexahedron"),
       wxT("Octahedron"),
       wxT("Dodecahedron"),
       wxT("Icosahedron"),
    };
    auto shape_selection = wx_utility::new_selction_box(
        _control_panel, wxID_ANY, controls_sizer, std::move(shape_names), 0, this, &MyFrame::shape_changed);
    
    auto polygone_mode_text = wx_utility::new_static_text(_control_panel, wxID_ANY, controls_sizer, wxT("Polygon mode"));
    auto polygon_mode_names = std::vector<std::wstring>
    {
       wxT("Point"),
       wxT("Line"),
       wxT("Fill"),
    };
    auto polygon_mode_selection = wx_utility::new_selction_box(
        _control_panel, wxID_ANY, controls_sizer, std::move(polygon_mode_names), 2, this, &MyFrame::polygon_mode_changed);

    auto culling_mode_text = wx_utility::new_static_text(_control_panel, wxID_ANY, controls_sizer, wxT("Culling mode"));
    auto culling_mode_names = std::vector<std::wstring>
    {
       wxT("None"),
       wxT("Back face culling"),
       wxT("Front face culling"),
    };
    auto culling_mode_selection = wx_utility::new_selction_box(
        _control_panel, wxID_ANY, controls_sizer, std::move(culling_mode_names), 0, this, &MyFrame::face_culling_changed);

    _view_panel->add_mouse_event_client(_view.get());
}

void MyFrame::shape_changed(wxCommandEvent& event)
{
    _view->select_shape(event.GetInt());
}

void MyFrame::polygon_mode_changed(wxCommandEvent& event)
{
    _view->select_polygon_mode(event.GetInt());
}

void MyFrame::face_culling_changed(wxCommandEvent& event)
{
    _view->select_culling_mode(event.GetInt());
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
    gl_Position = u_proj * u_view * world_pos;
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
    , _controls{ std::make_unique<controls::SpinningControls>(*this) }
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

    auto tetrahedron_mesh_data = mesh::MeshDefinitonTetrahedron<float, unsigned int>(1.0f).generate_mesh_data();
    auto hexahedron_mesh_data = mesh::MeshDefinitonHexahedron<float, unsigned int>(1.0f).generate_mesh_data();
    auto octahedron_mesh_data = mesh::MeshDefinitonOctahedron<float, unsigned int>(1.0f).generate_mesh_data();
    auto dodecahedron_mesh_data = mesh::MeshDefinitonDodecahedron<float, unsigned int>(1.0f).generate_mesh_data();
    auto icosahedron_mesh_data = mesh::MeshDefinitonIcosahedron<float, unsigned int>(1.0f).generate_mesh_data();
    _meshs = OpenGL::mesh::MeshVector(std::vector<std::shared_ptr<OpenGL::mesh::MeshInterface>>
    {
        std::make_shared<OpenGL::mesh::SingleMeshSeparateAttributeFormat>(*tetrahedron_mesh_data),
        std::make_shared<OpenGL::mesh::SingleMeshSeparateAttributeFormat>(*hexahedron_mesh_data),
        std::make_shared<OpenGL::mesh::SingleMeshSeparateAttributeFormat>(*octahedron_mesh_data),
        std::make_shared<OpenGL::mesh::SingleMeshSeparateAttributeFormat>(*dodecahedron_mesh_data),
        std::make_shared<OpenGL::mesh::SingleMeshSeparateAttributeFormat>(*icosahedron_mesh_data),
    });

    glGenBuffers(1, &_shader_storag_buffer_object);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _shader_storag_buffer_object);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _shader_storag_buffer_object);

    glUseProgram(_program);

    _view_matrix = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0), glm::vec3(0, 1, 0));
    glProgramUniformMatrix4fv(_program, 1, 1, false, glm::value_ptr(_view_matrix));

    resize(canvas);
    _start_time = std::chrono::high_resolution_clock::now();
    _controls->set_attenution(1.0f, 0.05f, 0.0f);
}

void MyOpenGLView::resize(const view::CanvasInterface& canvas)
{
    const auto [cx, cy] = canvas.get_size();
    _viewport_rectangle = { 0, 0, static_cast<float>(cx), static_cast<float>(cy) };
    glViewport(0, 0, cx, cy);


    if (_program == 0)
        return;
    
    float aspect = static_cast<float>(cx) / static_cast<float>(cy);
    _projection_matrix = glm::perspective(glm::radians(30.0f), aspect, 0.01f, 10.0f);
    glProgramUniformMatrix4fv(_program, 0, 1, false, glm::value_ptr(_projection_matrix));
}

void MyOpenGLView::render(const view::CanvasInterface& canvas)
{
    auto model_matrix = _controls->update();

    glEnable(GL_DEPTH_TEST);
    if (_selected_culling_mode == 0)
    {
        glDisable(GL_CULL_FACE);
    }
    else
    {
        glEnable(GL_CULL_FACE);
        glCullFace(_selected_culling_mode == 1 ? GL_BACK : GL_FRONT);
        glFrontFace(GL_CCW);
    }

    GLenum polygon_mode = _selected_polygon_mode == 0
        ? GL_POINT
        : _selected_polygon_mode == 1 ? GL_LINE : GL_FILL;
    glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 model(1.0f);
    model = glm::rotate(model, _angle1, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, _angle2, glm::vec3(0.0f, 1.0f, 0.0f));
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(model_matrix));
    _angle1 += 0.02f;
    _angle2 += 0.01f;

    _meshs.draw(_selected_shape, _selected_shape+1);
}