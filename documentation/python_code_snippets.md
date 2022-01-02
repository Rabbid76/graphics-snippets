
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
