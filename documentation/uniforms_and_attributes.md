
# Uniforms and Attributes

## Aktive and inactive uniform variables

### OpenGL and ### OpenGL ES 3.2

See [OpenGL 4.6 Core Profile Specification - 7.6 Uniform Variables - p. 130](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf):
See [OpenGL ES Shading Language 3.20 Specification - 7.3.1 Program Interfaces - p. 101](https://www.khronos.org/registry/OpenGL/index_es.php#specs32):

> 7.6 Uniform Variables
>
> Shaders can declare named uniform variables, as described in the OpenGL Shading Language Specification. A uniform is considered an active uniform if the compiler and linker determine that the uniform will actually be accessed when the executable code is executed. In cases where the compiler and linker cannot make a conclusive determination, the uniform will be considered active.


See [OpenGL 4.6 Core Profile Specification - 7.3.1 Program Interfaces - p. 101](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf):
See [OpenGL ES Shading Language 3.20 Specification - 7.6 Uniform Variables - p. 79](https://www.khronos.org/registry/OpenGL/index_es.php#specs32):

> 7.3.1 Program Interfaces
>
> When a program object is made part of the current rendering state, its executable code may communicate with other GL pipeline stages or application code through a variety of interfaces. When a program is linked, the GL builds a list of active resources for each interface. Examples of active resources include variables, interface blocks, and subroutines used by shader code. Resources referenced in shader code are considered active unless the compiler and linker can conclusively determine that they have no observable effect on the results produced by the executable code of the program. For example, **variables might be considered inactive if they are declared but not used in executable code**, used only in a clause of an if statement that would never be executed, used only in functions that are never called, or used only in computations of temporary variables having no effect on any shader output. In cases where the compiler or linker cannot make a conclusive determination, any resource referenced by shader code will be considered active. The set of active resources for any interface is implementation-dependent because it depends on various analysis and optimizations performed by the compiler and linker
>
> If a program is linked successfully, the GL will generate lists of active resources based on the executable code produced by the link.

This means that, if the compiler and linker determine that the an attribute or an unifor variable is **not accessed**, when the executable code is executed, then the uniform is **inactive**.


See [OpenGL 4.6 Core Profile Specification - 7.6 Uniform Variables - p. 133](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf):
See [OpenGL ES Shading Language 3.20 Specification - 7.6 Uniform Variables - p. 104](https://www.khronos.org/registry/OpenGL/index_es.php#specs32):

> Additionally, several dedicated commands are provided to query properties of **active** uniforms. The command
>
> `int GetUniformLocation( uint program, const char *name );`


See [OpenGL 4.6 Core Profile Specification - 11.1 Vertex Shaders - p. 384](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf):
See [OpenGL ES Shading Language 3.20 Specification - 11.1 Vertex Shaders - p. 288](https://www.khronos.org/registry/OpenGL/index_es.php#specs32):

> `int GetAttribLocation( uint program, const char *name );`
>
> can be used to determine the location assigned to the **active** input variable named name in program object program.


<br/>

### OpenGL ES 2.0

See [OpenGL ES 2 Specifications - 2.10.4 Shader Variables - p. 35](https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf):

> A uniform is considered active if it is determined by the compiler and linker that the uniform will actually be accessed when the executable code is executed. In cases where the compiler and linker cannot make a conclusive determination, the uniform will be considered active.
>
> .....
>
> To find the location of an **active** uniform variable within a program object, use the command
>
> `int GetUniformLocation( uint program, const char *name );`


See [OpenGL ES 2.0 Online Manual Pages. - `glGetActiveUniform `](https://www.khronos.org/registry/OpenGL-Refpages/es2.0/):

> A uniform variable (either built-in or user-defined) is considered active if it is determined during the link operation that it may be accessed during program execution.


See [OpenGL ES 2 Specifications - 2.10.4 Shader Variables - p. 32](https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf):

> A generic attribute variable is considered active if it is determined by the compiler and linker that the attribute may be accessed when the shader is executed. Attribute variables that are declared in a vertex shader but never used are not considered active. In cases where the compiler and linker cannot make a conclusive determination, an attribute will be considered active.
>
> .....
>
> To determine the set of **active** vertex attributes used by a program, and to determine their types, use the command:
>
> `void GetActiveAttrib( uint program, uint index, sizei bufSize, sizei *length, int *size, enum *type, char *name );`
>
> .....
>
> After a program object has been linked successfully, the bindings of attribute variable names to indices can be queried. The command
>
> `int GetAttribLocation( uint program, const char *name );`


