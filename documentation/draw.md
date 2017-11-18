# Draw geometry

## Primitives

There are different types of [Primitives](https://www.khronos.org/opengl/wiki/Primitive) in OGL:

- Point primitives: `GL_POINTS`

- Line primitives: `GL_LINES`, `GL_LINE_STRIP`, `GL_LINE_LOOP`

- Line primitives with adjacency information: `GL_LINES_ADJACENCY`, `GL_LINE_STRIP_ADJACENCY`<br/>
  Adjacencies do not create a further geometry, but the provide information which can be use in a geometry shader stage.

- Triangle primitives: `GL_TRIANGLES`, `GL_TRIANGLE_STRIP`, `GL_TRIANGLE_FAN`

- Triangle primitives with adjacency information: `GL_TRIANGLES_ADJACENCY`, `GL_TRIANGLE_STRIP_ADJACENCY`<br/>
  Adjacencies do not create a further geometry, but the provide information which can be use in a geometry shader stage.

For the sickness of completeness there are, the list of **deprecated** primitive types: `GL_QUADS`, `GL_QUAD_STRIP`, and `GL_POLYGON`:

For tessellation ther is the specila primitive type `GL_PATCH`.

### Point primitives

### Line primitives

### Triangle primitives

**Triangle fan**

See [Wikipedia, Triangle fan](https://en.wikipedia.org/wiki/Triangle_fan).

**Trinagle stripe**

See [Wikipedia, Triangle strip](https://en.wikipedia.org/wiki/Triangle_strip).

![quad to triangle](image/quad_to_triangles.svg)

**Triangle adjacency**

![triangle adjacency](image/trianglesAdjacency.svg)

**Triangle stripe adjacency**


<br/><hr/>
## Vertex buffer object (Array buffer)

<br/><hr/>
## Index buffer object (Element array buffer)

<br/><hr/>
## Vertex array object

See [OpenGL 4.6 core Specification - 10.3.1 Vertex Array Objects](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf):
>The buffer objects that are to be used by the vertex stage of the GL are collected
together to form a vertex array object. All state related to the definition of data
used by the vertex processor is encapsulated in a vertex array object.
>
>....
>
>The currently bound vertex array object is used for all commands which modify
vertex array state, such as VertexAttribPointer and EnableVertexAttribArray;
all commands which draw from vertex arrays, such as DrawArrays and DrawElements;

<br/>
the `GL_ELEMENT_ARRAY_BUFFER` has to be bound **after** the vertex array object has been bound (`glBindVertexArray`). The `GL_ELEMENT_ARRAY_BUFFER` object is stored in the vertex array objects state vector.<br/>
If the vertex array object has been unbound and is bound again, then the `GL_ELEMENT_ARRAY_BUFFER` is known and bound again, too. But if the element array buffer explicitly gets unbound while the vertex array object is bound, it is removed form the state vector.

See the [OpenGL 4.6 core specification - 10.3. VERTEX ARRAYS)](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf):
>A vertex array object is created by binding a name returned by `GenVertexArray` with the command<br/>
>
>    void BindVertexArray( uint array );
>
>`array` is the vertex array object name.<br/>
**The resulting vertex array object is a new state vector**, comprising all the state and with the same initial values listed in tables 23.3 and 23.4.<br/>
`BindVertexArray` may also be used to bind an existing vertex array object. If the bind is successful no change is made to the state of the bound vertex array object, and **any previous binding is broken**.
>
>Tables 23.3, Vertex Array Object State<br/>
>*VERTEX_ATTRIB_ARRAY_ENABLED*, *VERTEX_ATTRIB_ARRAY_SIZE*, *VERTEX_ATTRIB_ARRAY_STRIDE*, *VERTEX_ATTRIB_ARRAY_TYPE*, *VERTEX_ATTRIB_ARRAY_NORMALIZED*, *VERTEX_ATTRIB_ARRAY_INTEGER*, *VERTEX_ATTRIB_ARRAY_LONG*, *VERTEX_ATTRIB_ARRAY_DIVISOR*, *VERTEX_ATTRIB_ARRAY_POINTER*
>
>Table 23.4, Vertex Array Object State<br/>
>*ELEMENT_ARRAY_BUFFER_BINDING*, *VERTEX_ATTRIB_ARRAY_BUFFER_BINDING*, *VERTEX_ATTRIB_BINDING*, *VERTEX_ATTRIB_RELATIVE_OFFSET*, *VERTEX_BINDING_OFFSET*, *VERTEX_BINDING_STRIDE*, *VERTEX_BINDING_DIVISOR*, *VERTEX_BINDING_BUFFER*.
>
>Table 23.5, Vertex Array Data (**not in Vertex Array objects**)<br/>
>*ARRAY_BUFFER_BINDING*, *DRAW_INDIRECT_BUFFER_BINDING*, *VERTEX_ARRAY_BINDING*, *PARAMETER_BUFFER_BINDING*, *PRIMITIVE_RESTART*, *PRIMITIVE_RESTART_FIXED_INDEX*, *PRIMITIVE_RESTART_INDEX*


<br/><hr/>
## Fixed Function Pipeline - Deprecated, old school

This technique of drawing is only listed here, as unfortunately it is still often seen. This technology is only supported (partially) because it is so widespread. The support is limited to windows and linux and it requires a compatibility context, to be used. Further this is not supported by OpenGL ES or WebGL.<br/>

See [Khronos wiki - OpenGL Context](https://www.khronos.org/opengl/wiki/OpenGL_Context):
> OpenGL version 3.0 introduced the idea of deprecating functionality. Many OpenGL functions were declared deprecated, which means that users should avoid using them because they may be removed from later API versions. OpenGL 3.1 removed almost all of the functionality deprecated in OpenGL 3.0. This includes the [Fixed Function Pipeline](https://www.khronos.org/opengl/wiki/Fixed_Function_Pipeline).
>
>....
>
>A new extension, [ARB_compatibility](https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_compatibility.txt), was introduced when OpenGL 3.1 was revealed. The presence of this extension is a signal to the user that deprecated or removed features are still available through the original entrypoints and enumerations. The behavior of such implementations is defined with a separate, much larger, [OpenGL Specification](https://www.khronos.org/opengl/wiki/OpenGL_Specification). Thus, there was a backwards-compatible specification and a non-backwards compatible specification.<br/>
However, since many implementations support the deprecated and removed features anyway, some implementations want to be able to provide a way for users of higher GL versions to gain access to the old APIs. Several techniques were tried, and it has settled down into a division between Core and Compatibility contexts.

See [Khronos wiki - Fixed Function Pipeline](https://www.khronos.org/opengl/wiki/Fixed_Function_Pipeline:
>OpenGL 3.0 was the last revision of the specification which fully supported both fixed and programmable functionality. Even so, most hardware since the OpenGL 2.0 generation lacked the actual fixed-function hardware. Instead, fixed-function processes are emulated with shaders built by the system.
In OpenGL 3.2, the [Core Profile](https://www.khronos.org/opengl/wiki/OpenGL_Context#Context_types) lacks these fixed-function concepts. The compatibility profile keeps them around. However, most newer features of OpenGL cannot work with fixed function, even when it might seem theoretically possible for them to interact.

See [Khronos wiki - Legacy OpenGL](https://www.khronos.org/opengl/wiki/Legacy_OpenGL):
>In 2008, version 3.0 of the OpenGL specification was released. With this revision, the [Fixed Function Pipeline](https://www.khronos.org/opengl/wiki/Fixed_Function_Pipeline) as well as most of the related OpenGL functions and constants were declared deprecated. These deprecated elements and concepts are now commonly referred to as legacy OpenGL.<br/>
Legacy OpenGL is still supported by certain implementations that support core OpenGL 3.1 or higher and the [GL_ARB_compatibility](https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_compatibility.txt) extension. Implementations that do not expose this extension do only offer features defined in the core OpenGL specification the implementation is based upon.
>
>.....
>
>**Implementations of compatibility contexts**<br/>
>
>Both AMD and NVIDIA provide backwards-compatible implementations at least on Windows and Linux. Apple does only provide an implementation of the core profile and supports core OpenGL 3.2 on Mac OSX. Intel provides an implementation for Windows up to OpenGL 3.1 with Sandy Bridge CPUs and OpenGL 4.0 with Ivy Bridge CPUs. However, Intel's Linux open-source driver developers have recently stated that they will not provide backward-compatibility on Linux.

See also [Khronos forums - Forward compatible vs Core profile](https://www.opengl.org/discussion_boards/showthread.php/175052-Forward-compatible-vs-Core-profile)<br/>

<br/>
e.g. 

    glBegin(GL_TRIANGLE_FAN)
    glVertex2f(-0.5f, -0.5f)
    glVertex2f( 0.5f, -0.5f)
    glVertex2f( 0.5f,  0.5f)
    glVertex2f(-0.5f,  0.5f)
    glEnd()


----
# TODO 

## Investigate

- WebgGL draw colored triangle, without shader
- OpenGL darw colored trinagle with vertex buffer object/certex array object, without shader


-----
#Answer to stackoverflow Question: OpenGL instanced rendering not working, only one instance appears

[https://stackoverflow.com/questions/46012044/opengl-instanced-rendering-not-working-only-one-instance-appears/46012517#46012517]


You have to use `glVertexAttribIPointer`, when defining the array of generic vertex attribute data, for the vertex attrbute `in ivec3 Glob_pos;`.
`glVertexAttribPointer` is for floating point attributes only (integral data will be converted to floating point).

See the Khronos group reference page for [`glVertexAttribPointer`][1]:

>For `glVertexAttribPointer`, if normalized is set to `GL_TRUE`, it indicates that values stored in an integer format are to be mapped to the range [-1,1] (for signed values) or [0,1] (for unsigned values) when they are accessed and **converted to floating point**. Otherwise, values **will be converted to floats directly** without normalization.

>For `glVertexAttribIPointer`, only the integer types `GL_BYTE`, `GL_UNSIGNED_BYTE`, `GL_SHORT`, `GL_UNSIGNED_SHORT`, `GL_INT`, `GL_UNSIGNED_INT` are accepted. **Values are always left as integer values**.

Note, probably `Glob_pos` is never set and has always the same, but undefined value.

 <br/>
Since you don't use [`glGetAttribLocation`][2] to get the attribute indices for the vertex attributes (`position`, `color`, `Glob_pos`), you should use [Layout Qualifiers][3]:

    layout (location = 0) in vec3 position;
    layout (location = 1) in vec3 color;
    layout (location = 2) in ivec3 Glob_pos;

  [1]: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glVertexAttribPointer.xhtml  
  [2]: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetAttribLocation.xhtml
  [3]: https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL) 



-----
#Deleted Answer to Stckoverflow question: WebGL 2.0 Integer attribute throws type error if VertexAttribArray is disabled#

[https://stackoverflow.com/questions/46437800/webgl-2-0-integer-attribute-throws-type-error-if-vertexattribarray-is-disabled/46439710#46439710]


For vertex attributes with an integral data type you have to use [**`vertexAttribIPointer`**][1] (focus on the **`I`**), to define an array of generic vertex attribute data.

See the [WebGL documentation for `WebGL2RenderingContext.vertexAttribIPointer()`][1]  which says:
>The `WebGL2RenderingContext.vertexAttribIPointer()` method of the WebGL 2 API specifies integer data formats and locations of vertex attributes in a vertex attributes array.

See the [OpenGL ES specification - Chapter 10.3 Vertex Arrays][2] which clearly says:
>When values for a vertex shader attribute variable are sourced from an enabled
generic vertex attribute array, the array must be specified by a command compatible with the data type of the variable. The values loaded into a shader attribute variable bound to generic attribute index are undefined if the array for index was not specified by:

>- VertexAttribFormat, for floating-point base type attributes;
>- VertexAttribIFormat with type BYTE, SHORT, or INT for signed integer
base type attributes; or
>- VertexAttribIFormat with type UNSIGNED_BYTE, UNSIGNED_SHORT, or
UNSIGNED_INT for unsigned integer base type attributes.


 [1]: https://developer.mozilla.org/en-US/docs/Web/API/WebGL2RenderingContext/vertexAttribIPointer
 [2]: https://www.khronos.org/registry/OpenGL/specs/es/3.2/es_spec_3.2.pdf


-----
# answer to question : Is the glVertexAttribPointer state bound to the current GL_ARRAY_BUFFER? #

[https://stackoverflow.com/questions/45751705/is-the-glvertexattribpointer-state-bound-to-the-current-gl-array-buffer/45755577#45755577]


>Is it right, that glVertexAttribPointer operations have to be called once for a GL_ARRAY_BUFFER to save the attribute states until I want to change them? Or do I need to call glVertexAttribPointer each time.

[`glVertexAttribPointer`][1] specifies the location and data format of the array of generic vertex attribute at the specified index.

This has to be done before an object is drawn:

    glBindBuffer( ARRAY_BUFFER, posBufObj );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, .... );
    glBindBuffer( nvBufObj );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, .... );

    glDrawArrays( .... )
    glDisableVertexAttribArray( 0 );
    glDisableVertexAttribArray( 1 );
    glBindBuffer( ARRAY_BUFFER, 0 );

Yes, it is sufficient to call [`glVertexAttribPointer`][1] once per vetrex attribute. The [Vertex Specification][4] is keept until it is not redefined. Of course, the buffer object, where [`glVertexAttribPointer`][1] refers to must not be deleted. Also the state, whether the vertex attribute is enabled ([`glEnableVertexAttribArray`][3]) or not is kept until the vertex attribute is disabled again ([`glDisableVertexAttribArray`][3]).<br/>
The Khronos OpenGL wiki about [Vertex Specification][4] clearly says:

>The [`glVertexAttribPointer`][1] functions **state** where an attribute index gets its array data from. 

This state can be retrieved with [`glGetVertexAttrib`][].

More information about vertex attrbutes can be found in the [OpenGL CoreProfile specification][6] from Chapter 10.2 to 10.6

To handle different vertex attribute pointers and not to specify and enable or disable them alternately, a vertex array object can be generated ([`glGenVertexArrays`][2]), which stores all the information about buffer location, data format, state and attribute index:

*generated and specify vertex array object*

    vaoObj = glGenVertexArrays();
    glBindVertexArray( vaoObj );
    glBindBuffer( ARRAY_BUFFER, posBufObj );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, .... );
    glBindBuffer( nvBufObj );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, .... );
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray( 0 );

*bind vertex array object*

    glBindVertexArray( vaoObj );
    glDrawArrays( .... )
    glBindVertexArray( 0 );


  [1]: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glVertexAttribPointer.xhtml
  [2]: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGenVertexArrays.xhtml
  [3]: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glEnableVertexAttribArray.xhtml
  [4]: https://www.khronos.org/opengl/wiki/Vertex_Specification
  [5]: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetVertexAttrib.xhtml
  [6]: https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf


# Answer to Stackoverflow question : Why is the texture coordinate in my fragment shader always (0, 0)? #

[https://stackoverflow.com/questions/45740160/why-is-the-texture-coordinate-in-my-fragment-shader-always-0-0/45740310#45740310]


>I am aware that the Java snippet is using deprecated immediate-mode, but I don't know how to use glDrawArrays or any other commonly suggested method to accomplish the same. Could you help me to change this?


Since you do not need the attribute `in_Color` anymore, you have to delete the attribute from the vertex shader (and of course also `pass_Color` from the vertex shader and the fragment shader). <br/>
Otherwise, you have to expand my solution logically by the color attribute.

Set up an array for the vertex postions an for the texture coordinates:

    float[] posData = {
        screenMinX, screenMinY, 0.0, 1.0,
        screenMaxX, screenMinY, 0.0, 1.0,
        screenMaxX, screenMaxY, 0.0, 1.0,
        screenMinX, screenMaxY, 0.0, 1.0 };

    float[] texData = { 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0 };

Generate a vertex array object:

    int  vaoObj = glGenVertexArrays();
    glBindVertexArray(vaoObj);

Generate array buffers for the vertices and texture coordinates, enable the attribute indices and associate them buffers to the attribute indices:

    FloatBuffer posBuffer = MemoryUtil.memAllocFloat(posData.length);
    posBuffer.put(posData).flip();
    FloatBuffer texBuffer = MemoryUtil.memAllocFloat(texData.length);
    texBuffer.put(texData).flip();

    int vboPosObj = glGenBuffers();
    glBindBuffer(GL_ARRAY_BUFFER, vboPosObj);
    glBufferData(GL_ARRAY_BUFFER, posBuffer, GL_STATIC_DRAW);
    
    // index 0 to associate with "in_Position"            
    glVertexAttribPointer(0, 4, GL_FLOAT, false, 0, 0); 
    glEnableVertexAttribArray(0); // 0 = attribute index of "in_Position"
    
    int vboTexObj = glGenBuffers();
    glBindBuffer(GL_ARRAY_BUFFER, vboTexObj);
    glBufferData(GL_ARRAY_BUFFER, texBuffer, GL_STATIC_DRAW);
    
    // index 0 to associate with "in_TextureCoord"
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(1); // 1 = attribute index of "in_TextureCoord"
    
Release the vertex array object:

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

<br/>
You have to specify the attribute indices of the attributes `in_Position` and `in_TextureCoord`. 

Either you use explicit layout specifications in the vertex shader:

    layout (location = 0) in vec4 in_Position;
    layout (location = 1) in vec2 in_TextureCoord;

Or you specify the attribute indices in the shader program, right **before** you link the shader program (`glLinkProgram`).

    glBindAttribLocation(shaderProgramID, 0, "in_Position");
    glBindAttribLocation(shaderProgramID, 1, "in_TextureCoord");

<br/>
If the object is to be drawn, it is sufficient to bind the vertexArrayObject:

    glBindVertexArray(vaoObj);
    glDrawArrays(GL_QUADS, 0, 4); // 4 = number of vertices 
    glBindVertexArray(0);

Note, if a buffer objects or a vertex array object is not further uses, it has to be deleted, to prevent memory leaks. Buffer objects ard deleted by `glDeleteBuffers` and vertex array objects are deleted by `glDeleteVertexArrays`.<br/> Buffer objects are not "created under" vertex array objects, it is not sufficient to delete the  vertex array object only (see [OpenGL Vertex Array/Buffer Objects][1])

  [1]: https://stackoverflow.com/questions/13342403/opengl-vertex-array-buffer-objects

  
