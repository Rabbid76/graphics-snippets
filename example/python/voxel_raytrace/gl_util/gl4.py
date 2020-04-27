# PyOpenGL import

import os
import ctypes

from OpenGL.GL import *
from OpenGL.GLUT import *

from OpenGL.GL.shaders import *


#
# OpenGL context base class
#
class GL_Context:

    __printExtensions = False

    def __init__(self, debugcontext):

        self.__debugcontext = debugcontext

        self.__vendor       = glGetString( GL_VENDOR )
        self.__renderer     = glGetString( GL_RENDERER )
        self.__version      = glGetString( GL_VERSION )
        self.__glsl_version = glGetString( GL_SHADING_LANGUAGE_VERSION )
        self.__major        = glGetInteger( GL_MAJOR_VERSION )
        self.__minor        = glGetInteger( GL_MINOR_VERSION )
        self.__numOfExt     = glGetInteger( GL_NUM_EXTENSIONS )
        self.__extensions   = []
        for i in range(self.__numOfExt):
            self.__extensions.append( glGetStringi( GL_EXTENSIONS, i ) )

        print( str(self.__vendor ) )
        print( str(self.__renderer ) )
        print( str(self.__vendor ) )
        print( str(self.__glsl_version ) )
        print('')
        print( 'OpenGL Version {}.{}'.format( self.__major, self.__minor ) )
        print('')
        print( '{} extensions'.format( self.__numOfExt ) )
        if self.__printExtensions:
            for i in range(self.__numOfExt):
                print( '    {}'.format( str(self.__extensions[i]) ) )
        print('')

        if self.__debugcontext:

            glDebugMessageCallback(self.__CB_OpenGL_DebugMessage, None)

            errors_only = False
            if errors_only:
                glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, None, GL_FALSE)
                glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, None, GL_TRUE)
            else:
                glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, None, GL_TRUE)

            glEnable(GL_DEBUG_OUTPUT)
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS)
            glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Starting debug messaging service")

    @GLDEBUGPROC
    def __CB_OpenGL_DebugMessage(source, type, id, severity, length, message, userParam):
        msg = message[0:length]
        print('debug:', msg)

#
# OpenGL program base class 
#
class GL_Program:

    __program = 0

    def __init__(self, program):
        self.__program = program

    def Object(self):
        return self.__program

    def Use(self):
        pass#glUseProgram(self.__program)

    def AttributeLocation(self, name):
        return glGetAttribLocation(self.__program, name)

    def UniformLocation(self, name):
        return glGetUniformLocation(self.__program, name)

    def AttributeLocations(self, names):
        return { a : self.AttributeLocation(a) for a in names }

    def UniformLocations(self, names):
        return { u : self.UniformLocation(u) for u in names }

#
# OpenGL GLSL progam
#
class GL_Program_GLSL(GL_Program):

    def __init__(self, vs, fs):

        vert_code = ""
        try:
            with open(vs, 'r') as vsFile:
                vert_code = vsFile.read()
        except:
            vert_code = vs

        frag_code = ""
        try:
            with open(fs, 'r') as vsFile:
                frag_code = vsFile.read()
        except:
            frag_code = fs

        sh_code_list = [(GL_VERTEX_SHADER, vert_code), (GL_FRAGMENT_SHADER, frag_code)]

        # OpenGL.GL.shaders
        # http://pyopengl.sourceforge.net/pydoc/OpenGL.GL.shaders.html
        #sh_objs = [compileShader(sh_code[1], sh_code[0]) for sh_code in sh_code_list]
        #program = compileProgram(vs_obj, fs_obj)

        sh_objs = []
        for sh_code in sh_code_list:
            sh_obj = glCreateShader(sh_code[0])
            glShaderSource(sh_obj, sh_code[1])
            glCompileShader(sh_obj)
            result = glGetShaderiv(sh_obj, GL_COMPILE_STATUS )
            #if verbose or not result:
            #    print( '\n%s shader code:' % nameMap.get( shaderStage, '' ) )
            #    print( sh_code[1] )
            if not result:
                print( glGetShaderInfoLog(sh_obj) )
            sh_objs.append(sh_obj)
        
        program = glCreateProgram()
        for shObj in sh_objs: glAttachShader(program, shObj)

        # programs has to be declare separable for the use with program pipeline - this is crucial!
        glProgramParameteri(program, GL_PROGRAM_SEPARABLE, GL_TRUE)

        glLinkProgram(program)
        if not glGetProgramiv(program, GL_LINK_STATUS):
            print( 'link error:' )
            print( glGetProgramInfoLog(program) )

        super().__init__(program)

#
# OpenGL Spir-V progam
#
class GL_Program_SpirV(GL_Program):

    def __init__(self, vs, fs):

        sh_objs = []
        sh_l = [(GL_VERTEX_SHADER, vs), (GL_FRAGMENT_SHADER, fs)]
        for sh_type, sh_file in sh_l:
            try:
                sh_code = None
                sh_size = 0
                with open(sh_file, 'rb') as vsFile:
                    sh_code = vsFile.read()
                    sh_size = os.stat(sh_file).st_size
                sh_obj = glCreateShader(sh_type)
                sho = (ctypes.c_uint32 * 1)(sh_obj)
                glShaderBinary(1, sho, GL_SHADER_BINARY_FORMAT_SPIR_V, sh_code, sh_size)
                glSpecializeShader(sh_obj, 'main', 0, None, None)
                result = glGetShaderiv(sh_obj, GL_COMPILE_STATUS)
                if not result:
                    print(glGetShaderInfoLog(sh_obj))
                sh_objs.append(sh_obj)
            except:
                pass

        program = glCreateProgram()
        for sh_obj in sh_objs: glAttachShader(program, sh_obj)

        # programs has to be declare separable for the use with program pipeline - this is crucial!
        glProgramParameteri(program, GL_PROGRAM_SEPARABLE, GL_TRUE)

        glLinkProgram(program)
        if not glGetProgramiv(program, GL_LINK_STATUS):
            print( 'link error:' )
            print( glGetProgramInfoLog(program) )
        for sh_obj in sh_objs: glDeleteShader(sh_obj)

        super().__init__(program)

#
# OpenGL mesh base class
#
class GL_Mesh:

    __vao = 0
    __no_of_vertices = 0
    __no_of_indices = 0

    def __init__(self, vao, no_vertices, no_indices, primitive = GL_TRIANGLES):
        self.__vao            = vao
        self.__no_of_vertices = no_vertices
        self.__no_of_indices  = no_indices
        self.__primitive      = primitive

    def Bind(self):
        if self.__no_of_vertices > 0:
            glBindVertexArray(self.__vao)

    def Draw(self):
        if self.__no_of_vertices > 0:
            glBindVertexArray(self.__vao)
            glDrawArrays(self.__primitive, 0, self.__no_of_vertices)
