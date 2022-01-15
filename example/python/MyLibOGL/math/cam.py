import math

# Numpy improt [http://www.numpy.org/]
import numpy

# MyLibOGL import
from MyLibOGL.math import mat

# TODO numpy -> array [https://docs.python.org/3/library/array.html]
# TODO use double array 

class Camera:
    def __init__(self):
        self.pos    = (0, -1, 0)
        self.target = (0, 0, 0)
        self.up     = (0, 0, 1)
        self.fov_y  = 90
        self.vp     = (800, 600)
        self.near   = 0.5
        self.far    = 100.0
    def Perspective(self):
        fn, f_n = self.far + self.near, self.far - self.near
        r, t = self.vp[0] / self.vp[1], 1.0 / math.tan( math.radians( self.fov_y ) / 2.0 )
        return numpy.matrix( [ [t/r,0,0,0], [0,t,0,0], [0,0,-fn/f_n,-1], [0,0,-2*self.far*self.near/f_n,0] ] )
    # TODO def Orthographic
    def LookAt(self):
        mz = mat.Normalize( (self.pos[0]-self.target[0], self.pos[1]-self.target[1], self.pos[2]-self.target[2]) ) # inverse line of sight
        mx = mat.Normalize( mat.Cross( self.up, mz ) )
        my = mat.Normalize( mat.Cross( mz, mx ) )
        tx =  mat.Dot( mx, self.pos )
        ty =  mat.Dot( my, self.pos )
        tz = -mat.Dot( mz, self.pos )   
        return numpy.matrix( [ [mx[0], my[0], mz[0], 0], [mx[1], my[1], mz[1], 0], [mx[2], my[2], mz[2], 0], [tx, ty, tz, 1] ] )