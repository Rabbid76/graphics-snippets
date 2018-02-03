import sys
import math
import enum
import warnings

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
                #glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, cx, cy, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, None )
                glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, cx, cy, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT, None )
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
        firstTex = 0
        if self.__depthStencilTexture > 0:
            firstTex = 1
            glActiveTexture( GL_TEXTURE0 + textureUnits[0] )
            glBindTexture( GL_TEXTURE_2D, self.__depthStencilTexture ) 
        for i_tex in range(0, min( len(textureUnits)-firstTex, len(self.__textures) )):
            glActiveTexture( GL_TEXTURE0 + textureUnits[i_tex+firstTex] )
            glBindTexture( GL_TEXTURE_2D, self.__textures[i_tex] )  
        glActiveTexture( GL_TEXTURE0 )  

# framebuffer attachment types
@enum.unique
class BufferType(enum.Enum):
    DEFAULT      =  0,
    COLOR1       =  1,
    COLOR2       =  2,
    COLOR3       =  3,
    COLOR4       =  4,
    DEPTH        = -1,
    STENCIL      = -2,
    DEPTHSTENCIL = -4

# frambuffer data format types
@enum.unique
class BufferDataType(enum.Enum):
    DEFAULT            =  0,
    UINT8              =  1,
    SNORM8             = 16,
    SNORM16            = 17,
    F16                = 32,
    F32                = 33,
    DEPTH24            = 48,
    DEPTH32            = 49,
    DEPTH_STENCIL_24_8 = 50

# depth test types
@enum.unique
class DepthTest(enum.Enum):
    OFF           = 0,
    LESS          = 1,
    LESS_OR_EQUAL = 2

# blending types
@enum.unique
class Blending(enum.Enum):
    OVERWRITE               = 0,
    MIX                     = 1, # dest * (1-alpha) + source * alpha
    MIX_PREMULTIPLIED_ALPHA = 2, # dest * (1-alpha) + source
    ADD                     = 3  # dest + source  


# buffer:
#     type        : `BufferType`
#     format      : `BufferDataFormat`
#     clear_color : clear color
#     scale       : buffer scale
#
# stage:
#
#     sources[]:
#         buffer ID : ID of the source buffer
#         binding   : binding point of the source buffer
#      
#     target[]:
#         buffer ID : ID of the target buffer
#         targets   : target location of the buffer -1 (depth/stencil), 0, 1, 2, ... , n
#
#     clear_targets[] : buffer IDs of the buffers which have to be cleared
#
#     depth_test : `DepthTest`
#
#     blending   : `Blending`
#
#
class RenderProcess:

    # ctor
    def __init__(self):
        self.__valid = False
        self.__complete = False
        self.__size = (0, 0)
        self.__buffers = {}
        self.__stages = {}
        self.__buffer_scale = {}
        self.__buffer_tex = {}
        self.__stage_scale = {}
        self.__stage_size = {}
        self.__stage_fb = {}
        
    # specify rendering process buffers
    def SpecifyBuffers(self, buffers):
        for buffer_id in buffers: self.SpecifyBuffer( buffer_id, buffers[buffer_id] )
    def SpecifyBuffer(self, buffer_id, buffer):
        self.__valid = self.__complete = False
        type, format, clear_color, scale = buffer
        self.__buffers[buffer_id] = buffer
    
    # specify rendering process stages
    def SpecifyStages(self, stage):
        for stage_id in stage: self.SpecifyStage( stage_id, stage[stage_id] )
    def SpecifyStage(self, stage_id, stage):
        self.__valid = self.__complete = False
        sources, targets, clear_targets, depth_test, blending = stage
        self.__stages[stage_id] = (sources, targets, clear_targets, depth_test, blending)

    # validate rendering process stages and buffers    
    def Validate(self):
        if self.__valid: return True
        self.__valid = True 
        
        for stage_id in self.__stages:
            stage = self.__stages[stage_id]
            sources, targets, clear_targets, depth_test, blending = stage

            # validate scale
            max_scale = 0
            min_scale = 100
            for buffer_id in targets:
                buffer = self.__buffers[buffer_id]
                type, format, clear_color, scale = buffer
                min_scale = max([min_scale, scale])
                max_scale = max([max_scale, scale])
            self.__stage_scale[stage_id] = min_scale if min_scale == max_scale else 1
            for buffer_id in targets: 
                self.__buffer_scale[buffer_id] = self.__stage_scale[stage_id]; 

        # validate format
        for buffer_id in self.__buffers: 
            type, format, clear_color, scale = self.__buffers[buffer_id]
            if ( type == BufferType.DEFAULT or
                 type == BufferType.COLOR1 or
                 type == BufferType.COLOR2 or
                 type == BufferType.COLOR3 or
                 type == BufferType.COLOR4 ):
                if ( format == BufferDataType.DEPTH24 or
                     format == BufferDataType.DEPTH32 or
                     format == BufferDataType.DEPTH_STENCIL_24_8 ):
                    warnings.warn( "illegal internal format", RuntimeWarning )
                    format = BufferDataType.DEFAULT
            elif type == BufferType.DEPTH:
                if (format != BufferDataType.DEPTH24 and format != BufferDataType.DEPTH32 and format != BufferDataType.DEFAULT ):
                    warnings.warn( "illegal internal format", RuntimeWarning )
                    format = BufferDataType.DEFAULT 
            else:
                warnings.warn( "not yet implemented", UserWarning )
                self.__valid = False
            self.__buffers[buffer_id] = (type, format, clear_color, scale)

        return self.__valid

    # get internal buffer format
    def InternalFormat(self, buffer_type, data_type):

        if buffer_type == BufferType.COLOR1:
            
            if   data_type == BufferDataType.DEFAULT: return GL_RED
            elif data_type == BufferDataType.UINT8:   return GL_R8 
            elif data_type == BufferDataType.SNORM8:  return GL_R8_SNORM 
            elif data_type == BufferDataType.SNORM16: return GL_R16_SNORM 
            elif data_type == BufferDataType.F16:     return GL_R16F 
            elif data_type == BufferDataType.F32:     return GL_R32F 

        elif buffer_type == BufferType.COLOR2:

            if   data_type == BufferDataType.DEFAULT: return GL_RG
            elif data_type == BufferDataType.UINT8:   return GL_RG8 
            elif data_type == BufferDataType.SNORM8:  return GL_RG8_SNORM 
            elif data_type == BufferDataType.SNORM16: return GL_RG16_SNORM 
            elif data_type == BufferDataType.F16:     return GL_RG16F 
            elif data_type == BufferDataType.F32:     return GL_RG32F  

        elif buffer_type == BufferType.COLOR3:

            if   data_type == BufferDataType.DEFAULT: return GL_RGB
            elif data_type == BufferDataType.UINT8:   return GL_RGB8 
            elif data_type == BufferDataType.SNORM8:  return GL_RGB8_SNORM 
            elif data_type == BufferDataType.SNORM16: return GL_RGB16_SNORM 
            elif data_type == BufferDataType.F16:     return GL_RGB16F 
            elif data_type == BufferDataType.F32:     return GL_RGB32F 

        elif (buffer_type == BufferType.COLOR4 or buffer_type == BufferType.DEFAULT):

            if   data_type == BufferDataType.DEFAULT: return GL_RGBA
            elif data_type == BufferDataType.UINT8:   return GL_RGBA8 
            elif data_type == BufferDataType.SNORM8:  return GL_RGBA8_SNORM 
            elif data_type == BufferDataType.SNORM16: return GL_RGBA16_SNORM 
            elif data_type == BufferDataType.F16:     return GL_RGBA16F 
            elif data_type == BufferDataType.F32:     return GL_RGBA32F 

        elif buffer_type == BufferType.DEPTH: 

            if   data_type == BufferDataType.DEFAULT: return GL_DEPTH_COMPONENT
            elif data_type == BufferDataType.DEPTH24: return GL_DEPTH_COMPONENT24 
            elif data_type == BufferDataType.DEPTH32: return GL_DEPTH_COMPONENT32

        warnings.warn( "not yet implemented", UserWarning )
        return 0

    # get buffer format
    def Format(self, buffer_type, data_type):

        if   buffer_type == BufferType.COLOR1:                                        return GL_RED
        elif buffer_type == BufferType.COLOR2:                                        return GL_RG
        elif buffer_type == BufferType.COLOR3:                                        return GL_RGB
        elif (buffer_type == BufferType.COLOR4 or buffer_type == BufferType.DEFAULT): return GL_RGBA
        elif buffer_type == BufferType.DEPTH:                                         return GL_DEPTH_COMPONENT

        warnings.warn( "not yet implemented", UserWarning )
        return 0

    # get buffer format
    def Type(self, buffer_type, data_type):

        if buffer_type == BufferType.DEPTH: return GL_FLOAT

        if   data_type == BufferDataType.DEFAULT: return GL_BYTE
        elif data_type == BufferDataType.UINT8:   return GL_BYTE 
        elif data_type == BufferDataType.SNORM8:  return GL_FLOAT 
        elif data_type == BufferDataType.SNORM16: return GL_FLOAT 
        elif data_type == BufferDataType.F16:     return GL_FLOAT 
        elif data_type == BufferDataType.F32:     return GL_FLOAT 

        warnings.warn( "not yet implemented", UserWarning )
        return 0
    
    # create frambuffers and frambuffer textures 
    def Create(self, size):
        if not self.__valid and not self.Validate():
            return False
        if self.__complete and size == self.__size:
            return True

        # set viewport size
        self.__size = size

        # create buffer textures
        for buffer_id in self.__buffers: 
            type, format, clear_color, scale = self.__buffers[buffer_id]

            cx             = self.__size[0] * self.__buffer_scale[buffer_id]
            cy             = self.__size[1] * self.__buffer_scale[buffer_id]
            internalFormat = self.InternalFormat( type, format ) 
            bufferFormat   = self.Format( type, format ) 
            bufferType     = self.Type( type, format ) 

            texObj = glGenTextures( 1 )
            glActiveTexture( GL_TEXTURE0 )
            glBindTexture( GL_TEXTURE_2D, texObj )
            glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, cx, cy, 0, bufferFormat, bufferType, None )
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST )
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST )
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE )
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE )
            self.__buffer_tex[buffer_id] = texObj 

        # create stage frambuffers
        for stage_id in self.__stages:
            stage = self.__stages[stage_id]
            sources, targets, clear_targets, depth_test, blending = stage

            # set size
            cx = self.__size[0] * self.__stage_scale[stage_id]
            cy = self.__size[1] * self.__stage_scale[stage_id]
            self.__stage_size[stage_id] = (cx, cy)

            # create frame buffer
            self.__stage_fb[stage_id] = glGenFramebuffers(1)
            glBindFramebuffer( GL_FRAMEBUFFER, self.__stage_fb[stage_id] )

            # add color attachments
            for buffer_id in targets:
                if targets[buffer_id] == - 1:
                    if depth_test != DepthTest.OFF:
                        if buffer_id in self.__buffer_tex: 
                            glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, self.__buffer_tex[buffer_id], 0 )
                        else:     
                            renderbuffer = glGenRenderbuffers(1)
                            glBindRenderbuffer( GL_RENDERBUFFER, renderbuffer )
                            glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, cx, cy )
                            glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer )
                elif buffer_id in self.__buffer_tex:
                    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + targets[buffer_id], GL_TEXTURE_2D, self.__buffer_tex[buffer_id], 0 )
                else:
                    type, format, clear_color, scale = self.__buffers[buffer_id]
                    internalFormat = self.InternalFormat( type, format ) 
                    renderbuffer = glGenRenderbuffers(1)
                    glBindRenderbuffer( GL_RENDERBUFFER, renderbuffer )
                    glRenderbufferStorage( GL_RENDERBUFFER, internalFormat, cx, cy )
                    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + targets[buffer_id], GL_RENDERBUFFER, renderbuffer )
             
        
        # $$$ TODO
        return self.__complete 

