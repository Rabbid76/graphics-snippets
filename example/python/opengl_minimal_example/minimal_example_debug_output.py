from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *
import OpenGL.GL.shaders
import numpy as np
from ctypes import c_void_p
import glm
import math

sh_vert = """
#version 460 core

layout (location = 0) in vec4 a_position;
layout (location = 1) in vec4 a_color;
out vec4 v_color;
layout (location = 0) uniform mat4 u_projection;

void main() 
{
    v_color = a_color;
    gl_Position = u_projection * a_position;
}
"""

sh_frag = """
#version 460 core

out vec4 frag_color;
in vec4 v_color;

void main()
{
    frag_color = v_color;
}
"""

def display():
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    
    if aspect >= 1:
        projection = glm.ortho(-aspect, aspect, -1, 1, -1, 1)
    else:
        projection = glm.ortho(-1, 1, -1/aspect, 1/aspect, -1, 1)  
    glUniformMatrix4fv(0, 1, GL_FALSE, glm.value_ptr(projection))

    glDrawArrays(GL_TRIANGLES, 0, 3)

    glutSwapBuffers()
    glutPostRedisplay()

def reshape(width, height):
    global aspect
    glViewport(0, 0, width, height)
    aspect = width / height    

glutInit(sys.argv)
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE)
glutSetOption(GLUT_MULTISAMPLE, 8)
glutInitWindowSize(640, 480)
glutCreateWindow(b"OpenGL Window")
glutDisplayFunc(display)
glutReshapeFunc(reshape)

vendor, renderer = glGetString(GL_VENDOR).decode("utf-8"), glGetString(GL_RENDERER).decode("utf-8")
version, glsl_version = glGetString(GL_VERSION).decode("utf-8"), glGetString(GL_SHADING_LANGUAGE_VERSION).decode("utf-8")
major, minor = glGetInteger(GL_MAJOR_VERSION), glGetInteger(GL_MINOR_VERSION)
extensions = [glGetStringi(GL_EXTENSIONS, i) for i in range(glGetInteger(GL_NUM_EXTENSIONS))]

print(f"{vendor}, {renderer}, OpenGL: {version}, GLSL: {glsl_version}, Context {major}.{minor}")
#print(f'{len(extensions)} extensions')
#for e in extensions:
#    print(f'    {e}')
print('')

@GLDEBUGPROC
def __CB_OpenGL_DebugMessage(source, type, id, severity, length, message, userParam):
    msg = message[0:length]
    print(msg.decode("utf-8"))
glDebugMessageCallback(__CB_OpenGL_DebugMessage, None)

errors_only = False
if errors_only:
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, None, GL_FALSE)
    glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, None, GL_TRUE)
else:
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, None, GL_TRUE)

glEnable(GL_DEBUG_OUTPUT)
glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS)
glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Starting debug messaging service")

attributes = np.array([
#    x       y     z    R  G  B  A
    -0.866, -0.75, 0,   1, 0, 0, 1, 
     0.866, -0.75, 0,   1, 1, 0, 1,
     0,      0.75, 0,   0, 0, 1, 1
], dtype=np.float32)

vao = glGenVertexArrays(1)
vbo = glGenBuffers(1)
ebo = glGenBuffers(1)
glBindVertexArray(vao)
glBindBuffer(GL_ARRAY_BUFFER, vbo)
glBufferData(GL_ARRAY_BUFFER, attributes, GL_STATIC_DRAW)
glVertexAttribPointer(0, 3, GL_FLOAT, False, 7 * attributes.itemsize, None)
glVertexAttribPointer(1, 4, GL_FLOAT, False, 7 * attributes.itemsize, c_void_p(3 * attributes.itemsize))
glEnableVertexAttribArray(0)
glEnableVertexAttribArray(1)

program = OpenGL.GL.shaders.compileProgram(
    OpenGL.GL.shaders.compileShader(sh_vert, GL_VERTEX_SHADER),
    OpenGL.GL.shaders.compileShader(sh_frag, GL_FRAGMENT_SHADER)
)
glUseProgram(program)

glEnable(GL_MULTISAMPLE) # default
glEnable(GL_DEPTH_TEST)

glClearColor(0.0, 0.0, 0.0, 0.0)
glutMainLoop()