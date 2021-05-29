from OpenGL.GL import *
from OpenGL.GLU import *
import glfw

if glfw.init() == glfw.FALSE:
    exit()

window = glfw.create_window(400, 400, "OpenGL Window", None, None)
glfw.make_context_current(window)

while not glfw.window_should_close(window):
    
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

    glfw.swap_buffers(window)
    glfw.poll_events()

glfw.terminate()
exit()
