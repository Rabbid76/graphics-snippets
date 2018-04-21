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
        camera.pos = (0, -1.3, 1)
        return camera    

    # draw event
    def OnDraw(self):
        # set up projection matrix
        prjMat = self.Perspective() 
        # set up view matrix
        viewMat = self.LookAt()

        # set up light source
        lightSourceBuffer.BindDataFloat(b'u_lightSource.dir', mat.TransformVec4([-1.0, -1.0, -5.0, 0.0], viewMat) )
    
        # set up icosahedron model matrix
        modelMat = mat.IdentityMat44() 
        modelMat = self.AutoModelMatrix()
        #modelMat = mat.RotateX( modelMat, self.CalcAng( 13.0 ) )
        #modelMat = mat.RotateY( modelMat, self.CalcAng( 17.0 ) )

        # set up attributes and shader program
        glEnable( GL_DEPTH_TEST )
        glClearColor( 1, 1, 1, 0 )
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT )
        progTess.Use()
        progTess.SetUniformM44( b"u_projectionMat44", prjMat )
        lightSourceBuffer.BindToTarget()
    
        # draw icosahedron
        icoMaterialBuffer.BindToTarget()
        modelViewMat = mat.Multiply(viewMat, modelMat)
        progTess.SetUniformM44( b"u_modelViewMat44", modelViewMat )
        progTess.SetUniformM33( b"u_normalMat33", mat.ToMat33(modelViewMat) )
        icoVAO.Draw()


def AddToBuffer( buffer, data, count=1 ): 
    for inx_c in range(0, count):
        for inx_s in range(0, len(data)): buffer.append( data[inx_s] ) 


#window.printExtensions = True
#shader.printShaderCode = shader.PrintShaderCode.ALWAYS

# create window
wnd = MyWindow( 800, 600, True )

# define icosahedron vertex array opject
icoPts = [
    ( 0.000,  0.000,  1.000), ( 0.894,  0.000,  0.447), ( 0.276,  0.851,  0.447), (-0.724,  0.526,  0.447),
    (-0.724, -0.526,  0.447), ( 0.276, -0.851,  0.447), ( 0.724,  0.526, -0.447), (-0.276,  0.851, -0.447), 
    (-0.894,  0.000, -0.447), (-0.276, -0.851, -0.447), ( 0.724, -0.526, -0.447), ( 0.000,  0.000, -1.000) ]
icoCol = [ [1.0, 0.0, 0.0], [0.0, 0.0, 1.0], [1.0, 1.0, 0.0], [0.0, 1.0, 0.0], [1.0, 0.5, 0.0], [1.0, 0.0, 1.0] ]
icoIndices = [
  2,  0,  1,  3,  0,  2,  4,  0,  3,  5,  0,  4,  1,  0,  5, 11,  7,  6, 11,  8,  7, 11,  9,  8, 11, 10,  9, 11,  6, 10, 
  1,  6,  2,  2,  7,  3,  3,  8,  4,  4,  9,  5,  5, 10,  1,  2,  6,  7,  3,  7,  8,  4,  8,  9,  5,  9, 10,  1, 10,  6  ]
icoPosData = []
for inx in icoIndices: AddToBuffer( icoPosData, icoPts[inx] )
icoNVData = []
for inx_nv in range(0, len(icoIndices) // 3):
    nv = [0.0, 0.0, 0.0]
    for inx_p in range(0, 3): 
        for inx_s in range(0, 3): nv[inx_s] += icoPts[ icoIndices[inx_nv*3 + inx_p] ][inx_s]
    AddToBuffer( icoNVData, nv, 3 )
icoVAO = vertex.VAObject( [ (3, icoPosData), (3, icoNVData)], [], GL_PATCHES, 3 )

# load, compile and link shader
progTess = shader.ShaderProgram( 
    [ ('resource/shader/tess.vert', GL_VERTEX_SHADER),
      ('resource/shader/tess.tesc', GL_TESS_CONTROL_SHADER),
      ('resource/shader/tess.tese', GL_TESS_EVALUATION_SHADER),
      ('resource/shader/tess.frag', GL_FRAGMENT_SHADER ) ] )
# linke uniform blocks
ubMaterial = uniform.UniformBlock(progTess.Prog(), "UB_material")
ubLightSource = uniform.UniformBlock(progTess.Prog(), "UB_lightSource")
ubMaterial.Link(1)
ubLightSource.Link(2)

# create uniform block buffers
lightSourceBuffer = uniform.UniformBlockBuffer(ubLightSource)
lightSourceBuffer.BindDataFloat(b'u_lightSource.ambient', [0.2, 0.2, 0.2, 1.0])
lightSourceBuffer.BindDataFloat(b'u_lightSource.diffuse', [0.2, 0.2, 0.2, 1.0])
lightSourceBuffer.BindDataFloat(b'u_lightSource.specular', [1.0, 1.0, 1.0, 1.0])

icoMaterialBuffer = uniform.UniformBlockBuffer(ubMaterial)
icoMaterialBuffer.BindDataFloat(b'u_roughness', [0.45])
icoMaterialBuffer.BindDataFloat(b'u_fresnel0', [0.4])
icoMaterialBuffer.BindDataFloat(b'u_color', [0.6, 0.5, 0.8, 1.0])
icoMaterialBuffer.BindDataFloat(b'u_specularTint',[1.0, 0.5, 0.5, 0.8])

# start main loop
wnd.Run()