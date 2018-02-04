import sys
import math
from enum import Enum

# Numpy improt [http://www.numpy.org/]
import numpy

# PyOpenGL import [http://pyopengl.sourceforge.net/]
from OpenGL.GL import *

# TODO numpy -> array [https://docs.python.org/3/library/array.html]
# TODO use double array


class PrintShaderCode(Enum):
    OFF = 1
    ON_ERROR = 2
    ALWAYS = 3

printShaderCode = PrintShaderCode.ON_ERROR

# shader program object
class ShaderProgram:
    def __init__( self, shaderList ):
        self.__printShader = printShaderCode
        shaderObjs = []
        for sh_info in shaderList: shaderObjs.append( self.CompileShader(sh_info[0], sh_info[1] ) )
        self.LinkProgram( shaderObjs )
        strLengthData = numpy.zeros(1, dtype=int)
        arraysizeData = numpy.zeros(1, dtype=int)
        typeData = numpy.zeros(1, dtype='uint32')
        nameData = numpy.chararray(glGetProgramiv( self.__prog, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH ) + 1)
        self.__attributeLocation = {}
        for i_attr in range(0, glGetProgramiv( self.__prog, GL_ACTIVE_ATTRIBUTES )):
            glGetActiveAttrib( self.__prog, i_attr, nameData.size-1, strLengthData, arraysizeData, typeData, nameData.data )
            name = nameData.tostring()[:strLengthData[0]]
            self.__attributeLocation[name] = glGetAttribLocation( self.__prog, name )
            print( "attribute  %-30s at loaction %d" % (name, self.__attributeLocation[name]) )
        nameData = numpy.chararray(glGetProgramiv( self.__prog, GL_ACTIVE_UNIFORM_MAX_LENGTH ) + 1)
        self.__unifomLocation = {}
        self.__unifomType = {}
        for i_attr in range(0, glGetProgramiv( self.__prog, GL_ACTIVE_UNIFORMS )):
            glGetActiveUniform( self.__prog, i_attr, nameData.size-1, strLengthData, arraysizeData, typeData, nameData.data )
            name = nameData.tostring()[:strLengthData[0]]
            self.__unifomLocation[name] = glGetUniformLocation( self.__prog, name ) 
            self.__unifomType[name] = numpy.copy(typeData)
            print( "uniform    %-30s at loaction %d type %s" % (name, self.__unifomLocation[name], shaderMaps.TypeName(self.__unifomType[name])))
        # glGetFragDataLocation( self.__prog, name )
        print('')
    
    
    def Prog( self ):
        return self.__prog
    def Use(self):
        glUseProgram( self.__prog )
    def SetUniforms( self, uniforms ):
        for name in uniforms: self.SetUniform( name, uniforms[name] )
    def Activate( self, uniforms ):
        self.Use()
        self.SetUniforms( uniforms )

    def UniformLocation( self, name ):
        return self.__unifomLocation[name]
    def SetUniformI1( self, name, val ):
        if name in self.__unifomLocation: glUniform1i( self.__unifomLocation[name], val )
    def SetUniformF1( self, name, arr ):
        if name in self.__unifomLocation: glUniform1fv( self.__unifomLocation[name], 1, numpy.array( arr, dtype="float" ) )
    def SetUniformF2( self, name, arr ):
        if name in self.__unifomLocation: glUniform2fv( self.__unifomLocation[name], 1, numpy.array( arr, dtype="float" ) )
    def SetUniformF3( self, name, arr ):
        if name in self.__unifomLocation: glUniform3fv( self.__unifomLocation[name], 1, numpy.array( arr, dtype="float" ) )
    def SetUniformF4( self, name, arr ):
        if name in self.__unifomLocation: glUniform4fv( self.__unifomLocation[name], 1, numpy.array( arr, dtype="float" ) )
    def SetUniformM33( self, name, mat ):
        if name in self.__unifomLocation: glUniformMatrix3fv( self.__unifomLocation[name], 1, GL_FALSE, mat )        
    def SetUniformM44( self, name, mat ):
        if name in self.__unifomLocation: glUniformMatrix4fv( self.__unifomLocation[name], 1, GL_FALSE, mat )
    def SetUniform( self, name, val ):
        if not name in self.__unifomType:
            return
        typeData = self.__unifomType[name]
        if typeData == GL_INT:
            self.SetUniformI1( name, val )
        elif typeData == GL_FLOAT:
            self.SetUniformF1( name, val )
        elif typeData == GL_FLOAT_VEC2:
            self.SetUniformF2( name, val )
        elif typeData == GL_FLOAT_VEC3:
            self.SetUniformF3( name, val )
        elif typeData == GL_FLOAT_VEC4:
            self.SetUniformF4( name, val )
        elif typeData == GL_FLOAT_MAT3:
            self.SetUniformM33( name, val )
        elif typeData == GL_FLOAT_MAT4:
            self.SetUniformM44( name, val )
        elif typeData == GL_SAMPLER_2D:
            self.SetUniformI1( name, val )
        
    # read shader program and compile shader
    def CompileShader(self, sourceFileName, shaderStage):
        with open( sourceFileName, 'r' ) as sourceFile:
            sourceCode = sourceFile.read()
        nameMap = { GL_COMPUTE_SHADER: 'compute', GL_VERTEX_SHADER: 'vertex', GL_TESS_CONTROL_SHADER: 'tessellation control', GL_TESS_EVALUATION_SHADER: 'tessellation evaluation', GL_GEOMETRY_SHADER: 'geometry', GL_FRAGMENT_SHADER: 'fragment' }    
        shaderObj = glCreateShader( shaderStage )
        glShaderSource( shaderObj, sourceCode )
        glCompileShader( shaderObj )
        result = glGetShaderiv( shaderObj, GL_COMPILE_STATUS )
        if self.__printShader == PrintShaderCode.ALWAYS or ((not result) and self.__printShader == PrintShaderCode.ON_ERROR):
            print( '\n%s shader code:' % nameMap.get( shaderStage, '' ) )
            print( sourceCode )
        if not (result):
            print( glGetShaderInfoLog( shaderObj ) )
            sys.exit()
        return shaderObj
    # linke shader objects to shader program
    def LinkProgram(self, shaderObjs):
        self.__prog = glCreateProgram()
        for shObj in shaderObjs: glAttachShader( self.__prog, shObj )
        glLinkProgram( self.__prog )
        result = glGetProgramiv( self.__prog, GL_LINK_STATUS )
        if not ( result ):
            print( 'link error:' )
            print( glGetProgramInfoLog( self.__prog ) )
            sys.exit()

class ShaderMaps:
    def __init__( self ):
        return

    def TypeName( self, typeData ): 
        if typeData == GL_INT:
            return 'int'
        elif typeData == GL_FLOAT:
            return 'float'
        elif typeData == GL_FLOAT_VEC2:
            return 'vec2'
        elif typeData == GL_FLOAT_VEC3:
            return 'vec3'
        elif typeData == GL_FLOAT_VEC4:
            return 'vec4'
        elif typeData == GL_FLOAT_MAT3:
            return 'mat3'
        elif typeData == GL_FLOAT_MAT4:
            return 'mat4'
        elif typeData == GL_SAMPLER_2D:
            return 'sampler2D'
        return str(typeData)

shaderMaps = ShaderMaps()