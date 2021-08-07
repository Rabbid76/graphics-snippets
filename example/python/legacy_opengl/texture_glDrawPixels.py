from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *
from PIL import Image
import os
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../resource'))

rotate = [33, 40, 20]

def display():
    glClear(GL_COLOR_BUFFER_BIT)
    
    glBegin(GL_LINE_LOOP)
    glVertex3f(-0.95, 0.95, 0)
    glVertex3f(-0.95, -0.95, 0)
    glVertex3f(0.95, -0.95, 0)
    glVertex3f(0.95, 0.95, 0)
    glEnd()
    
    glWindowPos2i(22, 22)
    glDrawPixels(*pilImage.size, GL_RGB, GL_UNSIGNED_BYTE, pilImage.tobytes())
    
    glutSwapBuffers()
    glutPostRedisplay()

glutInit(sys.argv)
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB)
glutInitWindowSize(300, 300)
glutCreateWindow(b"OpenGL Window")
glutDisplayFunc(display)

pilImage = Image.open('texture/woodtiles.jpg').transpose(Image.FLIP_TOP_BOTTOM)

glutMainLoop()