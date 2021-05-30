# How to draw a line cube with OpenGL?
# https://stackoverflow.com/questions/62081167/how-to-draw-a-line-cube-with-opengl/62081584#62081584

from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

#vertices = [(-1,-1,-1), ( 1,-1,-1), ( 1, 1,-1), (-1, 1,-1), (-1,-1, 1), ( 1,-1, 1), ( 1, 1, 1), (-1, 1, 1)]
#edges = [(0,1), (1,2), (2,3), (3,0), (4,5), (5,6), (6,7), (7,4), (0,4), (1,5), (2,6), (3,7)]

vertices = [((i%2)*2-1, (i//2%2)*2-1, (i//4)*2-1) for i in range(8)]
v_distance = lambda i, j : sum(abs(vertices[i][k]-vertices[j][k]) for k in range(3))
edges =  [(i, j) for i in range(8) for j in range(i+1, 8) if v_distance(i, j) == 2]

def display():
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

def reshape(width, height):
    glViewport(0, 0, width, height)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(45, width / height, 0.1, 10.0)
    glMatrixMode(GL_MODELVIEW)

glutInit(sys.argv)
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB)
glutInitWindowSize(400, 300)
glutCreateWindow(b"OpenGl Window")
glutReshapeFunc(reshape)
glutDisplayFunc(display)

glClearColor(0.0, 0.0, 0.0, 1.0) 
glMatrixMode(GL_MODELVIEW)    
glLoadIdentity()
glTranslate(0, 0, -5)

glutMainLoop()

