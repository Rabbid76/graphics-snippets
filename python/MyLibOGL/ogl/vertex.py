import sys
import math

# Numpy improt [http://www.numpy.org/]
import numpy

# PyOpenGL import [http://pyopengl.sourceforge.net/]
from OpenGL.GL import *

# TODO numpy -> array [https://docs.python.org/3/library/array.html]
# TODO use double array


# vertex array object
class VAObject:
    def __init__( self, dataArrays, indices = [], type = GL_TRIANGLES, patch_vertices = 3 ):
        self.__obj = glGenVertexArrays( 1 )
        self.__noOfIndices = len( indices )
        self.__indexArr = numpy.array( indices, dtype='uint' )
        self.__type = type
        self.__patch_vertices = patch_vertices
        self.__vertexSize = []
        self.__dataLength = []
        self.__noOfBuffers = len( dataArrays )
        self.__buffers = glGenBuffers( self.__noOfBuffers )
        glBindVertexArray( self.__obj )
        for i_buffer in range( 0, self.__noOfBuffers ):
            vertexSize, dataArr = dataArrays[i_buffer]
            self.__vertexSize.append( vertexSize )
            self.__dataLength.append( len( dataArr ) )
            glBindBuffer( GL_ARRAY_BUFFER, self.__buffers if self.__noOfBuffers == 1 else self.__buffers[i_buffer] )
            glBufferData( GL_ARRAY_BUFFER, numpy.array( dataArr, dtype='float32' ), GL_STATIC_DRAW )
            glEnableVertexAttribArray( i_buffer )
            glVertexAttribPointer( i_buffer, self.__vertexSize[i_buffer], GL_FLOAT, GL_FALSE, 0, None )
        self.__iBuffer = glGenBuffers( 1 )
        glBindBuffer( GL_ARRAY_BUFFER, 0 )
        if self.__noOfIndices > 0:
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self.__iBuffer )
            glBufferData( GL_ELEMENT_ARRAY_BUFFER, self.__indexArr, GL_STATIC_DRAW )
        glBindVertexArray( 0 )
    def DrawArray(self):
        glBindVertexArray( self.__obj )
        if self.__type == GL_PATCHES:
            glPatchParameteri( GL_PATCH_VERTICES, self.__patch_vertices )
        glDrawArrays( self.__type, 0, self.__dataLength[0] )
        glBindVertexArray( 0 )
    def Draw(self):
        if self.__noOfIndices == 0:
            self.DrawArray()
            return
        glBindVertexArray( self.__obj )
        #for i_buffer in range( 0, self.__noOfBuffers ):
        #    glBindBuffer( GL_ARRAY_BUFFER, self.__buffers if self.__noOfBuffers == 1 else self.__buffers[i_buffer] )
        #    glEnableVertexAttribArray( i_buffer )
        #    glVertexAttribPointer( i_buffer, self.__vertexSize[i_buffer], GL_FLOAT, GL_FALSE, 0, None )
        #glDrawElements( self.__type, self.__noOfIndices, GL_UNSIGNED_INT, self.__indexArr )
        if self.__type == GL_PATCHES:
            glPatchParameteri( GL_PATCH_VERTICES, self.__patch_vertices )
        glDrawElements( self.__type, self.__noOfIndices, GL_UNSIGNED_INT, None )
        glBindVertexArray( 0 )