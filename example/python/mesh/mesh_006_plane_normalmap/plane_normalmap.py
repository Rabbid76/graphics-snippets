import os
import sys
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir, os.pardir)) 
os.chdir(os.path.dirname(os.path.abspath(__file__))) 

import numpy

# PyOpenGL import
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *

# PLI import
from PIL import Image

# MyLibOGL import
from MyLibOGL.math import mat
from MyLibOGL.math import cam
from MyLibOGL.ogl import shader
from MyLibOGL.ogl import vertex
from MyLibOGL.ogl import uniform
from MyLibOGL.glut import window


def ReadTexture(filename, textureUnit):
      # PIL can open BMP, EPS, FIG, IM, JPEG, MSP, PCX, PNG, PPM
      # and other file types.  We convert into a texture using GL.
      print('trying to open', filename)
      try:
         image = Image.open(filename)
      except IOError as ex:
         print('IOError: failed to open texture file ', filename)
         sys.exit()
         return -1
      print('opened file: size=', image.size, 'format=', image.format)
      imageData = numpy.array(list(image.getdata()), numpy.uint8)

      glActiveTexture( GL_TEXTURE0 + textureUnit )
      textureObj = glGenTextures( 1  )
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4)
      glBindTexture(GL_TEXTURE_2D, textureObj)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0)
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.size[0], image.size[1],0, GL_RGB, GL_UNSIGNED_BYTE, imageData)

      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)

      image.close()
      return textureObj


class MyWindow(window.CameraWindow):
    def __init__( self, cx, cy, multisample=True ):
        super().__init__(cx, cy, multisample)

    def _InitCamera_(self):
        camera = super()._InitCamera_()
        #camera.fov_y = 120 
        camera.pos = (0, -2, 0)
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
        #modelMat = mat.RotateX( modelMat, 60.0 * math.pi / 180.0 )
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
            b"u_shininess"       : 10.0,
            b"u_texture"         : 0,
            b"u_normalmap"       : 1 } )
        
        # draw object
        planeVAO.Draw()
        

def AddToBuffer( buffer, data, count=1 ): 
    for inx_c in range(0, count):
        #buffer.extend( data )
        buffer += data
        #for inx_s in range(0, len(data)): buffer.append( data[inx_s] ) 


# create window
wnd = MyWindow( 800, 600, True )

# define cube vertex array opject
planePts = [(-1.0, -1.0,  0.1), ( 1.0, -1.0,  0.1), ( 1.0,  1.0,  0.1), (-1.0,  1.0,  0.1) ]
planeCol = [ [1.0, 0.0, 0.0], [1.0, 0.5, 0.0], [1.0, 0.0, 1.0], [1.0, 1.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0] ]
planeHlpInx = [0, 1, 2, 3] 
planePosData = []
for inx in planeHlpInx: AddToBuffer( planePosData, planePts[inx] )
planeNVData = []
for inx_nv in range(0, len(planeHlpInx) // 4):
    nv = [0.0, 0.0, 0.0]
    for inx_p in range(0, 4):
        for inx_s in range(0, 3): nv[inx_s] += planePts[ planeHlpInx[inx_nv*4 + inx_p] ][inx_s]
    AddToBuffer( planeNVData, nv, 4 )
planeColData = []
for inx_col in range(0, 6):
    AddToBuffer( planeColData, planeCol[inx_col % len(planeCol)], 4 )
planeUVData = []
for inx in range(0, 6):
    planeUVData += [0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0]
planeTVData = []    
for inx_tv in range(0, len(planeHlpInx) // 4):
    p0 = planePts[ planeHlpInx[inx_tv*4 + 0 ]]
    p1 = planePts[ planeHlpInx[inx_tv*4 + 1 ]]
    tv = [p1[0]-p0[0], p1[1]-p0[1], p1[2]-p0[2]]
    AddToBuffer( planeTVData, tv, 4 )
planeIndices = []
for inx in range(0, 6):
    for inx_s in [0, 1, 2, 0, 2, 3]: planeIndices.append( inx * 4 + inx_s )
planeVAO = vertex.VAObject( [ (3, planePosData), (3, planeNVData), (3, planeTVData), (3, planeColData), (2, planeUVData) ], planeIndices )

# load, compile and link shader
progDraw = shader.ShaderProgram( 
    [ ('resource/shader/cube_normalmap.vert', GL_VERTEX_SHADER),
      ('resource/shader/cube_normalmap.frag', GL_FRAGMENT_SHADER) ] ) 

#texture objects
texObj = ReadTexture('../../../../resource/texture/test1_texture.bmp', 0)
nmTexObj = texObj = ReadTexture('../../../../resource/texture/test1_heightmap_normalmap.bmp', 1);      

# start main loop
wnd.Run()