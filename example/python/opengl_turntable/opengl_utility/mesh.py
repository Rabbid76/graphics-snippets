import ctypes 
from OpenGL.GL import *

class SingleMesh:
    def __init__(self, mesh_specification, attribute_map = None):
        attr_array = mesh_specification.attributes
        index_array = mesh_specification.indices
        stride, format = mesh_specification.format

        self.__no_vertices = len(attr_array) // stride
        self.__no_indices = len(index_array) if index_array else 0
        vertex_attributes = (ctypes.c_float * len(attr_array))(*attr_array)
        if self.__no_indices:
            indices = (ctypes.c_uint32 * self.__no_indices)(*index_array)
        
        self.__vao = glGenVertexArrays(1)
        if self.__no_indices:
            self.__vbo, self.__ibo = glGenBuffers(2)
        else:
            self.__vbo = glGenBuffers(1)
        
        glBindVertexArray(self.__vao)

        if self.__no_indices:
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.__ibo)
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices, GL_STATIC_DRAW)

        glBindBuffer(GL_ARRAY_BUFFER, self.__vbo)
        glBufferData(GL_ARRAY_BUFFER, vertex_attributes, GL_STATIC_DRAW)

        offset = 0
        float_size = ctypes.sizeof(ctypes.c_float)
        for i, attribute_format in enumerate(format):
            attr_name = attribute_format[0]
            attribute_index = i if not attribute_map or attr_name not in attribute_map else attribute_map[attr_name] 
            tuple_size = attribute_format[1]
            glVertexAttribPointer(attribute_index, tuple_size, GL_FLOAT, False, stride*float_size, ctypes.c_void_p(offset))
            offset += tuple_size * float_size
            glEnableVertexAttribArray(i)

    def draw(self):
        glBindVertexArray(self.__vao)
        if self.__no_indices:
            glDrawElements(GL_TRIANGLES, self.__no_indices, GL_UNSIGNED_INT, None)
        else:
            glDrawArrays(GL_TRIANGLES, 0, self.__no_vertices)

