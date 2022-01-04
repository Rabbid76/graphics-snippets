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
#include <OpenGL/OpenGL_Matrix_Camera.h>
#include <OpenGL/OpenGL_SimpleShaderProgram_temp.h>



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

    void Resize(int cx, int cy);

    std::chrono::high_resolution_clock::time_point _start_time;
    std::chrono::high_resolution_clock::time_point _current_time;

    std::unique_ptr<OpenGL::ShaderProgram> _prog;
    std::unique_ptr<OpenGL::ShaderProgram> _transformFeedbackProg;
    unsigned int _tfo;
    unsigned int _vao_t;

    void InitScene(void);
    void Render(double time_ms);

public:

    virtual ~CWindow_Glfw();

    void Init(int width, int height, int multisampling, bool doubleBuffer, bool visible, bool debugcontext);
    static void CallbackResize(GLFWwindow* window, int cx, int cy);
    void MainLoop(void);
};


class COpenGLContext
{
public:

    enum class TDebugLevel
    {
        off,
        all,
        error_only
    };

    void Init(TDebugLevel debug_level);

    static void DebugCallback(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message, const void* userParam);
    void DebugCallback(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message);

private:

    TDebugLevel _debug_level = TDebugLevel::off;
};


COpenGLContext::TDebugLevel debug_level = COpenGLContext::TDebugLevel::all;
int mulit_samples = 0;

int main(int argc, char** argv)
{
    if (glfwInit() == GLFW_FALSE)
        throw std::runtime_error("error initializing glfw");

    // create OpenGL window and make OpenGL context current (`glfwInit` has to be done before).
    CWindow_Glfw window;
    window.Init(800, 600, mulit_samples, true, true, debug_level != COpenGLContext::TDebugLevel::off);

    // OpenGL context needs to be current for `glewInit`
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
        throw std::runtime_error("error initializing glew");

    COpenGLContext context;
    context.Init(debug_level);

    window.MainLoop();
    return 0;
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

void CWindow_Glfw::Init(int width, int height, int multisampling, bool doubleBuffer, bool visible, bool debugcontext)
{
    _doubleBuffer = doubleBuffer;

    // [GLFW Window guide; Window creation hints](http://www.glfw.org/docs/latest/window_guide.html#window_hints_values)

    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    glfwWindowHint(GLFW_SAMPLES, multisampling);
    glfwWindowHint(GLFW_DOUBLEBUFFER, _doubleBuffer ? GLFW_TRUE : GLFW_FALSE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, debugcontext ? GLFW_TRUE : GLFW_FALSE);

    //GLFW_CONTEXT_ROBUSTNESS 

    glfwWindowHint(GLFW_VISIBLE, visible ? GLFW_TRUE : GLFW_FALSE);

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

void CWindow_Glfw::Resize(int cx, int cy)
{
    _updateViewport = true;
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
    }
}


std::string sh_vert = R"(
#version 400

layout (location = 0) in vec3 inPos;

void main()
{
    gl_Position = vec4(inPos, 1.0);
}
)";

std::string sh_frag = R"(
#version 400

out vec4 fragColor;

void main()
{
    fragColor = vec4(0.9, 0.5, 0.2, 1.0);
}
)";

std::string tf_vert = R"(
#version 400

layout (location = 0) in vec3 inPos;

out vec2 out_pos;

void main()
{
    out_pos = inPos.yx;
}
)";

std::string tf_vert_in_shader = R"(
#version 460
// if version < 440:  
// #extension GL_ARB_enhanced_layouts : enable

layout (location = 0) in vec3 inPos;

layout(xfb_buffer = 0, xfb_offset = 0) out vec2 out_pos;

void main()
{
    out_pos = inPos.yx;
}
)";

// [Transform Feedback](https://www.khronos.org/opengl/wiki/Transform_Feedback)
// [Tessellation with Transform Feedback](https://www.opengl.org/discussion_boards/showthread.php/181664-Tessellation-with-Transform-Feedback)
// [Transform feedback (tutorial)](https://open.gl/feedback)

void CWindow_Glfw::InitScene(void)
{
    _prog.reset(new OpenGL::ShaderProgram(
        {
          { sh_vert, GL_VERTEX_SHADER },
          { sh_frag, GL_FRAGMENT_SHADER }
        }));

    static const std::vector<float> varray
    {
      -0.707f, -0.75f,
       0.707f, -0.75f,
       0.0f,    0.75f
    };

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, varray.size() * sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(*varray.data()), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    static bool tf_in_shader_specification = true;
    if (tf_in_shader_specification)
    {
        _transformFeedbackProg.reset(new OpenGL::ShaderProgram(
            {
              { tf_vert_in_shader, GL_VERTEX_SHADER }
            },
            {}, 0));
    }
    else
    {
        _transformFeedbackProg.reset(new OpenGL::ShaderProgram(
            {
              { tf_vert, GL_VERTEX_SHADER }
            },
            { "out_pos" }, GL_INTERLEAVED_ATTRIBS));
    }

    // TODO $$$ test tbo allown

    GLsizeiptr transform_bytes = sizeof(*varray.data()) * varray.size();
    GLuint tbo;
    glGenBuffers(1, &tbo);

    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glBufferData(GL_ARRAY_BUFFER, transform_bytes, nullptr, GL_STATIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // the following works too
    //glBindBuffer( GL_TRANSFORM_FEEDBACK_BUFFER, tbo );
    //glBufferData( GL_TRANSFORM_FEEDBACK_BUFFER, transform_bytes, nullptr, GL_STATIC_COPY );
    //glBindBuffer( GL_TRANSFORM_FEEDBACK_BUFFER, 0 );

    std::cout << "error: " << glGetError() << std::endl;

    glGenTransformFeedbacks(1, &_tfo);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, _tfo);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

    std::cout << "error: " << glGetError() << std::endl;

    _transformFeedbackProg->Use();
    glEnable(GL_RASTERIZER_DISCARD);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, _tfo);

    static bool test_pause_resume = false;
    if (test_pause_resume == false)
    {
        GLenum tf_primitive = GL_POINTS;
        //GLenum tf_primitive = GL_TRIANGLES; // does not work why?
        glBeginTransformFeedback(tf_primitive);
        glBindVertexArray(vao);
        glDrawArrays(tf_primitive, 0, 3);
        glBindVertexArray(0);
        glEndTransformFeedback();
    }
    else
    {
        glBeginTransformFeedback(GL_POINTS);
        glBindVertexArray(vao);

        glDrawArrays(GL_POINTS, 2, 1);
        glPauseTransformFeedback();
        glResumeTransformFeedback();
        glDrawArrays(GL_POINTS, 0, 2);

        glBindVertexArray(0);
        glEndTransformFeedback();
    }

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    glDisable(GL_RASTERIZER_DISCARD);
    glUseProgram(0);
    glFlush();

    std::cout << "error: " << glGetError() << std::endl;

    std::vector<float> tf_redaback(varray.size());
    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, transform_bytes, tf_redaback.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    std::cout << "error: " << glGetError() << std::endl;

    std::cout << "data: ";
    for (float value : tf_redaback)
        std::cout << value << "  ";
    std::cout << std::endl;

    glGenVertexArrays(1, &_vao_t);
    glBindVertexArray(_vao_t);
    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    std::cout << "error: " << glGetError() << std::endl;

    _prog->Use();
}

void CWindow_Glfw::Render(double time_ms)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glBindVertexArray(_vao_t);

    //glDrawArrays( GL_TRIANGLES, 0, 3 );
    glDrawTransformFeedback(GL_TRIANGLES, _tfo);

    glBindVertexArray(0);
}


void COpenGLContext::Init(COpenGLContext::TDebugLevel debug_level)
{
    _debug_level = debug_level;

    bool enable_debug = _debug_level != TDebugLevel::off;

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

    glDebugMessageCallback(&COpenGLContext::DebugCallback, this);

    if (enable_debug)
    {
        // See also [How to use glDebugMessageControl](https://stackoverflow.com/questions/51962968/how-to-use-gldebugmessagecontrol/51963554#51963554)
        switch (_debug_level)
        {
        default:
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
            break;

        case TDebugLevel::error_only:
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
            glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, NULL, GL_TRUE);
            break;
        }

        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        std::string debug_message = "Starting debug messaging service";
        glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, (GLsizei)debug_message.size(), debug_message.c_str());
    }
}


void COpenGLContext::DebugCallback(
    unsigned int  source,     //!< I - 
    unsigned int  type,       //!< I - 
    unsigned int  id,         //!< I - 
    unsigned int  severity,   //!< I - 
    int           length,     //!< I - length of debug message
    const char* message,    //!< I - debug message
    const void* userParam) //!< I - user parameter
{
    if (userParam == nullptr)
        return;
    COpenGLContext* context_ptr = static_cast<COpenGLContext*>(const_cast<void*>(userParam));
    context_ptr->DebugCallback(source, type, id, severity, length, message);
}


void COpenGLContext::DebugCallback(
    unsigned int  source,   //!< I - 
    unsigned int  type,     //!< I - 
    unsigned int  id,       //!< I - 
    unsigned int  severity, //!< I - 
    int           length,   //!< I - length of debug message
    const char* message) //!< I - debug message
{
    static const std::vector<GLenum> error_ids
    {
        GL_INVALID_ENUM,
        GL_INVALID_VALUE,
        GL_INVALID_OPERATION,
        GL_STACK_OVERFLOW,
        GL_STACK_UNDERFLOW,
        GL_OUT_OF_MEMORY
    };

    auto error_it = std::find(error_ids.begin(), error_ids.end(), id);
    if (error_it != error_ids.end())
    {
        std::cout << "error: " << message << std::endl << std::endl;
    }
    else
    {
        std::cout << message << std::endl << std::endl;
    }
}