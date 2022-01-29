#include <stdafx.h>

#include <windows.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>
#include <stdexcept>
#include <iostream>

class shadoeGE
{
public:

    static LRESULT CALLBACK WindowProcedure(HWND hWnd, unsigned int msg, WPARAM wparam, LPARAM lparam);
    HWND Init(int width, int height);
    void DestroyWindow(void);
    void MessageLoop(void);
    void Display(void);

private:

    HWND hOGLWnd = NULL;
    HGLRC hOGLRenderContext = NULL;
};

std::wstring wnd_class(L"my_wnd_class");
shadoeGE wnd;

int main()
{
    int w = 800;
    int h = 600;

    HWND hWnd = wnd.Init(w, h);
    if (hWnd == 0)
        throw std::runtime_error("error initializing window");

    wnd.MessageLoop();
    wnd.DestroyWindow();
    return 0;
}

HWND shadoeGE::Init(int width, int height)
{
    // Get module handle
    HMODULE hModule = ::GetModuleHandle(0);
    if (!hModule)
        return NULL;

    // Create window class
    WNDCLASSEX wndClassData;
    memset(&wndClassData, 0, sizeof(WNDCLASSEX));
    wndClassData.cbSize = sizeof(WNDCLASSEX);
    wndClassData.style = CS_DBLCLKS;
    wndClassData.lpfnWndProc = WindowProcedure;
    wndClassData.cbClsExtra = 0;
    wndClassData.cbWndExtra = 0;
    wndClassData.hInstance = hModule;
    wndClassData.hIcon = ::LoadIcon(0, IDI_APPLICATION);
    wndClassData.hCursor = ::LoadCursor(0, IDC_ARROW);
    wndClassData.hbrBackground = ::CreateSolidBrush(COLOR_WINDOW + 1);
    wndClassData.lpszMenuName = 0;
    wndClassData.lpszClassName = wnd_class.c_str();
    wndClassData.hIconSm = 0;
    if (!::RegisterClassEx(&wndClassData))
        return NULL;

    // Creaate Window
    hOGLWnd = ::CreateWindow(wnd_class.c_str(), NULL, WS_OVERLAPPEDWINDOW, 0, 0, width, height, NULL, NULL, hModule, NULL);
    if (hOGLWnd == NULL)
        return NULL;

    // Get device context
    HDC hDC = ::GetDC(hOGLWnd);

    // Create OpenGL context
    DWORD pixelFormatFlags = PFD_SUPPORT_OPENGL | PFD_SUPPORT_COMPOSITION | PFD_GENERIC_ACCELERATED | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    PIXELFORMATDESCRIPTOR pfd =
    {
      sizeof(PIXELFORMATDESCRIPTOR),
      1,
      pixelFormatFlags,         //Flags
      PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
      32,                       //Colordepth of the framebuffer.
      0, 0, 0, 0, 0, 0,
      0,
      0,
      0,
      0, 0, 0, 0,
      24,                        //Number of bits for the depthbuffer
      8,                        //Number of bits for the stencilbuffer
      0,                        //Number of Aux buffers in the framebuffer.
      PFD_MAIN_PLANE,
      0,
      0, 0, 0
    };
    int pixelFormat = ::ChoosePixelFormat(hDC, &pfd);
    ::SetPixelFormat(hDC, pixelFormat, &pfd);
    hOGLRenderContext = ::wglCreateContext(hDC);

    // make OpenGL context the current context
    ::wglMakeCurrent(hDC, hOGLRenderContext);

    if (glewInit() != GLEW_OK)
        throw std::runtime_error("error initializing glew");

    int major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    std::cout << glGetString(GL_VERSION) << " - " << major << "." << minor << std::endl;

    // release device context
    ::ReleaseDC(hOGLWnd, hDC);

    // show the window
    ::ShowWindow(hOGLWnd, SW_SHOWDEFAULT);
    return hOGLWnd;
}

void shadoeGE::MessageLoop(void)
{
    MSG msg;
    while (::GetMessage(&msg, 0, 0, 0))
        ::DispatchMessage(&msg);
}

void shadoeGE::DestroyWindow(void)
{
    ::DestroyWindow(hOGLWnd);
    ::wglMakeCurrent(NULL, NULL);
    ::wglDeleteContext(hOGLRenderContext);

    HMODULE hModule = ::GetModuleHandle(0);
    if (!hModule)
        return;
    ::UnregisterClass(wnd_class.c_str(), hModule);
}

LRESULT CALLBACK shadoeGE::WindowProcedure(HWND hWnd, unsigned int msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_PAINT:
        wnd.Display();
        break;
    }
    return DefWindowProc(hWnd, msg, wparam, lparam);
}

void shadoeGE::Display(void)
{
    RECT clientRect;
    ::GetClientRect(hOGLWnd, &clientRect);

    glViewport(0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBegin(GL_TRIANGLES);

    glColor3f(0.1f, 0.2f, 0.3f);
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);
    glVertex3f(0, 1, 0);

    glEnd();

    glColor3f(1.0f, 0.0f, 0.0f);

    glEnableVertexAttribArray(0);
    GLfloat verts[] = { 0.5, 0.0, 0.0, -0.5, 0.0, 0.0, 0.0, 0.5, 0.0 };
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, verts);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableVertexAttribArray(0);

    HDC hDC = ::GetDC(hOGLWnd);
    ::SwapBuffers(hDC);
    ::ReleaseDC(hOGLWnd, hDC);
}