# Draw elements in Python

Answer to StackOverflow question [Using offsets into the buffer in PyOpenGL calls](https://stackoverflow.com/questions/6431103/using-offsets-into-the-buffer-in-pyopengl-calls/54355523#54355523)

In the following examples a list of 6 indices is used, which may form a quad which consists  of 2 triangle primitives.

```py
indices = [0, 1, 2, 0, 2, 3]
```

Since the data which is passed to the OpenGL functions has to be consist of fixed size units in a coherent buffer, the list has to be transformed to an array of floats either by [import ctypes](https://docs.python.org/3/library/ctypes.html) or[`numpy.array`](https://docs.scipy.org/doc/numpy-1.15.0/reference/generated/numpy.array.html).  
The type of the array elements has to match value type enumerator constant, which is set, at the call of `glDrawElements` or `glMultiDrawElements`:

```
ctypes.c_ubyte   /  numpy.uint8     <->    GL_UNSIGNED_BYTE
ctypes.c_ushort  /  numpy.uint16    <->    GL_UNSIGNED_SHORT
ctypes.c_uint    /  numpy.uint32    <->    GL_UNSIGNED_INT
```

Using `ctypes`:

```py
import ctypes

indexArray = (ctypes.c_uint * 6)(*indices)
``` 

Using `numpy`:

```py
import numpy

indexArray = numpy.array(indices, dtype=numpy.uint32)
```

For the use of the index buffer with [`glDrawElements `](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glDrawElements.xhtml) there are different opportunities.

Using [Legacy OpenGL](https://www.khronos.org/opengl/wiki/Legacy_OpenGL) ([compatibility profile xontext](https://www.khronos.org/opengl/wiki/OpenGL_Context)), the buffer can be directly passed to `glDrawElements`. The pointer to the array data is passed to the function.

```py
glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indexArray)
``` 

If named element array buffer object is stated in the vertex array object, the the last parameter of `glDrawElements` is treated as a byte offset into the buffer object's data store.

```py
glBindVertexArray(vao)

glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo)
glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexArray, GL_STATIC_DRAW)
```

The last parameter can be `None`, which is equivalent to `ctypes.c_void_p(0)`, if the indices should be drawn, starting by the 1st element of the buffer:

```py
glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, None)
```

If the drawing should not start, with the first index, then the byte offset of the start index has to be calculated. e.g. `3*4` sets the start to the 3 index, for a buffer of type `GL_UNSIGNED_INT`:

<!-- language: py -->

```py
glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, ctypes.c_void_p(3 * 4))
```  

The use of [`glMultiDrawElements`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glMultiDrawElements.xhtml) is very similar.

Using a [compatibility profile xontext](https://www.khronos.org/opengl/wiki/OpenGL_Context), the buffer pointers can be directly passed to the OpenGL function.

To arrays of indices have to be generated:

Using `ctypes`:

<!-- language: py -->

```py
indexArray1 = (ctypes.c_uint * 3)(0, 1, 2)
indexArray2 = (ctypes.c_uint * 3)(0, 2, 3)
```

Using `numpy`:

```py
indexArray1 = numpy.array([0, 1, 2], dtype=numpy.uint32)
indexArray2 = numpy.array([0, 2, 3], dtype=numpy.uint32)
```  

The pointers to the buffers have to be arranged to an array of pointers:

Using `ctypes` the pointer to the index data arrays is get by [`ctypes.addressof()`](https://docs.python.org/3/library/ctypes.html#ctypes.addressof):

```py
indexPtr = (ctypes.c_void_p * 2)(ctypes.addressof(indexArray1),ctypes.addressof(indexArray2))
```

Using `numpy` the pointer to the index data arrays is get by [`numpy.ndarray.ctypes`](https://docs.scipy.org/doc/numpy/reference/generated/numpy.ndarray.ctypes.html):

```py
indexPtr = numpy.array([indexArray1.ctypes.data, indexArray2.ctypes.data], dtype=numpy.intp)
```

This array of pointer can be passed to the OpenGL function

```py
counts   = [3, 3]
glMultiDrawElements(GL_TRIANGLES, counts, GL_UNSIGNED_INT, indexPtr, 2)
```

If a vertex array object with an named element array buffer is used,

```py
glBindVertexArray(vao)
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo)
```

then the index parameter is treated as an array of byte offsets. In the following an array with 2 offset is passed to the function. 0 identifies the 1st index in the array and 3*4 the 3rd index.

Using `ctypes`:

```py
indexPtr = (ctypes.c_void_p * 2)(0, 3 * 4)
counts   = [3, 3]
glMultiDrawElements(GL_TRIANGLES, counts, GL_UNSIGNED_INT, indexPtr, 2)
```

Using `numpy`:

```py
indexPtr = np.array([0, 3*4], dtype=numpy.intp)
counts   = [3, 3]
glMultiDrawElements(GL_TRIANGLES, counts, GL_UNSIGNED_INT, indexPtr, 2)
```