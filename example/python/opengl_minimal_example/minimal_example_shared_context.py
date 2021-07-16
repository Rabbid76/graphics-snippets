from OpenGL.GL import *
from OpenGL.GLU import *
import OpenGL.GL.shaders
import glfw
import time
import threading
import ctypes

sh_vert = """
#version 460 core

layout (location = 0) in vec4 a_position;
layout (location = 1) in vec2 a_uv;
out vec2 uv;

void main() 
{
    uv = a_uv;
    gl_Position = a_position;
}
"""

sh_frag = """
#version 460 core

out vec4 frag_color;
in vec2 uv;

void main() 
{
    frag_color = vec4(1.0-uv.x, 1.0-uv.y, uv.x*uv.y, 1.0);
}
"""

if glfw.init() == glfw.FALSE:
    exit()

loaded_lock = threading.Lock()
terminate_load_thread = False
def load_thread_function():
    global vbo, ebo, program

    loaded_lock.acquire()
    glfw.make_context_current(load_window)

    attributes = (GLfloat * 20)(*[-1,-1,0,0,1, 1,-1,0,1,1, 1,1,0,1,0, -1,1,0,0,0])
    indices = (GLuint * 6)(*[0,1,2, 0,2,3])

    default_vao = glGenVertexArrays(1)
    vbo = glGenBuffers(1)
    ebo = glGenBuffers(1)
    glBindVertexArray(default_vao)
    glBindBuffer(GL_ARRAY_BUFFER, vbo)
    glBufferData(GL_ARRAY_BUFFER, attributes, GL_STATIC_DRAW)
    glBindBuffer(GL_ARRAY_BUFFER, ebo)
    glBufferData(GL_ARRAY_BUFFER, indices, GL_STATIC_DRAW)

    program = OpenGL.GL.shaders.compileProgram(
        OpenGL.GL.shaders.compileShader(sh_vert, GL_VERTEX_SHADER),
        OpenGL.GL.shaders.compileShader(sh_frag, GL_FRAGMENT_SHADER)
    )

    loaded_lock.release()

    while not glfw.window_should_close(load_window) and not terminate_load_thread:
        time.sleep(0.01)
        glfw.swap_buffers(load_window)
        glfw.poll_events()
    print("terminate load thread")

glfw.window_hint(glfw.VISIBLE, glfw.FALSE)
load_window = glfw.create_window(400, 400, "OpenGL load Window", None, None)
#glfw.make_context_current(load_window)
glfw.window_hint(glfw.VISIBLE, glfw.TRUE)
window = glfw.create_window(400, 400, "OpenGL Window", None, load_window)
glfw.make_context_current(window)

load_thread = threading.Thread(target=load_thread_function)
load_thread.start()
loaded_lock.acquire()

vao = glGenVertexArrays(1)
glBindVertexArray(vao)
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo)
glBindBuffer(GL_ARRAY_BUFFER, vbo)
glVertexAttribPointer(0, 3, GL_FLOAT, False, 5 * 4, None)
glEnableVertexAttribArray(0)
glVertexAttribPointer(1, 2, GL_FLOAT, False, 5 * 4, ctypes.c_void_p(3 * 4))
glEnableVertexAttribArray(1)

glUseProgram(program)

while not glfw.window_should_close(window):
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
   
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, None)

    glfw.swap_buffers(window)
    glfw.poll_events()

terminate_load_thread = True
load_thread.join()
print("terminate")

glfw.terminate()
exit()