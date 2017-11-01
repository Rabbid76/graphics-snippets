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

        glutDisplayFunc( self.__Draw__ ) 
        glutIdleFunc( self.__Draw__ )
    
    def Run( self ):
        self.__startTime   = time()
        self.__currentTime = self.__startTime 
        glutMainLoop()

    def __Draw__(self):
        self.__currentTime = time()
        self.__vpsize      = ( glutGet( GLUT_WINDOW_WIDTH ), glutGet( GLUT_WINDOW_HEIGHT ) )
        self._StartDraw_()
        self.OnDraw()
        self._EndDraw_()
        glutSwapBuffers()

    def _StartDraw_(self): return
    def _EndDraw_(self): return

    def VPSize( self ):             return self.__vpsize
    def StartTime( self ):          return self.__startTime
    def CurrentTime( self ):        return self.__currentTime
    def Fract( self, val ):         return val - math.trunc(val)
    def CalcAng( self, intervall ): return self.Fract( (self.__currentTime  - self.__startTime) / intervall ) * 2.0 * math.pi

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
    def LookAt(self):      return self._camera.LookAt()
    