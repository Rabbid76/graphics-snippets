# How can I draw single 3D points?
# https://stackoverflow.com/questions/56957118/how-can-i-draw-single-3d-points/56957307#56957307

from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

def display():
    
    glClear(GL_COLOR_BUFFER_BIT)
    
    glPointSize(20)

    glDisable(GL_POINT_SMOOTH)
    glBegin(GL_POINTS)
    glVertex2f(-0.5, 0)
    glEnd()

    glEnable(GL_POINT_SMOOTH)
    glBegin(GL_POINTS)
    glVertex2f(0.5, 0)
    glEnd()

    glutSwapBuffers()
    glutPostRedisplay()

if __name__ == '__main__':
    glutInit()
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA)
    glutInitWindowSize(200, 200)
    glutCreateWindow(b"OpenGL Window")
    glutDisplayFunc(display)
    glutMainLoop()