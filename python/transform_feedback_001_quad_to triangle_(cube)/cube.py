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
             
        # draw object
        #cubeVAO.Draw()
        glBindVertexArray( transformed_vao )
        glDrawTransformFeedback(GL_TRIANGLES, tf)
        glBindVertexArray( 0 )
   

def AddToBuffer( buffer, data, count=1 ): 
    for inx_c in range(0, count):
        for inx_s in range(0, len(data)): buffer.append( data[inx_s] ) 


# create window
wnd = MyWindow( 800, 600, True )

# define cube vertex array opject
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
    for inx_s in [0, 1, 2, 3]: cubeIndices.append( inx * 4 + inx_s )
transformVAO = vertex.VAObject( [ (3, cubePosData), (3, cubeNVData), (3, cubeColData) ], cubeIndices, GL_PATCHES, 4 )

cubeVAO = vertex.VAObject( [ (3, cubePosData), (3, cubeNVData), (3, cubeColData) ], cubeIndices, GL_TRIANGLES )

# load, compile and link shader
progTransform = shader.ShaderProgram( 
    [ ('resource/shader/quad_triangle.vert', GL_VERTEX_SHADER),
      #('resource/shader/quad_triangle.tesc', GL_TESS_CONTROL_SHADER),
      ('resource/shader/quad_triangle.tese', GL_TESS_EVALUATION_SHADER) ], 
      [ b"out_pos", b"out_nv", b"out_col" ], GL_INTERLEAVED_ATTRIBS )
print( "error:", glGetError() )

progDraw = shader.ShaderProgram( 
    [ ('resource/shader/blinn_phong.vert', GL_VERTEX_SHADER),
      ('resource/shader/blinn_phong.frag', GL_FRAGMENT_SHADER) ] ) 
print( "error:", glGetError() )

# create trasnform feedback buffer
# https://www.opengl.org/discussion_boards/showthread.php/181664-Tessellation-with-Transform-Feedback
transform_attr_size = 6*6
transform_elem_size = transform_attr_size * (3+3+3)
transform_bytes = transform_elem_size * 4
tbo = glGenBuffers( 1 )
glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tbo)
glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, transform_bytes, None, GL_STATIC_COPY)
glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo, 0, transform_bytes)
glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0)
print( "error:", glGetError() )

tf = glGenTransformFeedbacks(1)
glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tf);	
glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo)
glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0)
print( "error:", glGetError() )

# fill trasnform feedback buffer
progTransform.Use()
glEnable(GL_RASTERIZER_DISCARD)
glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tf)
glBeginTransformFeedback(GL_TRIANGLES)
transformVAO.Draw()
glEndTransformFeedback()
glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0)
glDisable(GL_RASTERIZER_DISCARD)
glFlush()
glUseProgram(0)

test_arr = numpy.empty( [transform_elem_size], dtype=numpy.float32 )
glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tbo)
glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, transform_elem_size, test_arr)
glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0)
print(test_arr)

print( "error:", glGetError() )



# create the vertex array object which refers to the transfor feedback buffer
transformed_vao = glGenVertexArrays( 1 )
glBindVertexArray( transformed_vao )
glBindBuffer(GL_ARRAY_BUFFER, tbo)

stride = (3+3+3)*4
glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride, 0 )
glEnableVertexAttribArray( 0 )
glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, stride, 3*4 )
glEnableVertexAttribArray( 1 )
glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, stride, (3+3)*4 )
glEnableVertexAttribArray( 2 )

glBindBuffer(GL_ARRAY_BUFFER, 0)
glBindVertexArray( 0 )
print( "error:", glGetError() )


# start main loop
wnd.Run()