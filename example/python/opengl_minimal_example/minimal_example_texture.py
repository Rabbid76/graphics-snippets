from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *
import OpenGL.GL.shaders
import numpy as np
from ctypes import c_void_p
from PIL import Image
import os
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../resource'))

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
layout (binding=1) uniform sampler2D u_texture;

void main() 
{
    frag_color = texture(u_texture, uv);
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

attributes = np.array([-1,-1,0,0,1, 1,-1,0,1,1, 1,1,0,1,0, -1,1,0,0,0], dtype=np.float32) 
indices = np.array([0,1,2, 0,2,3], dtype=np.uint32)

vao = glGenVertexArrays(1)
vbo = glGenBuffers(1)
ebo = glGenBuffers(1)
glBindVertexArray(vao)
glBindBuffer(GL_ARRAY_BUFFER, vbo)
glBufferData(GL_ARRAY_BUFFER, attributes, GL_STATIC_DRAW)
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo)
glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices, GL_STATIC_DRAW)
glVertexAttribPointer(0, 3, GL_FLOAT, False, 5 * attributes.itemsize, None)
glEnableVertexAttribArray(0)
glVertexAttribPointer(1, 2, GL_FLOAT, False, 5 * attributes.itemsize, c_void_p(3 * attributes.itemsize))
glEnableVertexAttribArray(1)

pil_image = Image.open('texture/parrot_image.jpg')
texture_id = glGenTextures(1)
glActiveTexture(GL_TEXTURE1)
glBindTexture(GL_TEXTURE_2D, texture_id)
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) 
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) 
format = GL_RGBA if pil_image.mode == 'RGBA' else GL_RGB
glTexImage2D(GL_TEXTURE_2D, 0, format, *pil_image.size, 0, format, GL_UNSIGNED_BYTE, pil_image.tobytes())

pil_image = Image.open('texture/lookuptable.png')
texture_id = glGenTextures(1)
glActiveTexture(GL_TEXTURE2)
glBindTexture(GL_TEXTURE_2D, texture_id)
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) 
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) 
format = GL_RGBA if pil_image.mode == 'RGBA' else GL_RGB
glTexImage2D(GL_TEXTURE_2D, 0, format, *pil_image.size, 0, format, GL_UNSIGNED_BYTE, pil_image.tobytes())

program = OpenGL.GL.shaders.compileProgram(
    OpenGL.GL.shaders.compileShader(sh_vert, GL_VERTEX_SHADER),
    OpenGL.GL.shaders.compileShader(sh_frag, GL_FRAGMENT_SHADER)
)
glUseProgram(program)

glClearColor(0.0, 0.0, 0.0, 0.0)
glutMainLoop()