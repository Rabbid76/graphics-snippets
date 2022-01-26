[![StackOverflow](https://stackexchange.com/users/flair/7322082.png)](https://stackoverflow.com/users/5577765/rabbid76?tab=profile)

---

# C++ code snippets

## OpenGL Window

### GLUT

[GLUT](https://www.opengl.org/resources/libraries/glut/)  
[freeglut - The Open-Source OpenGL Utility Toolkit](http://freeglut.sourceforge.net/docs/api.php)

Set up [*GLUT*](https://www.opengl.org/resources/libraries/glut/) by [`glutInit`](https://www.opengl.org/resources/libraries/glut/spec3/node10.html)

```cpp
glutInit(&argc, argv);
glutCreateWindow("OGL window");
```

GLUT uses Legacy Profile as default for all created OpenGL contexts.

[freeglut](http://freeglut.sourceforge.net/docs/api.php) extends glut by `glutLeaveMainLoop` and `glutMainLoopEvent`.

e.g.:

```cpp
bool condtion = true;
while (condtion)
{
    glutMainLoopEvent(); // handle the main loop once
    glutPostRedisplay(); // cause `display` to be called in `glutMainLoopEvent`

    condtion = ...;
}
```

**[Render a frame on demand](https://stackoverflow.com/questions/52960066/render-a-frame-on-demand/52976088#52976088)**:  

```c++
void display( void )
{
    glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // [...]
    glFlush();
}

int main()
{
    // [...]
    glutDisplayFunc(display);  
    // [...]

    for(;;)
    {
        glutMainLoopEvent(); // handle the main loop once
        glutPostRedisplay(); // cause `display` to be called in `glutMainLoopEvent`
    }

    // [...]
}
```

It is even possible to set a dummy display function, which does nothing, and do the drawing in the loop:

e.g.

```c++
void dummyDisplay( void )
{
    // does nothing
}

int main()
{
    // [...]
    glutDisplayFunc(dummyDisplay);  
    // [...]

    for(;;)
    {
        glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        // Whatever I might have to do goes somewhere in here
        glFlush();

        glutMainLoopEvent(); // does the event handling once
    }

    // [...]
}
```

### GLFW

[GLFW](https://www.glfw.org/)  
[glfw - pip](https://pypi.org/project/glfw/)  

[`glfwInit`](http://www.glfw.org/docs/latest/group__init.html#ga317aac130a235ab08c6db0834907d85e) returns `GLFW_TRUE` if succeded:

```c++
if ( glfwInit() != GLFW_TRUE )
    return;

GLFWwindow *wnd = glfwCreateWindow( width, height, "OGL window", nullptr, nullptr );
if ( wnd == nullptr )
{
    glfwTerminate();
    return;
}

glfwMakeContextCurrent( wnd );
```

[Is there a way to remove 60 fps cap in GLFW?](https://stackoverflow.com/questions/50412575/is-there-a-way-to-remove-60-fps-cap-in-glfw/50427847#50427847):  

The easiest way is to use single buffering instead of double buffering. Since at single buffering is always use the same buffer there is no buffer swap and no "vsync".

Use the [`glfwWindowHint`](http://www.glfw.org/docs/latest/group__window.html#ga7d9c8c62384b1e2821c4dc48952d2033) to disable double buffering:

```++
glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
GLFWwindow *wnd = glfwCreateWindow(w, h, "OGL window", nullptr, nullptr);
```

Note, when you use singel buffering, then you have to explicite force execution of the GL commands by ([`glFlush`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glFlush.xml)), instead of the buffer swap ([`glfwSwapBuffers`](http://www.glfw.org/docs/3.0/group__context.html#ga15a5a1ee5b3c2ca6b15ca209a12efd14)).

Another possibility is to set the number of screen updates to wait from the time [`glfwSwapBuffers`](http://www.glfw.org/docs/3.0/group__context.html#ga15a5a1ee5b3c2ca6b15ca209a12efd14) was called before swapping the buffers to 0. This can be done by [`glfwSwapInterval`](http://www.glfw.org/docs/latest/group__context.html#ga6d4e0cdf151b5e579bd67f13202994ed), after making the OpenGL context current ([`glfwMakeContextCurrent`](http://www.glfw.org/docs/3.0/group__context.html#ga1c04dc242268f827290fe40aa1c91157)):

```cpp
glfwMakeContextCurrent(wnd);
glfwSwapInterval(0);
```

But note, whether this solution works or not, may depend on the hardware and the driver.

[glfwSetCursorPosCallback to function in another class](https://stackoverflow.com/questions/29356783/glfwsetcursorposcallback-to-function-in-another-class/59633789#59633789):  

An alternative solution would be to associate a pointer to `controls` to the `GLFWindow`. See [`glfwSetWindowUserPointer`](http://www.glfw.org/docs/latest/group__window.html#ga3d2fc6026e690ab31a13f78bc9fd3651).

The pointer can be retrieved at an time form the `GLFWWindow` object by [`glfwGetWindowUserPointer`](http://www.glfw.org/docs/latest/group__window.html#ga17807ce0f45ac3f8bb50d6dcc59a4e06). Of course the return type is `void*` and has to be casted to `Controls*`.  

Instead of the global function or static method a [Lambda expressions](https://en.cppreference.com/w/cpp/language/lambda) can be used. e.g:

```cpp
glfwSetWindowUserPointer(window, this->controls);

glfwSetCursorPosCallback( window, [](GLFWwindow *window, double x, double y)
{
    if (Controls *controls = static_cast<Controls*>(glfwGetWindowUserPointer(window)))
        controls->handleMouse(window, x, y);
} );
```

**[Switching Between windowed and full screen in OpenGL/GLFW 3.2](https://stackoverflow.com/questions/47402766/switching-between-windowed-and-full-screen-in-opengl-glfw-3-2/47462358#47462358)**  
**[Is there a way to remove 60 fps cap in GLFW?](https://stackoverflow.com/questions/50412575/is-there-a-way-to-remove-60-fps-cap-in-glfw/50427847#50427847)**  
[How to prevent GLFW window from showing up right in creating?](https://stackoverflow.com/questions/66299684/how-to-prevent-glfw-window-from-showing-up-right-in-creating/66300310#66300310)  
[Retrieving data from callback function](https://stackoverflow.com/questions/34316362/retrieving-data-from-callback-function/34316692#343166920)  
[How can i know which opengl version is supported by my system](https://stackoverflow.com/questions/46510889/how-can-i-know-which-opengl-version-is-supported-by-my-system/46511139#46511139)  
[PyOpenGL OpenGL Version on MacOs](https://stackoverflow.com/questions/52915196/pyopengl-opengl-version-on-macos/52915323#52915323)  

### SDL

Init [*SDL*](https://wiki.libsdl.org/).

```c++
SDL_Window *window = SDL_CreateWindow(""OGL window", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL );
SDL_GLContext glContext = SDL_GL_CreateContext( window );
```

Check for errors by [`SDL_GetError`](https://wiki.libsdl.org/SDL_GetError).

Make context current by [`SDL_GL_MakeCurrent`](https://wiki.libsdl.org/SDL_GL_MakeCurrent):

```c++
SDL_GL_MakeCurrent( window, glContext );
```

[screen resolution in c++](https://stackoverflow.com/questions/35733165/screen-resolution-in-c/35733427#35733427)  

```c++
SDL_Init(SDL_INIT_VIDEO);
const SDL_VideoInfo* videoInfo = SDL_GetVideoInfo(); 
width = videoInfo->current_w;
height = videoInfo->current_h;
```

[Using SDL2 and OpenGL to rotate camera and a triangle draw wont display anything?](https://stackoverflow.com/questions/53695394/using-sdl2-and-opengl-to-rotate-camera-and-a-triangle-draw-wont-display-anything/53695582#53695582)  

If you want to use the deprecated way of drawing, then you have to use a compatibility profile context instead of a core profile context (see [OpenGL Context](https://www.khronos.org/opengl/wiki/OpenGL_Context)):

```cpp
SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
```

### Windows, wgl, win32

[Render image using OpenGL in win32 window](https://stackoverflow.com/questions/47809267/render-image-using-opengl-in-win32-window/47821419#47821419)  

> TOOD

[Child window with OpenGL not responding to gluOrtho2D()](https://stackoverflow.com/questions/57056104/child-window-with-opengl-not-responding-to-gluortho2d/57056579#57056579)  

> TODO

### SFML

Your window (default framebuffer) doesn't have a stencil buffer at all. You have to setup a OpenGL window with a stencil buffer by using the [`sf::ContextSettings`](https://www.sfml-dev.org/documentation/2.5.1/structsf_1_1ContextSettings.php) class.

See [Using OpenGL in a SFML window](https://www.sfml-dev.org/tutorials/2.5/window-opengl.php):

```c++
sf::ContextSettings settings;
settings.depthBits = 24;
settings.stencilBits = 8;
settings.antialiasingLevel = 4;
settings.majorVersion = 4;
settings.minorVersion = 6;

sf::Window window(sf::VideoMode(1200, 720), "My window", sf::Style::Default, settings);
```

## Loader

### GLEW

[GLEW](http://glew.sourceforge.net/) is the OpenGL Extension Wrangler Library, it will be necessary if you want to use "modern" OpenGL.

The [GLEW](http://glew.sourceforge.net/) library has to be initialized, by `glewInit`, after the OpenGL context has become current by [`glfwMakeContextCurrent`](https://www.glfw.org/docs/latest/group__context.html#ga1c04dc242268f827290fe40aa1c91157).  
See [Initializing GLEW](http://glew.sourceforge.net/basic.html).

To link the [GLEW](http://glew.sourceforge.net/) library correctly, proper preprocessor definitions have to be set. See [GLEW - Installation](http://glew.sourceforge.net/install.html):

> [...] On Windows, you also need to define the `GLEW_STATIC` preprocessor token when building a static library or executable, and the `GLEW_BUILD` preprocessor token when building a dll [...]

You have to [Initialize GLEW](http://glew.sourceforge.net/basic.html). Call `glewInit` immediately after creating the OpenGL context:

```cpp
if ( glewInit() != GLEW_OK )
    return;
```

Note, that [`glewInit`](http://glew.sourceforge.net/basic.html) will return `GLEW_OK` f it was successful. [`glewInit`](http://glew.sourceforge.net/basic.html) initializes the function pointers for the OpenGL functions. If you try to call the function through an uninitialized function pointer, a segmentation fault occurs.

[GLEW - why should I define GLEW_STATIC?](https://stackoverflow.com/questions/49741322/glew-why-should-i-define-glew-static/49741747#49741747)  

The signature of a function which is exported by or imported from a static library is marked by the keyword [`extern`](http://en.cppreference.com/w/cpp/keyword/export). A function which is imported from a dynamic library has to be marked by [`extern __declspec(dllimport)`](https://msdn.microsoft.com/en-us/library/3y1sfaz2.aspx). `GLEW_STATIC` is a preprocessor definition which activates the first case.

The relevant code part in the "glew.h" file (verison 2.1.0) is:

```cpp
/*
  * GLEW_STATIC is defined for static library.
  * GLEW_BUILD  is defined for building the DLL library.
  */

#ifdef GLEW_STATIC
  define GLEWAPI extern
#else
  ifdef GLEW_BUILD
    define GLEWAPI extern __declspec(dllexport)
  else
    define GLEWAPI extern __declspec(dllimport)
  endif
#endif
```

### GLAD

[Glad](https://glad.dav1d.de/) Loader-Generator has to be initialized by either [`gladLoadGL`](https://github.com/Dav1dde/glad/blob/master/README.md) or [`gladLoadGLLoader`](https://github.com/Dav1dde/glad/blob/master/README.md), right after crating and making current the OpenGL context by [`SDL_GL_CreateContext`](https://wiki.libsdl.org/SDL_GL_CreateContext).  
See also [OpenGL Loading Library - glad](https://www.khronos.org/opengl/wiki/OpenGL_Loading_Library#glad_.28Multi-Language_GL.2FGLES.2FEGL.2FGLX.2FWGL_Loader-Generator.29)

e.g.:

```cpp
const SDL_GLContext context = SDL_GL_CreateContext(window);
if (context == nullptr) {
    std::cout << "SDL could not create context";
    return 1;
}

if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
{
    std::cout << "Failed to initialize OpenGL context" << std::endl;
    return -1;
}
```

## OpenGL extensions

[OpenGL Extension](https://www.khronos.org/opengl/wiki/OpenGL_Extension)

With [`glGetIntegerv(GL_NUM_EXTENSIONS, ...)`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGet.xhtml), the number of extension supported by the GL implementation can be get.  
With [`glGetStringi(GL_EXTENSIONS, ...)`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetString.xhtml) the name of an extension can be asked.

Read the extensions to a `std::set`

```cpp
#include <set>
#include <string>
```

```cpp
GLint no_of_extensions = 0;
glGetIntegerv(GL_NUM_EXTENSIONS, &no_of_extensions);

std::set<std::string> ogl_extensions;
for ( int i = 0; i < no_of_extensions; ++i )
    ogl_extensions.insert( (const char*)glGetStringi(GL_EXTENSIONS, i) );
```

Check if an extension is supported:

```cpp
bool filter_anisotropic = 
    ogl_extensions.find( "GL_ARB_texture_filter_anisotropic" ) != ogl_extensions.end();
```

Alternatively, you can use `glewIsSupported` to ask for an extension. See [GLEW - Checking for Extensions](http://glew.sourceforge.net/basic.html).

## Debug context and Debug output

[Debug Output](https://www.khronos.org/opengl/wiki/Debug_Output)

Related Stack Overflow questions:

- [How to use glDebugMessageControl](https://stackoverflow.com/questions/51962968/how-to-use-gldebugmessagecontrol/51963554#51963554)
- [How to interpret GL_DEBUG_OUTPUT messages?](https://stackoverflow.com/questions/65123385/how-to-interpret-gl-debug-output-messages/65129048#65129048)

```cpp
#include <stdexcept>
#include <iostream>

void GLAPIENTRY DebugCallback( 
    unsigned int  source,     //!< I - 
    unsigned int  type,       //!< I - 
    unsigned int  id,         //!< I - 
    unsigned int  severity,   //!< I - 
    int           length,     //!< I - length of debug message
    const char   *message,    //!< I - debug message
    const void   *userParam ) //!< I - user parameter
{
   std::cout << message << std::endl;
}

void init_opengl_debug() {
    glewExperimental = true;
    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    if ( glDebugMessageCallback != nullptr && glDebugMessageControl != nullptr )
    {
        glDebugMessageCallback( &DebugCallback, nullptr );
        glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE );
        //glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
        //glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, NULL, GL_TRUE);
        glEnable( GL_DEBUG_OUTPUT );
        glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
    }
}
```

## Load shader file

Related Stack Overflow questions:

- [OpenGL shaders don't compile](https://stackoverflow.com/questions/47853523/opengl-shaders-dont-compile/47853880#47853880)  

Pass `std::ifstream::failbit` to [`std::ios::exceptions`](http://www.cplusplus.com/reference/ios/ios/exceptions/), because if [`std::ifstream::open`](http://www.cplusplus.com/reference/fstream/ifstream/open/) fails, the failbit status flag is set.

```cpp
vShaderFile.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
fShaderFile.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
try
{
    vShaderFile.open( vertexPath );
    fShaderFile.open( fragmentPath );
    .....
}
catch ( std::ifstream::failure e )
{
    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
}
```

```cpp
sourceCode = std::string(std::istreambuf_iterator<char>(sourceFile), std::istreambuf_iterator<char>());
```

## Compile and link shader

Related Stack Overflow questions:

- [OpenGL ignores Quads and makes them Triangles](https://stackoverflow.com/questions/53218925/opengl-ignores-quads-and-makes-them-triangles/53220726#53220726)  
- [light shows but the cube does not appear](https://stackoverflow.com/questions/59235204/light-shows-but-the-cube-does-not-appear/59236020#59236020)
- [glGetAttribLocation is causing invalid operation](https://stackoverflow.com/questions/50805413/glgetattriblocation-is-causing-invalid-operation/50805535#50805535)  

I recommend to check if the shader compilation succeeded and if the program object linked successfully.

If the compiling of a shader succeeded can be checked by [`glGetShaderiv`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetShader.xhtml) and the parameter `GL_COMPILE_STATUS`. e.g.:

```cpp
#include <iostream>
#include <vector>
```

```cpp
bool CompileStatus( GLuint shader )
{
    GLint status = GL_TRUE;
    glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
    if (status == GL_FALSE)
    {
        GLint logLen;
        glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logLen );
        std::vector< char >log( logLen );
        GLsizei written;
        glGetShaderInfoLog( shader, logLen, &written, log.data() );
        std::cout << "compile error:" << std::endl << log.data() << std::endl;
    }
    return status != GL_FALSE;
}
```

If the linking of a program was successful can be checked by [`glGetProgramiv`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetProgram.xhtml) and the parameter `GL_LINK_STATUS`. e.g.:

```cpp
bool LinkStatus( GLuint program )
{
    GLint status = GL_TRUE;
    glGetProgramiv( program, GL_LINK_STATUS, &status );
    if (status == GL_FALSE)
    {
        GLint logLen;
        glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logLen );
        std::vector< char >log( logLen );
        GLsizei written;
        glGetProgramInfoLog( program, logLen, &written, log.data() );
        std::cout << "link error:" << std::endl << log.data() << std::endl;
    }
    return status != GL_FALSE;
}
```

## Query the alignment and stride for an SSBO structure

[Shader Storage Buffer Object SSBO](https://www.khronos.org/opengl/wiki/Shader_Storage_Buffer_Object)

Related Stack Overflow questions:

- [How do I query the alignment/stride for an SSBO struct?](https://stackoverflow.com/questions/56512216/how-do-i-query-the-alignment-stride-for-an-ssbo-struct/56513136#56513136)  

Use [`glGetProgramInterface`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetProgramInterface.xhtml) with the parameter `GL_SHADER_STORAGE_BLOCK` to get the number of the
[Shader Storage Buffer Objects](https://www.khronos.org/opengl/wiki/Shader_Storage_Buffer_Object) and the maximum name length.  
The maximum name length of the buffer variables can be get from the program interface `GL_BUFFER_VARIABLE`:

```cpp
GLuint prog_obj; // shader program object
```

```cpp
GLint no_of, ssbo_max_len, var_max_len;
glGetProgramInterfaceiv(prog_obj, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &no_of);
glGetProgramInterfaceiv(prog_obj, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &ssbo_max_len);
glGetProgramInterfaceiv(prog_obj, GL_BUFFER_VARIABLE, GL_MAX_NAME_LENGTH, &var_max_len);
```

The name of the SSBO can be get by [`glGetProgramResourceName`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetProgramResourceName.xhtml) and a resource index by [`glGetProgramResourceIndex`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetProgramResourceIndex.xhtml):

```cpp
std::vector< GLchar >name( max_len );
for( int i_resource = 0; i_resource < no_of; i_resource++ ) {

    // get name of the shader storage block
    GLsizei strLength;
    glGetProgramResourceName(
        prog_obj, GL_SHADER_STORAGE_BLOCK, i_resource, ssbo_max_len, &strLength, name.data());

    // get resource index of the shader storage block
    GLint resInx = glGetProgramResourceIndex(prog_obj, GL_SHADER_STORAGE_BLOCK, name.data());

    // [...]
}
```

Data of the shader storage block can be retrieved by [`glGetProgramResource`](https://www.khronos.org/opengl/wiki/GLAPI/glGetProgramResource). See also [Program Introspection](https://www.khronos.org/opengl/wiki/Program_Introspection).

Get the number of of buffer variables and its indices from program interface and `GL_SHADER_STORAGE_BLOCK` and the shader storage block resource `resInx`:

```cpp
for( int i_resource = 0; i_resource < no_of; i_resource++ ) {

    // [...]

    GLint resInx = ...

    // get number of the buffer variables in the shader storage block
    GLenum prop = GL_NUM_ACTIVE_VARIABLES;
    GLint num_var;
    glGetProgramResourceiv(
        prog_obj, GL_SHADER_STORAGE_BLOCK, resInx, 1, &prop,
        1, nullptr, &num_var);

    // get resource indices of the buffer variables
    std::vector<GLint> vars(num_var);
    prop = GL_ACTIVE_VARIABLES;
    glGetProgramResourceiv(
        prog_obj, GL_SHADER_STORAGE_BLOCK, resInx,
        1, &prop, (GLsizei)vars.size(), nullptr, vars.data());

    // [...]
}
```

Get the offsets of the buffer variables, in basic machine units, relative to the base of buffer and its names from the program interface `GL_BUFFER_VARIABLE` and the resource indices `vars[]`:

```cpp
for( int i_resource = 0; i_resource < no_of; i_resource++ ) {

    // [...]

    std::vector<GLint> offsets(num_var);
    std::vector<std::string> var_names(num_var);
    for (GLint i = 0; i < num_var; i++) {
        
        // get offset of buffer variable relative to SSBO
        GLenum prop = GL_OFFSET;
        glGetProgramResourceiv(
            prog_obj, GL_BUFFER_VARIABLE, vars[i],
            1, &prop, (GLsizei)offsets.size(), nullptr, &offsets[i]);

        // get name of buffer variable
        std::vector<GLchar>var_name(var_max_len);
        GLsizei strLength;
        glGetProgramResourceName(
            prog_obj, GL_BUFFER_VARIABLE, vars[i], 
            var_max_len, &strLength, var_name.data());
        var_names[i] = var_name.data();
    }

    // [...]
}
```

See also [`ARB_shader_storage_buffer_object`](https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_shader_storage_buffer_object.txt)

## Mesh

### Load Wave front file (OBJ)

Related Stack Overflow questions:

- [How do I sort the texture positions based on the texture indices given in a Wavefront (.obj) file?](https://stackoverflow.com/questions/51708275/how-do-i-sort-the-texture-positions-based-on-the-texture-indices-given-in-a-wave/51711080#51711080)  

If there are different indexes for vertex coordinates and texture coordinates, then the vertex positions must be "duplicated".  
The vertex coordinate and its attributes (like texture coordinate) form a tuple. Each vertex coordinate must have its own texture coordinates and attributes. You can think of a 3D vertex coordinate and a 2D texture coordinate as a single 5D coordinate.
See [Rendering meshes with multiple indices](https://stackoverflow.com/questions/11148567/rendering-meshes-with-multiple-indices/11148568#11148568).

The vertex attributes for each vertex position form a set of data. This means you have to create tuples of vertex coordinate, and texture coordiantes.

Let's assume that you have a *.obj* file like this:

```lang-none
v -1 -1 -1
v  1 -1 -1
v -1  1 -1
v  1  1 -1
v -1 -1  1
v  1 -1  1
v -1  1  1
v  1  1  1

vt 0 0
vt 0 1
vt 1 0
vt 1 1

vn -1  0  0
vn  0 -1  0
vn  0  0 -1
vn  1  0  0
vn  0  1  0
vn  0  0  1

f 3/1/1 1/2/1 5/4/1 7/3/1
f 1/1/2 2/2/2 3/4/2 6/3/2
f 3/1/3 4/2/3 2/4/3 1/3/3
f 2/1/4 4/2/4 8/4/4 6/3/4
f 4/1/5 3/2/5 7/4/5 8/3/5
f 5/1/6 6/2/6 8/4/6 7/3/6
```

From this you have to find all the combinations of vertex coordinate, texture texture coordinate and normal vector indices, which are used in the face specification:

```lang-none
0 : 3/1/1
1 : 1/2/1
2 : 5/4/1
3 : 7/3/1
4 : 1/1/2
5 : 2/2/2
6 : 3/4/2
7 : 6/3/2
8 : ...
```

Then you have to create a vertex coordinate, texture coordinate and normal vector array corresponding to the array of index combinations.
The vertex coordinates and its attributes can either be combined in one array to data sets, or to three arrays with equal number of attributes:

```lang-none
index   vx vy vz     u v     nx ny nz
0 :     -1  1 -1     0 0     -1  0  0
1 :     -1 -1 -1     0 1     -1  0  0
2 :     -1 -1  1     1 1     -1  0  0
3 :     -1  1  1     1 0     -1  0  0
4 :     -1 -1 -1     0 0      0 -1  0
5 :      1 -1 -1     0 1      0 -1  0
6 :     -1  1 -1     1 1      0 -1  0
7 :      1 -1  1     1 0      0 -1  0
8 : ...
```

See the very simple c++ function, which can read an *.obj* file, like that you linked to.
The function reads a file and writes the data to an element vector and an attribute vector.

Note, the function can be optimized and does not care about performance.
For a small file (like *cube3.obj* which you liked to), that doesn't matter, but for large file,
especially the linear search in the index table, will have to be improved.

I just tried to give you an idea how to read an *.obj* file and how to create an element and attribute vector, which can be directly used to draw an mesh with the use of OpenGL.

```cpp
#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <strstream>
#include <algorithm>

bool load_obj( 
    const std::string          filename, 
    std::vector<unsigned int> &elements,
    std::vector<float>        &attributes )
{
    std::ifstream obj_stream( filename, std::ios::in );
    if( !obj_stream )
        return false;

    // parse the file, line by line
    static const std::string white_space = " \t\n\r";
    std::string token, indices, index;
    float value;
    std::vector<float> v, vt, vn;
    std::vector<std::array<unsigned int, 3>> f;
    for( std::string line; std::getline( obj_stream, line ); )
    {
        // find first non whispce characterr in line
        size_t start = line.find_first_not_of( white_space );
        if ( start == std::string::npos )
            continue;

        // read the first token
        std::istringstream line_stream( line.substr(start) );
        line_stream.exceptions( 0 );
        line_stream >> token;

        // ignore comment lines
        if ( token[0] == '#' )
            continue;

        // read the line
        if ( token == "v" ) // read vertex coordinate
        {
            while ( line_stream >> value )  
                v.push_back( value );
        }
        else if ( token == "vt" ) // read normal_vectors 
        {
            while ( line_stream >> value )
                vt.push_back( value );
        }
        else if ( token == "vn" )  // read normal_vectors 
        {
            while ( line_stream >> value )
                vn.push_back( value );
        }
        else if ( token == "f" )
        {
            // read faces
            while( line_stream >> indices )
            {
                std::array<unsigned int, 3> f3{ 0, 0, 0 };
                // parse indices
                for ( int j=0; j<3; ++ j )
                {
                    auto slash = indices.find( "/" );
                    f3[j] = std::stoi(indices.substr(0, slash), nullptr, 10);
                    if ( slash == std::string::npos )
                        break;
                    indices.erase(0, slash + 1);
                }

                // add index
                auto it = std::find( f.begin(), f.end(), f3 );
                elements.push_back( (unsigned int)(it - f.begin()) );
                if ( it == f.end() )
                    f.push_back( f3 );
            }
        }
    }

    // create array of attributes from the face indices
    for ( auto f3 : f )
    {
        if ( f3[0] > 0 )
        {
            auto iv = (f3[0] - 1) * 3;
            attributes.insert( attributes.end(), v.begin() + iv, v.begin() + iv + 3 );
        }

        if ( f3[1] > 0 )
        {
            auto ivt = (f3[1] - 1) * 2;
            attributes.insert( attributes.end(), vt.begin() + ivt, vt.begin() + ivt + 2 );
        }

        if ( f3[2] > 0 )
        {
            auto ivn = (f3[2] - 1) * 3;
            attributes.insert( attributes.end(), vn.begin() + ivn, vn.begin() + ivn + 3 );
        }
    }

    return true;
}
```

### Spring

Related Stack Overflow questions:

```c++
void createSpring(
    GLfloat rounds, GLfloat height, GLfloat thickness, GLfloat radius, 
    std::vector<GLfloat> &vertices, std::vector<GLuint> &indices)
{
    const int slices = 32;
    const int step = 5;
    for (int i = -slices; i <= rounds * 360 + step; i += step)
    {
        for (int j = 0; j < slices; j ++)
        {
            GLfloat t = (GLfloat)i / 360 + (GLfloat)j / slices * step / 360;
            t = std::max(0.0f, std::min(rounds, t));
            GLfloat a1 = t * M_PI * 2;
            GLfloat a2 = (GLfloat)j / slices * M_PI * 2;
            GLfloat d = radius + thickness * cos(a2);
            vertices.push_back(d * cos(a1));
            vertices.push_back(d * sin(a1));
            vertices.push_back(thickness * sin(a2) + height * t / rounds);
        }
    }
    for (GLuint i = 0; i < (GLuint)vertices.size() / 3 - slices; ++i)
    {
        indices.push_back(i);
        indices.push_back(i + slices);
    }
}
```