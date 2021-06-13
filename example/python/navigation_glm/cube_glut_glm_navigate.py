# How to implement camera pan like in 3dsMax?
# https://stackoverflow.com/questions/53758319/how-to-implement-camera-pan-like-in-3dsmax

# How to implement zoom towards mouse like in 3dsMax?
# https://stackoverflow.com/questions/54057549/how-to-implement-zoom-towards-mouse-like-in-3dsmax

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
import ctypes
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

    def __init__(self, vao, no_vertices, no_indices):
        self.__vao            = vao
        self.__no_of_vertices = no_vertices
        self.__no_of_indices  = no_indices

    def Bind(self):
        if self.__no_of_vertices > 0:
            glBindVertexArray(self.__vao)

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

        temp_vbo = np.empty(1, dtype=np.uint32)
        glCreateBuffers(len(temp_vbo), temp_vbo)
        self.__vbo = temp_vbo[0]

        temp_vao = np.empty(1, dtype=np.uint32)
        glCreateVertexArrays(len(temp_vao), temp_vao)
        vao = temp_vao[0]

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
# Navigation controller
#
class CNavigationController:

    OFF    = 0
    ORBIT  = 1
    ROTATE = 2

    def __init__(self, view_mat, proj_mat, view_rect, depth_val, pivot_pt):

        self.__id = id
        self.__get_view_rect = view_rect
        self.__get_view_mat  = view_mat
        self.__get_proj_mat  = proj_mat
        self.__get_depth_val = depth_val
        self.__get_pivot     = pivot_pt

        self.__pan = False
        self.__pan_start = glm.vec3(0, 0, 1)
        self.__orbit = self.OFF
        self.__orbit_start = glm.vec3(0, 0, 1)
        self.__pivot_world = glm.vec3(0, 0, 0)

    def ProjectionMat(self):
        proj = self.__get_proj_mat()
        return proj, glm.inverse(proj)

    def ViewMat(self):
        view = self.__get_view_mat()
        return view, glm.inverse(view)

    def WindowMat(self):
        vp_rect = self.VpRect()
        inv_wnd = glm.translate(glm.mat4(1), glm.vec3(-1, -1, -1))
        inv_wnd = glm.scale(inv_wnd, glm.vec3(2/vp_rect[2], 2/vp_rect[3], 2))
        inv_wnd = glm.translate(inv_wnd, glm.vec3(vp_rect[0], vp_rect[1], 0))
        return glm.inverse(inv_wnd), inv_wnd

    def VpRect(self):
        return self.__get_view_rect()

    def Depth(self, cursor_pos):
        return self.__get_depth_val(*cursor_pos)

    def PivotWorld(self, cursor_pos):
        return self.__get_pivot(*cursor_pos)

    def StartPan(self, cursor_pos):
        self.__pan = True
        self.__pan_start = glm.vec3(*cursor_pos, self.Depth(cursor_pos))
        
    def EndPan(self, cursor_pos):
        self.__pan = False

    def StartOrbit(self, cursor_pos, mode = ORBIT):
        self.__orbit = mode if mode >= self.ORBIT and mode <= self.ROTATE else self.ORBIT
        self.__orbit_start = glm.vec3(*cursor_pos, self.Depth(cursor_pos))
        self.__pivot_world = self.PivotWorld(cursor_pos)
        
    def EndOrbit(self, cursor_pos):
        self.__orbit = self.OFF

    def MoveOnLineOfSight(self, cursor_pos, delta):

        # get viewport rectangle
        #vp_rect = self.VpRect()
       
        # get view, projection and window matrix
        proj, inv_proj = self.ProjectionMat()
        view, inv_view = self.ViewMat()
        wnd,  inv_wnd  = self.WindowMat() 

        # get world space position on view ray
        pt_wnd     = glm.vec3(*cursor_pos, 1.0)
        #pt_world  = glm.unProject(pt_wnd, view, proj, vp_rect)
        pt_h_world = inv_view * inv_proj * inv_wnd * glm.vec4(*pt_wnd, 1)
        pt_world   = glm.vec3(pt_h_world) / pt_h_world.w
         
        # get view position
        eye  = glm.vec3(inv_view[3])
        
        # get "zoom" direction and amount
        ray_cursor = glm.normalize(pt_world - eye)
        
        # translate view position and update view matrix
        inv_view = glm.translate(glm.mat4(1), ray_cursor * delta) * inv_view
        
        # return new view matrix
        return glm.inverse(inv_view), True

        
    def MoveCursorTo(self, cursor_pos):
        
        view_changed = False 

        # get view matrix and  viewport rectangle
        view, inv_view = self.ViewMat()
        view_rect      = self.VpRect()

        if self.__pan:
 
            # get drag start and end
            wnd_from = self.__pan_start
            wnd_to   = glm.vec3(*cursor_pos, self.__pan_start[2])
            self.__pan_start = wnd_to

            # get projection and window matrix
            proj, inv_proj = self.ProjectionMat()
            wnd,  inv_wnd  = self.WindowMat() 

            # calculate drag start and world coordinates
            pt_h_world = [inv_view * inv_proj * inv_wnd * glm.vec4(*pt, 1) for pt in [wnd_from, wnd_to]]
            pt_world   = [glm.vec3(pt_h) / pt_h.w for pt_h in pt_h_world]
           
            # calculate drag world translation
            world_vec = pt_world[1] - pt_world[0]

            # translate view position and update view matrix
            inv_view     = glm.translate(glm.mat4(1), world_vec * -1) * inv_view
            view         = glm.inverse(inv_view)
            view_changed = True

        elif self.__orbit == self.ORBIT:

            # get the drag start and end
            wnd_from = self.__orbit_start
            wnd_to   = glm.vec3(*cursor_pos, self.__orbit_start[2])
            self.__orbit_start = wnd_to

            # calculate the pivot, rotation axis and angle
            pivot     = glm.vec3(view * glm.vec4(*self.__pivot_world, 1))
            orbit_dir = wnd_to - wnd_from 
            axis  = glm.vec3(-orbit_dir.y, orbit_dir.x, 0)
            angle = glm.length(glm.vec2(orbit_dir.x/(view_rect[2]-view_rect[0]), orbit_dir.y/(view_rect[3]-view_rect[1]))) * math.pi

            # calculate the rotation matrix and the rotation around the pivot 
            rot_mat   = glm.rotate(glm.mat4(1), angle, axis)
            rot_pivot = glm.translate(glm.mat4(1), pivot) * rot_mat * glm.translate(glm.mat4(1), -pivot)
            
            #transform and update view matrix
            view         = rot_pivot * view
            view_changed = True 

        elif self.__orbit == self.ROTATE:

            # get the drag start and end
            wnd_from = self.__orbit_start
            wnd_to   = glm.vec3(*cursor_pos, self.__orbit_start[2])
            self.__orbit_start = wnd_to

            # calculate the pivot, rotation axis and angle
            pivot_view   = glm.vec3(view * glm.vec4(*self.__pivot_world, 1))
            orbit_dir    = wnd_to - wnd_from 

            # get the projection of the up vector to the view port 
            # TODO

            # calculate the rotation components for the rotation around the view space x axis and the world up vector 
            orbit_dir_x  = glm.vec2(0, 1)
            orbit_vec_x  = glm.vec2(0, orbit_dir.y)
            orbit_dir_up = glm.vec2(1, 0)
            orbit_vec_up = glm.vec2(orbit_dir.x, 0)

            # calculate the rotation matrix around the view space x axis through the pivot
            rot_pivot_x = glm.mat4(1)
            if glm.length(orbit_vec_x) > 0.5: 
                axis_x      = glm.vec3(-1, 0, 0)
                angle_x     = glm.dot(orbit_dir_x, glm.vec2(orbit_vec_x.x/(view_rect[2]-view_rect[0]), orbit_vec_x.y/(view_rect[3]-view_rect[1]))) * math.pi
                rot_mat_x   = glm.rotate(glm.mat4(1), angle_x, axis_x)
                rot_pivot_x = glm.translate(glm.mat4(1), pivot_view) * rot_mat_x * glm.translate(glm.mat4(1), -pivot_view)
            
            # calculate the rotation matrix around the world space up vector through the pivot
            rot_pivot_up = glm.mat4(1)
            if glm.length(orbit_vec_up) > 0.5: 
                axis_up      = glm.vec3(0, 0, 1)
                angle_up     = glm.dot(orbit_dir_up, glm.vec2(orbit_vec_up.x/(view_rect[2]-view_rect[0]), orbit_vec_up.y/(view_rect[3]-view_rect[1]))) * math.pi
                rot_mat_up   = glm.rotate(glm.mat4(1), angle_up, axis_up)
                rot_pivot_up = glm.translate(glm.mat4(1), self.__pivot_world) * rot_mat_up * glm.translate(glm.mat4(1), -self.__pivot_world)
            
            #transform and update view matrix
            view         = rot_pivot_x * view * rot_pivot_up
            view_changed = True 

        # return the view matrix
        return view, view_changed
        
#
# GLUT window class
#
class GL_Window:

    def __init__(self, cx, cy, caption=b'OGL window', multisample=True):

        glutInit()

        self.__vp_size = ( cx, cy )
        self.__view    = glm.mat4(1)
        self.__proj    = glm.mat4(1)

        self.__navigate_control = CNavigationController(
            lambda : self.__view,
            lambda : self.__proj,
            lambda : glm.vec4(0, 0, *self.__vp_size),
            lambda x, y : self.__GetDepth(x, y),
            lambda _, __ : glm.vec3(0, 0, 0) )

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
        wnd_pos = (x, self.__vp_size[1]-y) 
        if button == GLUT_RIGHT_BUTTON and state == GLUT_DOWN:
            self.__navigate_control.StartPan(wnd_pos)
        elif button == GLUT_RIGHT_BUTTON and state == GLUT_UP:
            self.__navigate_control.EndPan(wnd_pos)
        if button == GLUT_LEFT_BUTTON and state == GLUT_DOWN:
            #self.__navigate_control.StartOrbit(wnd_pos, self.__navigate_control.ORBIT)
            self.__navigate_control.StartOrbit(wnd_pos, self.__navigate_control.ROTATE)
        elif button == GLUT_LEFT_BUTTON and state == GLUT_UP:
            self.__navigate_control.EndOrbit(wnd_pos)

    def __MouseMotion(self, x, y ):
        wnd_pos = (x, self.__vp_size[1]-y)
        self.__view, self.__update_view = self.__navigate_control.MoveCursorTo(wnd_pos) 

    def __MousePassiveMotion(self, x, y ): 
        pass

    def __MouseEntry(self, state):
        pass

    def __MouseWheel(self, wheel, direction, x, y):        
        wnd_pos = (x, self.__vp_size[1]-y) 
        self.__view, self.__update_view = self.__navigate_control.MoveOnLineOfSight(wnd_pos, direction)

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

    def __GetDepth(self, x, y):
        depth_buffer = glReadPixels(x, self.__vp_size[1]-y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT)    
        depth = float(depth_buffer[0][0])
        if depth == 1:
            pt_drag  = glm.vec3(0, 0, 0)
            clip_pos = self.__proj * self.__view * glm.vec4(pt_drag, 1)
            ndc_pos  = glm.vec3(clip_pos) / clip_pos.w
            if ndc_pos.z > -1 and ndc_pos.z < 1:
                depth = ndc_pos.z * 0.5 + 0.5
        return depth

    def _Init(self):

        # create cube mesh

        self.__cube = GL_MeshCube()

        # model view projection data shader storage block

        self.__model = glm.mat4(1)
        self.__view  = glm.lookAt(glm.vec3(0,-3,0), glm.vec3(0,0,0), glm.vec3(0,0,1))
        self.__proj  = glm.perspective(glm.radians(90), self.__vp_size[0]/self.__vp_size[1], 0.1, 100)

        buffer_data = np.zeros(3*16, dtype=np.float32)
        for i in range(4):
            for j in range(4):
                buffer_data

        temp_ssbo = np.empty(1, dtype=np.uint32)
        glCreateBuffers(len(temp_ssbo), temp_ssbo)
        self.__mvp_ssbo = temp_ssbo[0]
        dynamic_mvp_data = True
        code = 0 if not dynamic_mvp_data else GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT| GL_MAP_PERSISTENT_BIT
        
        buffer_data = np.empty([3, 4, 4], dtype=np.float32)
        buffer_data[0] = self.__model
        buffer_data[1] = self.__view
        buffer_data[2] = self.__proj
        buffer_size = buffer_data.size * buffer_data.itemsize
        
        glNamedBufferStorage(self.__mvp_ssbo, 3*16*SIZEOF_FLAOT32, buffer_data, code)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, self.__mvp_ssbo)

        #glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,                      glm.sizeof(glm.mat4), glm.value_ptr(self.__model)) 
        #glBufferSubData(GL_SHADER_STORAGE_BUFFER, 1*glm.sizeof(glm.mat4), glm.sizeof(glm.mat4), glm.value_ptr(self.__view)) 
        #glBufferSubData(GL_SHADER_STORAGE_BUFFER, 2*glm.sizeof(glm.mat4), glm.sizeof(glm.mat4), glm.value_ptr(self.__proj)) 

        self.__update_view = False

        # light data shader storage block

        light_data = [-1.0, -0.5, -2.0, 0.0, 0.2, 0.8, 0.8, 10.0]
        light_data_buffer = np.array( light_data, dtype=np.float32 )

        temp_ssbo = np.empty(1, dtype=np.uint32)
        glCreateBuffers(len(temp_ssbo), temp_ssbo)
        self.__light_ssbo = temp_ssbo[0]
        dynamic_light_data = True
        code = 0 if not dynamic_light_data else GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT| GL_MAP_PERSISTENT_BIT
        glNamedBufferStorage(self.__light_ssbo, light_data_buffer.nbytes, light_data_buffer, code)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, self.__light_ssbo)

        # set states

        glEnable(GL_DEPTH_TEST)
        self.__cube.Bind()

        # initialize (compile and link) shader program 

        # This should be done after all OpenGL states have been set,
        # because NVIDIA optimize the current program for the current states an would have to recompile it if the states would change.
        # https://www.opengl.org/discussion_boards/showthread.php/175944-NVidia-280-x-and-GeForce-4xx?p=1229120&viewfull=1#post1229120 

        if os.path.isfile(spv_vert_draw_file) and os.path.isfile(spv_frag_draw_file):
            self.__draw_prog = GL_Program_SpirV(spv_vert_draw_file, spv_frag_draw_file)
        else:
            self.__draw_prog = GL_Program_GLSL(glsl_vert_draw_file, glsl_frag_draw_file)

        # create pipeline for draw mesh
 
        self.__draw_pipeline = np.empty(1, dtype=np.uint32)
        glGenProgramPipelines(1, self.__draw_pipeline)
        glUseProgramStages(self.__draw_pipeline[0], GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, self.__draw_prog.Object())

        #al = ['inPos', 'inNV', 'inCol']
        #self.___attrib = self.__draw_prog.AttributeLocations(al)
        #print(self.___attrib)

        #ul = ['u_model', 'u_view', 'u_proj']
        #self.__uniform = self.__draw_prog.UniformLocations(ul)
        #print(self.__uniform)

        # activate program  and set states

        glBindProgramPipeline(self.__draw_pipeline[0])
        #self.__draw_prog.Use()

    def _Draw(self, time):

        self.__model = glm.mat4(1)

        angle1 = time * 2 * math.pi / 13
        angle2 = time * 2 * math.pi / 17
        #self.__model = glm.rotate(self.__model, angle1, glm.vec3(1, 0, 0) )
        #self.__model = glm.rotate(self.__model, angle2, glm.vec3(0, 0, 1) )

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, self.__mvp_ssbo )
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, glm.sizeof(glm.mat4), glm.value_ptr(self.__model)) 
        if self.__update_view:
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 1*glm.sizeof(glm.mat4), glm.sizeof(glm.mat4), glm.value_ptr(self.__view)) 

        glClearColor(0.3, 0.3, 0.3, 1.0)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        self.__cube.Draw()

#window = GL_Window(300, 200)
window = GL_Window(800, 600)
window.Run()
