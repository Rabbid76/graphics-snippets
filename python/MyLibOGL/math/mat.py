import math

# Numpy improt [http://www.numpy.org/]
import numpy

# TODO numpy -> array [https://docs.python.org/3/library/array.html]
# TODO use double array 

def Cross( a, b ): return ( a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0], 0.0 )
def Dot( a, b ): return a[0]*b[0] + a[1]*b[1] + a[2]*b[2]
def Normalize( v ): 
    len = math.sqrt( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] )
    return (v[0] / len, v[1] / len, v[2] / len)

def IdentityMat44(): return numpy.matrix(numpy.identity(4), copy=False, dtype='float32')

def Translate(matA, trans):
    matB = numpy.copy(matA)
    for i in range(0, 4): matB[3,i] = matA[0,i] * trans[0] + matA[1,i] * trans[1] + matA[2,i] * trans[2] + matA[3,i] 
    return matB

def Scale(matA, s):
    matB = numpy.copy(matA)
    for i0 in range(0, 3):
        for i1 in range(0, 4): matB[i0,i1] = matA[i0,i1] * s[i0] 
    return matB

def RotateHlp(matA, angRad, a0, a1):
    matB = numpy.copy(matA)
    sinAng, cosAng = math.sin(angRad), math.cos(angRad)
    for i in range(0, 4):
        matB[a0,i] = matA[a0,i] * cosAng + matA[a1,i] * sinAng
        matB[a1,i] = matA[a0,i] * -sinAng + matA[a1,i] * cosAng
    return matB

def RotateX(matA, angRad): return RotateHlp(matA, angRad, 1, 2)
def RotateY(matA, angRad): return RotateHlp(matA, angRad, 2, 0)
def RotateZ(matA, angRad): return RotateHlp(matA, angRad, 0, 1)
def RotateView(matA, angRad): return RotateZ(RotateY(RotateX(matA, angRad[0]), angRad[1]), angRad[2])

def Rotate(matA, angRad, axis):
    matB = numpy.copy(matA)
    s, c = math.sin(angRad), math.cos(angRad)
    x, y, z = axis[0], axis[1], axis[2] 
    return numpy.matrix( [
        [x*x*(1-c)+c,   x*y*(1-c)-z*s, x*z*(1-c)+y*s, 0],
        [y*x*(1-c)+z*s, y*y*(1-c)+c,   y*z*(1-c)-x*s, 0],
        [z*x*(1-c)-y*s, z*y*(1-c)+x*s, z*z*(1-c)+c,   0],
        [0,             0,             0,             1] ] )    

def Multiply(matA, matB):
    matC = numpy.copy(matA)
    for i0 in range(0, 4):
        for i1 in range(0, 4):
            matC[i0,i1] = matB[i0,0] * matA[0,i1] + matB[i0,1] * matA[1,i1] + matB[i0,2] * matA[2,i1] + matB[i0,3] * matA[3,i1]    
    return matC

def ToMat33(mat44):
    mat33 = numpy.matrix(numpy.identity(3), copy=False, dtype='float32')
    for i0 in range(0, 3):
        for i1 in range(0, 3): mat33[i0, i1] = mat44[i0, i1]
    return mat33

def TransformVec4(vecA,mat44):
    vecB = numpy.zeros(4, dtype='float32')
    for i0 in range(0, 4):
        vecB[i0] = vecA[0] * mat44[0,i0] + vecA[1] * mat44[1,i0] + vecA[2] * mat44[2,i0]  + vecA[3] * mat44[3,i0]
    return vecB