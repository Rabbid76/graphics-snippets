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
from MyLibOGL.ogl import framebuffer
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
        
        # set up attributes and shader program
        fbX.Bind()
        glEnable( GL_DEPTH_TEST )
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT )
        progDraw.Use()
        progDraw.SetUniform( b"u_projectionMat44", self.Perspective() )
        viewMat = self.LookAt()
        modelMat = mat.IdentityMat44() 
        modelMat = self.AutoModelMatrix()
        #modelMat = mat.RotateX( modelMat, self.CalcAng( 13.0 ) )
        #modelMat = mat.RotateY( modelMat, self.CalcAng( 17.0 ) )
        progDraw.SetUniform( b"u_modelViewMat44", mat.Multiply(viewMat, modelMat) )
        progDraw.SetUniform( b"u_glow", 3.0 )
        objVAO.Draw()

        sigma = 0.8
        blurWidth = 20

        # blur X
        fbX.UnBind()
        fbY.Bind()
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT )
        texUnitAttach0 = 1
        fbX.BindTextures( [texUnitAttach0] )
        progBlurX.Use()
        progBlurX.SetUniform( b"u_textureCol", texUnitAttach0 )
        progBlurX.SetUniform( b"u_textureSize", self.VPSize() )
        progBlurX.SetUniform( b"u_sigma", sigma )
        progBlurX.SetUniform( b"u_width", blurWidth )
        # TODO SetUniforms( { } )

        # draw screen sapce
        quadVAO.Draw()

        # blur Y
        fbY.UnBind()
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT )
        texUnitAttach0 = 2
        fbY.BindTextures( [texUnitAttach0] )
        progBlurY.Use()
        progBlurY.SetUniform( b"u_textureCol", texUnitAttach0 )
        progBlurY.SetUniform( b"u_textureSize", self.VPSize() )
        progBlurY.SetUniform( b"u_sigma", sigma )
        progBlurY.SetUniform( b"u_width", blurWidth )

        # draw screen sapce
        quadVAO.Draw()
    
def AddToBuffer( buffer, data, count=1 ): 
    for inx_c in range(0, count):
        for inx_s in range(0, len(data)): buffer.append( data[inx_s] ) 


#window.printExtensions = True
#shader.printShaderCode = shader.PrintShaderCode.ALWAYS

# create window
wnd = MyWindow( 800, 600, True )

# crate framebuffer
fbX = framebuffer.FrameBuffer( 800, 600, [ (GL_RGBA8, GL_RGBA) ], True, True, False )
fbY = framebuffer.FrameBuffer( 800, 600, [ (GL_RGBA8, GL_RGBA) ], False )

# define screenspace quad vertex array opject
quadVAO = vertex.VAObject( [ (2, [ -1.0, -1.0, 1.0, -1.0, 1.0, 1.0, -1.0, 1.0 ]) ], [ 0, 1, 2, 0, 2, 3 ] )

# define tetrahedron vertex array opject
cubePts = [
    (-1.0, -1.0,  1.0), ( 1.0, -1.0,  1.0), ( 1.0,  1.0,  1.0), (-1.0,  1.0,  1.0),
    (-1.0, -1.0, -1.0), ( 1.0, -1.0, -1.0), ( 1.0,  1.0, -1.0), (-1.0,  1.0, -1.0) ]
cubeCol = [ [1.0, 0.0, 0.0], [1.0, 0.5, 0.0], [1.0, 0.0, 1.0], [1.0, 1.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0] ]
cubeHlpInx = [ 0, 1, 2, 3, 1, 5, 6, 2, 5, 4, 7, 6, 4, 0, 3, 7, 3, 2, 6, 7, 1, 0, 4, 5 ] 
cubePosData = []
for inx in cubeHlpInx: AddToBuffer( cubePosData, cubePts[inx] )
cubeNVData = []
for inx_nv in range(0, len(cubeHlpInx) // 4):
    nv = [0.0, 0.0, 0.0]
    for inx_p in range(0, 4):
        for inx_s in range(0, 3): nv[inx_s] += cubePts[ cubeHlpInx[inx_nv*4 + inx_p] ][inx_s]
    AddToBuffer( cubeNVData, nv, 4 )
cubeColData = []
for inx_col in range(0, 6):
    AddToBuffer( cubeColData, cubeCol[inx_col % len(cubeCol)], 4 )
cubeIndices = []
for inx in range(0, 6):
    for inx_s in [0, 1, 2, 0, 2, 3]: cubeIndices.append( inx * 4 + inx_s )
objVAO = vertex.VAObject( [ (3, cubePosData), (3, cubeNVData), (3, cubeColData) ], cubeIndices )

# load, compile and link shader
progDraw = shader.ShaderProgram( 
    [ ('resource/shader/glow.vert', GL_VERTEX_SHADER),
      ('resource/shader/glow.frag', GL_FRAGMENT_SHADER) ] ) 

progBlurX = shader.ShaderProgram( 
    [ ('resource/shader/gauss.vert', GL_VERTEX_SHADER),
      ('resource/shader/gaussX.frag', GL_FRAGMENT_SHADER) ] )

progBlurY = shader.ShaderProgram( 
    [ ('resource/shader/gauss.vert', GL_VERTEX_SHADER),
      ('resource/shader/gaussY.frag', GL_FRAGMENT_SHADER) ] ) 

# start main loop
wnd.Run()