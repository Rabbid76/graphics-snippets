# glBitmap: bitmap size issue
# https://stackoverflow.com/questions/67916860/glbitmap-bitmap-size-issue

from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *
import numpy

size = 80
a = numpy.full(size//8, 0x55, dtype=numpy.uint8)
b = numpy.full(size//8, 0xaa, dtype=numpy.uint8)
raster = numpy.full((size, size//8), 0x55, dtype=numpy.uint8)
for i in range(size):
    raster[i, :] = a if i % 2 == 0 else b

def display():
    glMatrixMode(GL_MODELVIEW)
    glClear(GL_COLOR_BUFFER_BIT)
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1)

    x1, y1 = 20, 20
    x2, y2 = x1 + size * 2 // 3 - 1, y1 + size * 2 // 3
    x3, y3 = x2 + size * 2 // 3 + 1, y2 + size * 2 // 3

    glColor3f(1, 0, 0)
    glWindowPos2i(x2, y1)  
    glBitmap(size, size, 0, 0, 0, 0, raster)

    glColor3f(0, 1, 0)
    glWindowPos2i(x1, y2)  
    glBitmap(size, size, 0, 0, 0, 0, raster)

    glColor3f(0, 0, 1)
    glWindowPos2i(x2, y3)  
    glBitmap(size, size, 0, 0, 0, 0, raster)

    glColor3f(1, 1, 0)
    glWindowPos2i(x3, y2)  
    glBitmap(size, size, 0, 0, 0, 0, raster)

    glutSwapBuffers()
    glutPostRedisplay()

def reshape(width, height):
    glViewport(0, 0, width, height)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    glOrtho(0, width, height, 0, 1, -1)
    glMatrixMode(GL_MODELVIEW)

glutInit(sys.argv)
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB)
glutInitWindowSize(227, 227)
glutCreateWindow(b"OpenGL Window")
glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)
glClearColor(0.0, 0.0, 0.0, 0.0)
glutDisplayFunc(display)
glutReshapeFunc(reshape)
glutMainLoop()