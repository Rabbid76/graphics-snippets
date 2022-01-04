#include <stdafx.h>

// OpenGL
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// freeglut
#include <GLFW/glfw3.h>

// stl
#include <vector>
#include <stdexcept>
#include <chrono>
#include <memory>
#include <cmath>

#define _USE_MATH_DEFINES
#include <math.h>

// Own
#include <OpenGL/OpenGLError.h>
#include <render/Render_IDrawLine.h>
#include <OpenGL/OpenGLLine_1_0.h>
#include <OpenGL/OpenGLLine_2_0.h>
#include <OpenGL/OpenGLLine_core_and_es.h>
#include <OpenGL/OpenGL_Matrix_Camera.h>
#include <OpenGL/OpenGL_SimpleShaderProgram.h>


static int g_size[]{ 800, 600 };
static int g_multisampling = 2;

// [Switching Between windowed and full screen in OpenGL/GLFW 3.2](https://stackoverflow.com/questions/47402766/switching-between-windowed-and-full-screen-in-opengl-glfw-3-2/47462358#47462358)
class CWindow_Glfw
{
public:

    CWindow_Glfw(void);
    virtual ~CWindow_Glfw();

    void Init(int width, int height, int multisampling, bool doubleBuffer);
    void InitDebug(void);
    static void CallbackResize(GLFWwindow* window, int cx, int cy);
    void MainLoop(void);

private:

    void InitScene(void);
    void Render(double time_ms);
    void RenderTestScene(Render::Line::IRender& line_render);

    void SetViewport(int x, int y, int w, int h);
    void SetDepthRange(float n, float f);
    void SetProjection(const float p[]);
    void SetView(const float v[]);
    void SetModel(const float m[]);

    std::array< int, 2 > _wndPos{ 0, 0 };
    std::array< int, 2 > _wndSize{ 0, 0 };
    std::array< int, 2 > _vpSize{ 0, 0 };
    bool                 _updateViewport = true;
    bool                 _doubleBuffer = true;
    GLFWwindow* _wnd = nullptr;
    GLFWmonitor* _monitor = nullptr;

    void Resize(int cx, int cy);

    std::chrono::high_resolution_clock::time_point _start_time;
    std::chrono::high_resolution_clock::time_point _current_time;

    std::unique_ptr<Render::IDebug>              _debug;
    std::unique_ptr<OpenGL::ShaderProgramSimple> _prog;

    std::unique_ptr<Render::Line::IRender> _line_1;
    std::unique_ptr<Render::Line::IRender> _line_2;
    std::unique_ptr<Render::Line::IRender> _line_3;
    std::unique_ptr<Render::Line::IRender> _line_4;

    Render::TModelAndViewPtr _view_data_ptr;
};

int main(int argc, char** argv)
{
    if (glfwInit() == GLFW_FALSE)
        throw std::runtime_error("error initializing glfw");

    // create OpenGL window and make OpenGL context current (`glfwInit` has to be done before).
    CWindow_Glfw window;
    window.Init(g_size[0], g_size[1], g_multisampling, true);

    // OpenGL context needs to be current for `glewInit`
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
        throw std::runtime_error("error initializing glew");
    window.InitDebug();

    std::cout << glGetString(GL_VENDOR) << std::endl;
    std::cout << glGetString(GL_RENDERER) << std::endl;
    std::cout << glGetString(GL_VERSION) << std::endl;
    std::cout << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    GLint major = 0, minor = 0, contex_mask = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &contex_mask);
    std::cout << "context: " << major << "." << minor << " ";
    if (contex_mask & GL_CONTEXT_CORE_PROFILE_BIT)
        std::cout << "core";
    else if (contex_mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
        std::cout << "compatibility";
    if (contex_mask & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
        std::cout << ", forward compatibility";
    if (contex_mask & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT)
        std::cout << ", robust access";
    if (contex_mask & GL_CONTEXT_FLAG_DEBUG_BIT)
        std::cout << ", debug";
    std::cout << std::endl;

    // extensions
    //std::cout << glGetStringi( GL_EXTENSIONS, ... ) << std::endl;

    std::cout << std::endl;

    window.MainLoop();
    return 0;
}


CWindow_Glfw::CWindow_Glfw(void)
{
    _view_data_ptr = std::make_shared<Render::CModelAndView>();
}

CWindow_Glfw::~CWindow_Glfw()
{
    if (_wnd != nullptr)
        glfwDestroyWindow(_wnd);
    glfwTerminate();
}

void CWindow_Glfw::CallbackResize(GLFWwindow* window, int cx, int cy)
{
    void* ptr = glfwGetWindowUserPointer(window);
    if (CWindow_Glfw* wndPtr = static_cast<CWindow_Glfw*>(ptr))
        wndPtr->Resize(cx, cy);
}

void CWindow_Glfw::Init(int width, int height, int multisampling, bool doubleBuffer)
{
    _doubleBuffer = doubleBuffer;

    // [GLFW Window guide; Window creation hints](http://www.glfw.org/docs/latest/window_guide.html#window_hints_values)

    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    glfwWindowHint(GLFW_SAMPLES, multisampling);
    glfwWindowHint(GLFW_DOUBLEBUFFER, _doubleBuffer ? GLFW_TRUE : GLFW_FALSE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    //glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    //glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE );
#if defined(_DEBUG)
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    //GLFW_CONTEXT_ROBUSTNESS 

    _wnd = glfwCreateWindow(width, height, "OGL window", nullptr, nullptr);
    if (_wnd == nullptr)
    {
        glfwTerminate();
        throw std::runtime_error("error initializing window");
    }

    glfwMakeContextCurrent(_wnd);

    glfwSetWindowUserPointer(_wnd, this);
    glfwSetWindowSizeCallback(_wnd, CWindow_Glfw::CallbackResize);

    _monitor = glfwGetPrimaryMonitor();
    glfwGetWindowSize(_wnd, &_wndSize[0], &_wndSize[1]);
    glfwGetWindowPos(_wnd, &_wndPos[0], &_wndPos[1]);
    _updateViewport = true;
}


void CWindow_Glfw::InitDebug(void) // has to be done after GLEW initialization!
{
#if defined(_DEBUG)
    static bool synchromous = true;
    _debug = std::make_unique<OpenGL::CDebug>();
    _debug->Init(Render::TDebugLevel::error_only);
    _debug->Activate(synchromous);
#endif
}


void CWindow_Glfw::Resize(int cx, int cy)
{
    _updateViewport = true;
}


void CWindow_Glfw::SetViewport(int x, int y, int w, int h)
{
    glViewport(x, y, w, h);

    if (_view_data_ptr != nullptr)
        _view_data_ptr->Viewport((float)x, (float)y, (float)w, (float)h);
}


void CWindow_Glfw::SetDepthRange(float n, float f)
{
    if (_view_data_ptr != nullptr)
        _view_data_ptr->DepthRage(n, f);
}


void CWindow_Glfw::SetProjection(const float p[])
{
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(p);

    if (_view_data_ptr != nullptr)
        _view_data_ptr->Projection(p);
}


void CWindow_Glfw::SetView(const float v[])
{
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(v);

    if (_view_data_ptr != nullptr)
        _view_data_ptr->View(v);
}


void CWindow_Glfw::SetModel(const float m[])
{
    glMatrixMode(GL_MODELVIEW);
    glMultMatrixf(m);

    if (_view_data_ptr != nullptr)
        _view_data_ptr->Model(m);
}


void CWindow_Glfw::MainLoop(void)
{
    InitScene();

    _start_time = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(_wnd))
    {
        if (_updateViewport)
        {
            glfwGetFramebufferSize(_wnd, &_vpSize[0], &_vpSize[1]);
            SetViewport(0, 0, _vpSize[0], _vpSize[1]);
            _updateViewport = false;
        }

        _current_time = std::chrono::high_resolution_clock::now();
        auto   delta_time = _current_time - _start_time;
        double time_ms = (double)std::chrono::duration_cast<std::chrono::milliseconds>(delta_time).count();

        Render(time_ms);

        if (_doubleBuffer)
            glfwSwapBuffers(_wnd);
        else
            glFinish();

        glfwPollEvents();
    }
}


void CWindow_Glfw::InitScene(void)
{
    _line_1 = std::make_unique<OpenGL::Line::CLineOpenGL_1_00>();
    _line_1->Init();

    _line_2 = std::make_unique<OpenGL::Line::CLineOpenGL_2_00>(0);
    _line_2->Init();

    auto line_core_forward = std::make_unique<OpenGL::Line::CLinOpenGL_core_forward_compatibility>(0);
    line_core_forward->Init(_view_data_ptr);
    _line_3 = std::move(line_core_forward);

    auto line_core = std::make_unique<OpenGL::Line::CLinOpenGL_core_4>(0);
    line_core->Init(_view_data_ptr);
    _line_4 = std::move(line_core);
}


void CWindow_Glfw::Render(double time_ms)
{
    // Test lines with and without multisampling!

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glm::mat4 proj(1.0f);
    SetProjection(glm::value_ptr(proj));
    SetDepthRange(-1.0f, 1.0f);

    glm::mat4 view(1.0f);
    SetView(glm::value_ptr(view));

    if (_line_1 != nullptr)
    {
        glPushMatrix();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.5f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f));
        SetModel(glm::value_ptr(model));

        RenderTestScene(*_line_1.get());

        glPopMatrix();
    }

    if (_line_2 != nullptr)
    {
        glPushMatrix();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f));
        SetModel(glm::value_ptr(model));

        RenderTestScene(*_line_2.get());

        glPopMatrix();
    }

    if (_line_3 != nullptr)
    {
        glPushMatrix();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f));
        SetModel(glm::value_ptr(model));

        RenderTestScene(*_line_2.get());

        glPopMatrix();
    }

    if (_line_4 != nullptr)
    {
        glPushMatrix();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -0.5f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f));
        SetModel(glm::value_ptr(model));

        RenderTestScene(*_line_2.get());

        glPopMatrix();
    }
}


void CWindow_Glfw::RenderTestScene(Render::Line::IRender& line_render)
{
    line_render.StartSuccessiveLineDrawings();

    line_render.SetStyle({ 8.0f, 2 });
    line_render.SetColor(Render::TColor{ 0.9f, 0.5f, 0.1f, 1.0f });

    static const std::vector<double> line_test_1{ -0.4, -0.5, 0.6, -0.5, 0.6, 0.5 };
    line_render.Draw(Render::TPrimitive::lineloop, 2, line_test_1.size(), line_test_1.data());

    line_render.SetStyle({ 8.0f, 6 });
    line_render.SetColor(Render::TColor{ 0.1f, 0.5f, 0.9f, 1.0f });

    static const std::vector<double> line_x_test_2{ -0.6,  0.4, -0.6 };
    static const std::vector<double> line_y_test_2{ -0.5,  0.5,  0.5 };
    line_render.Draw(Render::TPrimitive::lineloop, line_x_test_2.size(), line_x_test_2.data(), line_y_test_2.data());

    line_render.SetStyle({ 10.0f, 1 });
    line_render.SetColor(Render::TColor{ 0.9f, 0.9f, 0.4f, 1.0f });

    static const std::vector<double> line_test_3{ -0.7, -0.6, 0.7, -0.6, 0.7, 0.6, -0.7, 0.6 };
    line_render.StartSequence(Render::TPrimitive::lineloop, 2);
    line_render.DrawSequence(line_test_3.size(), line_test_3.data());
    line_render.EndSequence();

    line_render.FinishSuccessiveLineDrawings();
}