from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
from PIL import Image
import os
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../resource'))

vertices = [(-1,-1,-1), ( 1,-1,-1), ( 1, 1,-1), (-1, 1,-1), (-1,-1, 1), ( 1,-1, 1), ( 1, 1, 1), (-1, 1, 1)]
#edges = [(0,1), (1,2), (2,3), (3,0), (4,5), (5,6), (6,7), (7,4), (0,4), (1,5), (2,6), (3,7)]
faces = [(4,0,3,7), (1,0,4,5), (0,1,2,3), (1,5,6,2), (3,2,6,7), (5,4,7,6)]
normals = [(-1, 0, 0), (0, -1, 0),  (0, 0, -1),  (1, 0, 0), (0, 1, 0), (0, 0, 1)]
#colors = [(1, 0, 0), (0, 1, 0), (0, 0, 1), (1, 1, 0), (1, 0, 1), (0, 1, 1)]
texture_coordinates = [(0, 0), (0, 1), (1, 1), (1, 0)]

def display():
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    glRotatef(1, 3, 1, 1)
    glBegin(GL_QUADS)
    for i, face in enumerate(faces):
        #glColor3fv(colors[i])
        glNormal3fv(normals[i])
        for j, vertex in enumerate(face):
            glTexCoord2fv(texture_coordinates[j])
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
glEnable(GL_DEPTH_TEST)

glEnable(GL_LIGHTING)
glEnable(GL_LIGHT0)
glEnable(GL_COLOR_MATERIAL)
glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE)

#glLight(GL_LIGHT0, GL_POSITION,  (0, 0, 1, 0)) # directional light from the front
glLight(GL_LIGHT0, GL_POSITION,  (5, 5, 5, 1)) # point light from the left, top, front
glLightfv(GL_LIGHT0, GL_AMBIENT, (0, 0, 0, 1))
glLightfv(GL_LIGHT0, GL_DIFFUSE, (1, 1, 1, 1))

pil_image = Image.open('texture/ObjectSheet.png')
texture_id = glGenTextures(1)
glClientActiveTexture(GL_TEXTURE0)
glBindTexture(GL_TEXTURE_2D, texture_id)
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) 
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) 
format = GL_RGBA if pil_image.mode == 'RGBA' else GL_RGB
glTexImage2D(GL_TEXTURE_2D, 0, format, *pil_image.size, 0, format, GL_UNSIGNED_BYTE, pil_image.tobytes())

glEnable(GL_TEXTURE_2D)

glutMainLoop()

