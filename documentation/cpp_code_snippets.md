[![StackOverflow](https://stackexchange.com/users/flair/7322082.png)](https://stackoverflow.com/users/5577765/rabbid76?tab=profile)

---

# C++ code snippets

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
