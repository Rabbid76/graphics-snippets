# Visualize a single character in an OpenGL window (python)
# https://stackoverflow.com/questions/60738691/visualize-a-single-character-in-an-opengl-window-python

import os
import sys
import numpy
import freetype
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../resource'))

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, font_bitmap_width, font_height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, font_bitmap)

def display():
    glClearColor(0, 0, 0, 0)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    glColor(1, 1, 1, 1)

    glEnable(GL_BLEND)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
    glEnable(GL_TEXTURE_2D)
    glBindTexture(GL_TEXTURE_2D, font_texture_id)

    glBegin(GL_QUADS)
    x, y = 40, 20
    for c in "Hello World!":
        if c in font_map:
            w, h = font_map[c][0], font_height
            u0, u1 = font_map[c][1], font_map[c][2]
            glTexCoord2f(u0, 0)
            glVertex2f(x, y)
            glTexCoord2f(u1, 0)
            glVertex2f(x + w, y)
            glTexCoord2f(u1, 1)
            glVertex2f(x + w, y + h)
            glTexCoord2f(u0, 1)
            glVertex2f(x, y + h)
            x += w + 5
    glEnd()
   
    glutSwapBuffers()

def reshape(width, height):
    glViewport(0, 0, width, height)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    glOrtho(0, width, height, 0, -1, 1)
    glMatrixMode(GL_MODELVIEW)

glutInit(sys.argv)
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
glutInitWindowSize(400, 100)
glutCreateWindow('Hello World - FreeType')
glutDisplayFunc(display)
glutReshapeFunc(reshape)
load_font('font/FreeSans.ttf', 64)
glutMainLoop( )