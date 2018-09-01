# Stackoverflow

Using Interface Block and Uniform Block : A Cook-Torrance light model in OGL 4.0 GLSL
[https://stackoverflow.com/documentation/glsl/5480/getting-started-with-glsl/31886/using-interface-block-and-uniform-block-a-cook-torrance-light-model-in-ogl-4-0#t=201707171704165888596]


# GLSL documentation : Using Interface Block and Uniform Block : A Cook-Torrance light model in OGL 4.0 GLSL

A simple OGL 4.0 GLSL shader program that shows the use of a interface block and a uniform block on a Cook-Torrance microfacet light model implementation.
The program is executed with a python script. To run the script, PyOpenGL and NumPy must be installed.

An Interface Block is a group of GLSL input, output, uniform, or storage buffer variables.
An Uniform Block is an Interface Block with the storage qualifier? `uniform`.

## Vertex shader

*ibub.vert*

```glsl
#version 400

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNV;
layout (location = 2) in vec3 inCol;

out TVertexData
{
    vec3 pos;
    vec3 nv;
    vec3 col;
} outData;

uniform mat4 u_projectionMat44;
uniform mat4 u_modelViewMat44;
uniform mat3 u_normalMat33;

void main()
{
    vec4 viewPos = u_modelViewMat44 * vec4( inPos, 1.0 );

    outData.pos = viewPos.xyz / viewPos.w;
    outData.nv  = u_normalMat33 * normalize( inNV );
    outData.col = inCol;

    gl_Position = u_projectionMat44 * viewPos;
}
```

## Fragment shader

*ibub.frag*

```glsl
#version 400

in TVertexData
{
    vec3 pos;
    vec3 nv;
    vec3 col;
} inData;

out vec4 fragColor;

uniform UB_material
{
    float u_roughness;
    float u_fresnel0;
    vec4  u_specularTint;
};

struct TLightSource
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 dir;
};

uniform UB_lightSource
{
    TLightSource u_lightSource;
};

vec3 CookTorrance( vec3 esPt, vec3 esPtNV, vec3 col, vec4 specularTint, float roughness, float fresnel0 )
{
    vec3  esVLight      = normalize( -u_lightSource.dir.xyz );
    vec3  esVEye        = normalize( -esPt );
    vec3  halfVector    = normalize( esVEye + esVLight );
    vec3  reflVector    = normalize( reflect( -esVLight, esPtNV ) );
    float VdotR         = dot( esVEye, reflVector );
    float HdotL         = dot( halfVector, esVLight );
    float NdotL         = dot( esPtNV, esVLight );
    float NdotV         = dot( esPtNV, esVEye );
    float NdotH         = dot( esPtNV, halfVector );
    float NdotH2        = NdotH * NdotH;
    float NdotL_clamped = max( NdotL, 0.0 );
    float NdotV_clamped = max( NdotV, 0.0 );
    float m2            = roughness * roughness;

    // Lambertian diffuse
    float k_diffuse = NdotL_clamped;

    // Cook-Torrance fresnel
    float theta = HdotL;
    float n = (1.0 + sqrt(fresnel0)) / (1.0 - sqrt(fresnel0));
    float g = sqrt( n*n + theta * theta + 1.0 );
    float gc = g + theta;
    float g_c = g - theta;
    float q = (gc * theta - 1.0) / (g_c * theta + 1.0);
    float fresnel = 0.5 * (g_c * g_c) / (gc * gc) * (1.0 + q * q);

    // Gaussian  distribution
    float psi = acos( VdotR );
    float distribution = max( 0.0, HdotL * exp( - psi * psi / m2 ) );

    // Torrance-Sparrow geometric term
    float geometric_att = min( 1.0, min( 2.0 * NdotH * NdotV_clamped / HdotL, 2.0 * NdotH * NdotL_clamped / HdotL ) );

    // Microfacet bidirectional reflectance distribution function 
    float brdf_spec = fresnel * distribution * geometric_att / ( 4.0 * NdotL_clamped * NdotV_clamped );
    float k_specular = brdf_spec;

    vec3 lightColor = col.rgb * u_lightSource.ambient.rgb
                    + max( 0.0, k_diffuse ) * col.rgb * u_lightSource.diffuse.rgb +
                    + max( 0.0, k_specular ) * mix( col.rgb, specularTint.rgb, specularTint.a ) *     u_lightSource.specular.rgb;
    return lightColor;
}

void main()
{
    vec3 lightCol = CookTorrance( inData.pos, inData.nv, inData.col, u_specularTint, u_roughness, u_fresnel0 );
    fragColor = vec4( lightCol, 1.0 );
}
```

### Python script ###

```python
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
import numpy as np
from time import time
import math
import sys

sin120 = 0.8660254
rotateCamera = False

# draw event
def OnDraw():
    dist = 3.0
    currentTime = time()
    comeraRotAng = CalcAng( currentTime, 10.0 ) 
    wndW = glutGet(GLUT_WINDOW_WIDTH)
    wndH = glutGet(GLUT_WINDOW_HEIGHT)
    # set up projection matrix
    prjMat = Perspective(90.0, wndW/wndH, 0.5, 100.0) 
    # set up view matrix
    viewMat = Translate( np.matrix(np.identity(4), copy=False, dtype='float32'), np.array( [0.0, 0.0, -12.0] ) )
    viewMat = RotateView( viewMat, [30.0, comeraRotAng if rotateCamera else 0.0, 0.0] )

    # set up light source
    lightSourceBuffer.BindDataFloat(b'u_lightSource.dir', TransformVec4([-3.0, -2.0, -1.0, 0.0], viewMat) )

    # set up tetrahedron model matrix
    tetModelMat = np.matrix(np.identity(4), copy=False, dtype='float32')
    if not rotateCamera: tetModelMat = RotateY( tetModelMat, comeraRotAng )
    tetModelMat = RotateX( tetModelMat, -90.0 )
    tetModelMat = Scale( tetModelMat, np.repeat( 2.4, 3 ) )
    tetModelMat = Translate( tetModelMat, np.array( [0.0, dist, 0.0] ) )
    tetModelMat = RotateY( tetModelMat, CalcAng( currentTime, 20.0 ) )
    tetModelMat = RotateX( tetModelMat, CalcAng( currentTime, 9.0 ) )

    # set up icosahedron model matrix
    icoModelMat = np.matrix(np.identity(4), copy=False, dtype='float32')
    if not rotateCamera: icoModelMat = RotateY( icoModelMat, comeraRotAng )
    icoModelMat = RotateX( icoModelMat, -90.0 )
    icoModelMat = Scale( icoModelMat, np.repeat( 2.0, 3 ) )
    icoModelMat = Translate( icoModelMat, np.array( [dist * -sin120, dist * -0.5, 0.0] ) )
    icoModelMat = RotateY( icoModelMat, CalcAng( currentTime, 20.0 ) )
    icoModelMat = RotateX( icoModelMat, CalcAng( currentTime, 11.0 ) )

    # set up cube model matrix
    cubeModelMat = np.matrix(np.identity(4), copy=False, dtype='float32')
    if not rotateCamera: cubeModelMat = RotateY( cubeModelMat, comeraRotAng )
    cubeModelMat = RotateX( cubeModelMat, -90.0 )
    cubeModelMat = Scale( cubeModelMat, np.repeat( 1.6, 3 ) )
    cubeModelMat = Translate( cubeModelMat, np.array( [dist * sin120, dist * -0.5, 0.0] ) )
    cubeModelMat = RotateY( cubeModelMat, CalcAng( currentTime, 20.0 ) )
    cubeModelMat = RotateX( cubeModelMat, CalcAng( currentTime, 13.0 ) )

    # set up attributes and shader program
    glEnable( GL_DEPTH_TEST )
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT )
    glUseProgram( shaderProgram )
    glUniformMatrix4fv( projectionMatLocation, 1, GL_FALSE, prjMat )
    lightSourceBuffer.BindToTarget()

    # draw tetrahedron
    tetMaterialBuffer.BindToTarget()
    modelViewMat = Multiply(viewMat, tetModelMat)
    glUniformMatrix4fv( modelViewMatLocation, 1, GL_FALSE, modelViewMat )
    glUniformMatrix3fv( normalMatLocation, 1, GL_FALSE, ToMat33(modelViewMat) )
    glBindVertexArray( tetVAObj )
    glDrawArrays( GL_TRIANGLES, 0, len(tetPosData) )

    # draw icosahedron
    icoMaterialBuffer.BindToTarget()
    modelViewMat = Multiply(viewMat, icoModelMat)
    glUniformMatrix4fv( modelViewMatLocation, 1, GL_FALSE, modelViewMat )
    glUniformMatrix3fv( normalMatLocation, 1, GL_FALSE, ToMat33(modelViewMat) )
    glBindVertexArray( icoVAObj )
    glDrawArrays( GL_TRIANGLES, 0, len(icoPosData) )

    # draw cube
    cubeMaterialBuffer.BindToTarget()
    modelViewMat = Multiply(viewMat, cubeModelMat)
    glUniformMatrix4fv( modelViewMatLocation, 1, GL_FALSE, modelViewMat )
    glUniformMatrix3fv( normalMatLocation, 1, GL_FALSE, ToMat33(modelViewMat) )
    glBindVertexArray( cubeVAObj )
    glDrawElements(GL_TRIANGLES, len(cubeIndices), GL_UNSIGNED_INT, cubeIndices)

    glutSwapBuffers()

def Fract(val): return val - math.trunc(val)
def CalcAng(currentTime, intervall): return Fract( (currentTime - startTime) / intervall ) * 360.0
def CalcMove(currentTime, intervall, range):
    pos = Fract( (currentTime - startTime) / intervall ) * 2.0
    pos = pos if pos < 1.0 else (2.0-pos)
    return range[0] + (range[1] - range[0]) * pos

# read shader program and compile shader
def CompileShader( sourceFileName, shaderStage ):
    with open( sourceFileName, 'r' ) as sourceFile:
        sourceCode = sourceFile.read()
    nameMap = { GL_VERTEX_SHADER: 'vertex', GL_FRAGMENT_SHADER: 'fragment' }    
    print( '\n%s shader code:' % nameMap.get(shaderStage, '') )
    print( sourceCode )
    shaderObj = glCreateShader( shaderStage )
    glShaderSource( shaderObj, sourceCode )
    glCompileShader( shaderObj )
    result = glGetShaderiv( shaderObj, GL_COMPILE_STATUS )
    if not (result):
        print( glGetShaderInfoLog( shaderObj ) )
        sys.exit()
    return shaderObj

# linke shader objects to shader program
def LinkProgram( shaderObjs ):
    shaderProgram = glCreateProgram()
    for shObj in shaderObjs:
        glAttachShader( shaderProgram, shObj )
    glLinkProgram( shaderProgram )
    result = glGetProgramiv( shaderProgram, GL_LINK_STATUS )
    if not (result):
        print( 'link error:' )
        print( glGetProgramInfoLog( shaderProgram ) )
        sys.exit()
    return shaderProgram

# create vertex array object
def CreateVAO( dataArrays ):
    noOfBuffers = len(dataArrays)
    buffers = glGenBuffers(noOfBuffers)
    newVAObj = glGenVertexArrays( 1 )
    glBindVertexArray( newVAObj )
    for inx in range(0, noOfBuffers):
        vertexSize, dataArr = dataArrays[inx]
        arr = np.array( dataArr, dtype='float32' )
        glBindBuffer( GL_ARRAY_BUFFER, buffers[inx] )
        glBufferData( GL_ARRAY_BUFFER, arr, GL_STATIC_DRAW )
        glEnableVertexAttribArray( inx )
        glVertexAttribPointer( inx, vertexSize, GL_FLOAT, GL_FALSE, 0, None )
    return newVAObj 

# representation of a uniform block
class UniformBlock:
    def __init__(self, shaderProg, name):
        self.shaderProg = shaderProg 
        self.name = name
        def Link(self, bindingPoint):
        self.bindingPoint = bindingPoint
        self.noOfUniforms = glGetProgramiv(self.shaderProg, GL_ACTIVE_UNIFORMS)
        self.maxUniformNameLen = glGetProgramiv(self.shaderProg, GL_ACTIVE_UNIFORM_MAX_LENGTH)
        self.index = glGetUniformBlockIndex(self.shaderProg, self.name)
        intData = np.zeros(1, dtype=int)
        glGetActiveUniformBlockiv(self.shaderProg, self.index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, intData)
        self.count = intData[0]
        self.indices = np.zeros(self.count, dtype=int)
        glGetActiveUniformBlockiv(self.shaderProg, self.index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, self.indices)
        self.offsets = np.zeros(self.count, dtype=int)
        glGetActiveUniformsiv(self.shaderProg, self.count, self.indices, GL_UNIFORM_OFFSET, self.offsets)
        self.size = 0
        strLengthData = np.zeros(1, dtype=int)
        arraysizeData = np.zeros(1, dtype=int)
        typeData = np.zeros(1, dtype='uint32')
        nameData = np.chararray(self.maxUniformNameLen+1)
        self.namemap = {}
        self.dataSize = 0 
        for inx in range(0, len(self.indices)):
            glGetActiveUniform( self.shaderProg, self.indices[inx], self.maxUniformNameLen, strLengthData, arraysizeData,     typeData, nameData.data )
            name = nameData.tostring()[:strLengthData[0]]
            self.namemap[name] = inx
            self.dataSize = max(self.dataSize, self.offsets[inx] + arraysizeData * 16) 
        glUniformBlockBinding(self.shaderProg, self.index, self.bindingPoint)
        print('\nuniform block %s size:%4d' % (self.name, self.dataSize))
        for uName in self.namemap:
            print( '    %-40s index:%2d    offset:%4d' % (uName, self.indices[self.namemap[uName]], self.offsets    [self.namemap[uName]]) ) 

# representation of a uniform block buffer
class UniformBlockBuffer:
    def __init__(self, ub):
        self.namemap = ub.namemap
        self.offsets = ub.offsets
        self.bindingPoint = ub.bindingPoint
        self.object = glGenBuffers(1)
        self.dataSize = ub.dataSize
        glBindBuffer(GL_UNIFORM_BUFFER, self.object)
        dataArray = np.zeros(self.dataSize//4, dtype='float32')
        glBufferData(GL_UNIFORM_BUFFER, self.dataSize, dataArray, GL_DYNAMIC_DRAW)
    def BindToTarget(self):
        glBindBuffer(GL_UNIFORM_BUFFER, self.object)
        glBindBufferBase(GL_UNIFORM_BUFFER, self.bindingPoint, self.object)
    def BindDataFloat(self, name, dataArr):
        glBindBuffer(GL_UNIFORM_BUFFER, self.object)
        dataArray = np.array(dataArr, dtype='float32')
        glBufferSubData(GL_UNIFORM_BUFFER, self.offsets[self.namemap[name]], len(dataArr)*4, dataArray)

def Translate(matA, trans):
    matB = np.copy(matA)
    for i in range(0, 4): matB[3,i] = matA[0,i] * trans[0] + matA[1,i] * trans[1] + matA[2,i] * trans[2] + matA[3,i] 
    return matB

def Scale(matA, s):
    matB = np.copy(matA)
    for i0 in range(0, 3):
        for i1 in range(0, 4): matB[i0,i1] = matA[i0,i1] * s[i0] 
    return matB

def RotateHlp(matA, angDeg, a0, a1):
    matB = np.copy(matA)
    ang = math.radians(angDeg)
    sinAng, cosAng = math.sin(ang), math.cos(ang)
    for i in range(0, 4):
        matB[a0,i] = matA[a0,i] * cosAng + matA[a1,i] * sinAng
        matB[a1,i] = matA[a0,i] * -sinAng + matA[a1,i] * cosAng
    return matB

def RotateX(matA, angDeg): return RotateHlp(matA, angDeg, 1, 2)
def RotateY(matA, angDeg): return RotateHlp(matA, angDeg, 2, 0)
def RotateZ(matA, angDeg): return RotateHlp(matA, angDeg, 0, 1)
def RotateView(matA, angDeg): return RotateZ(RotateY(RotateX(matA, angDeg[0]), angDeg[1]), angDeg[2])

def Multiply(matA, matB):
    matC = np.copy(matA)
    for i0 in range(0, 4):
        for i1 in range(0, 4):
            matC[i0,i1] = matB[i0,0] * matA[0,i1] + matB[i0,1] * matA[1,i1] + matB[i0,2] * matA[2,i1] + matB[i0,3] * matA    [3,i1]    
    return matC

def ToMat33(mat44):
    mat33 = np.matrix(np.identity(3), copy=False, dtype='float32')
    for i0 in range(0, 3):
        for i1 in range(0, 3): mat33[i0, i1] = mat44[i0, i1]
    return mat33

def TransformVec4(vecA,mat44):
    vecB = np.zeros(4, dtype='float32')
    for i0 in range(0, 4):
        vecB[i0] = vecA[0] * mat44[0,i0] + vecA[1] * mat44[1,i0] + vecA[2] * mat44[2,i0]  + vecA[3] * mat44[3,i0]
    return vecB

def Perspective(fov, aspectRatio, near, far):
    fn, f_n = far + near, far - near
    r, t = aspectRatio, 1.0 / math.tan( math.radians(fov) / 2.0 )
    return np.matrix( [ [t/r,0,0,0], [0,t,0,0], [0,0,-fn/f_n,-2.0*far*near/f_n], [0,0,-1,0] ] )

def AddToBuffer( buffer, data, count=1 ): 
    for inx_c in range(0, count):
        for inx_s in range(0, len(data)): buffer.append( data[inx_s] ) 

# initialize glut
glutInit()

# create window
wndW, wndH = 800, 600
glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH)
glutInitWindowPosition(0, 0)
glutInitWindowSize(wndW, wndH)
wndID = glutCreateWindow(b'OGL window') 
glutDisplayFunc(OnDraw) 
glutIdleFunc(OnDraw)

# define tetrahedron vertex array opject
tetPts = [ (0.0, 0.0, 1.0), (0.0, -sin120, -0.5), (sin120 * sin120, 0.5 * sin120, -0.5), (-sin120 * sin120, 0.5 * sin120,     -0.5) ]
tetCol = [ [1.0, 0.0, 0.0], [1.0, 1.0, 0.0], [0.0, 0.0, 1.0], [0.0, 1.0, 0.0], ]
tetInxdices = [ 0, 1, 2, 0, 2, 3, 0, 3, 1, 1, 3, 2 ]
tetPosData = []
for inx in tetInxdices: AddToBuffer( tetPosData, tetPts[inx] )
tetNVData = []
for inx_nv in range(0, len(tetInxdices) // 3):
    nv = [0.0, 0.0, 0.0]
    for inx_p in range(0, 3): 
        for inx_s in range(0, 3): nv[inx_s] += tetPts[ tetInxdices[inx_nv*3 + inx_p] ][inx_s]
    AddToBuffer( tetNVData, nv, 3 )
tetColData = []
for inx_col in range(0, len(tetInxdices) // 3): AddToBuffer( tetColData, tetCol[inx_col % len(tetCol)], 3 )
tetVAObj = CreateVAO( [ (3, tetPosData), (3, tetNVData), (3, tetColData) ] )

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
for inx in icoIndices: AddToBuffer( icoNVData, icoPts[inx] )
#for inx_nv in range(0, len(icoIndices) // 3):
#    nv = [0.0, 0.0, 0.0]
#    for inx_p in range(0, 3): 
#        for inx_s in range(0, 3): nv[inx_s] += icoPts[ icoIndices[inx_nv*3 + inx_p] ][inx_s]
#    AddToBuffer( icoNVData, nv, 3 )
icoColData = []
for inx_col in range(0, len(icoIndices) // 3): AddToBuffer( icoColData, icoCol[inx_col % len(icoCol)], 3 )
icoVAObj = CreateVAO( [ (3, icoPosData), (3, icoNVData), (3, icoColData) ] )

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
    for inx_s in [0, 1, 2, 0, 2, 3]: cubeIndices.append( inx * 4 + inx_s )
cubeVAObj = CreateVAO( [ (3, cubePosData), (3, cubeNVData), (3, cubeColData) ] )

# load, compile and link shader
shaderProgram = LinkProgram( [
        CompileShader( 'ibub.vert', GL_VERTEX_SHADER ), 
        CompileShader( 'ibub.frag', GL_FRAGMENT_SHADER )
    ] )
# get unifor locations
projectionMatLocation = glGetUniformLocation(shaderProgram, "u_projectionMat44")
modelViewMatLocation  = glGetUniformLocation(shaderProgram, "u_modelViewMat44")
normalMatLocation     = glGetUniformLocation(shaderProgram, "u_normalMat33")
# linke uniform blocks
ubMaterial = UniformBlock(shaderProgram, "UB_material")
ubLightSource = UniformBlock(shaderProgram, "UB_lightSource")
ubMaterial.Link(1)
ubLightSource.Link(2)

# create uniform block buffers
lightSourceBuffer = UniformBlockBuffer(ubLightSource)
lightSourceBuffer.BindDataFloat(b'u_lightSource.ambient', [0.1, 0.1, 0.1, 1.0])
lightSourceBuffer.BindDataFloat(b'u_lightSource.diffuse', [0.4, 0.4, 0.4, 1.0])
lightSourceBuffer.BindDataFloat(b'u_lightSource.specular', [1.0, 1.0, 1.0, 1.0])

tetMaterialBuffer = UniformBlockBuffer(ubMaterial)
tetMaterialBuffer.BindDataFloat(b'u_roughness', [0.3])
tetMaterialBuffer.BindDataFloat(b'u_fresnel0', [0.5])
tetMaterialBuffer.BindDataFloat(b'u_specularTint',[1.0, 1.0, 1.0, 0.7])

icoMaterialBuffer = UniformBlockBuffer(ubMaterial)
icoMaterialBuffer.BindDataFloat(b'u_roughness', [0.1])
icoMaterialBuffer.BindDataFloat(b'u_fresnel0', [0.2])
icoMaterialBuffer.BindDataFloat(b'u_specularTint',[1.0, 1.0, 1.0, 0.7])

cubeMaterialBuffer = UniformBlockBuffer(ubMaterial)
cubeMaterialBuffer.BindDataFloat(b'u_roughness', [0.5])
cubeMaterialBuffer.BindDataFloat(b'u_fresnel0', [0.3])
cubeMaterialBuffer.BindDataFloat(b'u_specularTint',[1.0, 1.0, 1.0, 0.7])

# start main loop
startTime = time()
glutMainLoop()
```

<br/><hr/>

<a href="https://stackexchange.com/users/7322082/rabbid76"><img src="https://stackexchange.com/users/flair/7322082.png" width="208" height="58" alt="profile for Rabbid76 on Stack Exchange, a network of free, community-driven Q&amp;A sites" title="profile for Rabbid76 on Stack Exchange, a network of free, community-driven Q&amp;A sites" /></a>
