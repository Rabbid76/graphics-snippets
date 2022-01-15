# Parallax Occlusion Mapping in GLSL [http://sunandblackcat.com/tipFullView.php?topicid=28]
# Followup: Normal Mapping Without Precomputed Tangents [http://www.thetenthplanet.de/archives/1180]

import os
import sys
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir, os.pardir)) 
os.chdir(os.path.dirname(os.path.abspath(__file__))) 

import numpy
from PIL import Image
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *

# MyLibOGL import
from MyLibOGL.math import mat
from MyLibOGL.ogl import shader
from MyLibOGL.ogl import vertex
from MyLibOGL.glut import window

def ReadTexture(filename):
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
      return image
 
def CreateTexture(image, textureUnit, is_height_map):
      imageData = numpy.array(list(image.getdata()), numpy.uint8)

      glActiveTexture( GL_TEXTURE0 + textureUnit )
      textureObj = glGenTextures( 1  )
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4)
      glBindTexture(GL_TEXTURE_2D, textureObj)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0)
      internal_format = GL_R8 if is_height_map else GL_RGBA8
      glTexImage2D(GL_TEXTURE_2D, 0, internal_format, image.size[0], image.size[1],0, GL_RGB, GL_UNSIGNED_BYTE, imageData)
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)

      image.close()
      return textureObj

def CreateTextureFromFile(filename, textureUnit):
      image = ReadTexture(filename)
      textureObj = CreateTexture(image, textureUnit, False) 
      return textureObj

def CreateHeightMapFromFile(filename, textureUnit):
      image = ReadTexture(filename)
      textureObj = CreateTexture(image, textureUnit, True) 
      return textureObj


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
        #modelMat = mat.RotateX( modelMat, 60.0*math.pi/180.0 )
        #modelMat = mat.RotateY( modelMat, -10.0*math.pi/180.0 )

        progDraw.SetUniforms( {
            b"u_projectionMat44"    : self.Perspective(),
            b"u_viewMat44"          : self.LookAt(),
            b"u_modelMat44"         : modelMat,
            b"u_lightDir"           : [-1.0, -0.5, -2.0],
            b"u_ambient"            : 0.2,
            b"u_diffuse"            : 0.8,
            b"u_specular"           : 0.8,
            b"u_shininess"          : 10.0,
            b"u_texture"            : 0,
            b"u_displacement_map"   : 1,
            b"u_normal_map"         : 2,
            b"u_displacement_scale" : 0.2, 
            b"u_parallax_quality"   : [2.0, 1.0]
        } )
        
        # draw object
        cubeVAO.DrawIB( GL_TRIANGLES )
    
  
def AddToBuffer( buffer, data, count=1 ): 
    for inx_c in range(0, count):
        #buffer.extend( data )
        buffer += data
        #for inx_s in range(0, len(data)): buffer.append( data[inx_s] ) 


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
cubeUVData = []
for inx in range(0, 6):
    cubeUVData += [0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0]
cubeIndices = []
for inx in range(0, 6):
    for inx_s in [0, 1, 2, 0, 2, 3]: cubeIndices.append( inx * 4 + inx_s )

cubeVAO = vertex.MeshBuffer()
cubeVAO.DefineVA( [
  ( [ (0, 3, 0) ], 0, cubePosData ),
  ( [ (1, 3, 0) ], 0, cubeNVData ),
  ( [ (2, 3, 0) ], 0, cubeColData ),
  ( [ (3, 2, 0) ], 0, cubeUVData ),
] )
cubeVAO.DefineIB( cubeIndices )

# load, compile and link shader
progDraw = shader.ShaderProgram( 
    [ ('resource/shader/displacement_map.vert', GL_VERTEX_SHADER),
      ('resource/shader/displacement_map.frag', GL_FRAGMENT_SHADER) ] ) 

#texture objects
textureObj         = CreateTextureFromFile('../../../../resource/texture/example_1_texture.bmp', 0)
displacementmapObj = CreateHeightMapFromFile('../../../../resource/texture/example_1_heightmap.bmp', 1)

# start main loop
wnd.Run()

# clean up
del cubeVAO