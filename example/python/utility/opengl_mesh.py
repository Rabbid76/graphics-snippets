
import ctypes 
from OpenGL.GL import *

class SingleMesh:
    def __init__(self, mesh_specification):
        attr_array = mesh_specification.attributes
        index_array = mesh_specification.indices
        stride, format = mesh_specification.format

        self.__no_indices = len(index_array)
        vertex_attributes = (ctypes.c_float * len(attr_array))(*attr_array)
        indices = (ctypes.c_uint32 * self.__no_indices)(*index_array)
        
        self.__vao = glGenVertexArrays(1)
        self.__vbo, self.__ibo = glGenBuffers(2)
        
        glBindVertexArray(self.__vao)

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.__ibo)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices, GL_STATIC_DRAW)

        glBindBuffer(GL_ARRAY_BUFFER, self.__vbo)
        glBufferData(GL_ARRAY_BUFFER, vertex_attributes, GL_STATIC_DRAW)

        offset = 0
        float_size = ctypes.sizeof(ctypes.c_float)
        for i, attribute_format in enumerate(format):   
            tuple_size = attribute_format[1]
            glVertexAttribPointer(i, tuple_size, GL_FLOAT, False, stride*float_size, ctypes.c_void_p(offset))
            offset += tuple_size * float_size
            glEnableVertexAttribArray(i)

    def draw(self):
        glBindVertexArray(self.__vao)
        glDrawElements(GL_TRIANGLES, self.__no_indices, GL_UNSIGNED_INT, None)


class MultiMesh:
    def __init__(self, mesh_specifications, stride, format):
        
        attributes = [mesh.attributes for mesh in mesh_specifications]
        indices = [mesh.indices for mesh in mesh_specifications]
        attributes_len = sum(len(a) for a in attributes)
        indices_len = sum(len(i) for i in indices)

        self.__no_of_meshes = len(mesh_specifications)
        draw_indirect_list = []
        first_index = 0
        base_vertex = 0
        for attr_list, index_list in zip(attributes, indices):
            no_of_indices = len(index_list)
            no_of_attributes = len(attr_list) // stride
            draw_indirect_list += [no_of_indices, 1, first_index, base_vertex, 0]
            first_index += no_of_indices
            base_vertex += no_of_attributes

        self.__vao = glGenVertexArrays(1)
        self.__vbo, self.__ibo, self.__dbo = glGenBuffers(3)

        draw_indirect_array = (ctypes.c_uint32 * len(draw_indirect_list))(*draw_indirect_list)
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, self.__dbo)
        glBufferData(GL_DRAW_INDIRECT_BUFFER, draw_indirect_array, GL_STATIC_DRAW)

        glBindBuffer(GL_ARRAY_BUFFER, self.__vbo)
        glBufferData(GL_ARRAY_BUFFER, attributes_len*4, None, GL_STATIC_DRAW)
        offset = 0
        for attr_list in attributes:
            no_of_values = len(attr_list)
            value_array = (ctypes.c_float * no_of_values)(*attr_list)
            glBufferSubData(GL_ARRAY_BUFFER, offset, no_of_values*4, value_array)
            offset += no_of_values*4

        glBindVertexArray(self.__vao)

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.__ibo)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_len*4, None, GL_STATIC_DRAW)
        offset = 0
        for index_list in indices:
            no_of_indices = len(index_list)
            index_array = (ctypes.c_uint32 * no_of_indices)(*index_list)
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, no_of_indices*4, index_array)
            offset += no_of_indices*4

        offset = 0
        float_size = ctypes.sizeof(ctypes.c_float)
        for i, attribute_format in enumerate(format):   
            tuple_size = attribute_format[1]
            glVertexAttribPointer(i, tuple_size, GL_FLOAT, False, stride*float_size, ctypes.c_void_p(offset))
            offset += tuple_size * float_size
            glEnableVertexAttribArray(i)

    def draw(self):
        self.draw_range(0, self.__no_of_meshes)
    
    def draw_range(self, start, end):
        if start < end <= self.__no_of_meshes:
            # GLAPI/glDrawElementsIndirect
            # https://www.khronos.org/opengl/wiki/GLAPI/glDrawElementsIndirect
            glBindVertexArray(self.__vao)
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, self.__dbo)
            glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, ctypes.c_void_p(start*4*5), end-start, 4*5)
