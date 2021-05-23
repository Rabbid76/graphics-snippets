# How can I create a cylinder linking two points with Python withour using blender's bpy?
# https://stackoverflow.com/questions/56360147/how-can-i-create-a-cylinder-linking-two-points-with-python-withour-using-blender/56361187#56361187

from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *
import math

def cross(a, b):
    return [a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]]

def cylinder_between(x1, y1, z1, x2, y2, z2, rad):
    v = [x2-x1, y2-y1, z2-z1]
    height = math.sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2])
    axis = (1, 0, 0) if math.hypot(v[0], v[1]) < 0.001 else cross(v, (0, 0, 1))
    angle = -math.atan2(math.hypot(v[0], v[1]), v[2])*180/math.pi
    
    glPushMatrix()
    glTranslate(x1, y1, z1)
    glRotate(angle, *axis)
    glutSolidCylinder(rad, height, 32, 16)
    glPopMatrix()

def draw():

    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(45, wnd_w/wnd_h, 0.1, 10)
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()
    gluLookAt(0, -2, 0, 0, 0, 0, 0, 0, 1)

    glClearColor(0.5, 0.5, 0.5, 1.0)  
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    # glEnable(GL_DEPTH_TEST)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)
    glColor(1, 1, 0.5)
    #cylinder_between(0.0, 0.0, -0.5, 0.0, 0.0, 0.5, 0.3)
    cylinder_between(0.2, 0.4, -0.5, -0.2, -0.4, 0.5, 0.3)

    glutSwapBuffers()
    glutPostRedisplay()

wnd_w, wnd_h = 300, 300
glutInit()
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
glutInitWindowSize(wnd_w, wnd_h)
glutInitWindowPosition(50, 50)
glutCreateWindow(b"OpenGL Window")
glutDisplayFunc(draw)
glutMainLoop()

glutMainLoop()