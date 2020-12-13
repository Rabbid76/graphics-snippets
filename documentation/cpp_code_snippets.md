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
