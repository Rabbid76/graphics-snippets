import sys
import math

# Numpy improt [http://www.numpy.org/]
import numpy

# PyOpenGL import [http://pyopengl.sourceforge.net/]
from OpenGL.GL import *

# TODO numpy -> array [https://docs.python.org/3/library/array.html]
# TODO use double array


# framebuffer
class FrameBuffer:
    def  __init__( self, cx, cy, attachmnets, depthAttachment=True, stencilAttachment=False, depthStencilTexture=False ):
        self.__size = (cx, cy)
        
        glActiveTexture( GL_TEXTURE0 )
        
        # depth and stencil texture
        self.__depthStencilTexture = 0
        if depthStencilTexture and (depthAttachment or stencilAttachment):
            self.__depthStencilTexture = glGenTextures( 1 )
            glBindTexture( GL_TEXTURE_2D, self.__depthStencilTexture )
            if depthAttachment and stencilAttachment:
                #internal formats: GL_DEPTH24_STENCIL8, GL_DEPTH32F_STENCIL8
                #glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, cx, cy, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, None )
                glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, cx, cy, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, None )
                #glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, cx, cy, 0, GL_DEPTH_STENCIL, 34042, None )
            elif depthAttachment:
                glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, cx, cy, 0, GL_DEPTH_COMPONENT, GL_FLOAT, None )
            else:
                glTexImage2D( GL_TEXTURE_2D, 0, GL_STENCIL_INDEX8, cx, cy, 0, GL_STENCIL_INDEX, GL_BYTE, None )
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST )
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST )
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP )
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP )
            if depthAttachment:
                glTexParameteri( GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY )
        
        # color attackment textures
        self.__textures = []
        for attachtype in attachmnets:
            texObj = glGenTextures( 1 )
            glBindTexture( GL_TEXTURE_2D, texObj )
            glTexImage2D( GL_TEXTURE_2D, 0, attachtype[0], cx, cy, 0, attachtype[1], GL_UNSIGNED_BYTE, None )
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST )
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST )
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE )
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE )
            self.__textures.append(texObj)

        # create an bind frame buffer
        self.__fbObj = glGenFramebuffers(1)
        glBindFramebuffer( GL_FRAMEBUFFER, self.__fbObj )

        # attachment depth and/or stencil buffer
        if depthAttachment or stencilAttachment:
            if depthStencilTexture:
                if depthAttachment and stencilAttachment:
                    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, self.__depthStencilTexture, 0 )
                elif depthAttachment:
                    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, self.__depthStencilTexture, 0 )
                else:
                    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT , GL_TEXTURE_2D, self.__depthStencilTexture, 0 )
            else:
                renderbuffer = glGenRenderbuffers(1)
                glBindRenderbuffer( GL_RENDERBUFFER, renderbuffer )
                if depthAttachment and stencilAttachment:
                    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, cx, cy )
                    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer )
                elif depthAttachment:
                    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, cx, cy )
                    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer )
                else:
                    glRenderbufferStorage( GL_RENDERBUFFER, GL_STENCIL_INDEX8, cx, cy )
                    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer )

        # attach color buffers        
        for i_tex in range(0, len(self.__textures)):
           glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i_tex, GL_TEXTURE_2D, self.__textures[i_tex], 0 )
        
        # unbind an check for completeness
        glBindTexture( GL_TEXTURE_2D, 0 )
        glBindRenderbuffer( GL_RENDERBUFFER, 0 )
        frameBufferStatus = glCheckFramebufferStatus( GL_FRAMEBUFFER )
        if frameBufferStatus != GL_FRAMEBUFFER_COMPLETE:
            print( 'frame buffer incomplete: {}'.format( frameBufferStatus ) )
            sys.exit()
        glBindFramebuffer( GL_FRAMEBUFFER, 0 ) 

    def Bind( self ):
        glBindFramebuffer( GL_FRAMEBUFFER, self.__fbObj )
    def UnBind( self ):
        glBindFramebuffer( GL_FRAMEBUFFER, 0 )
    def BindTextures( self, textureUnits ):
        for i_tex in range(0, min( len(textureUnits), len(self.__textures) )):
            glActiveTexture( GL_TEXTURE0 + textureUnits[i_tex] )
            glBindTexture( GL_TEXTURE_2D, self.__textures[i_tex] )  
        glActiveTexture( GL_TEXTURE0 )  