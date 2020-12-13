[![StackOverflow](https://stackexchange.com/users/flair/7322082.png)](https://stackoverflow.com/users/5577765/rabbid76?tab=profile)

---

# Resources and Program

## Program

### Compile

### Link

Things like uniforms, attributes etc. are program resources. The locations of active resources are finally determined when linking the program.
Of course the user can specify the location. In modern OpenGL this can be done by [Layout Qualifier](https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)).
In earlier version a generic vertex attribute can be associated to an attribute name by [`glBindAttribLocation`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBindAttribLocation.xhtml),
before linking the program. The location information is considered when the program is get linked.<br/>
But resource information, like the attribute location ([`glGetAttribLocation`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetAttribLocation.xhtml)), can only be queried, if the program was successfully linked. The linking of the program "finalize" the program, to be introspective and to be used.<br/>
The "hint" that there is a named attribute associated to a location, does not make an active resource. The active resource is set in the during the link process.

<br/>

## Active and inactive uniform variables

### OpenGL and ### OpenGL ES 3.2

See [OpenGL 4.6 Core Profile Specification - 7.6 Uniform Variables - p. 130](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf):

> 7.6 Uniform Variables
>
> Shaders can declare named uniform variables, as described in the OpenGL Shading Language Specification. A uniform is considered an active uniform if the compiler and linker determine that the uniform will actually be accessed when the executable code is executed. In cases where the compiler and linker cannot make a conclusive determination, the uniform will be considered active.

See [OpenGL ES Shading Language 3.00 Specification - 2.12.6 Uniform Variables; page 58](https://www.khronos.org/registry/OpenGL/specs/es/3.0/es_spec_3.0.pdf):

> 7.6 Uniform Variables
>
> Shaders can declare named uniform variables, as described in the OpenGL ES Shading Language Specification. Values for these uniforms are constant over a
primitive, and typically they are constant across many primitives. A uniform is considered active if it is determined by the compiler and linker that the uniform
will actually be accessed when the executable code is executed. In cases where the compiler and linker cannot make a conclusive determination, the uniform will be
considered active.

See [OpenGL 4.6 Core Profile Specification - 7.3.1 Program Interfaces](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf#page=124&zoom=100,0,173), page 102:
See [OpenGL ES 3.2 Specification - 7.3.1 Program Interfaces](https://www.khronos.org/registry/OpenGL/specs/es/3.2/es_spec_3.2.pdf#page=97&zoom=100,0,173), page 79:

> 7.3.1 Program Interfaces
>
> When a program object is made part of the current rendering state, its executable code may communicate with other GL pipeline stages or application code through a variety of interfaces. When a program is linked, the GL builds a list of active resources for each interface. Examples of active resources include variables, interface blocks, and subroutines used by shader code. Resources referenced in shader code are considered active unless the compiler and linker can conclusively determine that they have no observable effect on the results produced by the executable code of the program. For example, **variables might be considered inactive if they are declared but not used in executable code**, used only in a clause of an if statement that would never be executed, used only in functions that are never called, or used only in computations of temporary variables having no effect on any shader output. In cases where the compiler or linker cannot make a conclusive determination, any resource referenced by shader code will be considered active. The set of active resources for any interface is implementation-dependent because it depends on various analysis and optimizations performed by the compiler and linker
>
> If a program is linked successfully, the GL will generate lists of active resources based on the executable code produced by the link.

This means that, if the compiler and linker determine that the an attribute or an uniform variable is **not accessed**, when the executable code is executed, then the uniform is **inactive**.

See [OpenGL 4.6 Core Profile Specification - 7.6 Uniform Variables - p. 133](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf):
See [OpenGL ES Shading Language 3.20 Specification - 7.6 Uniform Variables - p. 104](https://www.khronos.org/registry/OpenGL/index_es.php#specs32):

> Additionally, several dedicated commands are provided to query properties of **active** uniforms. The command
>
> ```cpp
> int GetUniformLocation( uint program, const char *name );
> ```

See [OpenGL 4.6 Core Profile Specification - 11.1 Vertex Shaders - p. 384](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf):
See [OpenGL ES Shading Language 3.20 Specification - 11.1 Vertex Shaders - p. 288](https://www.khronos.org/registry/OpenGL/index_es.php#specs32):

> ```cpp
> int GetAttribLocation( uint program, const char *name );
> ```
>
> can be used to determine the location assigned to the **active** input variable named name in program object program.

### OpenGL ES 2.0

See [OpenGL ES 2 Specifications - 2.10.4 Shader Variables - p. 35](https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf):

> A uniform is considered active if it is determined by the compiler and linker that the uniform will actually be accessed when the executable code is executed. In cases where the compiler and linker cannot make a conclusive determination, the uniform will be considered active.
>
> .....
>
> To find the location of an **active** uniform variable within a program object, use the command
>
> ```cpp
> int GetUniformLocation( uint program, const char *name );
> ```

See [OpenGL ES 2.0 Online Manual Pages. - `glGetActiveUniform `](https://www.khronos.org/registry/OpenGL-Refpages/es2.0/):

> A uniform variable (either built-in or user-defined) is considered active if it is determined during the link operation that it may be accessed during program execution.

See [OpenGL ES 2 Specifications - 2.10.4 Shader Variables - p. 32](https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf):

> A generic attribute variable is considered active if it is determined by the compiler and linker that the attribute may be accessed when the shader is executed. Attribute variables that are declared in a vertex shader but never used are not considered active. In cases where the compiler and linker cannot make a conclusive determination, an attribute will be considered active.
>
> .....
>
> To determine the set of **active** vertex attributes used by a program, and to determine their types, use the command:
>
> ```cpp
> void GetActiveAttrib( uint program, uint index, sizei bufSize, sizei *length, int *size, enum *type, char *name );
> ```
>
> .....
>
> After a program object has been linked successfully, the bindings of attribute variable names to indices can be queried. The command
>
> ```cpp
> int GetAttribLocation( uint program, const char *name );
> ```

<br/><hr/>

<a href="https://stackexchange.com/users/7322082/rabbid76"><img src="https://stackexchange.com/users/flair/7322082.png" width="208" height="58" alt="profile for Rabbid76 on Stack Exchange, a network of free, community-driven Q&amp;A sites" title="profile for Rabbid76 on Stack Exchange, a network of free, community-driven Q&amp;A sites" /></a>
