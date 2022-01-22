// TODO: fix drawing


#include <stdafx.h>

#include <GL/glew.h>
#include <GL/wglew.h>

#include <iostream>
#include <string>

#include <windows.h>

// project includes
#include <gl/gl_debug.h>


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void display(void);

std::string errorStr = "none";

int InitOpengl(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow,
    HWND& hwnd,
    HDC& hdc,
    HGLRC& hrc)
{
    //---- fake Window
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"test_class";
    wcex.hIconSm = NULL;

    if (!RegisterClassEx(&wcex))
    {
        errorStr = "RegisterClassEx";
        return 0;
    }

    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 32;
    pfd.iLayerType = PFD_MAIN_PLANE;

    HWND temp_hwnd = CreateWindow(
        L"test_class",
        L"test",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        500, 500,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    HDC temp_hdc = GetDC(temp_hwnd);

    int temp_nPixelFormat = ChoosePixelFormat(temp_hdc, &pfd);
    SetPixelFormat(temp_hdc, temp_nPixelFormat, &pfd);

    HGLRC temp_hrc = wglCreateContext(temp_hdc);
    wglMakeCurrent(temp_hdc, temp_hrc);
    glewInit();

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
    OpenGL::CContext context;
    context.Init(debug_level);

    bool wg_support = wglewIsSupported("WGL_ARB_create_context") == 1;

    if (wg_support == false)
    {
        hwnd = temp_hwnd;
        hdc = temp_hdc;
        hrc = temp_hrc;
        errorStr = "WGL_ARB_create_context";
        return 0;
    }

    // OpenGL context window

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(temp_hrc);
    ReleaseDC(temp_hwnd, temp_hdc);
    DestroyWindow(temp_hwnd);

    hwnd = CreateWindow(
        L"test_class",
        L"OpenGL context window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        500, 500,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    hdc = GetDC(hwnd);

    const int iPixelFormatAttribList[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
        0 // End of attributes list
    };
    int attributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        0
    };

    int nPixelFormat = 0;
    UINT iNumFormats = 0;
    wglChoosePixelFormatARB(hdc, iPixelFormatAttribList, NULL, 1, &nPixelFormat, (UINT*)&iNumFormats);
    SetPixelFormat(hdc, nPixelFormat, &pfd);
    hrc = wglCreateContextAttribsARB(hdc, 0, attributes);
    wglMakeCurrent(NULL, NULL);
    wglMakeCurrent(hdc, hrc);

    return 1;
}


bool progRun = false;
int CALLBACK WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow
)
{
    HWND hwnd = NULL;
    HDC  hdc = NULL;
    HGLRC hrc = NULL;
    InitOpengl(hInstance, hPrevInstance, lpCmdLine, nCmdShow, hwnd, hdc, hrc);

    ShowWindow(hwnd, SW_SHOW);

    glClearColor(0.2, 0.4, 0.3, 1);

    MSG msg;

    progRun = true;
    while (progRun)
    {

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, 500, 500);
        display();
        GLint errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
        {
            std::cout << "error: " << errorCode << std::endl;
        }

        SwapBuffers(hdc);
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        // CreateContext( hWnd );
        break;

    case WM_DESTROY:
        //DestroyContext( hWnd );
        PostQuitMessage(0);  // TODO $$$ close and destroy window ???
        progRun = false;
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void display(void)
{
    glLineWidth(5.0);
    glColor4f(1, 1, 1, 1);
    glBegin(GL_LINES);
    glVertex2f(-0.8, -0.8);
    glVertex2f(0.8, 0.8);
    glEnd();
}