from OpenGL.GLUT import *
from OpenGL.GL import *
import OpenGL.GL.shaders
import glm
import math

sh_vert = """
#version 460 core

layout (location = 0) in vec2 a_position;

layout (location = 0) uniform mat4 u_projection;
layout (location = 1) uniform mat4 u_model_view;

void main() 
{
    gl_Position = u_projection * u_model_view * vec4(a_position, 0.0, 1.0);
}
"""

sh_frag = """
#version 460 core

out vec4 frag_color;

void main()
{
    frag_color = vec4(1.0, 0.0, 0.0, 1.0);
}
"""

def display():
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    
    projection = glm.ortho(-aspect, aspect, -1, 1, -1, 1)
    if aspect < 1:
        projection = glm.ortho(-1, 1, 1/-aspect, 1/aspect, -1, 1)

    elapsed_ms = glutGet(GLUT_ELAPSED_TIME)
    angle = elapsed_ms * math.pi * 2 / 5000.0
    model_view = glm.translate(glm.mat4(1), glm.vec3(math.cos(angle)*0.5, math.sin(angle)*0.5, 0))
    model_view = glm.scale(model_view, glm.vec3(0.2, 0.2, 1))
    glUniformMatrix4fv(0, 1, GL_FALSE, glm.value_ptr(projection))
    glUniformMatrix4fv(1, 1, GL_FALSE, glm.value_ptr(model_view))

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4)

    glutSwapBuffers()
    glutPostRedisplay()

aspect = 1
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

attributes = (GLfloat * 8)(-1,-1, 1,-1, 1, 1, -1, 1) 

vao = glGenVertexArrays(1)
vbo = glGenBuffers(1)
glBindVertexArray(vao)
glBindBuffer(GL_ARRAY_BUFFER, vbo)
glBufferData(GL_ARRAY_BUFFER, attributes, GL_STATIC_DRAW)
glVertexAttribPointer(0, 2, GL_FLOAT, False, 2*4, None)
glEnableVertexAttribArray(0)

program = OpenGL.GL.shaders.compileProgram(
    OpenGL.GL.shaders.compileShader(sh_vert, GL_VERTEX_SHADER),
    OpenGL.GL.shaders.compileShader(sh_frag, GL_FRAGMENT_SHADER)
)
glUseProgram(program)

glEnable(GL_DEPTH_TEST)
glClearColor(0.0, 0.0, 0.0, 0.0)
glutMainLoop()