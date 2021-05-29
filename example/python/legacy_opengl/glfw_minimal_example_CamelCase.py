from OpenGL.GL import *
from OpenGL.GLU import *
from glfw.GLFW import *

if glfwInit() == GLFW_FALSE:
    exit()

window = glfwCreateWindow(400, 400, "OpenGL Window", None, None)
glfwMakeContextCurrent(window)

while not glfwWindowShouldClose(window):
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
   
    glBegin(GL_QUADS)
    glColor3f(1, 0, 0)
    glVertex2f(-0.5, 0.5)
    glColor3f(1, 1, 0)
    glVertex2f(-0.5, -0.5)
    glColor3f(0, 1, 0)
    glVertex2f(0.5, -0.5)
    glColor3f(0, 0, 1)
    glVertex2f(0.5, 0.5)
    glEnd()

    glfwSwapBuffers(window)
    glfwPollEvents()

glfwTerminate()
exit()
