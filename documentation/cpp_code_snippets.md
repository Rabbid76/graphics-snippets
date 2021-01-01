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

## Load Wave front file (OBJ) 

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
