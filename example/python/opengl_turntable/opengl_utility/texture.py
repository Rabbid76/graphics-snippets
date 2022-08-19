from OpenGL.GL import *
from PIL import Image
import numpy
import sys

def read_texture(filename, texture_unit):
    # PIL can open BMP, EPS, FIG, IM, JPEG, MSP, PCX, PNG, PPM
    # and other file types.  We convert into a texture using GL.
    print('trying to open', filename)
    try:
        image = Image.open(filename)
    except IOError as ex:
        print('IOError: failed to open texture file ', filename)
        sys.exit()

    print('opened file: size=', image.size, 'format=', image.format)
    image_data = numpy.array(list(image.getdata()), numpy.uint8)
    image.close()

    texture_obj = glGenTextures(1)
    glActiveTexture(GL_TEXTURE0 + texture_unit)
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
    glBindTexture(GL_TEXTURE_2D, texture_obj)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.size[0], image.size[1],0, GL_RGB, GL_UNSIGNED_BYTE, image_data)

    return texture_obj

def create_texture_from_rgba_color(color, texture_unit):
    image_data = numpy.array([255 * v for v in color], numpy.uint8)
    texture_obj = glGenTextures(1)

    glActiveTexture(GL_TEXTURE0 + texture_unit)
    glBindTexture(GL_TEXTURE_2D, texture_obj)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data)

    return texture_obj