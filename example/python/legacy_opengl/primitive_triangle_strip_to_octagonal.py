# Convert triangle strips to Octagonal?
# https://stackoverflow.com/questions/65141581/convert-triangle-strips-to-octagonal/65144613#65144613

from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *
import math

class MyWindow:

    def __init__(self, w, h):
        glutInit()
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
        glutInitWindowSize(640, 480)
        self.__glut_wnd = glutCreateWindow('OpenGL Window')
        glutReshapeFunc(self.__reshape)
        glutDisplayFunc(self.__mainloop)

    def run(self):
        glutMainLoop()

    def __reshape(self, w, h):
        glViewport(0, 0, w, h)
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        aspect = w / h
        glOrtho(-aspect, aspect, -1, 1, -1, 1)
        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()

    def __mainloop(self):
        glClearColor(0, 0, 0, 1.0)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glEnable(GL_DEPTH_TEST)

        for renderpass in range(2):

            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE if renderpass==0 else GL_FILL)
            if renderpass == 1:
                glColor3f(0, 0.5, 0)
            else:
                glColor3f(1, 1, 1)

            glBegin(GL_TRIANGLE_STRIP)
            radius = 0.5
            step = 8
            for i in range(step+1):
                strip_i = i // 2 if (i % 2) == 0 else step - i // 2
                angle = 3.1415926 * 2.0 * strip_i / step
                newX =   radius * math.sin(angle)
                newY = - radius * math.cos(angle)
                glVertex3f(newX, newY, 0.0)
            glEnd()

        glutSwapBuffers()
        glutPostRedisplay()


window = MyWindow(800, 600)
window.run()