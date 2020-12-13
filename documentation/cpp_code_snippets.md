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
