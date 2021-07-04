from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *
import OpenGL.GL.shaders
from ctypes import c_void_p

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

def display():
    glClear(GL_COLOR_BUFFER_BIT)
    
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, None)

    glutSwapBuffers()
    glutPostRedisplay()

def reshape(width, height):
    glViewport(0, 0, width, height)
    

glutInit(sys.argv)
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB)
glutInitWindowSize(640, 480)
glutCreateWindow(b"OpenGL Window")
glutDisplayFunc(display)
glutReshapeFunc(reshape)

attributes = (GLfloat * 20)(*[-1,-1,0,0,1, 1,-1,0,1,1, 1,1,0,1,0, -1,1,0,0,0])
indices = (GLuint * 6)(*[0,1,2, 0,2,3])

vao = glGenVertexArrays(1)
vbo = glGenBuffers(1)
ebo = glGenBuffers(1)
glBindVertexArray(vao)
glBindBuffer(GL_ARRAY_BUFFER, vbo)
glBufferData(GL_ARRAY_BUFFER, attributes, GL_STATIC_DRAW)
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo)
glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices, GL_STATIC_DRAW)
glVertexAttribPointer(0, 3, GL_FLOAT, False, 5 * 4, None)
glEnableVertexAttribArray(0)
glVertexAttribPointer(1, 2, GL_FLOAT, False, 5 * 4, c_void_p(3 * 4))
glEnableVertexAttribArray(1)

program = OpenGL.GL.shaders.compileProgram(
    OpenGL.GL.shaders.compileShader(sh_vert, GL_VERTEX_SHADER),
    OpenGL.GL.shaders.compileShader(sh_frag, GL_FRAGMENT_SHADER)
)
glUseProgram(program)

glClearColor(0.0, 0.0, 0.0, 0.0)
glutMainLoop()