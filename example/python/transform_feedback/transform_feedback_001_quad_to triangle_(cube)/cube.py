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
             
        # draw original cube     
        #cubeVAO.Draw()

        # draw object
        glBindVertexArray( vao_t )
        glDrawTransformFeedback(GL_TRIANGLES, tfo)
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

cubeVAO = vertex.VAObject( [ (3, cubePosData), (3, cubeNVData), (3, cubeColData) ], cubeIndices, GL_QUADS )

# load, compile and link shader
progDraw = shader.ShaderProgram( 
    [ ('resource/shader/blinn_phong.vert', GL_VERTEX_SHADER),
      ('resource/shader/blinn_phong.frag', GL_FRAGMENT_SHADER) ] ) 
print( "error:", glGetError() )

# load, compile and link transform feedback shader
progTransform = shader.ShaderProgram( 
    [ ('resource/shader/quad_triangle.vert', GL_VERTEX_SHADER),
      #('resource/shader/quad_triangle.tesc', GL_TESS_CONTROL_SHADER),
      ('resource/shader/quad_triangle.tese', GL_TESS_EVALUATION_SHADER) ], 
      [ b"out_pos", b"out_nv", b"out_col" ], GL_INTERLEAVED_ATTRIBS )
print( "error:", glGetError() )

# create trasnform feedback buffer
# https://www.opengl.org/discussion_boards/showthread.php/181664-Tessellation-with-Transform-Feedback
transform_attr_size = 6*6
transform_elem_size = transform_attr_size * (3+3+3)
transform_bytes = transform_elem_size * 4

# create the buffer object which holds the data
tbo = glGenBuffers( 1 )
glBindBuffer( GL_ARRAY_BUFFER, tbo )
glBufferData( GL_ARRAY_BUFFER, transform_bytes, None, GL_STATIC_COPY )
glBindBuffer( GL_ARRAY_BUFFER, 0 )

# create the transform feedback object 
tfo = glGenTransformFeedbacks(1)
glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfo);	
glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo)
glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0)

# fill trasnform feedback buffer
progTransform.Use()
glEnable(GL_RASTERIZER_DISCARD)
glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, tfo )

glBeginTransformFeedback(GL_TRIANGLES)
transformVAO.Draw()
glEndTransformFeedback()

glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, 0 )
glDisable( GL_RASTERIZER_DISCARD )
glUseProgram( 0 )
glFlush()

# read back the transfor feedback data
tf_arr = numpy.empty( [transform_elem_size], dtype=numpy.float32 )
tf_arr_ptr = tf_arr.ctypes.data_as(ctypes.POINTER(ctypes.c_float))
glBindBuffer( GL_ARRAY_BUFFER, tbo )
glGetBufferSubData( GL_ARRAY_BUFFER, 0, transform_bytes, tf_arr_ptr )
glBindBuffer( GL_ARRAY_BUFFER, 0 )
print(tf_arr)

# create the vertex array object which refers to the transfor feedback buffer
vao_t = glGenVertexArrays( 1 )
glBindVertexArray( vao_t )
glBindBuffer(GL_ARRAY_BUFFER, tbo)
#glBindBuffer( GL_ARRAY_BUFFER, vbo )

stride = (3+3+3)*4
glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride, ctypes.cast(0, ctypes.c_void_p) )
glEnableVertexAttribArray( 0 )
glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, stride, ctypes.cast(3*4, ctypes.c_void_p) )
glEnableVertexAttribArray( 1 )
glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, stride, ctypes.cast((3+3)*4, ctypes.c_void_p) )
glEnableVertexAttribArray( 2 )

glBindBuffer(GL_ARRAY_BUFFER, 0)
glBindVertexArray( 0 )

print( "error:", glGetError() )

# start main loop
wnd.Run()