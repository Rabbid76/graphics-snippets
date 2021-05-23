# How to make animation of coordinate Y in opengl
# https://stackoverflow.com/questions/53105959/how-to-make-animation-of-coordinate-y-in-opengl/53106392#53106392

import math
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

def display():
    timeSinceStart = glutGet(GLUT_ELAPSED_TIME)

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
   
    glMaterialfv(GL_FRONT, GL_DIFFUSE, (0, 0, 1, 0.8)) 
    glPushMatrix()                   
    glTranslatef(0.0, 0.0, math.cos(timeSinceStart*0.002)-0.5)          
    glutSolidCylinder(0.5, 1, 20, 20)
    glPopMatrix()    

    glutSwapBuffers()
    glutPostRedisplay()

glutInit(sys.argv)
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA)
glutInitWindowSize(400, 400)
glutCreateWindow(b"OpenGL Window")
glutDisplayFunc(display)

glEnable(GL_DEPTH_TEST)
glLightModelfv(GL_LIGHT_MODEL_AMBIENT, (1, 1, 1, 1)) 
glEnable(GL_LIGHTING)                           
glEnable(GL_LIGHT0)                           
glLightfv(GL_LIGHT0, GL_POSITION, (1, 1, 1))   

glMatrixMode(GL_PROJECTION)
glLoadIdentity()
gluPerspective(90, 1, 0.1, 10)
glMatrixMode(GL_MODELVIEW)
glLoadIdentity()
glTranslatef(0, 0, -2)
glRotatef(-90, 1, 0, 0)   

glutMainLoop()