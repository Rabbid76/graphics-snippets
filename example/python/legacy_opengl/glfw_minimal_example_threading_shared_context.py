# How to add to opengl pygame VBOs from another process
# https://stackoverflow.com/questions/70795978/how-to-add-to-opengl-pygame-vbos-from-another-process/70816345#70816345

from OpenGL.GL import *
from OpenGL.GLU import *
import glfw
import threading
import time
import math

if glfw.init() == glfw.FALSE:
    exit()

end_trhead = False
def shard_context(window, vbo):
    global end_trhead

    window2 = glfw.create_window(400, 400, "Window 2", None, window)
    glfw.make_context_current(window2)

    glBindBuffer(GL_ARRAY_BUFFER, vbo)
    glEnableClientState(GL_VERTEX_ARRAY)
    glVertexPointer(2, GL_FLOAT, 0, None)

    while not end_trhead and not glfw.window_should_close(window2):
    
        glClear(GL_COLOR_BUFFER_BIT)
    
        glColor3f(0, 1, 0)
        glDrawArrays(GL_TRIANGLES, 0, 3)

        glfw.swap_buffers(window2)
        glfw.poll_events()
        time.sleep(0.001)

window = glfw.create_window(400, 400, "Window 1", None, None)
glfw.make_context_current(window)

r = 1 / math.sqrt(3)
px, py = r * math.cos(math.radians(30)), r * math.sin(math.radians(30)) 
vertices = [-px, -py, px, -py, 0, r]

vbo = glGenBuffers(1)
glBindBuffer(GL_ARRAY_BUFFER, vbo)
glBufferData(GL_ARRAY_BUFFER, (GLfloat * 12)(*vertices), GL_STATIC_DRAW)

thread = threading.Thread(target=shard_context, args=[window, vbo])
thread.start()

glEnableClientState(GL_VERTEX_ARRAY)
glVertexPointer(2, GL_FLOAT, 0, None)
  
while not glfw.window_should_close(window):
    
    glClear(GL_COLOR_BUFFER_BIT)
   
    glColor3f(1, 0, 0)
    glDrawArrays(GL_TRIANGLES, 0, 3)

    glfw.swap_buffers(window)
    glfw.poll_events()
    time.sleep(0.001)

end_trhead = True
thread.join()

glfw.terminate()
exit()
