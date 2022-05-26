# Using offsets into the buffer in PyOpenGL calls
# https://stackoverflow.com/questions/6431103/using-offsets-into-the-buffer-in-pyopengl-calls

import os 

from OpenGL.GLUT import *
from OpenGL.GL import *
from OpenGL.GL.shaders import *
from OpenGL.arrays import *
import numpy
import ctypes

class MyWindow:

    __caption = 'OpenGL Window'
    __vp_size = [800, 600]
    __vp_valid = False
    __glut_wnd = None
    
    __glsl_vert = """
        #version 450 core

        layout (location = 0) in vec3 a_pos;
        layout (location = 1) in vec4 a_col;

        out vec4 v_color;

        void main()
        {
            v_color     = a_col;
            gl_Position = vec4(a_pos.xyz, 1.0);
        }
    """

    __glsl_frag = """
        #version 450 core
        
        out vec4 frag_color;
        in  vec4 v_color;

        void main()
        {
            frag_color = v_color;
        }
    """

    __program = None
    __vao = None
    __vbo = None

    def __init__(self, w, h):
        
        self.__vp_size = [w, h]

        glutInit()
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
        glutInitWindowSize(self.__vp_size[0], self.__vp_size[1])
        __glut_wnd = glutCreateWindow(self.__caption)

        self.__program = compileProgram( 
            compileShader( self.__glsl_vert, GL_VERTEX_SHADER ),
            compileShader( self.__glsl_frag, GL_FRAGMENT_SHADER ),
        )

        self.___attrib = { a : glGetAttribLocation (self.__program, a) for a in ['a_pos','a_col'] }
        print(self.___attrib)

        vertex_attributes = numpy.array([
        #    x      y     z    R  G  B  A
            -0.5,  -0.5,  0,   1, 0, 0, 1, 
             0.5,  -0.5,  0,   1, 1, 0, 1,
             0.5,   0.5,  0,   0, 0, 1, 1,
            -0.5,   0.5,  0,   0, 1, 0, 1
        ], dtype=numpy.float32)

        indices = [0, 1, 2, 0, 2, 3]
        
        self.indexArray = (ctypes.c_uint32 * 6)(*indices)
        #self.indexArray = numpy.array(indices, dtype=numpy.uint32)
        
        self.__vbo = glGenBuffers(2)
        glBindBuffer(GL_ARRAY_BUFFER, self.__vbo[0])
        glBufferData(GL_ARRAY_BUFFER, vertex_attributes, GL_STATIC_DRAW)
        
        self.__vao = glGenVertexArrays(1)
        glBindVertexArray(self.__vao)

        glVertexAttribPointer(0, 3, GL_FLOAT, False, 7*vertex_attributes.itemsize, None)
        glEnableVertexAttribArray(0)
        glVertexAttribPointer(1, 4, GL_FLOAT, False, 7*vertex_attributes.itemsize, ctypes.c_void_p(3*vertex_attributes.itemsize))
        glEnableVertexAttribArray(1)

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.__vbo[1])
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, self.indexArray, GL_STATIC_DRAW)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)    

        self.__darw_indirect_bo = glGenBuffers(2)
        
        """
        typedef  struct {
            uint  count;
            uint  instanceCount;
            uint  firstIndex;
            uint  baseVertex;
            uint  baseInstance;
        } DrawElementsIndirectCommand;
        """

        indirect_1 = [6, 1, 0, 0, 0] 
        indirect_array_1 = (ctypes.c_uint32 * len(indirect_1))(*indirect_1)       
        
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, self.__darw_indirect_bo[0])
        glBufferData(GL_DRAW_INDIRECT_BUFFER, indirect_array_1, GL_STATIC_DRAW)
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0)

        indirect_2 = [3, 1, 0, 0, 0, 3, 1, 3, 0, 0] 
        indirect_array_2 = (ctypes.c_uint32 * len(indirect_2))(*indirect_2)       
        
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, self.__darw_indirect_bo[1])
        glBufferData(GL_DRAW_INDIRECT_BUFFER, indirect_array_2, GL_STATIC_DRAW)
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0)

        glUseProgram(self.__program)

        glutReshapeFunc(self.__reshape)
        glutDisplayFunc(self.__mainloop)

    def run(self):
        glutMainLoop()

    def __reshape(self, w, h):
        self.__vp_valid = False

    def __mainloop(self):

        if not self.__vp_valid:
            self.__vp_size = [glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT)]
            self.__vp_valid = True
            glViewport(0, 0, self.__vp_size[0], self.__vp_size[1])

        glClearColor(0.2, 0.3, 0.3, 1.0)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
          
        index_case = 12
        if index_case == 0:  
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, self.indexArray)

        elif index_case == 1:
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.__vbo[1])
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, None)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)

        elif index_case == 2:
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.__vbo[1])
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, ctypes.c_void_p(3 * 4))
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)

        elif index_case == 3:
            indexArray1 = (ctypes.c_uint32 * 3)(0, 1, 2)
            indexArray2 = (ctypes.c_uint32 * 3)(0, 2, 3)
            #indexArray1 = numpy.array([0, 1, 2], dtype=numpy.uint32)
            #indexArray2 = numpy.array([0, 2, 3], dtype=numpy.uint32)
            #offset   = numpy.array([3, 3], dtype=numpy.uint32)
            counts   = [3, 3]
            indexPtr = (ctypes.c_void_p * 2)(ctypes.addressof(indexArray1), ctypes.addressof(indexArray2))
            #indexPtr = numpy.array([indexArray1.ctypes.data, indexArray2.ctypes.data], dtype=numpy.intp)
            glMultiDrawElements(GL_TRIANGLES, counts, GL_UNSIGNED_INT, indexPtr, 2)

        elif index_case == 4:
            indexArray1 = (GLsizei * 3)(0, 1, 2)
            indexArray2 = (GLsizei * 3)(0, 2, 3)
            counts   = [3, 3]
            indexPtr = (GLvoidp * 2)(ctypes.addressof(indexArray1), ctypes.addressof(indexArray2))
            glMultiDrawElements(GL_TRIANGLES, counts, GL_UNSIGNED_INT, indexPtr, 2)

        elif index_case == 5:
            counts   = [3, 3]
            indexPtr = (ctypes.c_void_p * 2)(0, 3 * 4)
            #indexPtr = numpy.array([0, 3 * 4], dtype=numpy.intp)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.__vbo[1])
            glMultiDrawElements(GL_TRIANGLES, counts, GL_UNSIGNED_INT, indexPtr, 2)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)

        elif index_case == 6:  
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.__vbo[1])
            glDrawElementsBaseVertex(GL_TRIANGLES, 6, GL_UNSIGNED_INT, None, 0)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)

        if index_case == 7:  
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.__vbo[1])
            glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, None, 1)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)

        if index_case == 8:  
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.__vbo[1])
            glDrawElementsInstancedBaseVertex(GL_TRIANGLES, 6, GL_UNSIGNED_INT, None, 1, 0)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)

        if index_case == 9:  
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.__vbo[1])
            glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 6, GL_UNSIGNED_INT, None, 1, 0)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)

        if index_case == 10:  
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.__vbo[1])
            glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, 6, GL_UNSIGNED_INT, None, 1, 0, 0)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)

        elif index_case == 11:
            # GLAPI/glDrawElementsIndirect
            # https://www.khronos.org/opengl/wiki/GLAPI/glDrawElementsIndirect
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, self.__darw_indirect_bo[0])
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.__vbo[1])
            glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, None)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0)
        
        elif index_case == 12:
            # GLAPI/glMultiDrawElementsIndirect
            # https://www.khronos.org/opengl/wiki/GLAPI/glMultiDrawElementsIndirect
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, self.__darw_indirect_bo[1])
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.__vbo[1])
            glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, None, 2, 4*5)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0)

        elif index_case == 13:
            first = [0, 1]
            counts = [3, 3]
            glMultiDrawArrays(GL_TRIANGLES, first, counts, 2)

        glutSwapBuffers()
        glutPostRedisplay()


window = MyWindow(800, 600)
window.run()