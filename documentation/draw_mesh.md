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

<b>Triangle fan</b>

See [Wikipedia, Triangle fan](https://en.wikipedia.org/wiki/Triangle_fan).

<b>Trinagle stripe</b>

See [Wikipedia, Triangle strip](https://en.wikipedia.org/wiki/Triangle_strip).

![quad to triangle](image/quad_to_triangles.svg)

<b>Triangle adjacency</b>

![triangle adjacency](image/trianglesAdjacency.svg)

<b>Triangle stripe adjacency</b>


<br/><hr/>

## Vertex array

<b>Separated tightly packed buffers for different attributes</b>

e.g. Buffers for vertices (x, y, z), normals (x, y, z) and texture oordinates (u, v):

Draw the array in core mode:

    GLsizei              no_of_points; // number of vertices and attrbuts
    std::vector<GLfloat> vertex;       // linearized array (no_of_points * 3): [ Vx0, Vy0, Vz0, Vx1, Vy1, Vz1, .... ] 
    std::vector<GLfloat> normal;       // linearized array (no_of_points * 3): [ Nx0, Ny0, Nz0, Nx1, Ny1, Nz1, .... ] 
    std::vector<GLfloat> color;        // linearized array (no_of_points * 5): [ R0, G0, B0, A0, R1, G1, B1, A1, .... ] 

    GLuint vetexAttribIndex;  // index of the vertex attrbute (shader)
    GLuint normalAttribIndex; // index of the normal attribute (shader)
    GLuint colorAttribIndex;  // index of the color attribute (shader)

    glVertexAttribPointer( vetexAttribIndex,  3, GL_FLOAT, GL_FALSE, 0, vertex.data() ); // 3: Vx, Vy, Vz
    glVertexAttribPointer( normalAttribIndex, 3, GL_FLOAT, GL_TRUE,  0, normal.data() ); // 3: Nx, Ny, Nz  -  GL_TRUE: values should be normalized
    glVertexAttribPointer( colorAttribIndex,  4, GL_FLOAT, GL_FALSE, 0, color.data() );  // 4: R, G, B, A 

    glEnableVertexAttribArray( vetexAttribIndex );
    glEnableVertexAttribArray( normalAttribIndex );
    glEnableVertexAttribArray( colorAttribIndex );

    glDrawArrays( GL_TRIANGLES, 0, no_of_points ); 

    glDisableVertexAttribArray( vetexAttribIndex );
    glDisableVertexAttribArray( normalAttribIndex );
    glDisableVertexAttribArray( colorAttribIndex );

See:

- [`glVertexAttribPointer`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glVertexAttribPointer.xhtml)
- [`glEnableVertexAttribArray` / `glDisableVertexAttribArray`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glEnableVertexAttribArray.xhtml)
- [`glDrawArrays`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glDrawArrays.xml)    

See the Khronos group reference page for [`glVertexAttribPointer`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glVertexAttribPointer.xhtml):

> For `glVertexAttribPointer`, if normalized is set to `GL_TRUE`, it indicates that values stored in an integer format are to be mapped to the range [-1,1] (for signed values) or [0,1] (for unsigned values) when they are accessed and **converted to floating point**. Otherwise, values **will be converted to floats directly** without normalization.
  

<br/>

Draw the array in compatibility mode (**deprecated**):

    GLsizei              no_of_points; // number of vertices and attrbuts
    std::vector<GLfloat> vertex;       // linearized array (no_of_points * 3): [ Vx0, Vy0, Vz0, Vx1, Vy1, Vz1, .... ] 
    std::vector<GLfloat> normal;       // linearized array (no_of_points * 3): [ Nx0, Ny0, Nz0, Nx1, Ny1, Nz1, .... ] 
    std::vector<GLfloat> color;        // linearized array (no_of_points * 5): [ R0, G0, B0, A0, R1, G1, B1, A1, .... ] 
    
    glVertexPointer( 3, GL_FLOAT, 0, vertex.data() ); // 3: Vx, Vy, Vz 
    glNormalPointer(    GL_FLOAT, 0, normal.data() );
    glColorPointer(  4, GL_FLOAT, 0, color.data() );  // 4: R, G, B, A  

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_NORMAL_ARRAY );
    glEnableClientState( GL_COLOR_ARRAY );

    glDrawArrays( GL_TRIANGLES, 0, no_of_points );  

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );

See:
- [`glVertexPointer`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertexPointer.xml)
- [`glNormalPointer`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glNormalPointer.xml)
- [`glTexCoordPointer`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glTexCoordPointer.xml)
- [`glColorPointer`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColorPointer.xml)
- [`glEnableClientState` / `glDisableClientState`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glEnableClientState.xml)


<br/><hr/>

## Vertex buffer object (Array buffer)

It is sufficient to call [`glVertexAttribPointer`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glVertexAttribPointer.xhtml) once per vetrex attribute. The [Vertex Specification](https://www.khronos.org/opengl/wiki/Vertex_Specification) is keept until it is not redefined. Of course, the buffer object, where [`glVertexAttribPointer`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glVertexAttribPointer.xhtml) refers to must not be deleted. Also the state, whether the vertex attribute is enabled ([`glEnableVertexAttribArray`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glEnableVertexAttribArray.xhtml)) or not is kept until the vertex attribute is disabled again ([`glDisableVertexAttribArray`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glEnableVertexAttribArray.xhtml)).

The Khronos OpenGL wiki about [Vertex Specification](https://www.khronos.org/opengl/wiki/Vertex_Specification) clearly says:

> The [`glVertexAttribPointer`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glVertexAttribPointer.xhtml) functions **state** where an attribute index gets its array data from. 

This state can be retrieved with [`glGetVertexAttrib`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetVertexAttrib.xhtml).

More information about vertex attrbutes can be found in the [OpenGL 4.6. core specification](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf) from Chapter 10.2 to 10.6.

<br/>

<b>Separated tightly packed buffers for different attributes</b>

e.g. Buffers for vertices (x, y, z), normals (x, y, z) and texture oordinates (u, v):

Create the vertex array buffer:

    GLsizei              no_of_points; // number of vertices and attrbuts
    std::vector<GLfloat> vertex;       // linearized array (no_of_points * 3): [ Vx0, Vy0, Vz0, Vx1, Vy1, Vz1, .... ] 
    std::vector<GLfloat> normal;       // linearized array (no_of_points * 3): [ Nx0, Ny0, Nz0, Nx1, Ny1, Nz1, .... ] 
    std::vector<GLfloat> texture;      // linearized array (no_of_points * 3): [ Tu0, Tv0, Tu1, Tv1, .... ] 

    GLuint vbos[3];

    glGenBuffers( 3, vbos );
    
    glBindBuffer( GL_ARRAY_BUFFER, vbos[0] );
    glBufferData( GL_ARRAY_BUFFER, vertex.size() * sizeof(GLfloat), vertex.data(), GL_STATIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, vbos[1] );
    glBufferData( GL_ARRAY_BUFFER, normal.size() * sizeof(GLfloat), normal.data(), GL_STATIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, vbos[2] );
    glBufferData( GL_ARRAY_BUFFER, texture.size() * sizeof(GLfloat), texture.data(), GL_STATIC_DRAW );
    
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

See:

- [`glGenBuffers`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGenBuffers.xhtml)
- [`glDeleteBuffers`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glDeleteBuffers.xhtml)
- [`glBindBuffer`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBindBuffer.xhtml)

<br/>

Draw the array in core mode:

    GLuint vetexAttribIndex;  // index of the vertex attrbute (shader)
    GLuint normalAttribIndex; // index of the normal attribute (shader)
    GLuint texCorAttribIndex; // index of the texture coordinate attribute (shader)

    glBindBuffer( GL_ARRAY_BUFFER, vbos[0] );
    glVertexAttribPointer( vetexAttribIndex,  3, GL_FLOAT, GL_FALSE, 0, nullptr ); // 3: Vx, Vy, Vz
    glBindBuffer( GL_ARRAY_BUFFER, vbos[1] );
    glVertexAttribPointer( normalAttribIndex, 3, GL_FLOAT, GL_TRUE,  0, nullptr ); // 3: Nx, Ny, Nz  -  GL_TRUE: values should be normalized
    glBindBuffer( GL_ARRAY_BUFFER, vbos[2] );
    glVertexAttribPointer( texCorAttribIndex,  2, GL_FLOAT, GL_FALSE, 0, nullptr ); // 2: Tu, Tv 

    glEnableVertexAttribArray( vetexAttribIndex );
    glEnableVertexAttribArray( normalAttribIndex );
    glEnableVertexAttribArray( texCorAttribIndex );

    glDrawArrays( GL_TRIANGLES, 0, no_of_points ); 

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( vetexAttribIndex );
    glDisableVertexAttribArray( normalAttribIndex );
    glDisableVertexAttribArray( texCorAttribIndex );

<br/>

Draw the array in compatibility mode (**deprecated**):

    glBindBuffer( GL_ARRAY_BUFFER, vbos[0] );
    glVertexPointer(   3, GL_FLOAT, 0, nullptr ); // 3: Vx, Vy, Vz 
    glBindBuffer( GL_ARRAY_BUFFER, vbos[1] );
    glNormalPointer(      GL_FLOAT, 0, nullptr );
    glBindBuffer( GL_ARRAY_BUFFER, vbos[1] );
    glTexCoordPointer( 2, GL_FLOAT, 0, nullptr ); // 2: Tu, Tv  

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_NORMAL_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );

    glDrawArrays( GL_TRIANGLES, 0, no_of_points );  

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );  


<br/>    

<b>Vertex attribute records set - Stride packed</b>

e.g. Vertex, Normal vector and Texture coordiante
     
    [ Vx0, Vy0, Vz0, Nx0, Ny0, Nz0, Tv0, Tu0,
      Vx1, Vy1, Vz1, Nx1, Ny1, Nz1, Tv1, Tu1,
      .....
    ]

<br/>     

Create the vertex array buffer:

    GLsizei no_of_points;
    std::vector<GLfloat> data; // attribute set: [ Vx0, Vy0, Vz0, Nx0, Ny0, Nz0, Tv0, Tu0, Vx1, Vy1, Vz1, Nx1, Ny1, Nz1, Tv1, Tu1, .... ]

    GLuint vbo;

    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), data.data(), GL_STATIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

<br/>

Draw the array in core mode:

    GLuint vetexAttribIndex;  // index of the vertex attrbute (shader)
    GLuint normalAttribIndex; // index of the normal attribute (shader)
    GLuint texCorAttribIndex; // index of the texture coordinate attribute (shader)

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    
    GLsizei stride = 8 * sizeof(GL_float); // size of one record in bytes: 8 * float [ Vx, Vy, Vz, Nx, Ny, Nz, Tv, Tu]
    GLsizei offsV  = 0 * sizeof(GL_float); // offset of the vertex inside the reccord
    GLsizei offsNV = 3 * sizeof(GL_float); // offset of the normal vector inside the reccord
    GLsizei offsTC = 6 * sizeof(GL_float); // offset of the tecture coordinate inside the reccord
    
    glVertexAttribPointer( vetexAttribIndex,  3, GL_FLOAT, GL_FALSE, stride, offsV );  // 3: Vx, Vy, Vz
    glVertexAttribPointer( normalAttribIndex, 3, GL_FLOAT, GL_TRUE,  stride, offsNV ); // 3: Nx, Ny, Nz  -  GL_TRUE: values should be normalized
    glVertexAttribPointer( texCorAttribIndex, 2, GL_FLOAT, GL_FALSE, stride, offsTC ); // 2: Tu, Tv 

    glEnableVertexAttribArray( vetexAttribIndex );
    glEnableVertexAttribArray( normalAttribIndex );
    glEnableVertexAttribArray( texCorAttribIndex );

    glDrawArrays( GL_TRIANGLES, 0, no_of_points ); 

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( vetexAttribIndex );
    glDisableVertexAttribArray( normalAttribIndex );
    glDisableVertexAttribArray( texCorAttribIndex );

<br/>

Draw the array in compatibility mode (**deprecated**):

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    
    GLsizei stride = 8 * sizeof(GL_float); // size of one record in bytes: 8 * float [ Vx, Vy, Vz, Nx, Ny, Nz, Tv, Tu]
    GLsizei offsV  = 0 * sizeof(GL_float); // offset of the vertex inside the reccord
    GLsizei offsNV = 3 * sizeof(GL_float); // offset of the normal vector inside the reccord
    GLsizei offsTC = 6 * sizeof(GL_float); // offset of the tecture coordinate inside the reccord
    
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glVertexPointer(   3, GL_FLOAT, stride, offsV );  // 3: Vx, Vy, Vz 
    glNormalPointer(      GL_FLOAT, stride, offsNV );
    glTexCoordPointer( 2, GL_FLOAT, stride, offsTC ); // 2: Tu, Tv  

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_NORMAL_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );

    glDrawArrays( GL_TRIANGLES, 0, no_of_points );  

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );


<br/><hr/>

## Index buffer object (Element array buffer)

e.g. Vertex, Normal vector and Texture coordiante
     
    [ Vx0, Vy0, Vz0, Nx0, Ny0, Nz0, Tv0, Tu0,
      Vx1, Vy1, Vz1, Nx1, Ny1, Nz1, Tv1, Tu1,
      .....
    ]

<br/>

Create the vertex array buffer and the index buffer:

    GLsizei no_of_points;
    std::vector<GLfloat> data;    // attribute set: [ Vx0, Vy0, Vz0, Nx0, Ny0, Nz0, Tv0, Tu0, Vx1, Vy1, Vz1, Nx1, Ny1, Nz1, Tv1, Tu1, .... ]
    std::vector<GLuint>  indices; // indces: [ I0, I1, I2, I3, I4, ..... ]

    GLuint vbo;

    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), data.data(), GL_STATIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    
    GLuint ibo;

    glGenBuffers( 1, &ibo );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

<br/>

Draw the array in core mode:

    GLuint vetexAttribIndex;  // index of the vertex attrbute (shader)
    GLuint normalAttribIndex; // index of the normal attribute (shader)
    GLuint texCorAttribIndex; // index of the texture coordinate attribute (shader)

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    
    GLsizei stride = 8 * sizeof(GL_float); // size of one record in bytes: 8 * float [ Vx, Vy, Vz, Nx, Ny, Nz, Tv, Tu]
    GLsizei offsV  = 0 * sizeof(GL_float); // offset of the vertex inside the reccord
    GLsizei offsNV = 3 * sizeof(GL_float); // offset of the normal vector inside the reccord
    GLsizei offsTC = 6 * sizeof(GL_float); // offset of the tecture coordinate inside the reccord
    
    glVertexAttribPointer( vetexAttribIndex,  3, GL_FLOAT, GL_FALSE, stride, offsV );  // 3: Vx, Vy, Vz
    glVertexAttribPointer( normalAttribIndex, 3, GL_FLOAT, GL_TRUE,  stride, offsNV ); // 3: Nx, Ny, Nz  -  GL_TRUE: values should be normalized
    glVertexAttribPointer( texCorAttribIndex,  2, GL_FLOAT, GL_FALSE, stride, offsTC ); // 2: Tu, Tv 

    glEnableVertexAttribArray( vetexAttribIndex );
    glEnableVertexAttribArray( normalAttribIndex );
    glEnableVertexAttribArray( texCorAttribIndex );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    glDrawElements( GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, nullptr );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ); 

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( vetexAttribIndex );
    glDisableVertexAttribArray( normalAttribIndex );
    glDisableVertexAttribArray( texCorAttribIndex );

<br/>

Draw the array in compatibility mode (**deprecated**):

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    
    GLsizei stride = 8 * sizeof(GL_float); // size of one record in bytes: 8 * float [ Vx, Vy, Vz, Nx, Ny, Nz, Tv, Tu]
    GLsizei offsV  = 0 * sizeof(GL_float); // offset of the vertex inside the reccord
    GLsizei offsNV = 3 * sizeof(GL_float); // offset of the normal vector inside the reccord
    GLsizei offsTC = 6 * sizeof(GL_float); // offset of the tecture coordinate inside the reccord
    
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glVertexPointer(   3, GL_FLOAT, stride, offsV );  // 3: Vx, Vy, Vz 
    glNormalPointer(      GL_FLOAT, stride, offsNV );
    glTexCoordPointer( 2, GL_FLOAT, stride, offsTC ); // 2: Tu, Tv  

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_NORMAL_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );

    glDrawArrays( GL_TRIANGLES, 0, no_of_points );  

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );


<br/><hr/>

## Vertex array object

To handle different vertex attribute pointers and not to specify and enable or disable them alternately, a vertex array object can be generated ([`glGenVertexArrays`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGenVertexArrays.xhtml), which stores all the information about buffer location, data format, state and attribute index:

See [OpenGL 4.6 core Specification - 10.3.1 Vertex Array Objects](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf):

> The buffer objects that are to be used by the vertex stage of the GL are collected together to form a vertex array object.
> All state related to the definition of data used by the vertex processor is encapsulated in a vertex array object.
>
> ....
>
> The currently bound vertex array object is used for all commands which modify vertex array state, such as VertexAttribPointer and EnableVertexAttribArray;
> all commands which draw from vertex arrays, such as DrawArrays and DrawElements;

<br/>

The `GL_ELEMENT_ARRAY_BUFFER` has to be bound **after** the vertex array object has been bound (`glBindVertexArray`). The `GL_ELEMENT_ARRAY_BUFFER` object is stored in the vertex array objects state vector.<br/>
If the vertex array object has been unbound and is bound again, then the `GL_ELEMENT_ARRAY_BUFFER` is known and bound again, too. But if the element array buffer explicitly gets unbound while the vertex array object is bound, it is removed form the state vector.

See the [OpenGL 4.6 core specification - 10.3. VERTEX ARRAYS](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf):

> A vertex array object is created by binding a name returned by `GenVertexArray` with the command<br/>
>
>    void BindVertexArray( uint array );
>
> `array` is the vertex array object name.<br/>
> **The resulting vertex array object is a new state vector**, comprising all the state and with the same initial values listed in tables 23.3 and 23.4.<br/>
> `BindVertexArray` may also be used to bind an existing vertex array object. If the bind is successful no change is made to the state of the bound vertex array object, and **any previous binding is broken**.
>
> Tables 23.3, Vertex Array Object State<br/>
> *VERTEX_ATTRIB_ARRAY_ENABLED*, *VERTEX_ATTRIB_ARRAY_SIZE*, *VERTEX_ATTRIB_ARRAY_STRIDE*, *VERTEX_ATTRIB_ARRAY_TYPE*, *VERTEX_ATTRIB_ARRAY_NORMALIZED*, *VERTEX_ATTRIB_ARRAY_INTEGER*, *VERTEX_ATTRIB_ARRAY_LONG*, *VERTEX_ATTRIB_ARRAY_DIVISOR*, *VERTEX_ATTRIB_ARRAY_POINTER*
>
> Table 23.4, Vertex Array Object State<br/>
> *ELEMENT_ARRAY_BUFFER_BINDING*, *VERTEX_ATTRIB_ARRAY_BUFFER_BINDING*, *VERTEX_ATTRIB_BINDING*, *VERTEX_ATTRIB_RELATIVE_OFFSET*, *VERTEX_BINDING_OFFSET*, *VERTEX_BINDING_STRIDE*, *VERTEX_BINDING_DIVISOR*, *VERTEX_BINDING_BUFFER*.
>
> Table 23.5, Vertex Array Data (**not in Vertex Array objects**)<br/>
> *ARRAY_BUFFER_BINDING*, *DRAW_INDIRECT_BUFFER_BINDING*, *VERTEX_ARRAY_BINDING*, *PARAMETER_BUFFER_BINDING*, *PRIMITIVE_RESTART*, *PRIMITIVE_RESTART_FIXED_INDEX*, *PRIMITIVE_RESTART_INDEX*


<br/>

e.g. Vertex, Normal vector and Texture coordiante
     
    [ Vx0, Vy0, Vz0, Nx0, Ny0, Nz0, Tv0, Tu0,
      Vx1, Vy1, Vz1, Nx1, Ny1, Nz1, Tv1, Tu1,
      .....
    ]

<br/>
Create the vertex array buffer and the index buffer:

    GLsizei no_of_points;
    std::vector<GLfloat> data;    // attribute set: [ Vx0, Vy0, Vz0, Nx0, Ny0, Nz0, Tv0, Tu0, Vx1, Vy1, Vz1, Nx1, Ny1, Nz1, Tv1, Tu1, .... ]
    std::vector<GLuint>  indices; // indces: [ I0, I1, I2, I3, I4, ..... ]

    GLuint vbo;

    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), data.data(), GL_STATIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    
    GLuint ibo;

    glGenBuffers( 1, &ibo );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

<br/>

Create the vertex array object in core mode:

    GLuint vao;

    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    GLuint vetexAttribIndex;  // index of the vertex attrbute (shader)
    GLuint normalAttribIndex; // index of the normal attribute (shader)
    GLuint texCorAttribIndex; // index of the texture coordinate attribute (shader)

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    
    GLsizei stride = 8 * sizeof(GL_float); // size of one record in bytes: 8 * float [ Vx, Vy, Vz, Nx, Ny, Nz, Tv, Tu]
    GLsizei offsV  = 0 * sizeof(GL_float); // offset of the vertex inside the reccord
    GLsizei offsNV = 3 * sizeof(GL_float); // offset of the normal vector inside the reccord
    GLsizei offsTC = 6 * sizeof(GL_float); // offset of the tecture coordinate inside the reccord
    
    glVertexAttribPointer( vetexAttribIndex,  3, GL_FLOAT, GL_FALSE, stride, offsV );  // 3: Vx, Vy, Vz
    glVertexAttribPointer( normalAttribIndex, 3, GL_FLOAT, GL_TRUE,  stride, offsNV ); // 3: Nx, Ny, Nz  -  GL_TRUE: values should be normalized
    glVertexAttribPointer( texCorAttribIndex,  2, GL_FLOAT, GL_FALSE, stride, offsTC ); // 2: Tu, Tv 

    glEnableVertexAttribArray( vetexAttribIndex );
    glEnableVertexAttribArray( normalAttribIndex );
    glEnableVertexAttribArray( texCorAttribIndex );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo ); // Associate the element array buffer (index buffer) to the vertex array object

    glBindVertexArray( 0 ); // Unbind the vertex array object
    
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ); // Unbinde the element array buffer. This has to be done after the vertex array object is unbound, otherwise the association to the vertex array object would be lost.

See:

- [`glGenVertexArrays`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGenVertexArrays.xhtml)
- [`glBindVertexArray`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBindVertexArray.xhtml)
- [`glDeleteVertexArrays`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glDeleteVertexArrays.xhtml)

<br/>

Create the vertex array object compatibility mode (**deprecated**):

    GLuint vao;

    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    
    GLsizei stride = 8 * sizeof(GL_float); // size of one record in bytes: 8 * float [ Vx, Vy, Vz, Nx, Ny, Nz, Tv, Tu]
    GLsizei offsV  = 0 * sizeof(GL_float); // offset of the vertex inside the reccord
    GLsizei offsNV = 3 * sizeof(GL_float); // offset of the normal vector inside the reccord
    GLsizei offsTC = 6 * sizeof(GL_float); // offset of the tecture coordinate inside the reccord
    
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glVertexPointer(   3, GL_FLOAT, stride, offsV );  // 3: Vx, Vy, Vz 
    glNormalPointer(      GL_FLOAT, stride, offsNV );
    glTexCoordPointer( 2, GL_FLOAT, stride, offsTC ); // 2: Tu, Tv  

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_NORMAL_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo ); // Associate the element array buffer (index buffer) to the vertex array object

    glBindVertexArray( 0 ); // Unbind the vertex array object
    
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ); // Unbinde the element array buffer. This has to be done after the vertex array object is unbound, otherwise the association to the vertex array object would be lost.

<br/>

Draw the array in core mode:

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glDrawElements( GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, nullptr );
    glBindVertexArray( 0 );

<br/>

Note, if a buffer objects or a vertex array object is not further uses, it has to be deleted, to prevent memory leaks. Buffer objects ard deleted by [`glDeleteBuffers`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glDeleteBuffers.xhtml) and vertex array objects are deleted by [`glDeleteVertexArrays`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glDeleteVertexArrays.xhtml).<br/>
Buffer objects are not "created under" vertex array objects, it is not sufficient to delete the vertex array object only.<br/>
See [OpenGL Vertex Array/Buffer Objects](https://stackoverflow.com/questions/13342403/opengl-vertex-array-buffer-objects)).

<br/>

See [OpenGL ES 4.6 core Specification 3.2 - 5.1.2 Automatic Unbinding of Deleted Objects](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf):<br/>
See [OpenGL ES 3.2 Specification - 5.1.2 Automatic Unbinding of Deleted Objects](https://www.khronos.org/registry/OpenGL/specs/es/3.2/es_spec_3.2.pdf):

> When a buffer, texture, transform feedback or renderbuffer object is successfully deleted, it is unbound from any bind points it is bound to in the current context, and detached from any attachments of container objects that are bound to the current context ....
>
> Attachments to unbound container objects, such as deletion of a buffer attached to a vertex array object which is not bound to the context, are not affected and continue to act as references on the deleted object ....
>
> When a buffer, query, renderbuffer, sampler, sync, or texture object is deleted, its name immediately becomes invalid (e.g. is marked unused), but the underlying object will not be deleted until it is no longer in use.


<br/><hr/>

## Fixed Function Pipeline - Deprecated, old school

This technique of drawing is only listed here, as unfortunately it is still often seen. This technology is only supported (partially) because it is so widespread. The support is limited to windows and linux and it requires a compatibility context, to be used. Further this is not supported by OpenGL ES or WebGL.<br/>

See [Khronos wiki - OpenGL Context](https://www.khronos.org/opengl/wiki/OpenGL_Context):

> OpenGL version 3.0 introduced the idea of deprecating functionality. Many OpenGL functions were declared deprecated, which means that users should avoid using them because they may be removed from later API versions. OpenGL 3.1 removed almost all of the functionality deprecated in OpenGL 3.0. This includes the [Fixed Function Pipeline](https://www.khronos.org/opengl/wiki/Fixed_Function_Pipeline).
>
> ....
>
> A new extension, [ARB_compatibility](https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_compatibility.txt), was introduced when OpenGL 3.1 was revealed. The presence of this extension is a signal to the user that deprecated or removed features are still available through the original entrypoints and enumerations. The behavior of such implementations is defined with a separate, much larger, [OpenGL Specification](https://www.khronos.org/opengl/wiki/OpenGL_Specification). Thus, there was a backwards-compatible specification and a non-backwards compatible specification.<br/>
> However, since many implementations support the deprecated and removed features anyway, some implementations want to be able to provide a way for users of higher GL versions to gain access to the old APIs. Several techniques were tried, and it has settled down into a division between Core and Compatibility contexts.

See [Khronos wiki - Fixed Function Pipeline](https://www.khronos.org/opengl/wiki/Fixed_Function_Pipeline):

> OpenGL 3.0 was the last revision of the specification which fully supported both fixed and programmable functionality. Even so, most hardware since the OpenGL 2.0 generation lacked the actual fixed-function hardware. Instead, fixed-function processes are emulated with shaders built by the system.
> In OpenGL 3.2, the [Core Profile](https://www.khronos.org/opengl/wiki/OpenGL_Context#Context_types) lacks these fixed-function concepts. The compatibility profile keeps them around. However, most newer features of OpenGL cannot work with fixed function, even when it might seem theoretically possible for them to interact.

See [Khronos wiki - Legacy OpenGL](https://www.khronos.org/opengl/wiki/Legacy_OpenGL):

> In 2008, version 3.0 of the OpenGL specification was released. With this revision, the [Fixed Function Pipeline](https://www.khronos.org/opengl/wiki/Fixed_Function_Pipeline) as well as most of the related OpenGL functions and constants were declared deprecated. These deprecated elements and concepts are now commonly referred to as legacy OpenGL.<br/>
> Legacy OpenGL is still supported by certain implementations that support core OpenGL 3.1 or higher and the [GL_ARB_compatibility](https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_compatibility.txt) extension. Implementations that do not expose this extension do only offer features defined in the core OpenGL specification the implementation is based upon.
>
> .....
>
> **Implementations of compatibility contexts**<br/>
>
> Both AMD and NVIDIA provide backwards-compatible implementations at least on Windows and Linux. Apple does only provide an implementation of the core profile and supports core OpenGL 3.2 on Mac OSX. Intel provides an implementation for Windows up to OpenGL 3.1 with Sandy Bridge CPUs and OpenGL 4.0 with Ivy Bridge CPUs. However, Intel's Linux open-source driver developers have recently stated that they will not provide backward-compatibility on Linux.

See also [Khronos forums - Forward compatible vs Core profile](https://www.opengl.org/discussion_boards/showthread.php/175052-Forward-compatible-vs-Core-profile)<br/>

<br/>
e.g. 

    glBegin(GL_TRIANGLE_FAN)
    glVertex2f(-0.5f, -0.5f)
    glVertex2f( 0.5f, -0.5f)
    glVertex2f( 0.5f,  0.5f)
    glVertex2f(-0.5f,  0.5f)
    glEnd()


<br/><hr/>

## Vertex attributes with integral data types

For vertex attributes with an integral data it has to be used [**`glVertexAttribIPointer`**](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glVertexAttribPointer.xhtml) (focus on the **`I`**), to define an array of generic vertex attribute data.

See the [OpenGL ES specification - Chapter 10.3 Vertex Arrays](https://www.khronos.org/registry/OpenGL/specs/es/3.2/es_spec_3.2.pdf) which clearly says:

> When values for a vertex shader attribute variable are sourced from an enabled generic vertex attribute array,
> the array must be specified by a command compatible with the data type of the variable. 
> The values loaded into a shader attribute variable bound to generic attribute index are undefined if the array for index was not specified by:
>
> - VertexAttribFormat, for floating-point base type attributes;
> - VertexAttribIFormat with type BYTE, SHORT, or INT for signed integer base type attributes; or
> - VertexAttribIFormat with type UNSIGNED_BYTE, UNSIGNED_SHORT, or UNSIGNED_INT for unsigned integer base type attributes.

See the Khronos group reference page for [`glVertexAttribPointer`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glVertexAttribPointer.xhtml):

> For `glVertexAttribPointer`, if normalized is set to `GL_TRUE`, it indicates that values stored in an integer format are to be mapped to the range [-1,1] (for signed values) or [0,1] (for unsigned values) when they are accessed and **converted to floating point**. Otherwise, values **will be converted to floats directly** without normalization.
>
> For `glVertexAttribIPointer`, only the integer types `GL_BYTE`, `GL_UNSIGNED_BYTE`, `GL_SHORT`, `GL_UNSIGNED_SHORT`, `GL_INT`, `GL_UNSIGNED_INT` are accepted. **Values are always left as integer values**.

## WebGL

See the [WebGL documentation for `WebGL2RenderingContext.vertexAttribIPointer()`](https://developer.mozilla.org/en-US/docs/Web/API/WebGL2RenderingContext/vertexAttribIPointer)  which says:

> The `WebGL2RenderingContext.vertexAttribIPointer()` method of the WebGL 2 API specifies integer data formats and locations of vertex attributes in a vertex attributes array.


----
# TODO 

## Investigate

- WebgGL draw colored triangle, without shader
- OpenGL darw colored trinagle with vertex buffer object/certex array object, without shader

