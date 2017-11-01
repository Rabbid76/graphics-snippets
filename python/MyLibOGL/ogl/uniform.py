import sys
import math

# Numpy improt [http://www.numpy.org/]
import numpy

# PyOpenGL import [http://pyopengl.sourceforge.net/]
from OpenGL.GL import *

# TODO numpy -> array [https://docs.python.org/3/library/array.html]
# TODO use double array


# representation of a uniform block
class UniformBlock:
     def __init__(self, shaderProg, name):
        self.shaderProg = shaderProg 
        self.name = name
     def Link(self, bindingPoint):
        self.bindingPoint = bindingPoint
        self.noOfUniforms = glGetProgramiv(self.shaderProg, GL_ACTIVE_UNIFORMS)
        self.maxUniformNameLen = glGetProgramiv(self.shaderProg, GL_ACTIVE_UNIFORM_MAX_LENGTH)
        self.index = glGetUniformBlockIndex(self.shaderProg, self.name)
        intData = numpy.zeros(1, dtype=int)
        glGetActiveUniformBlockiv(self.shaderProg, self.index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, intData)
        self.count = intData[0]
        self.indices = numpy.zeros(self.count, dtype=int)
        glGetActiveUniformBlockiv(self.shaderProg, self.index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, self.indices)
        self.offsets = numpy.zeros(self.count, dtype=int)
        glGetActiveUniformsiv(self.shaderProg, self.count, self.indices, GL_UNIFORM_OFFSET, self.offsets)
        strLengthData = numpy.zeros(1, dtype=int)
        arraysizeData = numpy.zeros(1, dtype=int)
        typeData = numpy.zeros(1, dtype='uint32')
        nameData = numpy.chararray(self.maxUniformNameLen+1)
        self.namemap = {}
        self.dataSize = 0 
        for inx in range(0, len(self.indices)):
            glGetActiveUniform( self.shaderProg, self.indices[inx], self.maxUniformNameLen, strLengthData, arraysizeData, typeData, nameData.data )
            name = nameData.tostring()[:strLengthData[0]]
            self.namemap[name] = inx
            self.dataSize = max(self.dataSize, self.offsets[inx] + arraysizeData * 16) 
        glUniformBlockBinding(self.shaderProg, self.index, self.bindingPoint)
        print('\nuniform block %s size:%4d' % (self.name, self.dataSize))
        for uName in self.namemap:
            print( '    %-40s index:%2d    offset:%4d' % (uName, self.indices[self.namemap[uName]], self.offsets[self.namemap[uName]]) ) 

# representation of a uniform block buffer
class UniformBlockBuffer:
    def __init__(self, ub):
        self.namemap = ub.namemap
        self.offsets = ub.offsets
        self.bindingPoint = ub.bindingPoint
        self.object = glGenBuffers(1)
        self.dataSize = ub.dataSize
        glBindBuffer(GL_UNIFORM_BUFFER, self.object)
        dataArray = numpy.zeros(self.dataSize//4, dtype='float32')
        glBufferData(GL_UNIFORM_BUFFER, self.dataSize, dataArray, GL_DYNAMIC_DRAW)
    def BindToTarget(self):
        glBindBuffer(GL_UNIFORM_BUFFER, self.object)
        glBindBufferBase(GL_UNIFORM_BUFFER, self.bindingPoint, self.object)
    def BindDataFloat(self, name, dataArr):
        glBindBuffer(GL_UNIFORM_BUFFER, self.object)
        dataArray = numpy.array(dataArr, dtype='float32')
        glBufferSubData(GL_UNIFORM_BUFFER, self.offsets[self.namemap[name]], len(dataArr)*4, dataArray)