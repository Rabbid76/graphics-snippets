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
import numpy

# PyOpenGL import
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *

# MyLibOGL import
from MyLibOGL.math import mat
from MyLibOGL.math import cam
from MyLibOGL.ogl import shader
from MyLibOGL.ogl import vertex
from MyLibOGL.ogl import uniform
from MyLibOGL.glut import window


class MyWindow(window.CameraWindow):
    def __init__( self, cx, cy, multisample=True ):
        super().__init__(cx, cy, multisample)

    def _InitCamera_(self):
        camera = super()._InitCamera_()
        #camera.fov_y = 120 
        camera.pos = (0, -1.8, 0)
        return camera    

    # draw event
    def OnDraw(self):
        # set up projection matrix
        prjMat = self.Perspective() 
        # set up view matrix
        viewMat = self.LookAt()
        
        # set up attributes and shader program
        glEnable( GL_DEPTH_TEST )
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT )
        progDraw.Use()
        modelMat = mat.IdentityMat44()
        modelMat = mat.RotateX( modelMat, self.CalcAng( 13.0 ) )
        modelMat = mat.RotateY( modelMat, self.CalcAng( 17.0 ) )

        progDraw.SetUniforms( {
            b"u_projectionMat44" : self.Perspective(),
            b"u_viewMat44"       : self.LookAt(),
            b"u_modelMat44"      : modelMat,
            b"u_lightDir"        : [-1.0, -0.5, -2.0],
            b"u_ambient"         : 0.2,
            b"u_diffuse"         : 0.8,
            b"u_specular"        : 0.8,
            b"u_shininess"       : 10.0,
            b"u_color"           : [0.9, 0.5, 0.2] } )
        
        # draw object
        icosahedronVAO.Draw()
        
      
def AddToBuffer( buffer, data, count=1 ): 
    for inx_c in range(0, count):
        for inx_s in range(0, len(data)): buffer.append( data[inx_s] ) 

class Cache:
    def __init__( self ):
        self.index = 0
        self.middlePointIndexCache = {}
    def AddVertex( self, buffer, v ):
        inx = self.index
        print( v )
        buffer.append( mat.Normalize( v ) )
        self.index = self.index + 1
        return inx
    def getMiddlePoint( self, vertices, p1, p2 ):
        smallerIndex = p1 if p1 < p2 else p2
        greaterIndex = p2 if p1 < p2 else p1
        key = (smallerIndex << 32) + greaterIndex
        if key in self.middlePointIndexCache:
            return self.middlePointIndexCache[key]
        x = (vertices[p1][0] + vertices[p2][0]) / 2.0
        y = (vertices[p1][1] + vertices[p2][1]) / 2.0
        z = (vertices[p1][2] + vertices[p2][2]) / 2.0
        inx = self.AddVertex( vertices, ( x, y, z ) )
        self.middlePointIndexCache[key] = inx
        return inx
    def Split( self, vertices, indices ):
        newIndices = []
        for i_face in range(0, len(indices) // 3):
            i_start = i_face * 3
            f_x = indices[i_start]
            f_y = indices[i_start + 1]
            f_z = indices[i_start + 2]
            a = self.getMiddlePoint( vertices, f_x, f_y )
            b = self.getMiddlePoint( vertices, f_y, f_z )
            c = self.getMiddlePoint( vertices, f_z, f_x )
            AddToBuffer( newIndices, (f_x, a, c) )
            AddToBuffer( newIndices, (f_y, b, a) )
            AddToBuffer( newIndices, (f_z, c, b) )
            AddToBuffer( newIndices, (a, b, c) )
        return newIndices

# create window
wnd = MyWindow( 800, 600, True )

# define icosahedron vertex array opject
cache = Cache()
t = (1.0 + math.sqrt(5.0)) / 2.0
icosVertices = []
cache.AddVertex( icosVertices, (-1,  t,  0) )
cache.AddVertex( icosVertices, ( 1,  t,  0) )
cache.AddVertex( icosVertices, (-1, -t,  0) )
cache.AddVertex( icosVertices, ( 1, -t,  0) )
cache.AddVertex( icosVertices, ( 0, -1,  t) )
cache.AddVertex( icosVertices, ( 0,  1,  t) )
cache.AddVertex( icosVertices, ( 0, -1, -t) )
cache.AddVertex( icosVertices, ( 0,  1, -t) )
cache.AddVertex( icosVertices, ( t,  0, -1) )
cache.AddVertex( icosVertices, ( t,  0,  1) )
cache.AddVertex( icosVertices, (-t,  0, -1) )
cache.AddVertex( icosVertices, (-t,  0,  1) )
icosIndices = [
    0, 11, 5,    0,  5,  1,     0,  1,  7,     0, 7, 10,    0, 10, 11,
    1,  5, 9,    5, 11,  4,    11, 10,  2,    10, 7,  6,    7,  1,  8,
    3,  9, 4,    3,  4,  2,     3,  2,  6,     3, 6,  8,    3,  8,  9,
    4,  9, 5,    2,  4, 11,     6,  2, 10,     8, 6,  7,    9,  8,  1
]

splitCount = 2
for i in range(0, splitCount): icosIndices = cache.Split( icosVertices, icosIndices )
icosahedronVAO = vertex.VAObject( [ (3, icosVertices) ], icosIndices )


# load, compile and link shader
progDraw = shader.ShaderProgram( 
    [ ('resource/shader/icosahedron.vert', GL_VERTEX_SHADER),
      ('resource/shader/icosahedron.frag', GL_FRAGMENT_SHADER) ] ) 

# start main loop
wnd.Run()