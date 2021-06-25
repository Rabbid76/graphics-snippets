# pyopengl - dynamically updating values in a vertex buffer object
# https://stackoverflow.com/questions/15672720/pyopengl-dynamically-updating-values-in-a-vertex-buffer-object
#
# "PyOpenGL/OpenGL/arrays/vbo.py": https://github.com/Distrotech/PyOpenGL/blob/master/OpenGL/arrays/vbo.py
# `OpenGL.arrays.vbo`: http://pyopengl.sourceforge.net/documentation/pydoc/OpenGL.arrays.vbo.html

from OpenGL.GL import *
from OpenGL.GLUT import *
import OpenGL.arrays
import numpy as np

class Jiggle(object):

    def __init__(self, nvert=100, jiggliness = 0.01):
        self.nvert = nvert
        self.jiggliness = jiggliness
        verts = 2*np.random.rand(nvert, 2) - 1
        self.verts = np.require(verts, np.float32, 'F')
        self.vbo = OpenGL.arrays.vbo.VBO(self.verts)

    def draw(self):

        glClearColor(0,0,0,0)
        glClear(GL_COLOR_BUFFER_BIT)

        self.vbo.bind()
        glColor(0, 1, 0, 1)
        glVertexPointer(2, GL_FLOAT, 0, None)
        glEnableClientState(GL_VERTEX_ARRAY)
        glDrawArrays(GL_LINE_LOOP, 0, self.vbo.data.shape[0])
        glDisableClientState(GL_VERTEX_ARRAY)
        self.vbo.unbind()

        self.jiggle()

        glutSwapBuffers()
        glutPostRedisplay()

    def jiggle(self):
        delta = (np.random.rand(self.nvert//2, 2) - 0.5) * self.jiggliness
        self.verts[:self.nvert:2] += delta
        self.vbo.bind()
        glBufferSubData(self.vbo.target, 0, self.vbo.data)

glutInit(sys.argv)
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA)
glutInitWindowSize(250, 250)
glutCreateWindow(b"OpenGL Window")
jiggle = Jiggle()
glutDisplayFunc(jiggle.draw)
glutMainLoop()