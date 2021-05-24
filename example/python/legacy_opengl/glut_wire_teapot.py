# I am trying Opengl in python and iam getting this error
# https://stackoverflow.com/questions/63689977/i-am-trying-opengl-in-python-and-iam-getting-this-error?noredirect=1

from OpenGL.GLUT import *
from OpenGL.GL import *
from OpenGL.GLU import *

def draw():
  glClear(GL_COLOR_BUFFER_BIT)
  glutWireTeapot(0.5)
  glFlush()

glutInit(sys.argv)
glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB)
glutInitWindowSize(250, 250)
glutCreateWindow(b"OpenGL Window")
glutDisplayFunc(draw)
glutMainLoop()
