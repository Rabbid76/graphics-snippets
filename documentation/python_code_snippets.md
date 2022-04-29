
[![StackOverflow](https://stackexchange.com/users/flair/7322082.png)](https://stackoverflow.com/questions/70552903/print-the-content-of-a-vbo/70554433#70554433)

---

# Python code snippets

## Print buffer

```py
glBindBuffer(GL_ARRAY_BUFFER, vbo)

no_of_floats = 12 # read 12 floats
float_array = (GLfloat * no_of_floats)()
glGetBufferSubData(GL_ARRAY_BUFFER, 0, no_of_floats * sizeof(GLfloat), float_array)
print(list(float_array))
```

## Buffer to byte array

Use [`numpy.frombuffer`](https://numpy.org/doc/stable/reference/generated/numpy.frombuffer.html) to convert the string to a byte array:

```py
import numpy
```

```py
data = glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE)
byte_array = numpy.frombuffer(data, dtype='uint8')
print(list(byte_array))
```

Or use [`ctypes.from_buffer_copy`](https://docs.python.org/3/library/ctypes.html#ctypes._CData.from_buffer_copy)

```py
import ctypes
```

```py
data = glReadPixels(0,0, w, h, GL_RGB,GL_UNSIGNED_BYTE)
byte_array = (ctypes.c_ubyte * (w * h * 3)).from_buffer_copy(data)
print(list(byte_array))
```

## Vector and matrix operations

### Arithmetic

```py
def add(a, b):
    return a[0]+b[0], a[1]+b[1], a[2]+b[2]
```

```py
def sub(a, b):
    return a[0]-b[0], a[1]-b[1], a[2]-b[2]
```

```py
def mul(v, s):
    return v[0]*s, v[1]*s, v[2]*s
```

### Dot product

[Dot product](https://en.wikipedia.org/wiki/Dot_product)

```py
def dot(a, b):
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2]
```

### Cross product

[Cross product](https://en.wikipedia.org/wiki/Cross_product)

```py
def cross(a, b):
    return (
        a[1]*b[2] - a[2]*b[1],
        a[2]*b[0] - a[0]*b[2],
        a[0]*b[1] - a[1]*b[0]
    )
```

## Intersection

### Point in triangle

```py
def point_in_segment(px, p0, a, b):
    cp1 = cross(sub(b, a), sub(px, a))
    cp2 = cross(sub(b, a), sub(p0, a))
    return dot(cp1, cp2) >= 0 
```

```py
def point_in_triangle(p, a, b, c):
    return point_in_segment(p, a, b, c) and point_in_segment(p, b, c, a) and point_in_segment(p, c, a, b)
```

### Intersect ray and plane

```py
def intersect_plane(o, d, p0, p1, p2):
    n = cross(sub(p1, p0), sub(p2, p0))
    t = dot(sub(p0, o), n) / dot(d, n)
    x = add(o, mul(d, t))
    return x, t
```

### Intersect ray and triangle

The algorithm does not depend on the winding order.

```py
def intersect_triangle(o, d, p0, p1, p2):
    x, t = intersect_plane(o, d, p0, p1, p2)
    return point_in_triangle(x, p0, p1, p2), x, t
```
