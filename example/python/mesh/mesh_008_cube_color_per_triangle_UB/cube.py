import os
import sys
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir, os.pardir)) 
os.chdir(os.path.dirname(os.path.abspath(__file__))) 

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
        camera.pos = (0, -3, 0)
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
        modelMat = self.AutoModelMatrix()
        #modelMat = mat.RotateX( modelMat, self.CalcAng( 13.0 ) )
        #modelMat = mat.RotateY( modelMat, self.CalcAng( 17.0 ) )

        progDraw.SetUniforms( {
            b"u_projectionMat44" : self.Perspective(),
            b"u_viewMat44"       : self.LookAt(),
            b"u_modelMat44"      : modelMat,
            b"u_lightDir"        : [-1.0, -0.5, -2.0],
            b"u_ambient"         : 0.2,
            b"u_diffuse"         : 0.8,
            b"u_specular"        : 0.8,
            b"u_shininess"       : 10.0 } )
             
        # draw object
        cubeVAO.Draw()
   

def AddToBuffer( buffer, data, count=1 ): 
    for inx_c in range(0, count):
        for inx_s in range(0, len(data)): buffer.append( data[inx_s] ) 


# create window
wnd = MyWindow( 800, 600, True )

# define cube vertex array opject
cubePts = [
    (-1.0, -1.0,  1.0), ( 1.0, -1.0,  1.0), ( 1.0,  1.0,  1.0), (-1.0,  1.0,  1.0),
    (-1.0, -1.0, -1.0), ( 1.0, -1.0, -1.0), ( 1.0,  1.0, -1.0), (-1.0,  1.0, -1.0) ]
cubeCol = [ [1.0, 0.0, 0.0, 1.0], [1.0, 0.5, 0.0, 1.0], [1.0, 0.0, 1.0, 1.0], [1.0, 1.0, 0.0, 1.0], [0.0, 1.0, 0.0, 1.0], [0.0, 0.0, 1.0, 1.0] ]
cubeHlpInx = [ 0, 1, 2, 3, 1, 5, 6, 2, 5, 4, 7, 6, 4, 0, 3, 7, 3, 2, 6, 7, 1, 0, 4, 5 ] 
cubePosData = []
for inx in cubeHlpInx: AddToBuffer( cubePosData, cubePts[inx] )
cubeNVData = []
for inx_nv in range(len(cubeHlpInx) // 4):
    nv = [0.0, 0.0, 0.0]
    for inx_p in range(4):
        for inx_s in range(0, 3): nv[inx_s] += cubePts[ cubeHlpInx[inx_nv*4 + inx_p] ][inx_s]
    AddToBuffer( cubeNVData, nv, 4 )
cubeColFaceData = []
for inx_col in range(6):
    for inx_c in range(0, 4): cubeColFaceData.append( cubeCol[inx_col][inx_c] )
    for inx_c in range(0, 4): cubeColFaceData.append( cubeCol[inx_col][inx_c] ) 
cubeIndices = []
for inx in range(6):
    for inx_s in [0, 1, 2, 0, 2, 3]: cubeIndices.append( inx * 4 + inx_s )
cubeVAO = vertex.VAObject( [ (3, cubePosData), (3, cubeNVData) ], cubeIndices )

# Uniform Buffer Object (uniform block) [https://www.khronos.org/opengl/wiki/Uniform_Buffer_Object]
ubo = glGenBuffers( 1 )
glBindBuffer( GL_UNIFORM_BUFFER, ubo )

# case 1
#dataArray = (ctypes.c_float * len(cubeColFaceData))(*cubeColFaceData)
#glBufferData( GL_UNIFORM_BUFFER, dataArray, GL_STATIC_DRAW )

# case 2
#dataArray = numpy.array( cubeColFaceData, dtype=numpy.float32 )
#glBufferData( GL_UNIFORM_BUFFER, dataArray, GL_STATIC_DRAW )

# case 3
#dataArray = (ctypes.c_float * len(cubeColFaceData))(*cubeColFaceData)
#glBufferData( GL_UNIFORM_BUFFER, len(dataArray)*4, (ctypes.c_void_p)(ctypes.addressof(dataArray)), GL_STATIC_DRAW )

# case 4
#dataArray = (ctypes.c_float * len(cubeColFaceData))(*cubeColFaceData)
#glBufferData( GL_UNIFORM_BUFFER, len(dataArray)*4, None, GL_STATIC_DRAW )
#glBufferSubData( GL_UNIFORM_BUFFER, 0, dataArray )

# case 5
#dataArray = (ctypes.c_float * len(cubeColFaceData))(*cubeColFaceData)
#glBufferData( GL_UNIFORM_BUFFER, len(dataArray)*4, None, GL_STATIC_DRAW )
#glBufferSubData( GL_UNIFORM_BUFFER, 0, len(dataArray)*4, (ctypes.c_void_p)(ctypes.addressof(dataArray)) )

# case 6
dataArray = numpy.array( cubeColFaceData, dtype=numpy.float32 )
glBufferData( GL_UNIFORM_BUFFER, len(dataArray)*4, None, GL_STATIC_DRAW )
glBufferSubData( GL_UNIFORM_BUFFER, 0, dataArray )


# case 7
#dataArray = numpy.array( cubeColFaceData, dtype=numpy.float32 )
#data = ???
#glBufferData( GL_UNIFORM_BUFFER, len(dataArray)*4, data, GL_STATIC_DRAW )

glBindBufferBase( GL_UNIFORM_BUFFER, 1, ubo )
glBindBuffer(GL_UNIFORM_BUFFER, 0)

# load, compile and link shader
progDraw = shader.ShaderProgram( 
    [ ('resource/shader/blinn_phong.vert', GL_VERTEX_SHADER),
      ('resource/shader/blinn_phong.frag', GL_FRAGMENT_SHADER) ] ) 

# start main loop
wnd.Run()