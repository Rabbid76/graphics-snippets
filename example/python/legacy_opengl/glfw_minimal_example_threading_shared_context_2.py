# How to add to opengl pygame VBOs from another process
# https://stackoverflow.com/questions/70795978/how-to-add-to-opengl-pygame-vbos-from-another-process/70816345#70816345

from OpenGL.GL import *
from OpenGL.GLU import *
import glfw
import threading
import math

if glfw.init() == glfw.FALSE:
    exit()

def shard_context(window, vbo):

    glfw.window_hint(glfw.VISIBLE, glfw.FALSE)
    window2 = glfw.create_window(400, 400, "Window 2", None, window)
    glfw.make_context_current(window2)

    r = 1 / math.sqrt(3)
    px, py = r * math.cos(math.radians(30)), r * math.sin(math.radians(30)) 
    vertices = [-px, -py, px, -py, 0, r]

    glBindBuffer(GL_ARRAY_BUFFER, vbo)
    glBufferSubData(GL_ARRAY_BUFFER, 0, 12*4, (GLfloat * 12)(*vertices))
    glFlush()


window = glfw.create_window(400, 400, "Window 1", None, None)
glfw.make_context_current(window)

vbo = glGenBuffers(1)
glBindBuffer(GL_ARRAY_BUFFER, vbo)
glBufferData(GL_ARRAY_BUFFER, 12 * 4, None, GL_STATIC_DRAW)

thread = threading.Thread(target=shard_context, args=[window, vbo])
thread.start()

glEnableClientState(GL_VERTEX_ARRAY)
glVertexPointer(2, GL_FLOAT, 0, None)
  
while not glfw.window_should_close(window):
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
   
    glColor3f(1, 0, 0)
    glDrawArrays (GL_TRIANGLES, 0, 3)

    glfw.swap_buffers(window)
    glfw.poll_events()

glfw.terminate()
exit()