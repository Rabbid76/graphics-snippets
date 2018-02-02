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
import random

# PyOpenGL import
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *

# MyLibOGL import
from MyLibOGL.math import mat
from MyLibOGL.math import cam
from MyLibOGL.ogl import shader
from MyLibOGL.ogl import vertex
from MyLibOGL.ogl import framebuffer
from MyLibOGL.ogl import uniform
from MyLibOGL.glut import window


class MyWindow(window.CameraWindow):
    def __init__( self, cx, cy, multisample=True ):
        super().__init__(cx, cy, multisample)

    def _InitCamera_(self):
        camera = super()._InitCamera_()
        #camera.fov_y = 120 
        camera.pos = (0, -4, 0)
        return camera    

    # draw event
    def OnDraw(self):
        # set up projection matrix
        prjMat = self.Perspective() 
        # set up view matrix
        viewMat = self.LookAt()

        ##########
        # 1. stage
        ##########
       
        # set up attributes and shader program
        fbSSAO.Bind()
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
            b"u_shininess"       : 10.0 } )
             
        # draw objects
        cubeVAO.DrawAllElements( GL_TRIANGLES )
        torusVAO.DrawAllElements( GL_TRIANGLES )


        ##########
        # 2. stage
        ##########

        fbSSAO.UnBind()
        fbBlur.Bind()
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT )
        texUnitDepth   = texUnitSSAONoise+1
        fbSSAO.BindTextures( [texUnitDepth] )
        progSSAO.Use()
        progSSAO.SetUniforms( {
            b"u_samplerDepth"     : texUnitDepth,
            b"u_samplerSSAONoise" : texUnitSSAONoise,
            b"u_viewportsize"     : vp,
            b"u_radius"           : 0.3  } )
               
        # draw screen sapce
        quadVAO.Draw()


        ##########
        # 3. stage
        ##########

        fbBlur.UnBind()
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT )
        texUnitDepth = texUnitSSAONoise+1
        texUnitAttach0 = texUnitSSAONoise+2
        texUnitSSAO   = texUnitSSAONoise+3
        fbSSAO.BindTextures( [texUnitDepth, texUnitAttach0] )
        fbBlur.BindTextures( [texUnitSSAO] )
        progBlur.Use()
        progBlur.SetUniforms( {
            b"u_samplerColor" : texUnitAttach0,
            b"u_samplerSSAO"  : texUnitSSAO,
            b"u_viewportsize" : vp,
            b"u_color_mix"    : 0.5  } )
               
        # draw screen sapce
        quadVAO.Draw()
       

def AddToBuffer( buffer, data, count=1 ): 
    for inx_c in range(0, count):
        for inx_s in range(0, len(data)): buffer.append( data[inx_s] ) 


# create window
vp = [800, 600]
wnd = MyWindow( vp[0], vp[1], True )

# SSAO noise
texUnitSSAONoise = 1
noiseSize = 4
noise = []
for i in range(noiseSize*noiseSize):
    x, y, z = random.uniform(-1, 1), random.uniform(-1, 1), random.uniform(-1, 1)
    l = math.sqrt(x*x + y*y + z*z) 
    noise.append( 255 * (x/l+1)/2,  )
    noise.append( 255 * (y/l+1)/2 )
    noise.append( 255 * (z/l+1)/2 )
    noise.append( 255 )
noisedata = numpy.matrix(noise, dtype='uint8')

glActiveTexture( GL_TEXTURE0+texUnitSSAONoise )
noiseTexture = glGenTextures( 1  )
glBindTexture( GL_TEXTURE_2D, noiseTexture )
glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, noiseSize, noiseSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, noisedata )
glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST )
glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST )
glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT )
glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT )
glActiveTexture( GL_TEXTURE0 )

# crate framebuffer
fbSSAO = framebuffer.FrameBuffer( vp[0], vp[1], [ (GL_RGBA8, GL_RGBA) ], True, False, True )
fbBlur = framebuffer.FrameBuffer( vp[0], vp[1], [ (GL_RGBA8, GL_RGBA) ], False )

# define screenspace quad vertex array opject
quadVAO = vertex.VAObject( [ (2, [ -1.0, -1.0, 1.0, -1.0, 1.0, 1.0, -1.0, 1.0 ]) ], [ 0, 1, 2, 0, 2, 3 ] )

# define cube vertex array object
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

cubeVAO = vertex.DrawBuffer()
cubeVAO.DefineVAO( [
    0, 3, 
    0, 0, 1,     0, 3, vertex.TYPE_float32, 0, 
    1, 0, 1,     1, 3, vertex.TYPE_float32, 0, 
    2, 0, 1,     2, 3, vertex.TYPE_float32, 0],
    [cubePosData, cubeNVData, cubeColData], cubeIndices )


# define torus vertex array opject
circum_size = 32
tube_size = 32
rad_circum = 1.5
rad_tube = 0.5
torus_pts = []
torus_nv = []
torus_col = []
torus_inx = []
col = [1, 0.5, 0.0]
for i_c in range(0, circum_size):
    center = [
        math.cos(2 * math.pi * i_c / circum_size),
        math.sin(2 * math.pi * i_c / circum_size) ]
    for i_t in range(0, tube_size):
        tubeX = math.cos(2 * math.pi * i_t / tube_size)
        tubeY = math.sin(2 * math.pi * i_t / tube_size)
        pt = [
           center[0] * ( rad_circum + tubeX * rad_tube ),
           center[1] * ( rad_circum + tubeX * rad_tube ),
           tubeY * rad_tube ]
        nv = [ pt[0] - center[0] * rad_tube, pt[1] - center[1] * rad_tube, tubeY * rad_tube ]
        torus_pts.extend( pt )
        torus_nv.extend( nv )
        torus_col.extend( col )
        i_cn = (i_c+1) % circum_size
        i_tn = (i_t+1) % tube_size
        i_c0 = i_c * tube_size; 
        i_c1 = i_cn * tube_size; 
        torus_inx.extend( [i_c0+i_t, i_c0+i_tn, i_c1+i_t, i_c0+i_tn, i_c1+i_t, i_c1+i_tn] )

torusVAO = vertex.DrawBuffer()
torusVAO.DefineVAO( [
    0, 3, 
    0, 0, 1,     0, 3, vertex.TYPE_float32, 0, 
    1, 0, 1,     1, 3, vertex.TYPE_float32, 0, 
    2, 0, 1,     2, 3, vertex.TYPE_float32, 0],
    [torus_pts, torus_nv, torus_col], torus_inx )

# load, compile and link shader
progDraw = shader.ShaderProgram( 
    [ ('resource/shader/blinn_phong.vert', GL_VERTEX_SHADER),
      ('resource/shader/blinn_phong.frag', GL_FRAGMENT_SHADER) ] ) 

progSSAO = shader.ShaderProgram( 
    [ ('resource/shader/ssao.vert', GL_VERTEX_SHADER),
      ('resource/shader/ssao.frag', GL_FRAGMENT_SHADER) ] )

progBlur = shader.ShaderProgram( 
    [ ('resource/shader/blur.vert', GL_VERTEX_SHADER),
      ('resource/shader/blur.frag', GL_FRAGMENT_SHADER) ] )

# start main loop
wnd.Run()