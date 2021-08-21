from OpenGL.GL import *
from OpenGL.GLU import *
from glfw.GLFW import *
from wavefrontloader import *
import os
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../resource'))

if glfwInit() == GLFW_FALSE:
    exit()

glfwWindowHint(GLFW_SAMPLES, 8)
window = glfwCreateWindow(300, 300, "OpenGL Window", None, None)
glfwMakeContextCurrent(window)

#model = WavefrontDisplayList('./model/wavefront/bunny.obj')
model = WavefrontDisplayList('./model/wavefront/buddha.obj')
model_center, model_size = model.center, model.size
angle_x = 0

glLightfv(GL_LIGHT0, GL_AMBIENT, (0.2, 0.2, 0.2, 1.0))
glLightfv(GL_LIGHT0, GL_DIFFUSE, (0.6, 0.6, 0.6, 1.0))
glLightModeliv(GL_LIGHT_MODEL_TWO_SIDE, 1)
glEnable(GL_LIGHT0)
glEnable(GL_LIGHTING)
glEnable(GL_COLOR_MATERIAL)
glEnable(GL_DEPTH_TEST)
glShadeModel(GL_SMOOTH)

while not glfwWindowShouldClose(window):

    vp_size = glfwGetFramebufferSize(window)
    glViewport(0, 0, *vp_size)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()

    glBegin(GL_QUADS)
    glColor3f(0, 0, 0)
    glVertex3f(-1, -1, 0.99)
    glVertex3f(1, -1, 0.99)
    glColor3f(0.1, 0.4, 0.2)
    glVertex3f(1, 1, 0.99)
    glVertex3f(-1, 1, 0.99)
    glEnd()

    glMatrixMode(GL_PROJECTION)
    gluPerspective(45, vp_size[0]/vp_size[1], min(model_size)/4, max(model_size)*4)
    glMatrixMode(GL_MODELVIEW)
    glTranslate(0, 0, -max(model_size)*2)

    glLightfv(GL_LIGHT0, GL_POSITION,  (0, 0, min(model_size), 0))
        
    glPushMatrix()
    glRotatef(angle_x, 0, 1, 0)
    glTranslatef(-model_center[0], -model_center[1], -model_center[2])
    model.render()
    glPopMatrix()
    angle_x += 0.5
    
    glfwSwapBuffers(window)
    glfwPollEvents()

glfwTerminate()
exit()
