from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
import math

"""
torus knot:
r = cos(q * t) + 2
x = r * cos(p * t)
y = r * sin(p * t)
z = - sin(q * t)

trefoil knot = torus knot (2, 3):
r = cos(3 * t) + 2
x = r * cos(2 * t)
y = r * sin(2 * t)
z = - sin(3 * t)
"""

def ggt(a, b):
    while b != 0:
        c = a % b
        a, b = b, c
    return a

def createTorusKnot(p, q, step):
    g = ggt(p, q)
    p, q = p//g, q//g
    f = max(p, q)
    vertices = []
    for t_degrees in range(0, 360 * f, step):
        t = math.radians(t_degrees / f)
        r = math.cos(q * t) + 2
        x = r * math.cos(p * t)
        y = r * math.sin(p * t)
        z = - math.sin(q * t)
        vertices.append((x, y, z))
    return vertices

p, q, step = 2, 3, 10
vertices = createTorusKnot(p, q, step)

def display():
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    glWindowPos2f(10, 10)
    for c in f'torus({p}, {q})':
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, ord(c))

    glPushMatrix()
    elapsed_seconds = glutGet(GLUT_ELAPSED_TIME) / 1000
    glRotatef(elapsed_seconds * 90, 0, 1, 0)
    glBegin(GL_LINE_LOOP)
    for v in vertices:
        glVertex3fv(v)
    glEnd()
    glPopMatrix()

    glutSwapBuffers()
    glutPostRedisplay()

def sepcial_key(key, x, y):
    global p, q, vertices
    if key == GLUT_KEY_DOWN:
        q -= 1
    elif key == GLUT_KEY_UP:
        q += 1
    if key == GLUT_KEY_LEFT:
        p -= 1
    elif key == GLUT_KEY_RIGHT:
        p += 1
    p = max(1, min(100, p))
    q = max(1, min(100, q))
    vertices = createTorusKnot(p, q, step)


def reshape(width, height):
    glViewport(0, 0, width, height)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(45, width / height, 0.1, 20.0)
    glMatrixMode(GL_MODELVIEW)

glutInit(sys.argv)
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB)
glutInitWindowSize(400, 300)
glutCreateWindow(b"OpenGl Window")
glutSpecialFunc(sepcial_key)
glutReshapeFunc(reshape)
glutDisplayFunc(display)

glClearColor(0.0, 0.0, 0.0, 1.0) 
glMatrixMode(GL_MODELVIEW)    
glLoadIdentity()
glTranslate(0, 0, -10)
glEnable(GL_DEPTH_TEST)

glutMainLoop()

