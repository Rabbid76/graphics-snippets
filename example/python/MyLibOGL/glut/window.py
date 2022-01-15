import sys
import math
from time import time

# Numpy improt [http://www.numpy.org/]
import numpy

# PyOpenGL import [http://pyopengl.sourceforge.net/]
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *

# MyLibOGL import
from MyLibOGL.math import mat
from MyLibOGL.math import cam


# TODO numpy -> array [https://docs.python.org/3/library/array.html]
# TODO use double array


printExtensions = False

# glut window
class Window:
    def __init__( self, cx, cy, multisample=False ):
        
        self.__vpsize     = ( cx, cy )
        self.__glut_param = GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH
        if multisample == True:
            self.__glut_param = self.__glut_param | GLUT_MULTISAMPLE 
        
        self.__multisampllevel = 8
        if multisample == True:
            glutSetOption( GLUT_MULTISAMPLE, self.__multisampllevel ) 
        glutInitDisplayMode( self.__glut_param )
        glutInitWindowPosition( 0, 0 )
        glutInitWindowSize( self.__vpsize[0], self.__vpsize[1] )
        self.__id = glutCreateWindow( b'OGL window' ) 
        
        self.__multisample  = multisample
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
        if printExtensions:
            for i in range(self.__numOfExt):
                print( '    {}'.format( str(self.__extensions[i]) ) )
        print('')
        print( 'Mutisample: {0}'.format( 'on ({})'.format(self.__multisampllevel) if self.__multisample else 'off' ) ) 
        print('')
        print('')

        self.__orbit_mat         = mat.IdentityMat44() # persistent orbit orientation matrix
        self.__current_orbit_mat = mat.IdentityMat44() # additional orbit orientation while dragging
        self.__mouse_drag        = False               # draging on or off
        self.__mouse_drag_axis   = (1, 0, 0)           # current drag axis
        self.__mouse_drag_angle  = 0                   # current drag distance
        self.__mouse_drag_time   = 0                   # current drag time
        self.__mouse_start       = (0, 0)              # start of mouse dragging operation
        self.__auto_rotate       = True                # auto ratate on or of
        self.__auto_spin         = False               # auto spin
        self.__model_mat         = mat.IdentityMat44() # persistent model matrix
        self.__current_model_mat = mat.IdentityMat44() # current model matrix

        glutMouseFunc( self.__MouseButton__ )
        glutMotionFunc( self.__MouseMotion__ )
        glutPassiveMotionFunc( self.__MousePassiveMotion__ )
        glutDisplayFunc( self.__Draw__ ) 
        glutIdleFunc( self.__Draw__ )

    def OrbitMatrix( self ):
        if self.__mouse_drag or (self.__auto_rotate and self.__auto_spin):
            return mat.Multiply(self.__current_orbit_mat, self.__orbit_mat)
        return self.__orbit_mat

    def AutoModelMatrix( self ):
        if self.__auto_rotate:
            return mat.Multiply(self.__current_model_mat, self.__model_mat)
        return self.__model_mat
    
    def Run( self ):
        self.__startTime       = time()
        self.__currentTime     = self.__startTime 
        self.__dragStartTime   = self.__startTime 
        self.__rotateStartTime = self.__startTime 
        glutMainLoop()

    def __Draw__(self):
        self.__currentTime = time()
        self.__vpsize      = ( glutGet( GLUT_WINDOW_WIDTH ), glutGet( GLUT_WINDOW_HEIGHT ) )
        
        self.__current_model_mat = mat.IdentityMat44()
        if self.__mouse_drag:
            self.__current_orbit_mat = mat.Rotate(mat.IdentityMat44(), self.__mouse_drag_angle, self.__mouse_drag_axis)
        elif self.__auto_rotate:
            if self.__auto_spin:
                if self.__mouse_drag_time > 0:
                    angle = self.__mouse_drag_angle * (self.__currentTime - self.__rotateStartTime) / self.__mouse_drag_time
                    self.__current_orbit_mat = mat.Rotate(mat.IdentityMat44(), angle, self.__mouse_drag_axis)
            else:
                auto_angle_x = self.Fract( (self.__currentTime - self.__rotateStartTime) / 13.0 ) * 2.0 * math.pi
                auto_angle_y = self.Fract( (self.__currentTime - self.__rotateStartTime) / 17.0 ) * 2.0 * math.pi
                self.__current_model_mat = mat.RotateX( self.__current_model_mat, auto_angle_x )
                self.__current_model_mat = mat.RotateY( self.__current_model_mat, auto_angle_y )

        self._StartDraw_()
        self.OnDraw()
        self._EndDraw_()
        glutSwapBuffers()

    def __MouseButton__(self, button, state, x, y ):
        # button: GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, or GLUT_RIGHT_BUTTON
        # state: GLUT_UP or GLUT_DOWN
        if button == GLUT_LEFT_BUTTON:
            if state == GLUT_UP:
                # was dragfing
                self._ChangeMotionMode_( False, True, True )
            elif state == GLUT_DOWN:
                # start dragging
                self.__mouse_start = (x, y) 
                self._ChangeMotionMode_( True, False, False )
        if button == GLUT_RIGHT_BUTTON and state == GLUT_UP:
            self._ChangeMotionMode_( False, False, not self.__auto_rotate )
        return

    def __MouseMotion__(self, x, y ):
        dx, dy = ((x-self.__mouse_start[0]) / self.__vpsize[0], (self.__mouse_start[1]-y) / self.__vpsize[1])
        len = math.sqrt(dx*dx + dy*dy)
        if self.__mouse_drag and len > 0:
            # is draging
            self.__mouse_drag_angle = math.pi*len
            self.__mouse_drag_axis = (dy/len, 0, -dx/len)
            self.__mouse_drag_time = time() - self.__dragStartTime
        return

    def __MousePassiveMotion__(self, x, y ): 
        return   

    def _ChangeMotionMode_(self, drag, spin, auto ):
        
        new_drag = drag
        new_auto = auto if not new_drag else False
        new_spin = spin if new_auto else False
        change = self.__mouse_drag != new_drag or self.__auto_rotate != new_auto or self.__auto_spin != new_spin 
        if not change:
          return

        if new_drag and not self.__mouse_drag:
            self.__dragStartTime = time() 
            self.__mouse_drag_angle = 0
            self.__mouse_drag_time = 0
        if new_auto and not self.__auto_rotate:
            self.__rotateStartTime = time()
        
        self.__mouse_drag = new_drag 
        self.__auto_rotate = new_auto  
        self.__auto_spin = new_spin

        self.__orbit_mat = mat.Multiply(self.__current_orbit_mat, self.__orbit_mat)
        self.__current_orbit_mat = mat.IdentityMat44()
        self.__model_mat = mat.Multiply(self.__current_model_mat, self.__model_mat)
        self.__current_model_mat = mat.IdentityMat44()
        return

    def _StartDraw_(self): return
    def _EndDraw_(self): return

    def VPSize( self ):             return self.__vpsize
    def StartTime( self ):          return self.__startTime
    def CurrentTime( self ):        return self.__currentTime
    def Fract( self, val ):         return val - math.trunc(val)    
    def CalcAng( self, intervall ): return self.Fract( (self.__currentTime - self.__startTime) / intervall ) * 2.0 * math.pi

    # events
    def OnDraw(self): return
   

class CameraWindow(Window):
    def __init__( self, cx, cy, multisample=False ):
        super().__init__(cx, cy, multisample)
        self._camera = self._InitCamera_()

    def _InitCamera_(self): return cam.Camera()

    def _StartDraw_(self):
        super()._StartDraw_()
        self._camera.vp = self.VPSize()

    def Camera(self):      return self._camera
    def Perspective(self): return self._camera.Perspective()
    # TODO def Orthographic
    
    def LookAtNoOrbit(self): return self._camera.LookAt()
    def LookAt(self):        return mat.Multiply(self._camera.LookAt(), self.OrbitMatrix())
    