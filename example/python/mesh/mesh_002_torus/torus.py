import os
import sys
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir, os.pardir)) 
os.chdir(os.path.dirname(os.path.abspath(__file__))) 

import math

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
        camera.pos = (0, -2.5, 0)
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
        torusVAO.Draw()
       

def AddToBuffer( buffer, data, count=1 ): 
    for inx_c in range(0, count):
        for inx_s in range(0, len(data)): buffer.append( data[inx_s] ) 


# create window
wnd = MyWindow( 800, 600, True )

# define torus vertex array opject
circum_size = 32
tube_size = 32
rad_circum = 1.0
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
torusVAO = vertex.VAObject( [ (3, torus_pts), (3, torus_nv), (3, torus_col) ], torus_inx )

# load, compile and link shader
progDraw = shader.ShaderProgram( 
    [ ('resource/shader/blinn_phong.vert', GL_VERTEX_SHADER),
      ('resource/shader/blinn_phong.frag', GL_FRAGMENT_SHADER) ] ) 

# start main loop
wnd.Run()