# PyOpenGL Glut Window
# https://stackoverflow.com/questions/67256146/pyopengl-glut-window

from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

def display():

    glClearStencil(0) # default
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)    
    glEnable(GL_STENCIL_TEST)

    glColorMask(False, False, False, False)
    glStencilFunc(GL_ALWAYS, 1, 1)
    glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT)

    glBegin(GL_TRIANGLES)
    glVertex3f(-1, -1, 0)
    glVertex3f(1, -1, 0)
    glVertex3f(0, 0, 0)
    glVertex3f(-1, -1, 0)
    glVertex3f(1, -1, 0)
    glVertex3f(0, 1, 0)
    glEnd()

    glColor4d(0, 0.8, 0.5, 0.5)
    glColorMask(True, True, True, True)
    glStencilFunc(GL_EQUAL, 1, 1)
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP)

    glBegin(GL_TRIANGLES)
    glVertex3f(-1, -1, 0)
    glVertex3f(1, -1, 0)
    glVertex3f(0, 0, 0)
    glVertex3f(-1, -1, 0)
    glVertex3f(1, -1, 0)
    glVertex3f(0, 1, 0)
    glEnd()

    glDisable(GL_STENCIL_TEST)

    glutSwapBuffers()
    glutPostRedisplay()

glutInit(sys.argv)
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA)
glutInitWindowSize(400, 400)
glutCreateWindow(b"OpenGL Window")
glutDisplayFunc(display)
glutMainLoop()