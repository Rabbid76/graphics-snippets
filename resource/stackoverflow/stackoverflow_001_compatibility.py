# https://stackoverflow.com/questions/47119517/opengl-strange-shadows-on-a-simple-cube-like-object
# Stackoveflow question: OpenGL strange shadows on a simple cube-like object

from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
import numpy
from time import time
import math
import sys

SCREEN_SIZE = (800, 600)

def resize(width, height):
    glViewport(0, 0, width, height)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(45.0, float(width) / height, 0.001, 10.0)
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()
    gluLookAt(0.0, 1.0, -5.0,
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0)

# glut window
class Window:
    def __init__( self, cx, cy ):
        self.__vpsize = ( cx, cy )
        glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH )
        glutInitWindowPosition( 0, 0 )
        glutInitWindowSize( self.__vpsize[0], self.__vpsize[1] )
        self.__id = glutCreateWindow( b'OGL window' ) 
        glutDisplayFunc( self.OnDraw ) 
        glutIdleFunc( self.OnDraw )

        self.num_faces = 6
        self.vertices = [ (-1.0, -0.25,  0.5),
                 ( 1.0, -0.25,  0.5),
                 ( 1.0, -0.25, -0.5),
                 (-1.0, -0.25, -0.5), # bottom y<0
                 (-1.0,  0.25,  0.5),
                 ( 1.0,  0.25,  0.5),
                 ( 1.0,  0.25, -0.5),
                 (-1.0,  0.25,- 0.5) ] # top y>0
        self.normals = [ ( 0.0, -1.0,  0.0),  # bottom
                ( 0.0, +1.0,  0.0),  # top
                (-1.0,  0.0,  0.0),  # left
                (+1.0,  0.0,  0.0),  # right
                ( 0.0,  0.0, -1.0),  # back
                ( 0.0,  0.0, +1.0) ] # front
        self.vertex_indices = [ (0, 1, 2, 3),  # bottom
                       (4, 5, 6, 7),  # top
                       (0, 3, 7, 4),  # left
                       (1, 2, 6, 5),  # right
                       (2, 6, 7, 3),  # back
                       (0, 1, 5, 4) ]  # fron
    
    def Run( self ):
        self.__startTime = time()
        glutMainLoop()

    # draw event
    def OnDraw(self):
        self.__vpsize = ( glutGet( GLUT_WINDOW_WIDTH ), glutGet( GLUT_WINDOW_HEIGHT ) )
        currentTime = time()

        width = self.__vpsize[0]
        height = self.__vpsize[1]
        
        glViewport(0, 0, width, height)
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        gluPerspective(45.0, float(width) / height, 0.001, 10.0)
        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()
        gluLookAt(0.0, 1.0, -5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0)
        
        # glEnable(GL_BLEND)
        # glEnable(GL_POLYGON_SMOOTH)
        # glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST)
        
        #glShadeModel(GL_FLAT)
        #glShadeModel(GL_SMOOTH)
        #glEnable(GL_COLOR_MATERIAL)
        #glEnable(GL_LIGHTING)   # this one
        #glEnable(GL_LIGHT0)
        #glLightfv(GL_LIGHT0, GL_AMBIENT, (0.3, 0.3, 0.3, 1.0))

        glEnable(GL_DEPTH_TEST)
        # The following don't seem to have any effect, so I've commented out
        # glClearColor(0.0, 0.0, 0.0, 0.0)
        # glShadeModel(GL_SMOOTH)
        # glEnable(GL_BLEND)
        # glEnable(GL_POLYGON_SMOOTH)
        # glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST)
        glEnable(GL_COLOR_MATERIAL)
        glCreateShader(GL_VERTEX_SHADER)
        glEnable(GL_LIGHTING)
        glEnable(GL_LIGHT0)
        glLightfv(GL_LIGHT0, GL_AMBIENT, (0.3, 0.3, 0.3, 1.0))
        
        glEnable( GL_DEPTH_TEST )
        glClearColor(0.0, 0.0, 0.0, 0.0)
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT )

        glPushMatrix()
        
        x_angle = -30.0
        y_angle =  60.0
        #x_angle = self.CalcAng( currentTime, 13.0 ) * 180.0 / math.pi
        #y_angle = self.CalcAng( currentTime, 17.0 ) * 180.0 / math.pi
        glRotate(x_angle, 1, 0, 0)
        glRotate(y_angle, 0, 1, 0)
        
        glColor([0.4,0.2,0.8,1.0])

        vertices = self.vertices
    
        glutSolidCube(1.0)

        #glBegin(GL_QUADS)
        #for face_no in range(0,self.num_faces):
        #    glNormal3dv(self.normals[face_no])
        #    v1, v2, v3, v4 = self.vertex_indices[face_no]
        #    glVertex(vertices[v1])
        #    glVertex(vertices[v2])
        #    glVertex(vertices[v3])
        #    glVertex(vertices[v4])
        #glEnd()

        glPopMatrix()
    
        glutSwapBuffers()

    def Fract( self, val ): return val - math.trunc(val)
    def CalcAng( self, currentTime, intervall ): return self.Fract( (currentTime - self.__startTime) / intervall ) * 2.0 * math.pi
    def CalcMove( self, currentTime, intervall, range ):
        pos = self.Fract( (currentTime - self.__startTime) / intervall ) * 2.0
        pos = pos if pos < 1.0 else (2.0-pos)
        return range[0] + (range[1] - range[0]) * pos

# initialize glut
glutInit()

# create window
wnd = Window( 800, 600 )

# start main loop
wnd.Run()
