# How to render text with PyOpenGL?
# https://stackoverflow.com/questions/63836707/how-to-render-text-with-pyopengl

from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *
import OpenGL.GL.shaders
from ctypes import c_void_p
import freetype
import numpy
import glm
import os
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../resource'))

sh_vert = """
#version 460 core

layout (location = 0) in vec4 a_position;
layout (location = 1) in vec2 a_uv;
layout (location = 0) uniform mat4 u_mvp;
out vec2 uv;

void main() 
{
    uv = a_uv;
    gl_Position = u_mvp * a_position;
}
"""

sh_frag = """
#version 460 core

out vec4 frag_color;
in vec2 uv;
layout (location = 1, binding = 0) uniform sampler2D u_texture;

void main() 
{
    frag_color = texture(u_texture, uv).rrrr;
}
"""

def load_font(filename, size):
    global font_texture_id, font_height, font_map

    face = freetype.Face(filename)
    face.set_char_size(size*64)
    
    face_list = []
    font_bitmap_width = 0
    ascender, descender = 0, 0
    for c in range(32, 128):
        face.load_char(chr(c), freetype.FT_LOAD_RENDER | freetype.FT_LOAD_FORCE_AUTOHINT)
        face_list.append((chr(c), font_bitmap_width))
        font_bitmap_width += face.glyph.bitmap.width
        ascender  = max(ascender, face.glyph.bitmap_top)
        descender = max(descender, face.glyph.bitmap.rows-face.glyph.bitmap_top) 
    font_height = ascender + descender

    font_map = {}
    font_bitmap = numpy.zeros((font_height, font_bitmap_width), dtype=numpy.ubyte)
    for c, x in face_list:
        face.load_char(c, freetype.FT_LOAD_RENDER | freetype.FT_LOAD_FORCE_AUTOHINT)
        y = ascender - face.glyph.bitmap_top
        w, h = face.glyph.bitmap.width, face.glyph.bitmap.rows
        font_bitmap[y:y+h, x:x+w].flat = face.glyph.bitmap.buffer
        font_map[c] = (w, x / font_bitmap_width, (x+w) / font_bitmap_width)

    font_texture_id = glGenTextures(1)
    glBindTexture(GL_TEXTURE_2D, font_texture_id)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, font_bitmap_width, font_height, 0, GL_RED, GL_UNSIGNED_BYTE, font_bitmap)

def display():
    glClear(GL_COLOR_BUFFER_BIT)
    glEnable(GL_BLEND)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
    
    glDrawElements(GL_TRIANGLES, no_of_indices, GL_UNSIGNED_INT, None)

    glutSwapBuffers()
    glutPostRedisplay()

def reshape(width, height):
    glViewport(0, 0, width, height)
    projection = glm.ortho(0, width, height, 0, -1, 1)
    glUniformMatrix4fv(0, 1, GL_FALSE, glm.value_ptr(projection))
    

glutInit(sys.argv)
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
glutInitWindowSize(400, 100)
glutCreateWindow('Hello World - FreeType')
glutDisplayFunc(display)
glutReshapeFunc(reshape)

load_font('font/FreeSans.ttf', 64)

# TODO instancing

attributes = [] 
indices = []
x, y = 40, 20
for c in "Hello World!":
    if c in font_map:
        w, h = font_map[c][0], font_height
        u0, u1 = font_map[c][1], font_map[c][2]
        s = len(attributes) // 4
        attributes += [x, y, u0, 0, x + w, y, u1, 0, x + w, y + h, u1, 1, x, y + h, u0, 1]
        indices += [s, s + 1, s + 2, s, s + 2, s + 3]
        x += w + 5

attributes = (GLfloat * len(attributes))(*attributes)
no_of_indices = len(indices)
indices = (GLuint * no_of_indices)(*indices)

vao = glGenVertexArrays(1)
vbo = glGenBuffers(1)
ebo = glGenBuffers(1)
glBindVertexArray(vao)
glBindBuffer(GL_ARRAY_BUFFER, vbo)
glBufferData(GL_ARRAY_BUFFER, attributes, GL_STATIC_DRAW)
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo)
glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices, GL_STATIC_DRAW)
glVertexAttribPointer(0, 2, GL_FLOAT, False, 4 * 4, None)
glEnableVertexAttribArray(0)
glVertexAttribPointer(1, 2, GL_FLOAT, False, 4 * 4, c_void_p(2 * 4))
glEnableVertexAttribArray(1)

program = OpenGL.GL.shaders.compileProgram(
    OpenGL.GL.shaders.compileShader(sh_vert, GL_VERTEX_SHADER),
    OpenGL.GL.shaders.compileShader(sh_frag, GL_FRAGMENT_SHADER)
)
glUseProgram(program)

glClearColor(0.0, 0.0, 0.0, 0.0)
glutMainLoop()