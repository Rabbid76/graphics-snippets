# How to draw a line cube with OpenGL?
# https://stackoverflow.com/questions/62081167/how-to-draw-a-line-cube-with-opengl/62081584#62081584

from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

vertices = (((1, -1, -1), (1, 1, -1), (-1, 1, -1), (-1, -1, -1), 
             (1, -1, 1), (1, 1, 1), (-1, -1, 1), (-1, 1, 1)))
edges = ((0,1), (0,3), (0,4), (2,1), (2,3), (2,7), (6,3), (6,4), (6,7), (5,1), (5,4), (5,7))

def Display():
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    glRotate(1, 0, 1, 0)
    glColor3f(0.2, 0.5, 0.4)
    glBegin(GL_LINES)
    for edge in edges:
        for vertex in edge:
            glVertex3fv(vertices[vertex])
    glEnd()

    glutSwapBuffers()
    glutPostRedisplay()

width, height = 400, 300
glutInit(sys.argv)
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB)
glutInitWindowSize(width, height)
glutCreateWindow(b"OpenGl Window")
glutDisplayFunc(Display)

glClearColor(0.0, 0.0, 0.0, 1.0) 
    
glMatrixMode(GL_PROJECTION)
glLoadIdentity()
gluPerspective(45, width / height, 0.1, 10.0)

glMatrixMode(GL_MODELVIEW)
glLoadIdentity()
glTranslate(0, 0, -5)

glutMainLoop()

