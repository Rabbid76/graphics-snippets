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
#include <string>
#include <sstream>
#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

// Own
#include <OpenGL/OpenGL_Matrix_Camera.h>
#include <OpenGL/OpenGL_SimpleShaderProgram.h>
#include <OpenGL/OpenGLBasicDraw.h>
#include <OpenGL/OpenGLError.h>


enum class TScene
{
    e_undefined,

    e_test_1,
    e_test_2,
    e_test_perspective,

    // transformation
    e_text_rotate,
    e_pivot,

    // intersection
    e_isect_line_line,
    e_isect_line_plane,
    e_isect_plane_cone,

    // depth, model, view, projection, clip space, NDC and viewport
    e_viewport_coordsys,
    e_perspective_distortion,
    e_NDC,
    e_orthographic_volume,
    e_model,
    e_world,
    e_view,
    e_projection,

    // parallax
    e_cone_step,

    e_NUMBER_OF
};


#define RENDER_BITMAP 0 

#if defined(RENDER_BITMAP) && RENDER_BITMAP == 4
static int          c_window_cx = 680;
static int          c_window_cy = 500;
static bool         c_frameless = true;
#elif defined(RENDER_BITMAP) && RENDER_BITMAP == 3
static int          c_window_cx = 600;
static int          c_window_cy = 450;
static bool         c_frameless = true;
#elif defined(RENDER_BITMAP) && RENDER_BITMAP == 2
static int          c_window_cx = 400;
static int          c_window_cy = 400;
static bool         c_frameless = true;
#elif defined(RENDER_BITMAP) && RENDER_BITMAP == 1
static int          c_window_cx = 400;
static int          c_window_cy = 300;
static bool         c_frameless = true;
#else
static int          c_window_cx = 600;
static int          c_window_cy = 450;
static bool         c_frameless = false;
#endif

static bool         c_core = true;
static float        c_scale = 1.0f;
static bool         c_fxaa = false;
static unsigned int c_samples = 16;




// [Switching Between windowed and full screen in OpenGL/GLFW 3.2](https://stackoverflow.com/questions/47402766/switching-between-windowed-and-full-screen-in-opengl-glfw-3-2/47462358#47462358)
class CWindow_Glfw
{
private:

    std::array< int, 2 > _wndPos{ 0, 0 };
    std::array< int, 2 > _wndSize{ 0, 0 };
    std::array< int, 2 > _vpSize{ 0, 0 };
    bool                 _updateViewport = true;
    bool                 _doubleBuffer = true;
    GLFWwindow* _wnd = nullptr;
    GLFWmonitor* _monitor = nullptr;

    static constexpr const Render::TColor Color_white(void) { return { 1.0f, 1.0f, 1.0f, 1.0f }; }
    static constexpr const Render::TColor Color_black(void) { return { 0.0f, 0.0f, 0.0f, 1.0f }; }
    static constexpr const Render::TColor Color_red(void) { return { 1.0f, 0.0f, 0.0f, 1.0f }; }
    static constexpr const Render::TColor Color_green(void) { return { 0.0f, 1.0f, 0.0f, 1.0f }; }
    static constexpr const Render::TColor Color_blue(void) { return { 0.0f, 0.0f, 1.0f, 1.0f }; }
    static constexpr const Render::TColor Color_darkred(void) { return { 0.6f, 0.0f, 0.0f, 1.0f }; }
    static constexpr const Render::TColor Color_darkgreen(void) { return { 0.0f, 0.5f, 0.0f, 1.0f }; }
    static constexpr const Render::TColor Color_darkblue(void) { return { 0.0f, 0.0f, 0.5f, 1.0f }; }
    static constexpr const Render::TColor Color_yellow(void) { return { 1.0f, 1.0f, 0.0f, 1.0f }; }
    static constexpr const Render::TColor Color_magenta(void) { return { 1.0f, 0.0f, 1.0f, 1.0f }; }
    static constexpr const Render::TColor Color_cyan(void) { return { 0.0f, 1.0f, 1.0f, 1.0f }; }
    static constexpr const Render::TColor Color_orange(void) { return { 1.0f, 0.5f, 0.0f, 1.0f }; }
    static constexpr const Render::TColor Color_red_2(void) { return { 0.8f, 0.0f, 0.0f, 1.0f }; }
    static constexpr const Render::TColor Color_green_2(void) { return { 0.0f, 0.7f, 0.0f, 1.0f }; }
    static constexpr const Render::TColor Color_blue_2(void) { return { 0.0f, 0.0f, 0.8f, 1.0f }; }
    static constexpr const Render::TColor Color_darkgray(void) { return { 0.2f, 0.2f, 0.2f, 1.0f }; }
    static constexpr const Render::TColor Color_gray(void) { return { 0.5f, 0.5f, 0.5f, 1.0f }; }
    static constexpr const Render::TColor Color_lightgray(void) { return { 0.75f, 0.75f, 0.75f, 1.0f }; }
    static constexpr const Render::TColor Color_ink(void) { return { 0.1f, 0.3f, 0.8f, 1.0f }; }
    static constexpr const Render::TColor Color_paper_nature(void) { return { 0.96f, 0.96f, 0.93f, 1.0f }; }
    static constexpr const Render::TColor Color_paper_line(void) { return { 0.8f, 0.9f, 0.9f, 1.0f }; }

    void Resize(int cx, int cy);
    void Key(int key, int scancode, int action, int mods);
    void CursorPos(double xpos, double ypos);
    void CursorEnter(int entered);
    void MouseButton(int button, int action, int mods);

    std::chrono::high_resolution_clock::time_point _start_time;
    std::chrono::high_resolution_clock::time_point _current_time;

    std::unique_ptr<Render::IDebug> _debug;
    std::unique_ptr<Render::IDraw>  _draw;
    float  _aspect = 1.0;
    float  _scale_x = 1.0;
    float  _scale_y = 1.0;
    TScene _scene = TScene::e_undefined;

    Render::TPoint2 BL(void) const { return{ -_scale_x, -_scale_y }; }
    Render::TPoint2 TL(void) const { return{ _scale_x, -_scale_y }; }
    Render::TPoint2 BR(void) const { return{ -_scale_x,  _scale_y }; }
    Render::TPoint2 TR(void) const { return{ _scale_x,  _scale_y }; }

    TScene SelectScene(void);
    void InitScene(void);
    void Render(double time_ms);

    Render::TMat44 ToM44(const glm::mat4& m);
    glm::vec3 ProjectV3(const glm::mat4& prj_m44, const glm::vec3& pt);

    void Lined(const Render::TPoint2& bl, const Render::TPoint2& tr, float dist = 0.08f, float z = 0.0f, float thickness = 1.0f, const Render::TColor color = Color_paper_line());
    void Checkered(const Render::TPoint2& bl, const Render::TPoint2& tr, Render::TVec2 dist = { 0.05f, 0.05f }, float z = 0.0f, float thickness = 1.0f, const Render::TColor color = Color_paper_line());
    void Setup2DCheckered(void);

    void BoxWired(float left, float right, float bottom, float top, float near, float far, const Render::TColor& color, float thickness);
    void AxisCross(float len_x, float len_y, float len_z, float thickness, const Render::TVec2& arr_size);
    void AxisCrossText(float len_x, float len_y, float len_z, int origin_x, int origin_y, int origin_z, float height, float scale_y);

    void TestScene_1(double time_ms);
    void TestScene_2(double time_ms);
    void TestScene_Perspecitve(double time_ms);
    void TextRotate(double time_ms);
    void PivotRotate(double time_ms);
    void IsectLineLine(double time_ms);
    void IsectLinePlane(double time_ms);
    void IsectPlaneCone(double time_ms);
    void ViewportCoordsys(double time_ms);
    void Model(double time_ms);
    void World(double time_ms);
    void View(double time_ms);
    void Projection(double time_ms);
    void PerspectiveDistortion(double time_ms);
    void NDC(double time_ms);
    void OrthographicVolume(double time_ms);
    void ConeStep(double time_ms);

public:

    virtual ~CWindow_Glfw();

    void Init(int width, int height, bool doubleBuffer);
    void InitDebug(void);
    static void CallbackResize(GLFWwindow* window, int cx, int cy);
    static void CallbackKey(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void CallbackCursorPos(GLFWwindow* window, double xpos, double ypos);
    static void CallbackCursorEnter(GLFWwindow* window, int entered);
    static void CallbackMouseButton(GLFWwindow* window, int button, int action, int mods);
    void MainLoop(void);
};

int main(int argc, char** argv)
{
    if (glfwInit() == GLFW_FALSE)
        throw std::runtime_error("error initializing GLFW");

    // create OpenGL window and make OpenGL context current (`glfwInit` has to be done before).
    CWindow_Glfw window;
    window.Init(c_window_cx, c_window_cy, true);

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
    _draw.reset(nullptr);
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


void CWindow_Glfw::CallbackKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    void* ptr = glfwGetWindowUserPointer(window);
    if (CWindow_Glfw* wndPtr = static_cast<CWindow_Glfw*>(ptr))
        wndPtr->Key(key, scancode, action, mods);
}


void CWindow_Glfw::CallbackCursorPos(GLFWwindow* window, double xpos, double ypos)
{
    void* ptr = glfwGetWindowUserPointer(window);
    if (CWindow_Glfw* wndPtr = static_cast<CWindow_Glfw*>(ptr))
        wndPtr->CursorPos(xpos, ypos);
}


void CWindow_Glfw::CallbackCursorEnter(GLFWwindow* window, int entered)
{
    void* ptr = glfwGetWindowUserPointer(window);
    if (CWindow_Glfw* wndPtr = static_cast<CWindow_Glfw*>(ptr))
        wndPtr->CursorEnter(entered);
}


void CWindow_Glfw::CallbackMouseButton(GLFWwindow* window, int button, int action, int mods)
{
    void* ptr = glfwGetWindowUserPointer(window);
    if (CWindow_Glfw* wndPtr = static_cast<CWindow_Glfw*>(ptr))
        wndPtr->MouseButton(button, action, mods);
}


void CWindow_Glfw::InitDebug(void) // has to be done after GLEW initialization!
{
#if defined(_DEBUG)
    static bool synchromous = true;
    _debug = std::make_unique<OpenGL::CDebug>();
    static Render::TDebugLevel debug_level = Render::TDebugLevel::all;
    _debug->Init(debug_level);
    _debug->Activate(synchromous);
#endif
}

void CWindow_Glfw::Init(int width, int height, bool doubleBuffer)
{
    _doubleBuffer = doubleBuffer;

    // [GLFW Window guide; Window creation hints](http://www.glfw.org/docs/latest/window_guide.html#window_hints_values)

    if (c_core)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    }

#if defined(_DEBUG)
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    //GLFW_CONTEXT_ROBUSTNESS

    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    glfwWindowHint(GLFW_SAMPLES, 1);
    glfwWindowHint(GLFW_DOUBLEBUFFER, _doubleBuffer ? GLFW_TRUE : GLFW_FALSE);

    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    glfwWindowHint(GLFW_DECORATED, c_frameless ? GLFW_FALSE : GLFW_TRUE);

    _wnd = glfwCreateWindow(width, height, "OGL window", nullptr, nullptr);
    if (_wnd == nullptr)
    {
        glfwTerminate();
        throw std::runtime_error("error initializing window");
    }

    glfwMakeContextCurrent(_wnd);

    glfwSetWindowUserPointer(_wnd, this);
    glfwSetWindowSizeCallback(_wnd, CWindow_Glfw::CallbackResize);
    glfwSetKeyCallback(_wnd, CWindow_Glfw::CallbackKey);
    glfwSetCursorPosCallback(_wnd, CWindow_Glfw::CallbackCursorPos);
    glfwSetCursorEnterCallback(_wnd, CWindow_Glfw::CallbackCursorEnter);
    glfwSetMouseButtonCallback(_wnd, CWindow_Glfw::CallbackMouseButton);

    _monitor = glfwGetPrimaryMonitor();
    glfwGetWindowSize(_wnd, &_wndSize[0], &_wndSize[1]);
    glfwGetWindowPos(_wnd, &_wndPos[0], &_wndPos[1]);
    _updateViewport = true;

    GLint max_color_samples = 0, max_framebuffer_samples = 0;
    glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &max_color_samples);
    glGetIntegerv(GL_MAX_FRAMEBUFFER_SAMPLES, &max_framebuffer_samples);
    unsigned int smaples = std::min(c_samples, static_cast<unsigned int>(std::min(max_color_samples, max_framebuffer_samples)));
    _draw = std::make_unique<OpenGL::CBasicDraw>(c_core, smaples, c_scale, c_fxaa);
}


void CWindow_Glfw::Resize(int cx, int cy)
{
    _updateViewport = true;
}


void CWindow_Glfw::Key(int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        TScene new_scene = SelectScene();
        if (new_scene != _scene)
        {
            _scene = new_scene;
            InitScene();
        }
    }
}


void CWindow_Glfw::CursorPos(double xpos, double ypos)
{
    // ...
}


void CWindow_Glfw::CursorEnter(int entered)
{
    // ...
}


void CWindow_Glfw::MouseButton(int button, int action, int mods)
{
    // ...
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
            glViewport(0, 0, _vpSize[0], _vpSize[1]);
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

        if (_scene == TScene::e_undefined)
            _scene = SelectScene();
    }

    _draw.reset(nullptr);
}


TScene CWindow_Glfw::SelectScene(void)
{
    std::map<int, std::string> scene_names
    {
        { (int)TScene::e_test_1,                 "Test 1" },
        { (int)TScene::e_test_2,                 "Test 2" },
        { (int)TScene::e_test_perspective,       "Perspective test" },
        { (int)TScene::e_text_rotate,            "Rotate text" },
        { (int)TScene::e_pivot,                  "pivot" },
        { (int)TScene::e_isect_line_line,        "Intersect line line" },
        { (int)TScene::e_isect_line_plane,       "Intersect line plane" },
        { (int)TScene::e_isect_plane_cone,       "Intersect plane cone" },
        { (int)TScene::e_viewport_coordsys,      "Viewport coordinate system" },
        { (int)TScene::e_perspective_distortion, "Perspective distortion" },
        { (int)TScene::e_NDC,                    "Normalized device space" },
        { (int)TScene::e_orthographic_volume,    "Orthographic volume" },
        { (int)TScene::e_model,                  "Model 2D" },
        { (int)TScene::e_world,                  "World 2D" },
        { (int)TScene::e_view,                   "View 2D" },
        { (int)TScene::e_projection,             "Projection 2D" },
        { (int)TScene::e_cone_step,              "Cone step" }
    };

    for (int i = (int)TScene::e_undefined + 1; i < (int)TScene::e_NUMBER_OF; ++i)
        assert(scene_names.find(i) != scene_names.end());

    int i_scene = 0;
    while (scene_names.find(i_scene) == scene_names.end())
    {
        static const std::string indent = "\t";

        std::cout << std::endl << "Select model:" << std::endl;
        for (auto scenes : scene_names)
        {
            std::cout << indent << scenes.first << ": " << scenes.second << std::endl;
        }

        std::cout << "Enter number: ";

        std::cin.clear();
        fflush(stdin);
        int number;
        std::cin >> number;
        std::cout << std::endl;

        i_scene = number;
    }

    return (TScene)i_scene;
}


void CWindow_Glfw::InitScene(void)
{
    //if ( _scene == )
    //_draw->BackgroundColor( Color_white() );
    _draw->BackgroundColor(Color_paper_nature());
    _draw->Init();
}


void CWindow_Glfw::Render(double time_ms)
{
    _aspect = (float)_vpSize[0] / (float)_vpSize[1];
    _scale_x = _aspect < 1.0f ? 1.0f : _aspect;
    _scale_y = _aspect < 1.0f ? 1.0f / _aspect : 1.0f;

    _draw->ViewportSize({ (size_t)_vpSize[0], (size_t)_vpSize[1] });
    _draw->Projection(OpenGL::Camera::Orthopraphic(_scale_x, _scale_y, { -10.0f, 10.0f }));
    _draw->View(OpenGL::Identity());
    _draw->Model(OpenGL::Identity());

    if (_scene == TScene::e_orthographic_volume)
        _draw->BackgroundColor(Color_white());

    _draw->Begin();

    // TODO $$$ loop all 2 seconds per scene

    switch (_scene)
    {

    default:
    case TScene::e_test_1:                 TestScene_1(time_ms); break;
    case TScene::e_test_2:                 TestScene_2(time_ms); break;
    case TScene::e_test_perspective:       TestScene_Perspecitve(time_ms); break;
    case TScene::e_text_rotate:            TextRotate(time_ms); break;
    case TScene::e_pivot:                  PivotRotate(time_ms); break;
    case TScene::e_isect_line_line:        IsectLineLine(time_ms); break;
    case TScene::e_isect_line_plane:       IsectLinePlane(time_ms); break;
    case TScene::e_isect_plane_cone:       IsectPlaneCone(time_ms); break;
    case TScene::e_viewport_coordsys:      ViewportCoordsys(time_ms); break;
    case TScene::e_perspective_distortion: PerspectiveDistortion(time_ms); break;
    case TScene::e_NDC:                    NDC(time_ms); break;
    case TScene::e_orthographic_volume:    OrthographicVolume(time_ms); break;
    case TScene::e_model:                  Model(time_ms); break;
    case TScene::e_world:                  World(time_ms); break;
    case TScene::e_view:                   View(time_ms); break;
    case TScene::e_projection:             Projection(time_ms); break;
    case TScene::e_cone_step:              ConeStep(time_ms); break;
    }

    _draw->Finish();
}


Render::TMat44 CWindow_Glfw::ToM44(const glm::mat4& m)
{
    Render::TMat44 temp_m;
    std::memcpy(&temp_m[0][0], glm::value_ptr(m), sizeof(Render::TMat44));
    return temp_m;
}

glm::vec3 CWindow_Glfw::ProjectV3(const glm::mat4& prj_m44, const glm::vec3& pt)
{
    glm::vec4 h_pt = prj_m44 * glm::vec4(pt, 1.0);
    return glm::vec3(h_pt) / h_pt.w;
}

void CWindow_Glfw::Lined(const Render::TPoint2& bl, const Render::TPoint2& tr, float dist, float z, float thickness, const Render::TColor color)
{
    _draw->DrawLines2D(bl, tr, dist, z, { 0.8f, 0.9f, 0.9f, 1.0f }, thickness);
}

void CWindow_Glfw::Checkered(const Render::TPoint2& bl, const Render::TPoint2& tr, Render::TVec2 dist, float z, float thickness, const Render::TColor color)
{
    // { 0.3f, 0.8f, 0.8f, 1.0f }
    _draw->DrawGrid2D(bl, tr, dist, z, { 0.8f, 0.9f, 0.9f, 1.0f }, thickness);
}

void CWindow_Glfw::Setup2DCheckered(void)
{
    _draw->Projection(OpenGL::Camera::Orthopraphic(_scale_x, _scale_y, { -10.0f, 10.0f }));
    _draw->View(Render::Identity());
    _draw->Model(Render::Identity());

    _draw->ActivateBackground();
    Checkered(BL(), TR());
}


void CWindow_Glfw::BoxWired(float left, float right, float bottom, float top, float near, float far, const Render::TColor& color, float thickness)
{
    _draw->DrawPolyline(3, { left,  bottom, near,   right, bottom, near,   right,    top, near,    left,     top, near }, color, thickness, true);
    _draw->DrawPolyline(3, { left,  bottom,  far,   right, bottom,  far,   right,    top,  far,    left,     top,  far }, color, thickness, true);
    _draw->DrawPolyline(3, { left,  bottom, near,   left,  bottom,  far }, color, thickness, false);
    _draw->DrawPolyline(3, { left,     top, near,   left,     top,  far }, color, thickness, false);
    _draw->DrawPolyline(3, { right, bottom,  far,   right, bottom, near }, color, thickness, false);
    _draw->DrawPolyline(3, { right,    top,  far,   right,    top, near }, color, thickness, false);
}


void CWindow_Glfw::AxisCross(float len_x, float len_y, float len_z, float thickness, const Render::TVec2& arr_size)
{
    _draw->DrawArrow(3, { 0.0f, 0.0f, 0.0f, len_x, 0.0f, 0.0f }, Color_red_2(), thickness, arr_size, false, true);
    _draw->DrawArrow(3, { 0.0f, 0.0f, 0.0f, 0.0f, len_y, 0.0f }, Color_green_2(), thickness, arr_size, false, true);
    _draw->DrawArrow(3, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, len_z }, Color_blue_2(), thickness, arr_size, false, true);
}


void CWindow_Glfw::AxisCrossText(float len_x, float len_y, float len_z, int origin_x, int origin_y, int origin_z, float height, float scale_y)
{
    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, "X", origin_x, height, scale_y, 0.0f, { len_x, 0.0f, 0.0f }, Color_red_2());
    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, "Y", origin_y, height, scale_y, 0.0f, { 0.0f, len_y, 0.0f }, Color_green_2());
    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, "Z", origin_z, height, scale_y, 0.0f, { 0.0f, 0.0f, len_z }, Color_blue_2());
}



//-------------------------------------------------------------------------------------------------
// e_test_1
//-------------------------------------------------------------------------------------------------


void CWindow_Glfw::TestScene_1(double time_ms)
{
    // TODO $$$ SSOA (3 frequences)
    // TODO $$$ text + greek letters: distance fields!
    // TODO $$$ post effects (cell (toon), sketch, gamma, hdr) - book of shaders
    // TOOO $$$ meshs
    // TODO $$$ input polyline
    // TODO $$$ An Efficient Way to Draw Approximate Circles in OpenGL [http://slabode.exofire.net/circle_draw.shtml]
    // TODO $$$ draw arcs, curves (nurbs, spline) by tessellation shader
    // TODO $$$ glLineWidth( > 1.0 ) is deprecated in core profile
    // TODO $$$ orbit controll

    // TODO view matrix from pitch, yaw (and roll) or quaternion

    _draw->ActivateBackground();

    Lined(BL(), { 0.0f, _scale_y });
    Checkered({ 0.0f, -_scale_y }, TR());

    _draw->ActivateOpaque();

    static float  text_height = 0.126f;
    static float  text_scale_y = 1.0f;
    const char* text = "Hello, a long text with a lot of different letters.";
    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, text, text_height, text_scale_y, { _scale_x * -0.96f, 0.3f, -0.01f }, Color_orange());
    const char* greek_text = "DEFGHIJKLMNOPQRSTUVW";
    _draw->DrawText2D(OpenGL::CBasicDraw::font_symbol, greek_text, 0.2f, 1.0f, { _scale_x * -0.96f, 0.0f, -0.01f }, Color_cyan());
    const char* letter_text = "Hallo letter";
    _draw->DrawText2D(OpenGL::CBasicDraw::font_pcifico, letter_text, 0.2f, 1.0f, { _scale_x * -0.96f, -0.3f, -0.01f }, Color_ink());

    _draw->DrawRectangle2D({ -0.8f, -0.8f }, { 0.8f, 0.8f }, 0.0f, Color_magenta(), 5);

    _draw->DrawArrow(
        3, { -_scale_x * 0.95f, _scale_y * 0.95f, 0.5f, _scale_x * 0.95f, -_scale_y * 0.95f, 0.5f },
        Color_red(), 3, { 0.05f, 0.03f }, true, true);


    _draw->ActivateTransparent();

    _draw->DrawConvexPolygon(2, { -0.8f, -0.8f,  0.8f, -0.8f,  0.0f,  0.8f }, { 1.0f, 0.0f, 0.0f, 0.5f });
    _draw->DrawConvexPolygon(2, { -0.8f,  0.8f,  0.8f,  0.8f,  0.0f, -0.8f }, { 0.0f, 1.0f, 0.0f, 0.5f });
    _draw->DrawConvexPolygon(2, { -0.8f, -0.8f, -0.8f,  0.8f,  0.8f,  0.0f }, { 0.0f, 0.0f, 1.0f, 0.5f });
    _draw->DrawConvexPolygon(2, { 0.8f, -0.8f,  0.8f,  0.8f, -0.8f,  0.0f }, { 1.0f, 1.0f, 0.0f, 0.5f });
}



//-------------------------------------------------------------------------------------------------
// e_test_2
//-------------------------------------------------------------------------------------------------


void CWindow_Glfw::TestScene_2(double time_ms)
{
    _draw->ActivateBackground();

    //Lined( BL(), { 0.0f, _scale_y } );
    //Checkered( { 0.0f, -_scale_y }, TR() )
    Setup2DCheckered();

    _draw->ActivateOpaque();

    static float  text_height = 0.126f;
    static float  text_scale_y = 1.0f;
    const char* text = "Hello, a long text with a lot of different letters.";
    _draw->DrawText2D(OpenGL::CBasicDraw::font_pixslim_2, text, text_height, text_scale_y, { _scale_x * -0.96f, 0.3f, -0.01f }, Color_orange());
}



//-------------------------------------------------------------------------------------------------
// e_test_perspective
//-------------------------------------------------------------------------------------------------


void CWindow_Glfw::TestScene_Perspecitve(double time_ms)
{
    Render::TVec2   arr_size{ 0.2f, 0.12f };
    Render::TVec2   arr_los_size{ arr_size[0] * 2.0f, arr_size[1] * 2.0f };
    float           axis_len = 2.2f;
    static float    axis_text_height = 0.126f;
    static float    axis_text_scale_y = 0.9f;

    OpenGL::Camera camera;
    camera._vp = { (int)_vpSize[0], (int)_vpSize[1] };
    camera._near = 0.5f;
    camera._far = 20.0f;
    camera._pos = { 0.0f, -6.0f, 0.0f };
    camera._target = { 0.0f, 0.0f, 0.0f };
    camera._up = { 0.0f, 0.0f, 1.0f };
    camera._fov_y = 60.0f;
    Render::TMat44 prj = camera.Perspective();
    Render::TMat44 view = camera.LookAt();

    static bool perspective_line = true;
    _draw->SetDrawProperty(Render::IDraw::TDrawProperty::perspective_line, perspective_line);

    _draw->Projection(prj);
    _draw->View(view);
    _draw->Model(Render::Identity());

    _draw->ActivateOpaque();

    _draw->DrawPolyline(3, { -2.7f, 0.0f, -1.0f,   -2.7f, 0.0f,  1.0f }, Color_darkgray(), 10.0f, false);
    _draw->DrawPolyline(3, { -2.5f, 0.0f, -1.2f,   -1.0f, 6.0f, -2.4f }, Color_darkgray(), 10.0f, false);
    _draw->DrawPolyline(3, { -0.8f, 6.0f, -2.0f,   -0.8f, 6.0f,  2.0f }, Color_darkgray(), 10.0f, false);
    _draw->DrawPolyline(3, { -2.5f, 0.0f,  1.2f,   -1.0f, 6.0f,  2.4f }, Color_darkgray(), 10.0f, false);

    _draw->DrawPolyline(3, { 2.7f, 0.0f, -1.2f,   2.7f, 0.0f,  1.2f,  0.8f, 6.0f,  2.4f,   0.8f, 6.0f, -2.4f, }, Color_darkgray(), 10.0f, true);

    //_draw->ActivateOpaque();
    //_draw->ClearDepth();
}



//-------------------------------------------------------------------------------------------------
// e_text_rotate
//-------------------------------------------------------------------------------------------------


void CWindow_Glfw::PivotRotate(double time_ms)
{
    _draw->Projection(OpenGL::Camera::Orthopraphic(_scale_x, _scale_y, { -10.0f, 10.0f }));
    _draw->View(Render::Identity());
    _draw->Model(Render::Identity());

    _draw->ActivateBackground();

    Setup2DCheckered();

    _draw->ActivateOpaque();

    static float rect_size = 0.4f;

    // pivot
    static float p_x = -0.2f;
    static float p_y = 0.4f;


    double intervall_s = 2.0;
    int step = (int)(time_ms / intervall_s / 1000.0) % 4;
    //step = 4;

    Render::IDraw::TStyle style;
    style._thickness = 2.0f;
    _draw->Draw(Render::TPrimitive::lines, 2, { -0.1f, 0.0f,  0.1f, 0.0f, 0.0f, -0.1f,  0.0f, 0.1f }, Color_red(), style);

    glm::mat4 model(1.0f);

    if (step > 0)
        _draw->DrawRectangle2D({ -rect_size, -rect_size }, { rect_size, rect_size }, 0.0f, Color_gray(), 3);

    float angle = 10.0f * (float)time_ms / 1000.0f;
    angle = 35.0f;
    for (int i = 0; i < step; ++i)
    {
        if (i == step - 3)
            model = glm::translate(model, glm::vec3(p_x, p_y, 1.0f));
        if (i == step - 2)
            model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
        if (i == step - 1)
            model = glm::translate(model, glm::vec3(-p_x, -p_y, 1.0f));
    }

    Render::TMat44 model_mat;
    memcpy(&model_mat[0][0], glm::value_ptr(model), sizeof(Render::TMat44));
    _draw->Model(model_mat);

    _draw->DrawRectangle2D({ -rect_size, -rect_size }, { rect_size, rect_size }, 0.0f, Color_ink(), 3);
    _draw->Draw(Render::TPrimitive::lines, 2, { p_x - 0.1f, p_y - 0.1f,  p_x + 0.1f, p_y + 0.1f, p_x - 0.1f, p_y + 0.1f, p_x + 0.1f, p_y - 0.1f }, Color_green(), style);
}



//-------------------------------------------------------------------------------------------------
// e_text_rotate
//-------------------------------------------------------------------------------------------------


void CWindow_Glfw::TextRotate(double time_ms)
{
    _draw->Projection(OpenGL::Camera::Orthopraphic(_scale_x, _scale_y, { -10.0f, 10.0f }));
    _draw->View(Render::Identity());
    _draw->Model(Render::Identity());

    _draw->ActivateBackground();

    Setup2DCheckered();

    _draw->ActivateOpaque();

    Render::IDraw::TStyle style;
    style._thickness = 2.0f;
    _draw->Draw(Render::TPrimitive::lines, 2, { -0.1f, 0.0f,  0.1f, 0.0f, 0.0f, -0.1f,  0.0f, 0.1f }, Color_red(), style);

    _draw->ActivateOpaque();

    static float  text_height = 0.2f;
    static float  text_scale_x = 1.0f;
    const char* text = "ABCD";

    std::array<float, 4> text_rect{ 0.0f };
    _draw->CalculateTextSize(OpenGL::CBasicDraw::font_sans, text, text_height, text_rect[2], text_rect[1], text_rect[3]);
    text_rect[0] *= text_scale_x;
    text_rect[2] *= text_scale_x;

    float tx = text_rect[2] - text_rect[0];
    float ty = text_rect[3] - text_rect[1];

    float rx = tx / 2.0f;
    float ry = ty / 2.0f;

    float origin_x = -0.7f;
    float origin_y = 0.2f;

    double intervall_s = 2.0;
    int step = (int)(time_ms / intervall_s / 1000.0) % 6;
    step = 6;

    float scale_x = 2.0f;
    float scale_y = 2.0f;

    glm::mat4 model(1.0f);

    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, text, text_height, text_scale_x, { 0.0f, 0.0f, -0.02f }, Color_gray());

    float angle = 10.0f * (float)time_ms / 1000.0f;
    angle = 45.0f;
    for (int i = 0; i < step; ++i)
    {
        if (i == step - 5)
            model = glm::translate(model, glm::vec3(origin_x, origin_y, 1.0f));
        if (i == step - 4)
            model = glm::translate(model, glm::vec3(rx * scale_x, ry * scale_y, 1.0f));
        if (i == step - 3)
            model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
        if (i == step - 2)
            model = glm::translate(model, glm::vec3(-rx * scale_x, -ry * scale_y, 1.0f));
        if (i == step - 1)
            model = glm::scale(model, glm::vec3(scale_x, scale_y, 1.0f));
    }

    Render::TMat44 model_mat;
    memcpy(&model_mat[0][0], glm::value_ptr(model), sizeof(Render::TMat44));
    _draw->Model(model_mat);

    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, text, text_height, text_scale_x, { 0.0f, 0.0f, -0.01f }, Color_ink());
}



//-------------------------------------------------------------------------------------------------
// e_isect_line_line
//-------------------------------------------------------------------------------------------------


void CWindow_Glfw::IsectLineLine(double time_ms)
{
    static Render::TVec2 arr_size{ 0.08f, 0.03f };
    static float arrow_th = 2.0f;
    static float point_size = 8.0f;
    static float text_height = 0.07f;
    static float text_scale_y = 1.0f;
    static float text_margin = 0.02f;

    Render::TColor color_line = Color_ink();
    Render::TColor color_source = Color_orange();
    Render::TColor color_result = Color_darkgreen();

    static glm::vec2 p0(0.9f, 0.6f);
    static glm::vec2 p1(-0.7f, -0.0f);
    static glm::vec2 q0(0.7f, -0.5f);
    static glm::vec2 q1(-0.7f, 0.3f);

    glm::vec2 R = normalize(p1 - p0);
    glm::vec2 P = p0 + R * 0.3f;
    glm::vec2 S = normalize(q1 - q0);
    glm::vec2 Q = q0 + S * 0.3f;
    glm::vec2 Q_P_mid = (P + Q) / 2.0f;

    //float t = glm::determinant(glm::mat2(Q-P, S)) / glm::determinant(glm::mat2(R, S));
    //float u = glm::determinant(glm::mat2(Q-P, R)) / glm::determinant(glm::mat2(R, S));
    float t = glm::dot(Q - P, glm::vec2(S.y, -S.x)) / glm::dot(R, glm::vec2(S.y, -S.x));
    float u = glm::dot(Q - P, glm::vec2(R.y, -R.x)) / glm::dot(R, glm::vec2(S.y, -S.x));

    float t_h = glm::dot(Q - P, R);
    glm::vec2 h_pt = P + R * t_h;
    glm::vec2 h_mid = (h_pt + Q) / 2.0f;

    glm::vec2 R_pt = P + R * t;
    glm::vec2 S_pt = Q + S * u;
    glm::vec2 X = R_pt;
    glm::vec2 R_mid_pt = P + R * t * 0.5f;
    glm::vec2 S_mid_pt = Q + S * u * 0.5f;

    Setup2DCheckered();

    _draw->ActivateOpaque();

    _draw->DrawPoint2D({ P[0], P[1] }, color_source, point_size);
    _draw->DrawPoint2D({ Q[0], Q[1] }, color_source, point_size);
    _draw->DrawPoint2D({ X[0], X[1] }, color_result, point_size);

    _draw->DrawArrow(2, { P[0], P[1], R_pt[0], R_pt[1] }, color_source, arrow_th, arr_size, false, true);
    _draw->DrawArrow(2, { Q[0], Q[1], S_pt[0], S_pt[1] }, color_source, arrow_th, arr_size, false, true);
    _draw->DrawArrow(2, { P[0], P[1], Q[0], Q[1] }, color_result, arrow_th, arr_size, false, true);

    _draw->DrawPolyline(2, { Q[0], Q[1], h_pt[0], h_pt[1] }, color_result, 2.0f, false);
    _draw->DrawPolyline(2, { p0[0], p0[1], p1[0], p1[1] }, color_line, 2.0f, false);
    _draw->DrawPolyline(2, { q0[0], q0[1], q1[0], q1[1] }, color_line, 2.0f, false);

    _draw->ActivateOpaque();
    _draw->ClearDepth();

    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "P", 3, text_height, text_scale_y, text_margin, { P[0], P[1], 0.0f }, color_source);
    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "Q", 9, text_height, text_scale_y, text_margin, { Q[0], Q[1], 0.0f }, color_source);
    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "R * t", 3, text_height, text_scale_y, text_margin, { R_mid_pt[0], R_mid_pt[1], 0.0f }, color_source);
    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "S * u", 9, text_height, text_scale_y, text_margin, { S_mid_pt[0], S_mid_pt[1], 0.0f }, color_source);

    char alpha[]{ 'a' - 29, 0 };
    char beta[]{ 'b' - 29, 0 };
    _draw->DrawText2D(OpenGL::CBasicDraw::font_symbol, alpha, 9, text_height * 1.4f, text_scale_y, 0.08f, { P[0], P[1], 0.0f }, color_result);
    _draw->DrawText2D(OpenGL::CBasicDraw::font_symbol, beta, 4, text_height * 1.4f, text_scale_y, 0.14f, { X[0], X[1], 0.0f }, color_result);

    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "Q-P", 4, text_height, text_scale_y, text_margin, { Q_P_mid[0], Q_P_mid[1], 0.0f }, color_result);
    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "X", 2, text_height, text_scale_y, 0.05f, { R_pt[0], R_pt[1], 0.0f }, color_result);
    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "h", 4, text_height, text_scale_y, 0.035f, { h_mid[0], h_mid[1], 0.0f }, color_result);
}



//-------------------------------------------------------------------------------------------------
// e_isect_line_plane
//-------------------------------------------------------------------------------------------------


void CWindow_Glfw::IsectLinePlane(double time_ms)
{
    // TODO $$$
}

// TODO $$$ intersect LOS viewport -> in documentation as example for intersection of a line and a plane



//-------------------------------------------------------------------------------------------------
// e_isect_plane_cone
//-------------------------------------------------------------------------------------------------


void CWindow_Glfw::IsectPlaneCone(double time_ms)
{
    // TODO $$$
}



//-------------------------------------------------------------------------------------------------
// e_viewport_coordsys
//-------------------------------------------------------------------------------------------------


void CWindow_Glfw::ViewportCoordsys(double time_ms)
{
    Render::TPoint2 vp_bl{ -1.0f * 16.0f / 9.0f, -1.0f };
    Render::TPoint2 vp_tr{ 1.0f * 16.0f / 9.0f,  1.0f };
    Render::TPoint2 vp_br{ vp_tr[0], vp_bl[1] };
    Render::TPoint2 vp_tl{ vp_bl[0], vp_tr[1] };
    Render::TColor  vp_col{ 0.3f, 0.3f, 0.8f, 1.0f };
    Render::TColor  vp_col_t{ vp_col[0], vp_col[1], vp_col[2], 0.5f };
    Render::TVec2   arr_size{ 0.2f, 0.12f };
    Render::TVec2   arr_los_size{ arr_size[0] * 2.0f, arr_size[1] * 2.0f };
    float           axis_len = 2.0f;
    static float    axis_text_height = 0.126f;
    static float    axis_text_scale_y = 0.9f;

    _draw->ActivateBackground();
    //Checkered( BL(), TR() );

    OpenGL::Camera camera;
    camera._vp = { (int)_vpSize[0], (int)_vpSize[1] };
    camera._near = 0.5f;
    camera._far = 10.0f;
    camera._pos = { 3.0f, 1.0f, 3.0f };
    camera._up = { 0.0f, 1.0f, 0.0f };
    camera._fov_y = 80.0f;
    _draw->Projection(camera.Perspective());
    _draw->View(camera.LookAt());

    _draw->ActivateOpaque();

    AxisCross(axis_len, axis_len, axis_len, 3.0f, arr_size);

    _draw->DrawRectangle2D(vp_bl, vp_tr, 0.0f, vp_col, 3);
    _draw->DrawArrow(3, { 0.0f, 0.0f, 2.5f, 0.0f, 0.0f, -6.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, 1, arr_los_size, false, true);

    _draw->ActivateTransparent();

    _draw->DrawConvexPolygon({ vp_bl, vp_br, vp_tr, vp_tl }, vp_col_t);

    _draw->ActivateOpaque();
    _draw->ClearDepth();

    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, "X", 7, axis_text_height, axis_text_scale_y, 0.0f, { axis_len, 0.0f, 0.0f }, Color_red_2());
    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, "Y", 1, axis_text_height, axis_text_scale_y, 0.0f, { 0.0f, axis_len, 0.0f }, Color_green_2());
    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, "Z", 3, axis_text_height, axis_text_scale_y, 0.0f, { 0.0f, 0.0f, axis_len }, Color_blue_2());

    const char* los_text = "Line of sight (-Z)";
    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, los_text, 1, 0.09f, 0.7f, 0.0f, { 0.0f, 0.0f, -6.0f }, Color_black());

    const char* title_text = "Viewport / view coordinate system";
    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, title_text, 2, 0.09f, 0.7f, 0.0f, { 0.0f, 2.5f, 0.0f }, Color_black());
}



//-------------------------------------------------------------------------------------------------
// e_perspective_distortion
//-------------------------------------------------------------------------------------------------


void CWindow_Glfw::PerspectiveDistortion(double time_ms)
{
    glm::vec3 bn(0.0f, -1.0f, -1.0f);
    glm::vec3 tf(0.0f, 1.0f, 1.0f);
    glm::vec3 bf(0.0f, -1.0f, 1.0f);
    glm::vec3 tn(0.0f, 1.0f, -1.0f);

    static float near = 0.5f;
    static float far = 2.5f;
    glm::mat4 prj_mat = glm::perspective(glm::radians(60.0f), 1.0f, near, far);
    glm::mat4 inv_prj_mat = glm::inverse(prj_mat);

    glm::vec3 bn_u = ProjectV3(inv_prj_mat, bn);
    glm::vec3 tf_u = ProjectV3(inv_prj_mat, tf);
    glm::vec3 bf_u = ProjectV3(inv_prj_mat, bf);
    glm::vec3 tn_u = ProjectV3(inv_prj_mat, tn);

    float pos_z = bf_u.z + (bn_u.z - bf_u.z) * abs(bf_u.y / (bf_u.y - bn_u.y));
    glm::vec3 eye_u(0.0f, 0.0f, pos_z);

    std::vector<glm::vec3> cube{ bn, bf, tf, tn };

    float cube_scale = 0.5f;
    static float cam_dist = 1.25f;
    Render::IDraw::TBuffer src_cube;
    for (auto& pt : cube)
    {
        src_cube.push_back(pt.z * cube_scale - cam_dist);
        src_cube.push_back(pt.y * cube_scale);
    }

    Render::IDraw::TBuffer dst_cube;
    for (size_t i = 0; i < src_cube.size(); i += 2)
    {
        glm::vec3 src_pt(0.0f, src_cube[i + 1], src_cube[i]);
        glm::vec3 dst_pt = ProjectV3(prj_mat, src_pt);
        dst_cube.push_back(dst_pt.z);
        dst_cube.push_back(dst_pt.y);
    }

    Render::TColor color_src = Color_ink();
    Render::TColor color_dst = Color_ink();
    Render::TColor color_near = Color_green_2();
    Render::TColor color_far = Color_red_2();
    Render::TColor color_limit = Color_gray();
    Render::TColor color_text = Color_black();
    Render::TColor color_ray = Color_orange();
    Render::TColor color_eye = Color_black();

    static float         text_height = 0.2f;
    static float         text_scale_y = 1.0f;
    static float         text_margin = 0.04f;
    static Render::TVec2 arr_size{ 0.15f, 0.10f };
    static float         arrow_th = 2.0f;
    static float         point_size = 8.0f;

    static int   segments = 5;
    static float scale = 0.4f;
    static float view_offset = 0.7f;
    static float ndc_offset = 0.7f;

    //Setup2DCheckered();

    // frustum

    float map_view = -cam_dist * scale - view_offset;
    glm::mat4 model_pos = glm::mat4(1.0f);
    model_pos = glm::translate(model_pos, glm::vec3(map_view, 0.0f, 0.0f));
    model_pos = glm::rotate(model_pos, (float)M_PI, glm::vec3(0.0f, 1.0f, 0.0f));
    model_pos = glm::scale(model_pos, glm::vec3(scale));
    _draw->Model(ToM44(model_pos));
    _draw->ActivateOpaque();

    _draw->DrawPoint2D({ eye_u.z, eye_u.y }, color_eye, point_size);
    _draw->DrawPolyline(2, src_cube, color_src, 2.0f, true);
    _draw->DrawPolyline(2, { bn_u.z, bn_u.y, tn_u.z, tn_u.y }, color_near, 2.0f, true);
    _draw->DrawPolyline(2, { bf_u.z, bf_u.y, tf_u.z, tf_u.y }, color_far, 2.0f, true);
    _draw->DrawPolyline(2, { eye_u.z, eye_u.y, bf_u.z, bf_u.y }, color_limit, 2.0f, true);
    _draw->DrawPolyline(2, { eye_u.z, eye_u.y, tf_u.z, tf_u.y }, color_limit, 2.0f, true);

    for (int i = 0; i < segments; ++i)
    {
        float y = bf_u.y + (tf_u.y - bf_u.y) * (float)(i + 0.5f) / (float)segments;
        _draw->DrawArrow(2, { eye_u.z, eye_u.y, bf_u.z, y }, color_ray, arrow_th, arr_size, false, true);
    }


    // frustum text

    model_pos = glm::mat4(1.0f);
    model_pos = glm::translate(model_pos, glm::vec3(map_view, 0.0f, 0.0f));
    model_pos = glm::scale(model_pos, glm::vec3(scale));
    _draw->Model(ToM44(model_pos));
    _draw->ActivateOpaque();
    _draw->ClearDepth();

    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "frustum", 2, text_height, text_scale_y, text_margin, { (-bn_u.z - bf_u.z) / 2.0f, 1.5f, 0.0f }, color_text);
    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "near", 8, text_height, text_scale_y, text_margin, { -bn_u.z, -1.45f, 0.0f }, color_near);
    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "far", 8, text_height, text_scale_y, text_margin, { -bf_u.z, -1.45f, 0.0f }, color_far);


    // NDC

    float map_ndc = ndc_offset;
    model_pos = glm::mat4(1.0f);
    model_pos = glm::translate(model_pos, glm::vec3(map_ndc, 0.0f, 0.0f));
    model_pos = glm::scale(model_pos, glm::vec3(scale));
    _draw->Model(ToM44(model_pos));

    _draw->ActivateOpaque();
    _draw->ClearDepth();

    _draw->DrawPolyline(2, dst_cube, color_dst, 2.0f, true);
    _draw->DrawPolyline(2, { bn.z, bn.y, tn.z, tn.y }, color_near, 2.0f, true);
    _draw->DrawPolyline(2, { bf.z, bf.y, tf.z, tf.y }, color_far, 2.0f, true);
    _draw->DrawPolyline(2, { bn.z, bn.y, bf.z, bf.y }, color_limit, 2.0f, true);
    _draw->DrawPolyline(2, { tn.z, tn.y, tf.z, tf.y }, color_limit, 2.0f, true);

    for (int i = 0; i < segments; ++i)
    {
        float y = bf.y + (tf.y - bf.y) * (float)(i + 0.5f) / (float)segments;
        _draw->DrawArrow(2, { -cam_dist, y, bf.z, y }, color_ray, arrow_th, arr_size, false, true);
    }

    // NDC text

    _draw->ActivateOpaque();
    _draw->ClearDepth();

    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "normalized device space", 2, text_height, text_scale_y, text_margin, { (bn.z + bf.z) / 2.0f, 1.5f, 0.0f }, color_text);
    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "near", 8, text_height, text_scale_y, text_margin, { bn.z, -1.45f, 0.0f }, color_near);
    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "far", 8, text_height, text_scale_y, text_margin, { bf.z, -1.45f, 0.0f }, color_far);

    //float scale
}



//-------------------------------------------------------------------------------------------------
// e_NDC
//-------------------------------------------------------------------------------------------------


void CWindow_Glfw::NDC(double time_ms)
{
    Render::TVec2   arr_size{ 0.2f, 0.12f };
    Render::TVec2   arr_los_size{ arr_size[0] * 2.0f, arr_size[1] * 2.0f };
    float           axis_len = 2.2f;
    static float    axis_text_height = 0.126f;
    static float    axis_text_scale_y = 0.9f;
    static float    ndc_text_height = 0.08f;
    static float    ndc_text_scale_y = 1.0f;

    OpenGL::Camera camera;
    camera._vp = { (int)_vpSize[0], (int)_vpSize[1] };
    camera._near = 2.5f;
    camera._far = 10.0f;
    camera._pos = { 3.0f, 1.5f, 3.8f };
    camera._target = { 0.0f, 0.0f, 0.0f };
    camera._up = { 0.0f, 1.0f, 0.0f };
    camera._fov_y = 60.0f;
    Render::TMat44 prj = camera.Perspective();
    Render::TMat44 view = camera.LookAt();

    _draw->SetDrawProperty(Render::IDraw::TDrawProperty::perspective_line, true);
    _draw->Projection(prj);
    _draw->View(view);
    _draw->Model(Render::Identity());

    _draw->ActivateOpaque();

    BoxWired(-1, 1, -1, 1, -1, 1, Color_darkgray(), 7.0f);
    AxisCross(axis_len, axis_len, -axis_len, 7.0f, arr_size);

    _draw->ActivateOpaque();
    _draw->ClearDepth();

    AxisCrossText(axis_len, axis_len, -axis_len, 7, 1, 7, axis_text_height, axis_text_scale_y);

    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, "(1, 1, 1)", 1, ndc_text_height * 0.8f, ndc_text_scale_y, 0.0f, { 1.0f, 1.1f, -1.0f }, Color_magenta());
    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, "(-1, -1, -1)", 9, ndc_text_height * 0.9f, ndc_text_scale_y, 0.0f, { -1.0f, -1.1f, 1.0f }, Color_magenta());

    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, "(1, -1, -1)", 8, ndc_text_height, ndc_text_scale_y, 0.0f, { 1.0f, -1.1f, 1.0f }, Color_darkgray());
    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, "(-1, 1, -1)", 3, ndc_text_height * 0.9f, ndc_text_scale_y, 0.0f, { -1.0f, 1.1f, 1.0f }, Color_darkgray());
    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, "(1, 1, -1)", 7, ndc_text_height * 0.9f, ndc_text_scale_y, 0.0f, { 1.05f, 0.95f, 1.0f }, Color_darkgray());
    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, "(1, -1, 1)", 7, ndc_text_height * 0.8f, ndc_text_scale_y, 0.0f, { 1.0f, -1.1f, -1.0f }, Color_darkgray());
    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, "(-1, 1, 1)", 3, ndc_text_height * 0.8f, ndc_text_scale_y, 0.0f, { -1.0f, 1.1f, -1.0f }, Color_darkgray());
    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, "(-1, -1, 1)", 3, ndc_text_height * 0.75f, ndc_text_scale_y, 0.0f, { -1.1f, -0.9f, -1.0f }, Color_darkgray());
}



//-------------------------------------------------------------------------------------------------
// e_orthographic_volume
//-------------------------------------------------------------------------------------------------


void CWindow_Glfw::OrthographicVolume(double time_ms)
{
    static Render::TVec2 arr_size{ 0.4f, 0.24f };
    static Render::TVec2 arr_los_size{ arr_size[0] * 2.0f, arr_size[1] * 2.0f };
    static float         axis_len = 2.6f;
    static float         view_axis_len = 3.0f;
    static float         view_axis_text_height = 0.08f;
    static float         view_axis_text_scale_y = 0.8f;
    static float         view_text_height = 0.07f;
    static float         view_text_scale_y = 0.7f;
    static float         axis_text_height = 0.08f;
    static float         axis_text_scale_y = 0.8f;
    static float         ndc_text_height = 0.07f;
    static float         ndc_text_scale_y = 1.0f;
    static float         ndc_scale = 0.7f;
    static float         left = -1.25f * 16.0f / 9.0f;
    static float         right = 1.25f * 16.0f / 9.0f;
    static float         top = 1.25f;
    static float         bottom = -1.25f;
    static float         near = 2.0f;
    static float         far = -2.0f;
    static Render::TVec3 camer_pos{ 4.2f, 5.4f, 4.8f };
    static float         vp_z = 0.0f;
    static float         line_th_1 = 2.0f;
    static float         line_th_2 = 1.5f;
    Render::TColor       trans_col = Color_darkred();
    Render::TColor       vp_col = Color_darkblue();
    Render::TColor       vp_col_t{ vp_col[0], vp_col[1], vp_col[2], 0.5f };

    OpenGL::Camera camera;
    camera._vp = { (int)_vpSize[0], (int)_vpSize[1] };
    camera._near = 0.7f;
    camera._far = 20.0f;
    camera._pos = camer_pos;
    camera._target = { 0.0f, 0.0f, 0.0f };
    camera._up = { 0.0f, 1.0f, 0.0f };
    camera._fov_y = 60.0f;
    Render::TMat44 prj = camera.Perspective();
    Render::TMat44 view = camera.LookAt();


    Render::TMat44 model_volume = OpenGL::Mat44().Translate({ -3.0f, 0.0f, 0.0f });
    Render::TMat44 model_ndc = OpenGL::Mat44().Translate({ 3.0f, 0.0f, 0.0f }).Scale({ ndc_scale, ndc_scale, ndc_scale });

    glm::mat4 mview;
    glm::mat4 mndc;

    std::memcpy(glm::value_ptr(mview), model_volume.data()->data(), sizeof(glm::mat4));
    std::memcpy(glm::value_ptr(mndc), model_ndc.data()->data(), sizeof(glm::mat4));

    glm::mat4 ndc_to_view = glm::inverse(mview) * mndc;
    glm::vec4 nv_lbn = ndc_to_view * glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f);
    glm::vec4 nv_ltn = ndc_to_view * glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec4 nv_lbf = ndc_to_view * glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f);
    glm::vec4 nv_ltf = ndc_to_view * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

    _draw->Projection(prj);
    _draw->View(view);
    _draw->Model(Render::Identity());

    //-------------------------------------------------------
    // draw opaque geometry
    //-------------------------------------------------------

    _draw->ActivateOpaque();

    // view volume geometry

    _draw->Model(model_volume);

    _draw->DrawPolyline(3, { right, bottom, near, nv_lbn.x, nv_lbn.y, nv_lbn.z }, trans_col, line_th_2, false);
    _draw->DrawPolyline(3, { right, top, near, nv_ltn.x, nv_ltn.y, nv_ltn.z }, trans_col, line_th_2, false);
    _draw->DrawPolyline(3, { right, bottom, far, nv_lbf.x, nv_lbf.y, nv_lbf.z }, trans_col, line_th_2, false);
    _draw->DrawPolyline(3, { right, top, far, nv_ltf.x, nv_ltf.y, nv_ltf.z }, trans_col, line_th_2, false);


    BoxWired(left, right, bottom, top, near, far, Color_darkgray(), line_th_1);

    AxisCross(view_axis_len * 0.9f, view_axis_len, view_axis_len * 1.05f, 3.0f, arr_size);

    _draw->DrawPolyline(3, { left, bottom, vp_z, right, bottom, vp_z, right, top, vp_z, left, top, vp_z }, vp_col, line_th_1, true);


    // NDC geometry

    _draw->Model(model_ndc);

    BoxWired(-1, 1, -1, 1, -1, 1, Color_darkgray(), line_th_1);
    AxisCross(axis_len * 0.9f, axis_len, -axis_len * 1.05f, 3.0f, arr_size);


    //-------------------------------------------------------
    // draw transparent geometry
    //-------------------------------------------------------

    _draw->ActivateTransparent();

    // view volume geometry

    _draw->Model(model_volume);

    _draw->DrawConvexPolygon(3, { left, bottom, vp_z, right, bottom, vp_z, right, top, vp_z, left, top, vp_z }, vp_col_t);

    // NDC geometry

    _draw->Model(model_ndc);

    //-------------------------------------------------------
    // draw text
    //-------------------------------------------------------

    _draw->ActivateOpaque();
    _draw->ClearDepth();


    // view volume text

    _draw->Model(model_volume);

    AxisCrossText(view_axis_len * 0.9f, view_axis_len, view_axis_len * 1.05f, 1, 1, 7, view_axis_text_height, view_axis_text_scale_y);

    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, "(right, top, far)", 1, view_text_height * 0.8f, view_text_scale_y, 0.0f, { right, top, far }, Color_darkgray());
    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, "(left, bottom, near)", 9, view_text_height * 0.85f, view_text_scale_y, 0.0f, { left, bottom, near }, Color_darkgray());

    const char* view_title_text = "View volume";
    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, view_title_text, 8, 0.07f, 0.7f, 0.0f, { 0.0f, 3.7f, 0.0f }, Color_black());

    // NDC text

    _draw->Model(model_ndc);

    AxisCrossText(axis_len * 0.9f, axis_len, -axis_len * 1.05f, 7, 1, 1, axis_text_height, axis_text_scale_y);

    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, "(1, 1, 1)", 1, ndc_text_height * 0.8f, ndc_text_scale_y, 0.0f, { 1.0f, 1.1f, -1.0f }, Color_darkgray());
    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, "(-1, -1, -1)", 9, ndc_text_height * 0.9f, ndc_text_scale_y, 0.0f, { -1.0f, -1.1f, 1.0f }, Color_darkgray());

    const char* ndc_title_text = "Normalized device coordinates (NDC)";
    _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, ndc_title_text, 8, 0.07f, 0.7f, 0.0f, { 0.0f, -4.5f, 0.0f }, Color_black());

}



//-------------------------------------------------------------------------------------------------
// e_model
//-------------------------------------------------------------------------------------------------


void CWindow_Glfw::Model(double time_ms)
{
    std::vector<std::array<int, 3>> pts_orig{
      { -1, -2, 9 },
      {  1, -2, 7 },
      {  1,  0, 7 },
      {  2,  0, 1 },
      {  0,  2, 2 },
      { -2,  0, 3 },
      { -1,  0, 9 }
    };

    std::vector<float> poly;
    for (auto& pt : pts_orig)
    {
        poly.push_back((float)pt[0] / 10.0f);
        poly.push_back((float)pt[1] / 10.0f);
    }

    _draw->ActivateBackground();
    Checkered(BL(), TR());

    _draw->ActivateOpaque();

    _draw->DrawPolyline(2, poly, Color_ink(), 3.0f, true);

    _draw->ActivateOpaque();
    _draw->ClearDepth();

    static float text_height = 0.05f;
    static float text_scale_y = 1.0f;
    for (size_t i = 0; i < pts_orig.size(); ++i)
    {
        auto& pt = pts_orig[i];
        std::stringstream strstr;
        strstr << "(" << pt[0] << ", " << pt[1] << ")";
        Render::TPoint3 pos{ (float)pt[0] / 10.0f, (float)pt[1] / 10.0f, 0.0f };
        pos[0] += (pos[0] < -0.001f) ? -0.02f : (pos[0] > 0.001f) ? 0.02f : 0.0f;
        pos[1] += (pos[1] < -0.001f || i == 2 || i == 6) ? -0.02f : (pos[1] > 0.001f || i == 3 || i == 5) ? 0.02f : 0.0f;
        _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, strstr.str().c_str(), pt[2], text_height, text_scale_y, 0.0f, pos, Color_darkgray());
    }
}



//-------------------------------------------------------------------------------------------------
// e_world
//-------------------------------------------------------------------------------------------------


void CWindow_Glfw::World(double time_ms)
{
    std::vector<std::array<int, 3>> pts_orig{
      { -1, -2, 9 },
      {  1, -2, 3 },
      {  1,  0, 3 },
      {  2,  0, 1 },
      {  0,  2, 4 },
      { -2,  0, 7 },
      { -1,  0, 9 }
    };

    std::vector<float> poly;
    for (auto& pt : pts_orig)
    {
        poly.push_back((float)pt[0] / 10.0f);
        poly.push_back((float)pt[1] / 10.0f);
    }

    _draw->ActivateBackground();
    Checkered(BL(), TR());

    _draw->ActivateOpaque();

    _draw->Model(Render::Identity());
    _draw->DrawPolyline(2, poly, Color_lightgray(), 3.0f, true);

    _draw->ActivateOpaque();
    _draw->ClearDepth();

    Render::TMat44 model_mat = Render::Identity();
    model_mat[0][0] = 0.0f;
    model_mat[0][1] = -0.5f;
    model_mat[1][0] = 2.0f;
    model_mat[1][1] = 0.0f;
    model_mat[3][0] = 0.4f;

    _draw->Model(model_mat);
    _draw->DrawPolyline(2, poly, Color_ink(), 3.0f, true);

    _draw->ActivateOpaque();
    _draw->ClearDepth();
    _draw->Model(Render::Identity());

    static float text_height = 0.05f;
    static float text_scale_y = 1.0f;
    for (size_t i = 0; i < pts_orig.size(); ++i)
    {
        auto& pt = pts_orig[i];
        float p[]{ (float)pt[0], (float)pt[1] };
        std::swap(p[0], p[1]);
        p[0] *= 2.0;
        p[0] += 4.0;
        p[1] *= 0.5f;

        std::stringstream strstr;
        strstr << "(" << p[0] << ", " << p[1] << ")";
        Render::TPoint3 pos{ (float)p[0] / 10.0f, (float)p[1] / 10.0f, 0.0f };
        pos[0] += (pos[0] < -0.001f || i == 2 || i == 6) ? -0.02f : (pos[0] > 0.001f) ? 0.02f : 0.0f;
        pos[1] += (pos[1] < -0.001f) ? -0.02f : (pos[1] > 0.001f) ? 0.02f : 0.0f;
        _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, strstr.str().c_str(), pt[2], text_height, text_scale_y, 0.0f, pos, Color_darkgray());
    }
}



//-------------------------------------------------------------------------------------------------
// e_view
//-------------------------------------------------------------------------------------------------


void CWindow_Glfw::View(double time_ms)
{
    // TODO $$$

    std::vector<std::array<int, 3>> pts_orig{
      { -1, -2, 3 },
      {  1, -2, 9 },
      {  1,  0, 9 },
      {  2,  0, 7 },
      {  0,  2, 4 },
      { -2,  0, 1 },
      { -1,  0, 9 }
    };

    std::vector<float> poly;
    for (auto& pt : pts_orig)
    {
        poly.push_back((float)pt[0]);
        poly.push_back((float)pt[1]);
    }

    _draw->ActivateBackground();
    Checkered(BL(), TR());

    OpenGL::Camera polyCamera;
    polyCamera._vp = { (int)_vpSize[0], (int)_vpSize[1] };
    polyCamera._near = 0.5f;
    polyCamera._far = 20.0f;
    polyCamera._pos = { -2.5f, -1.5f, 3.5f };
    polyCamera._target = { 2.0f, 0.0f, 0.0f };
    polyCamera._up = { 0.0f, 1.0f, 0.0f };
    polyCamera._fov_y = 100.0f;
    Render::TMat44 polyView = polyCamera.LookAt();

    OpenGL::Camera camera;
    camera._vp = { (int)_vpSize[0], (int)_vpSize[1] };
    camera._near = 0.5f;
    camera._far = 20.0f;
    camera._pos = { 0.0f, 2.0f, 4.0f };
    camera._target = { 0.0f, 0.0f, 0.0f };
    camera._up = { 0.0f, 1.0f, 0.0f };
    camera._fov_y = 100.0f;
    Render::TMat44 prj = OpenGL::Camera::Orthopraphic(_scale_x * 10.0f, _scale_y * 10.0f, { 0.0f, 20.0f });
    Render::TMat44 view = camera.LookAt();

    Render::TMat44 model_mat = Render::Identity();
    model_mat[0][0] = 0.0f;
    model_mat[0][1] = -0.5f;
    model_mat[1][0] = 2.0f;
    model_mat[1][1] = 0.0f;
    model_mat[3][0] = 0.4f;


    _draw->Projection(prj);
    _draw->View(view);
    _draw->Model(model_mat);

    _draw->ActivateOpaque();

    _draw->DrawPolyline(2, poly, Color_ink(), 3.0f, true);

    _draw->Model(Render::Identity());

    Render::TVec2 arr_size{ 0.2f, 0.12f };
    Render::TVec3 origin = polyCamera._pos;
    float a_len = 2.0f;
    Render::TVec3 ax{ (origin[0] + a_len * polyView[0][0]), (origin[1] + a_len * polyView[0][1]), (origin[2] + a_len * polyView[0][2]) };
    Render::TVec3 ay{ (origin[0] + a_len * polyView[1][0]), (origin[1] + a_len * polyView[1][1]), (origin[2] + a_len * polyView[1][2]) };
    Render::TVec3 az{ (origin[0] - a_len * polyView[2][0]), (origin[1] - a_len * polyView[2][1]), (origin[2] - a_len * polyView[2][2]) };
    _draw->DrawArrow(3, { origin[0], origin[1], origin[2], ax[0], ax[1], ax[2] }, Color_red_2(), 3, arr_size, false, true);
    _draw->DrawArrow(3, { origin[0], origin[1], origin[2], ay[0], ay[1], ay[2] }, Color_green_2(), 3, arr_size, false, true);
    _draw->DrawArrow(3, { origin[0], origin[1], origin[2], az[0], az[1], az[2] }, Color_blue_2(), 3, arr_size, false, true);

    _draw->ActivateOpaque();
    _draw->ClearDepth();
}



//-------------------------------------------------------------------------------------------------
// e_projection
//-------------------------------------------------------------------------------------------------


void CWindow_Glfw::Projection(double time_ms)
{
    std::vector<std::array<int, 3>> pts_orig{
      { -1, -2, 3 },
      {  1, -2, 9 },
      {  1,  0, 9 },
      {  2,  0, 7 },
      {  0,  2, 4 },
      { -2,  0, 1 },
      { -1,  0, 9 }
    };

    std::vector<float> poly;
    for (auto& pt : pts_orig)
    {
        poly.push_back((float)pt[0]);
        poly.push_back((float)pt[1]);
    }

    _draw->ActivateBackground();
    Checkered(BL(), TR());

    OpenGL::Camera camera;
    camera._vp = { (int)_vpSize[0], (int)_vpSize[1] };
    camera._near = 0.5f;
    camera._far = 20.0f;
    camera._pos = { -2.5f, -1.5f, 3.5f };
    camera._target = { 2.0f, 0.0f, 0.0f };
    camera._up = { 0.0f, 1.0f, 0.0f };
    camera._fov_y = 100.0f;
    Render::TMat44 polyView = camera.LookAt();

    Render::TMat44 model_mat = Render::Identity();
    model_mat[0][0] = 0.0f;
    model_mat[0][1] = -0.5f;
    model_mat[1][0] = 2.0f;
    model_mat[1][1] = 0.0f;
    model_mat[3][0] = 0.4f;

    _draw->Projection(camera.Perspective());
    _draw->View(camera.LookAt());
    _draw->Model(model_mat);

    _draw->ActivateOpaque();

    _draw->DrawPolyline(2, poly, Color_ink(), 3.0f, true);

    _draw->ActivateOpaque();
    _draw->ClearDepth();

    static float text_height = 0.05f;
    static float text_scale_y = 1.0f;
    for (size_t i = 0; i < pts_orig.size(); ++i)
    {
        auto& pt = pts_orig[i];
        Render::TPoint3 pos{ (float)pt[0], (float)pt[1] };
        Render::TPoint3 pos_prj = _draw->Project(pos);

        float p[]{ (float)pt[0], (float)pt[1] };

        std::stringstream strstr;
        strstr.precision(2);
        strstr << std::fixed << "(" << pos_prj[0] << ", " << pos_prj[1] << ", " << pos_prj[2] << ")";

        pos[0] += (pos[0] < -0.001f) ? -0.02f : (pos[0] > 0.001f) ? 0.02f : 0.0f;
        pos[1] += (pos[1] < -0.001f) ? -0.02f : (pos[1] > 0.001f) ? 0.02f : 0.0f;
        _draw->DrawText2DProjected(OpenGL::CBasicDraw::font_sans, strstr.str().c_str(), pt[2], text_height, text_scale_y, 0.0f, pos, Color_darkgray());
    }
}



//-------------------------------------------------------------------------------------------------
// e_cone_step
//-------------------------------------------------------------------------------------------------


void CWindow_Glfw::ConeStep(double time_ms)
{
    static Render::TVec2 arr_size{ 0.08f, 0.03f };
    static float arrow_th = 2.0f;
    static float point_size = 8.0f;
    static float text_height = 0.05f;
    static float text_scale_y = 1.0f;
    static float text_margin = 0.02f;

    static float top = 0.8f;
    static float btm = -0.8f;
    static float left = -1.0f;
    static float right = 1.0f;
    static glm::vec2 target{ -0.7f, -0.1f };
    static glm::vec2 P{ 0.9f, top };
    static glm::vec2 Q{ 0.0f, -0.4f };
    static glm::vec2 cone_hit{ target[0] - 0.1f, target[1] + 0.6f };

    static Render::TColor color_formula = Color_orange();
    static Render::TColor color_sample = Color_ink();
    static Render::TColor color_los = Color_darkgreen();
    static Render::TColor color_height_filed = Color_darkred();
    static Render::TColor color_height_map = Color_black();

    glm::vec2 cone_l = cone_hit - Q;
    cone_l = Q + glm::normalize(cone_l) * (glm::length(cone_l) + 0.2f);
    glm::vec2 cone_r = glm::vec2(2.0f * Q[0] - cone_l[0], cone_l[1]);

    glm::vec2 R = glm::normalize(target - P);
    glm::vec2 S = glm::normalize(cone_l - Q);

    //float t = glm::determinant(glm::mat2(Q-P, S)) / glm::determinant(glm::mat2(R, S));
    //float u = glm::determinant(glm::mat2(Q-P, R)) / glm::determinant(glm::mat2(R, S));
    float t = glm::dot(Q - P, glm::vec2(S.y, -S.x)) / glm::dot(R, glm::vec2(S.y, -S.x));
    float u = glm::dot(Q - P, glm::vec2(R.y, -R.x)) / glm::dot(R, glm::vec2(S.y, -S.x));

    glm::vec2 R_pt = P + R * t;
    glm::vec2 S_pt = Q + S * u;
    glm::vec2 R_mid_pt = P + R * t * 0.5f;
    glm::vec2 S_mid_pt = Q + S * u * 0.5f;

    float t_c = glm::dot(Q - P, glm::vec2(1.0f, 0.0f)) / glm::dot(R, glm::vec2(1.0f, 0.0f));
    glm::vec2 C_pt1 = P + R * t_c;
    glm::vec2 C_pt0 = glm::vec2(Q[0], btm - 0.05f);

    glm::vec2 los_0 = P - target;
    los_0 = target + glm::normalize(los_0) * (glm::length(los_0) + 0.2f);
    glm::vec2 los_1 = target - P;
    los_1 = P + glm::normalize(los_1) * (glm::length(los_1) + 0.4f);

    glm::vec2 sample_pt = glm::vec2(Q[0], btm);
    glm::vec2 sample_mid_pt = (Q + sample_pt) * 0.5f;
    glm::vec2 sample_top_pt = glm::vec2(Q[0], cone_l[1]);
    glm::vec2 sample_mid_top_pt = (sample_top_pt + Q) * 0.5f;
    glm::vec2 sample_cone_mid_pt = (sample_top_pt + cone_l) * 0.5f;

    std::vector<float> height_field{
      left,  cone_hit[1], cone_hit[0], cone_hit[1], target[0], target[1], target[0] + 0.1f, -0.7f, Q[0] - 0.4f, -0.7f,
      Q[0] - 0.3f, Q[1], Q[0], Q[1], Q[0] + 0.3f, Q[1], Q[0] + 0.4f, -0.7f,
      0.6f, -0.7f, 0.9f,  0.2f,     1.0f, 0.2f
    };

    Setup2DCheckered();

    _draw->ActivateOpaque();

    _draw->DrawPoint2D({ P[0], P[1] }, color_formula, point_size);
    _draw->DrawPoint2D({ Q[0], Q[1] }, color_formula, point_size);
    _draw->DrawPoint2D({ sample_pt[0], sample_pt[1] }, color_sample, point_size);
    _draw->DrawPoint2D({ R_pt[0], R_pt[1] }, color_los, point_size);

    _draw->DrawArrow(2, { P[0], P[1], R_pt[0], R_pt[1] }, color_formula, arrow_th, arr_size, false, true);
    _draw->DrawArrow(2, { Q[0], Q[1], S_pt[0], S_pt[1] }, color_formula, arrow_th, arr_size, false, true);
    _draw->DrawArrow(2, { los_0[0], los_0[1], target[0], target[1] }, color_los, arrow_th, arr_size, false, true);
    _draw->DrawPolyline(2, { target[0], target[1], los_1[0], los_1[1] }, color_los, 1.0, false);
    _draw->DrawArrow(2, { sample_pt[0], sample_pt[1], Q[0], Q[1] }, color_sample, arrow_th, arr_size, true, true);
    _draw->DrawArrow(2, { sample_top_pt[0], sample_top_pt[1], Q[0], Q[1] }, color_sample, arrow_th, arr_size, true, true);
    _draw->DrawArrow(2, { sample_top_pt[0], sample_top_pt[1], cone_l[0], cone_l[1] }, color_sample, arrow_th, arr_size, true, true);

    //_draw->DrawPolyline( 2, { C_pt0[0], C_pt0[1], C_pt1[0], C_pt1[1] }, color_los, arrow_th, false );
    _draw->DrawPolyline(2, { cone_l[0], cone_l[1], Q[0], Q[1], cone_r[0], cone_r[1] }, color_height_map, 2.0f, false);

    _draw->DrawPolyline(2, height_field, color_height_filed, 3.0f, false);

    _draw->DrawPolyline(2, { left, top, right, top }, Color_black(), 2.0f, false);
    _draw->DrawPolyline(2, { left, btm, right, btm }, Color_black(), 2.0f, false);

    _draw->ActivateTransparent();

    _draw->DrawConvexPolygon(2, { Q[0], Q[1], cone_l[0], cone_l[1], cone_r[0], cone_r[1] }, { 0.8f, 0.2f, 0.2f, 0.4f });

    // TODO $$$
    //
    //_draw->DrawConcavePolygon( 2, start ... height_field ... end, { 0.5f, 0.5f, 0.5f, 0.5f } );

    _draw->ActivateOpaque();
    _draw->ClearDepth();

    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "P", 7, text_height, text_scale_y, text_margin, { P[0], P[1], 0.0f }, color_formula);
    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "Q", 7, text_height, text_scale_y, text_margin, { Q[0], Q[1], 0.0f }, color_formula);
    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "R * t", 3, text_height, text_scale_y, text_margin, { R_mid_pt[0], R_mid_pt[1], 0.0f }, color_formula);
    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "S * u", 9, text_height, text_scale_y, text_margin, { S_mid_pt[0], S_mid_pt[1], 0.0f }, color_formula);
    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "X", 2, text_height, text_scale_y, 0.05f, { R_pt[0], R_pt[1], 0.0f }, color_los);

    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "h", 4, text_height, text_scale_y, text_margin, { sample_mid_pt[0], sample_mid_pt[1], 0.0f }, color_sample);
    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "1.0", 4, text_height, text_scale_y, text_margin, { sample_mid_top_pt[0], sample_mid_top_pt[1], 0.0f }, color_sample);
    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "c", 1, text_height, text_scale_y, text_margin, { sample_cone_mid_pt[0], sample_cone_mid_pt[1], 0.0f }, color_sample);
    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "Tx", 7, text_height, text_scale_y, text_margin, { sample_pt[0], sample_pt[1], 0.0f }, color_sample);

    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "Line of Sight", 3, text_height, text_scale_y, text_margin, { target[0], target[1], 0.0f }, color_los);
    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "Height field", 3, text_height, text_scale_y, text_margin, { *(height_field.rbegin() + 1), height_field.back(), 0.0f }, color_height_filed);
    _draw->DrawText2D(OpenGL::CBasicDraw::font_sans, "Height map texture ", 7, text_height, text_scale_y, text_margin, { left, btm, 0.0f }, color_height_map);
}

