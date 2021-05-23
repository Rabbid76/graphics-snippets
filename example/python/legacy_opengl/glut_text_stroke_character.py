# Why is my OpenGL program unable to load the GLUT bitmap fonts?
# https://stackoverflow.com/questions/55957159/why-is-my-opengl-program-unable-to-load-the-glut-bitmap-fonts/55957334#55957334

# How to change font size of bitmap characters in PyOpenGL?
# https://stackoverflow.com/questions/56754977/how-to-change-font-size-of-bitmap-characters-in-pyopengl/56758538#56758538

from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

def text(x, y, color, text):
    glColor3fv(color)
    for c in text:
        glutStrokeCharacter(GLUT_STROKE_ROMAN, ord(c))

text_x, add_x = 30, 1
def display():
    global text_x, add_x

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    glPushMatrix()
    glTranslate(text_x, 125, 0)
    glScale(0.5, -0.5, 1)
    glLineWidth(5)
    text(0, 0, (1, 0, 0), "Hello World!")
    glPopMatrix()

    glutSwapBuffers()
    glutPostRedisplay()

    text_x += add_x
    if text_x < 30:
        add_x = abs(add_x)
    elif text_x > 120:
        add_x = -abs(add_x)

glutInit(sys.argv)
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA)
glutInitWindowSize(600, 200)
glutCreateWindow(b"OpenGL Window")
glutDisplayFunc(display)

glMatrixMode(GL_PROJECTION)
glLoadIdentity()
glOrtho(0, 500, 200, 0, -1, 1)
glMatrixMode(GL_MODELVIEW)
glLoadIdentity()

glutMainLoop()