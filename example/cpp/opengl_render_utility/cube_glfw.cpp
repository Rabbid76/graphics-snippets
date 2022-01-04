#include <stdafx.h>

// OpenGL
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// FreeGLUT
#include <GLFW/glfw3.h>

// STL
#include <vector>
#include <stdexcept>
#include <chrono>
#include <memory>
#include <cmath>

#define _USE_MATH_DEFINES
#include <math.h>

// Own
#include <OpenGL/OpenGL_Matrix_Camera.h>
#include <OpenGL/OpenGL_SimpleShaderProgram_temp.h>
#include <OpenGL/OpenGLError.h>
#include <util/ModelControl.h>
#include <render/Render_GLSL.h>
#include <render/Render_GLM.h>


struct TUB_MVP
{
    Render::GLSL::mat4 _projection;
    Render::GLSL::mat4 _view;
    Render::GLSL::mat4 _model;
};

// [Switching Between windowed and full screen in OpenGL/GLFW 3.2](https://stackoverflow.com/questions/47402766/switching-between-windowed-and-full-screen-in-opengl-glfw-3-2/47462358#47462358)
class CWindow_Glfw
{
public:

    virtual ~CWindow_Glfw();

    void Init(int width, int height, int multisampling, bool doubleBuffer);
    void InitDebug(void);
    static void CallbackResize(GLFWwindow* window, int cx, int cy);
    static void CallbackMouseButton(GLFWwindow* window, int button, int action, int mode);
    static void CallbackCursorPos(GLFWwindow* window, double x, double y);
    void MainLoop(void);

    void Resize(int cx, int cy);
    void MouseButton(int button, int action, int mode);
    void CursorPos(double x, double y);

    std::chrono::high_resolution_clock::time_point _start_time;
    std::chrono::high_resolution_clock::time_point _current_time;

    std::unique_ptr<CModelControl>         _model_control;
    std::unique_ptr<Render::IDebug>        _debug;
    std::unique_ptr<OpenGL::ShaderProgram> _prog;

    void InitScene(void);
    void Render(double time_ms);

private:

    std::array< int, 2 > _wndPos{ 0, 0 };
    std::array< int, 2 > _wndSize{ 0, 0 };
    std::array< int, 2 > _vpSize{ 0, 0 };
    bool                 _updateViewport = true;
    bool                 _doubleBuffer = true;
    GLFWwindow* _wnd = nullptr;
    GLFWmonitor* _monitor = nullptr;

    GLuint  _ubo_mvp = 0;
    TUB_MVP _ubo_mvp_data;
};

int main(int argc, char** argv)
{
    if (glfwInit() == GLFW_FALSE)
        throw std::runtime_error("error initializing GLFW");

    // create OpenGL window and make OpenGL context current (`glfwInit` has to be done before).
    CWindow_Glfw window;
    window.Init(800, 600, 4, true);

    // OpenGL context needs to be current for `glewInit`
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
        throw std::runtime_error("error initializing GLEW");
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


CWindow_Glfw::~CWindow_Glfw()
{
    _model_control.reset(nullptr);
    _debug.reset(nullptr);

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


void CWindow_Glfw::CallbackMouseButton(GLFWwindow* window, int button, int action, int mode)
{
    void* ptr = glfwGetWindowUserPointer(window);
    if (CWindow_Glfw* wndPtr = static_cast<CWindow_Glfw*>(ptr))
        wndPtr->MouseButton(button, action, mode);
}


void CWindow_Glfw::CallbackCursorPos(GLFWwindow* window, double x, double y)
{
    void* ptr = glfwGetWindowUserPointer(window);
    if (CWindow_Glfw* wndPtr = static_cast<CWindow_Glfw*>(ptr))
        wndPtr->CursorPos(x, y);
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
    //glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE );
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#if defined(_DEBUG)
    //glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
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
    glfwSetMouseButtonCallback(_wnd, CWindow_Glfw::CallbackMouseButton);
    glfwSetCursorPosCallback(_wnd, CWindow_Glfw::CallbackCursorPos);

    _monitor = glfwGetPrimaryMonitor();
    glfwGetWindowSize(_wnd, &_wndSize[0], &_wndSize[1]);
    glfwGetWindowPos(_wnd, &_wndPos[0], &_wndPos[1]);
    _updateViewport = true;
}


void CWindow_Glfw::Resize(int cx, int cy)
{
    _updateViewport = true;
}


void CWindow_Glfw::MouseButton(int button, int action, int mode)
{
    if (_model_control == nullptr)
        return;

    double x, y;
    glfwGetCursorPos(_wnd, &x, &y);

    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
        if (action == GLFW_PRESS)
            _model_control->StartRotate({ (int)x, (int)y });
        else if (action == GLFW_RELEASE)
            _model_control->FinishRotate({ (int)x, (int)y });
        break;

    case GLFW_MOUSE_BUTTON_RIGHT:
        if (action == GLFW_RELEASE)
            _model_control->ToogleRotate();
        break;
    }
}


void CWindow_Glfw::CursorPos(double x, double y)
{
    if (_model_control == nullptr)
        return;
    _model_control->UpdatePosition({ (int)x, (int)y });
}


void CWindow_Glfw::MainLoop(void)
{
    InitScene();

    _start_time = std::chrono::high_resolution_clock::now();
    _model_control = std::make_unique<CModelControl>();
    _model_control->Init();

    while (!glfwWindowShouldClose(_wnd))
    {
        if (_updateViewport)
        {
            glfwGetFramebufferSize(_wnd, &_vpSize[0], &_vpSize[1]);
            glViewport(0, 0, _vpSize[0], _vpSize[1]);
            _model_control->VpSize(_vpSize);
            _updateViewport = false;
        }

        static std::array<float, 3>attenuation{ 1.0f, 0.05f, 0.0f };
        //static std::array<float, 3>attenuation{ 1.0f, 0.1f, 0.1f };
        //static std::array<float, 3>attenuation{ 1.0f, 0.05f, 0.0001f };
        _model_control->Attenuation(attenuation);

        _current_time = std::chrono::high_resolution_clock::now();
        auto   delta_time = _current_time - _start_time;
        double time_ms = (double)std::chrono::duration_cast<std::chrono::milliseconds>(delta_time).count();

        _model_control->Update();

        Render(time_ms);

        if (_doubleBuffer)
            glfwSwapBuffers(_wnd);
        else
            glFinish();

        glfwPollEvents();
    }
}


std::string sh_vert = R"(
#version 460 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;

out vec3 vertPos;
out vec4 vertCol;

layout (std140, binding = 1) uniform UB_MVP
{ 
    mat4 u_projection;
    mat4 u_view;
    mat4 u_model;
};

void main()
{
    mat4 model_view = u_view * u_model;
    vec4 vertx_pos  = model_view * vec4(inPos, 1.0);

    vertCol     = inColor;
		vertPos     = vertx_pos.xyz;
		gl_Position = u_projection * vertx_pos;
}
)";

std::string sh_frag = R"(
#version 460 core

in vec3 vertPos;
in vec4 vertCol;

out vec4 fragColor;

void main()
{
    fragColor  = vertCol;
}
)";


void CWindow_Glfw::InitScene(void)
{
    _prog.reset(new OpenGL::ShaderProgram(
        {
          { sh_vert, GL_VERTEX_SHADER },
          { sh_frag, GL_FRAGMENT_SHADER }
        }));

    // to do mesh buffer creator form multi indices mesh 
    // use in-source wave font file?

    static const std::vector<float> varray
    {
        // front
        -1.0f,  -1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,
         1.0f,  -1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,
         1.0f,  -1.0f,  1.0f,    1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f,  -1.0f,  1.0f,    1.0f, 0.0f, 0.0f, 1.0f,

        // back
         1.0f,   1.0f, -1.0f,    0.0f, 1.0f, 0.0f, 1.0f,
        -1.0f,   1.0f, -1.0f,    0.0f, 1.0f, 0.0f, 1.0f,
        -1.0f,   1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,
         1.0f,   1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,

         // left
        -1.0f,   1.0f, -1.0f,    0.0f, 0.0f, 1.0f, 1.0f,
        -1.0f,  -1.0f, -1.0f,    0.0f, 0.0f, 1.0f, 1.0f,
        -1.0f,  -1.0f,  1.0f,    0.0f, 0.0f, 1.0f, 1.0f,
        -1.0f,   1.0f,  1.0f,    0.0f, 0.0f, 1.0f, 1.0f,

        // right
        1.0f,  -1.0f, -1.0f,    1.0f, 0.5f, 0.0f, 1.0f,
        1.0f,   1.0f, -1.0f,    1.0f, 0.5f, 0.0f, 1.0f,
        1.0f,   1.0f,  1.0f,    1.0f, 0.5f, 0.0f, 1.0f,
        1.0f,  -1.0f,  1.0f,    1.0f, 0.5f, 0.0f, 1.0f,

        // bottom
       -1.0f,   1.0f, -1.0f,    1.0f, 1.0f, 0.0f, 1.0f,
        1.0f,   1.0f, -1.0f,    1.0f, 1.0f, 0.0f, 1.0f,
        1.0f,  -1.0f, -1.0f,    1.0f, 1.0f, 0.0f, 1.0f,
       -1.0f,  -1.0f, -1.0f,    1.0f, 1.0f, 0.0f, 1.0f,

       // top
      -1.0f,  -1.0f,  1.0f,    1.0f, 0.0f, 1.0f, 1.0f,
       1.0f,  -1.0f,  1.0f,    1.0f, 0.0f, 1.0f, 1.0f,
       1.0f,   1.0f,  1.0f,    1.0f, 0.0f, 1.0f, 1.0f,
      -1.0f,   1.0f,  1.0f,    1.0f, 0.0f, 1.0f, 1.0f
    };

    static const std::vector<unsigned int> indices
    {
       0,  1,  2,  0,  2,  3, // front
       4,  5,  6,  4,  6,  7, // back
       8,  9, 10,  8, 10, 11, // left
      12, 13, 14, 12, 14, 15, // right
      16, 17, 18, 16, 18, 19,  // bottom
      20, 21, 22, 20, 22, 23  // top
    };

    std::array<GLuint, 2> buffers;
    glGenBuffers(2, buffers.data());
    GLuint vbo = buffers[0];
    GLuint ibo = buffers[1];

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(*indices.data()), indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, varray.size() * sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(*varray.data()), 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(*varray.data()), (void*)(3 * sizeof(*varray.data())));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &_ubo_mvp);
    glBindBuffer(GL_UNIFORM_BUFFER, _ubo_mvp);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(TUB_MVP), nullptr, GL_STATIC_DRAW);
    // glBufferStorage(GL_UNIFORM_BUFFER, GLsizeiptr size?, const GLvoid * data?, GLbitfield flags?);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, _ubo_mvp);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
}

void CWindow_Glfw::Render(double time_ms)
{
    float     aspect = (float)_vpSize[0] / (float)_vpSize[1];

    Render::GLM::CMat4(_ubo_mvp_data._projection) = glm::perspective(glm::radians(70.0f), aspect, 0.01f, 100.0f);
    Render::GLM::CMat4(_ubo_mvp_data._view) = glm::lookAt(glm::vec3(0.0f, -4.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    Render::GLM::CMat4(_ubo_mvp_data._model) = _model_control != nullptr ? _model_control->OrbitMatrix() * _model_control->AutoModelMatrix() : glm::mat4(1.0f);

    glBindBuffer(GL_UNIFORM_BUFFER, _ubo_mvp);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(TUB_MVP), &_ubo_mvp_data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    _prog->Use();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
}