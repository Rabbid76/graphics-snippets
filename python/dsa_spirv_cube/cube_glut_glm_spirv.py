import os
import sys
currentWDir = os.getcwd()
print( 'current working directory: {}'.format( str(currentWDir) ) )
fileDir = os.path.dirname(os.path.abspath(__file__)) # det the directory of this file
print( 'current location of self: {}'.format( str(fileDir) ) )
parentDir = os.path.abspath(os.path.join(fileDir, os.pardir)) # get the parent directory of this file
sys.path.insert(0, parentDir)
print( 'insert system directory: {}'.format( str(parentDir) ) )
os.chdir( fileDir )
baseWDir = os.getcwd()
print( 'changed current working directory: {}'.format( str(baseWDir) ) )
print ( '' )

import math
from time import time
import numpy as np

# PyGLM
import glm

# PyOpenGL import
from OpenGL.GL import *
from OpenGL.GLUT import *

from OpenGL.GL.shaders import *

from subprocess import call
from shutil import which

SIZEOF_FLAOT32 = np.finfo(np.float32).bits // 8

#$(SolutionDir)..\$(VulkanDir)Bin32\glslangValidator.exe -V $(ProjectDir)shaders\shader_1.vert -o $(ProjectDir)shaders\shader_1_vert.spv
#$(SolutionDir)..\$(VulkanDir)Bin32\glslangValidator.exe -V $(ProjectDir)shaders\shader_1.frag -o $(ProjectDir)shaders\shader_1_frag.spv

glsl_vert_draw_file = 'cube_glut_glm_draw.vert'
glsl_frag_draw_file = 'cube_glut_glm_draw.frag'
spv_vert_draw_file = 'cube_glut_glm_draw_vert.spv'
spv_frag_draw_file = 'cube_glut_glm_draw_frag.spv'

fileConvert = [(glsl_vert_draw_file, spv_vert_draw_file), (glsl_frag_draw_file, spv_frag_draw_file)]
glslVaidator = which('glslangValidator.exe')

if glslVaidator != None:
    print('Create Spire-V shader program')
    for glslFile, spvFile in fileConvert:
        glslPath = str(baseWDir) + '\\' + glslFile
        spvPath  = str(baseWDir) + '\\' + spvFile
        vspir_vert_cmd = '"' + glslVaidator + '" -V "' + glslPath + '" -o "' + spvPath + '"'
        call( vspir_vert_cmd )


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

    def Use(self):
        glUseProgram(self.__program)

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

        # OpenGL.GL.shaders
        # http://pyopengl.sourceforge.net/pydoc/OpenGL.GL.shaders.html
        program = compileProgram( 
            compileShader( vert_code, GL_VERTEX_SHADER ),
            compileShader( frag_code, GL_FRAGMENT_SHADER ),
        )

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
                sho = np.array([sh_obj], dtype=np.uint32)
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

    def __init__(self, vao, no_vertices, no_indices):
        self.__vao            = vao
        self.__no_of_vertices = no_vertices
        self.__no_of_indices  = no_indices

    def Draw(self):
        if self.__no_of_vertices > 0:
            glBindVertexArray(self.__vao)
            glDrawArrays(GL_TRIANGLES, 0, self.__no_of_vertices)

#
# OpenGL cube mesh
#
class GL_MeshCube(GL_Mesh):

    def __init__(self):

        v = [ -1,-1,1,  1,-1,1,  1,1,1, -1,1,1, -1,-1,-1,  1,-1,-1,  1,1,-1, -1,1,-1 ]
        c = [ 1.0, 0.0, 0.0,   1.0, 0.5, 0.0,    1.0, 0.0, 1.0,   1.0, 1.0, 0.0,   0.0, 1.0, 0.0,   0.0, 0.0, 1.0 ]
        n = [ 0,0,1, 1,0,0, 0,0,-1, -1,0,0, 0,1,0, 0,-1,0 ]
        e = [ 0,1,2,3, 1,5,6,2, 5,4,7,6, 4,0,3,7, 3,2,6,7, 1,0,4,5 ]
        attr_array = []
        for si, vi, ci in [(si, vi, [0, 1, 2, 0, 2, 3][vi]) for si in range(6) for vi in range(6)]:
            i = si*4+ci
            attr_array.extend( [ v[e[i]*3], v[e[i]*3+1], v[e[i]*3+2] ] )
            attr_array.extend( [ n[si*3], n[si*3+1], n[si*3+2] ] )
            attr_array.extend( [ c[si*3], c[si*3+1], c[si*3+2], 1 ] ); 
        no_vert = len(attr_array) // 10

        vertex_attributes = np.array(attr_array, dtype=np.float32)

        # See PyOpenGL 4.5 Cheatsheet
        # https://github.com/henkeldi/opengl_cheatsheet

        self.__vbo = np.empty(1, dtype=np.uint32)
        glCreateBuffers(len(self.__vbo), self.__vbo)

        vao = np.empty(1, dtype=np.uint32)
        glCreateVertexArrays(len(vao), vao)

        dynamic_buffer = False
        code = 0 if not dynamic_buffer else GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT| GL_MAP_PERSISTENT_BIT
        glNamedBufferStorage(self.__vbo, vertex_attributes.nbytes, vertex_attributes, code)
        glVertexArrayVertexBuffer(vao, 0, self.__vbo, 0, 10*vertex_attributes.itemsize)

        vertex_spec = [
            (0, 3, GL_FLOAT, False, 0),
            (1, 3, GL_FLOAT, False, 3*vertex_attributes.itemsize),
            (2, 4, GL_FLOAT, False, 6*vertex_attributes.itemsize)
        ]
        for vspec in vertex_spec:
            glVertexArrayAttribFormat(vao, *vspec)
            glVertexArrayAttribBinding(vao, vspec[0], 0)
            glEnableVertexArrayAttrib(vao, vspec[0])

        super().__init__(vao, no_vert, 0)

#
# GLUT window class
#
class GL_Window:

    def __init__(self, cx, cy, caption=b'OGL window', multisample=True):

        glutInit()

        self.__vp_size    = ( cx, cy )

        self.__glut_param = GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH
        if multisample == True:
            self.__glut_param = self.__glut_param | GLUT_MULTISAMPLE 
        self.__multisample_level = 8
        if multisample == True:
            glutSetOption( GLUT_MULTISAMPLE, self.__multisample_level )

        glutInitContextVersion( 3, 3 )
        glutInitContextProfile( GLUT_CORE_PROFILE )
        glutInitDisplayMode( self.__glut_param )

        glutInitWindowPosition( 0, 0 )
        glutInitWindowSize( self.__vp_size[0], self.__vp_size[1] )
        self.__id = glutCreateWindow( caption )

        self.__multisample  = multisample
        self.__context_info = GL_Context(True)

        print( 'Mutisample: {0}'.format( 'on ({})'.format(self.__multisample_level) if self.__multisample else 'off' ) )
        print('')
        print('')

        self._Init()

        glutIdleFunc(self.__Idle)

        glutReshapeFunc(self.__Resize)

        glutKeyboardFunc(self.__Keyboard)
        glutSpecialFunc(self.__SpecialKey)

        glutMouseFunc(self.__MouseButton)
        glutMotionFunc(self.__MouseMotion)
        glutPassiveMotionFunc(self.__MousePassiveMotion)
        glutEntryFunc(self.__MouseEntry)
        glutMouseWheelFunc(self.__MouseWheel)

        glutJoystickFunc(self.__Joystick)

        glutDisplayFunc(self.__Display)
        glutOverlayDisplayFunc(self.__OverlayDisplay)

    def Run(self):
        self.__vp_valid     = False
        self.__start_time   = time()
        self.__current_time = self.__start_time
        glutMainLoop()

    def __Idle(self):
        pass

    def __Resize(self, x, y):
        self.__vp_valid = False

    def __Keyboard(self, key, x, y):
        pass

    def __SpecialKey(self, key, x, y):
        pass

    def __MouseButton(self, button, state, x, y ):
        pass

    def __MouseMotion(self, x, y ):
        pass

    def __MousePassiveMotion(self, x, y ): 
        pass

    def __MouseEntry(self, state):
        pass

    def __MouseWheel(self, wheel, direction, x, y):
        pass

    def __Joystick(self, buttons, xaxis, yaxis, zaxis):
        pass

    def __Display(self):
        self.__current_time = time()
        self.__time = self.__current_time - self.__start_time
        if self.__vp_valid == False:
            self.__vp_size  = ( glutGet( GLUT_WINDOW_WIDTH ), glutGet( GLUT_WINDOW_HEIGHT ) )
            self.__vp_valid = True
            glViewport(0, 0, *self.__vp_size)

        self._Draw(self.__time)
        glutSwapBuffers()
        glutPostRedisplay()

    def __OverlayDisplay(self):
        pass

    def _Init(self):

        # initialize shader program 

        if os.path.isfile(spv_vert_draw_file) and os.path.isfile(spv_frag_draw_file):
            self.__draw_prog = GL_Program_SpirV(spv_vert_draw_file, spv_frag_draw_file)
        else:
            self.__draw_prog = GL_Program_GLSL(glsl_vert_draw_file, glsl_frag_draw_file)

        #al = ['inPos', 'inNV', 'inCol']
        #self.___attrib = self.__draw_prog.AttributeLocations(al)
        #print(self.___attrib)

        #ul = ['u_model', 'u_view', 'u_proj']
        #self.__uniform = self.__draw_prog.UniformLocations(ul)
        #print(self.__uniform)

        # create cube mesh

        self.__cube = GL_MeshCube()

        # mpdel view projetion data shader storage block

        self.__model = glm.mat4(1)
        self.__view  = glm.lookAt(glm.vec3(0,-3,0), glm.vec3(0,0,0), glm.vec3(0,0,1))
        self.__proj  = glm.perspective(glm.radians(90), self.__vp_size[0]/self.__vp_size[1], 0.1, 100)

        buffer_data = np.zeros(3*16, dtype=np.float32)
        for i in range(4):
            for j in range(4):
                buffer_data

        self.__mvp_ssbo = np.empty(1, dtype=np.uint32)
        glCreateBuffers(len(self.__mvp_ssbo), self.__mvp_ssbo)
        dynamic_mvp_data = True
        code = 0 if not dynamic_mvp_data else GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT| GL_MAP_PERSISTENT_BIT
        glNamedBufferStorage(self.__mvp_ssbo, 3*16*SIZEOF_FLAOT32, None, code)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, self.__mvp_ssbo)

        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,                      glm.sizeof(glm.mat4), glm.value_ptr(self.__model)) 
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 1*glm.sizeof(glm.mat4), glm.sizeof(glm.mat4), glm.value_ptr(self.__view)) 
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 2*glm.sizeof(glm.mat4), glm.sizeof(glm.mat4), glm.value_ptr(self.__proj)) 

        # light data shader storage block

        light_data = [-1.0, -0.5, -2.0, 0.0, 0.2, 0.8, 0.8, 10.0]
        light_data_buffer = np.array( light_data, dtype=np.float32 )

        self.__light_ssbo = np.empty(1, dtype=np.uint32)
        glCreateBuffers(len(self.__light_ssbo), self.__light_ssbo)
        dynamic_light_data = True
        code = 0 if not dynamic_light_data else GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT| GL_MAP_PERSISTENT_BIT
        glNamedBufferStorage(self.__light_ssbo, light_data_buffer.nbytes, light_data_buffer, code)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, self.__light_ssbo)

        # activate program  and set states

        glEnable(GL_DEPTH_TEST)
        self.__draw_prog.Use()

    def _Draw(self, time):

        self.__model = glm.mat4(1)

        angle1 = time * 2 * math.pi / 13
        angle2 = time * 2 * math.pi / 17
        self.__model = glm.rotate(self.__model, angle1, glm.vec3(1, 0, 0) )
        self.__model = glm.rotate(self.__model, angle2, glm.vec3(0, 0, 1) )

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, self.__mvp_ssbo )
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, glm.sizeof(glm.mat4), glm.value_ptr(self.__model)) 

        glClearColor(0.3, 0.3, 0.3, 1.0)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        self.__cube.Draw()

window = GL_Window(800, 600)
window.Run()