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
        camera.pos = (0, -9, 1)
        return camera    

    # draw event
    def OnDraw(self):
        # set up projection matrix
        prjMat = self.Perspective() 
        # set up view matrix
        viewMat = self.LookAt()

        # set up light source
        lightSourceBuffer.BindDataFloat(b'u_lightSource.dir', mat.TransformVec4([-0.1, 1.0, -5.0, 0.0], viewMat) )
    
        # set up the model matrix
        modelMat = mat.IdentityMat44()
        modelMat = self.AutoModelMatrix()
        modelMat = mat.Scale( modelMat, numpy.repeat( 4, 3 ) )
 
        # set up attributes and shader program
        glEnable( GL_DEPTH_TEST )
        glClearColor( 1, 1, 1, 0 )
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT )
        progGeo.Use()
        progGeo.SetUniformM44( b"u_projectionMat44", prjMat )
        progGeo.SetUniformM44( b"u_viewMat44", viewMat )
        lightSourceBuffer.BindToTarget()
    
        # draw point
        progGeo.SetUniformM44( b"u_modelMat44", modelMat )
        materialBuffer.BindToTarget()
        pointVAObj.Draw()

# create window
rotateCamera = False
wnd = MyWindow( 800, 600, True )

# define location vertex array opject
pointVAObj = vertex.VAObject( [ (3, [ (0.0, 0.0, 0.0) ] ), (3, [ (0.0, 0.0, -1.0) ]), (3, [ (1.0, 0.0, 0.0) ]) ], [], GL_POINTS )

# load, compile and link shader
progGeo = shader.ShaderProgram( 
    [ ('resource/shader/geo.vert', GL_VERTEX_SHADER),
      ('resource/shader/geo.geom', GL_GEOMETRY_SHADER),
      ('resource/shader/geo.frag', GL_FRAGMENT_SHADER ) ] )

# linke uniform blocks
ubMaterial = uniform.UniformBlock(progGeo.Prog(), "UB_material")
ubLightSource = uniform.UniformBlock(progGeo.Prog(), "UB_lightSource")
ubMaterial.Link(1)
ubLightSource.Link(2)

# create uniform block buffers
lightSourceBuffer = uniform.UniformBlockBuffer(ubLightSource)
lightSourceBuffer.BindDataFloat(b'u_lightSource.ambient', [0.2, 0.2, 0.2, 1.0])
lightSourceBuffer.BindDataFloat(b'u_lightSource.diffuse', [0.2, 0.2, 0.2, 1.0])
lightSourceBuffer.BindDataFloat(b'u_lightSource.specular', [1.0, 1.0, 1.0, 1.0])

materialBuffer = uniform.UniformBlockBuffer(ubMaterial)
materialBuffer.BindDataFloat(b'u_roughness', [0.5])
materialBuffer.BindDataFloat(b'u_fresnel0', [0.2])
materialBuffer.BindDataFloat(b'u_specularTint',[1.0, 0.5, 0.5, 0.8])

# start main loop
wnd.Run()