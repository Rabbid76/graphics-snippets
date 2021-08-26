from OpenGL.GL import *
import numpy

def create_screenspace_vao():
    attributes = numpy.array([(-1,-1), (1,-1), (-1,1), (1,1)], dtype=numpy.float32) 
    vao = glGenVertexArrays(1)
    vbo = glGenBuffers(1)
    glBindVertexArray(vao)
    glBindBuffer(GL_ARRAY_BUFFER, vbo)
    glBufferData(GL_ARRAY_BUFFER, attributes, GL_STATIC_DRAW)
    glVertexAttribPointer(0, 2, GL_FLOAT, False, 2 * attributes.itemsize, None)
    glEnableVertexAttribArray(0)
    glBindVertexArray(0)
    glDeleteBuffers(1, [vbo])
    return vao

class FrameBuffer:
    def __init__(self, color_formats, depth_buffer, filter):
        self.color_formats = color_formats
        self.depth_buffer = depth_buffer
        self.filter = filter 
        self.fbo = 0
        self.color_textures = []
        self.depth_texture = 0

    def create(self, width, height):
        if not self.fbo:
            self.fbo = glGenFramebuffers(1)
        glBindFramebuffer(GL_FRAMEBUFFER, self.fbo)

        color_attachmnets = []
        for i, foramt_information in enumerate(self.color_formats):
            format, internal_format = foramt_information
            color_attachment = GL_COLOR_ATTACHMENT0 + i
            color_attachmnets.append(color_attachment)
            if len(self.color_textures) <= i:
                self.color_textures.append(glGenTextures(1))
            glBindTexture(GL_TEXTURE_2D, self.color_textures[i])
            glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, None);            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, self.filter)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, self.filter)
            glFramebufferTexture2D(GL_FRAMEBUFFER, color_attachment, GL_TEXTURE_2D, self.color_textures[i], 0)

        if self.depth_buffer:
            if not self.depth_texture:
                self.depth_texture = glGenTextures(1)
            glBindTexture(GL_TEXTURE_2D, self.depth_texture)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, None)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, self.filter)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, self.filter)       
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, self.depth_texture, 0)

        glDrawBuffers(len(color_attachmnets), color_attachmnets)
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER)
        if status != GL_FRAMEBUFFER_COMPLETE:
            print("frambuffer inclomplete")
        glBindFramebuffer(GL_FRAMEBUFFER, 0)