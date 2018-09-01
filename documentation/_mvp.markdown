# GLSL documentation : Put a texture on the model and use a texture matrix in OGL 4.0 GLSL

[https://stackoverflow.com/documentation/glsl/5480/getting-started-with-glsl/31873/put-a-texture-on-the-model-and-use-a-texture-matrix-in-ogl-4-0-glsl#t=201707171708249001278]

A simple OGL 4.0 GLSL shader program that shows the use of a model, view, and projection matrix.
The program is executed with a python script. To run the script, PyOpenGL and NumPy must be installed.

Refer to "first"

- Projection matrix:
  The projection matrix describes the mapping of a pinhole camera from 3D points in the world to 2D points of
  the viewport.
  In this example we use a projection matrix with a field of view of 90 degrees.

- View matrix:
  The view matrix defines the *eye* position and the viewing direction on the scene.
  In this example we are moving circular around the scene keeping a viewing direction to the center of the scene.
  In the coordinat system on the viewport, the X-axis points to the left, the Y-axis up and the Z-axis out of the view (Note in a right hand system the Z-Axis is the cross product of the X-Axis and the Y-Axis).

- Model matrix:
  The model matrix defines the location and the relative size of an object in the scene.
  In this example the model matrices move the objects up and down.

## Vertex shader

*mvp.vert*

```glsl
#version 400

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inCol;

out vec3 vertCol;

uniform mat4 projectionMat44;
uniform mat4 viewMat44;
uniform mat4 modelMat44;

void main()
{
    vertCol = inCol;
    vec4 modolPos = modelMat44 * vec4( inPos, 1.0 );
    vec4 viewPos = viewMat44 * modolPos;
    gl_Position = projectionMat44 * viewPos;
}
```

## Fragment shader

*mvp.frag*

```glsl
#version 400

in vec3 vertCol;

out vec4 fragColor;

void main()
{
    fragColor = vec4( vertCol, 1.0 );
}
```

## Python script 

```python
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
import numpy as np
from time import time
import math
import sys

# draw event
def OnDraw():
    currentTime = time()
    wndW = glutGet(GLUT_WINDOW_WIDTH)
    wndH = glutGet(GLUT_WINDOW_HEIGHT)
    # set up projection matrix
    prjMat = Perspective( 90.0, wndW/wndH, 0.5, 100.0) 
    # set up view matrix
    viewMat = Translate( np.matrix(np.identity(4), copy=False, dtype='float32'), np.array( [0.0, 0.0, -8.0] ) )
    viewMat = RotateView( viewMat, [10.0, CalcAng( currentTime, 10.0 ), 0.0] )

    # set up tetrahedron model matrix
    tetModelMat = np.matrix(np.identity(4), copy=False, dtype='float32')
    tetModelMat = RotateX( tetModelMat, -90.0 )
    tetModelMat = Scale( tetModelMat, np.repeat( 2.0, 3 ) )
    tetModelMat = Translate( tetModelMat, np.array( [-2.0, 0.0, CalcMove(currentTime, 6.0, [-1.0, 1.0])] ) )

    # set up icosahedron model matrix
    icoModelMat = np.matrix(np.identity(4), copy=False, dtype='float32')
    icoModelMat = RotateX( icoModelMat, -90.0 )
    icoModelMat = Scale( icoModelMat, np.repeat( 2.0, 3 ) )
    icoModelMat = Translate( icoModelMat, np.array( [2.0, 0.0, CalcMove(currentTime, 6.0, [1.0, -1.0])] ) )

    # set up attributes and shader program
    glEnable( GL_DEPTH_TEST )
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT )
    glUseProgram( shaderProgram )
    glUniformMatrix4fv( projectionMatLocation, 1, GL_FALSE, prjMat )
    glUniformMatrix4fv( viewMatLocation, 1, GL_FALSE, viewMat )

    # draw tetrahedron
    glUniformMatrix4fv( modelMatLocation, 1, GL_FALSE, tetModelMat )
    glBindVertexArray( tetVAObj )
    glDrawElements(GL_TRIANGLES, len(tetIndices), GL_UNSIGNED_INT, tetIndices)

    # draw icosahedron
    glUniformMatrix4fv( modelMatLocation, 1, GL_FALSE, icoModelMat )
    glBindVertexArray( icoVAObj )
    glDrawArrays( GL_TRIANGLES, 0, len(icoPosData) )

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

# link shader objects to shader program
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

def Perspective(fov, aspectRatio, near, far):
    fn, f_n = far + near, far - near
    r, t = aspectRatio, 1.0 / math.tan( math.radians(fov) / 2.0 )
    return np.matrix( [ [t/r,0,0,0], [0,t,0,0], [0,0,-fn/f_n,-2.0*far*near/f_n], [0,0,-1,0] ] )

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
sin120 = 0.8660254
tetPosData = [ 0.0, 0.0, 1.0, 0.0, -sin120, -0.5, sin120 * sin120, 0.5 * sin120, -0.5, -sin120 * sin120, 0.5 * sin120,     -0.5 ]
tetColData = [ 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, ]
tetIndices = [ 0, 1, 2, 0, 2, 3, 0, 3, 1, 1, 3, 2 ]
tetVAObj = CreateVAO( [ (3, tetPosData), (3, tetColData) ] )
tetInxArr = np.array( tetIndices, dtype='uint' )

# define icosahedron vertex array opject
icoPts = [
    [ 0.000,  0.000,  1.000], [ 0.894,  0.000,  0.447], [ 0.276,  0.851,  0.447], [-0.724,  0.526,  0.447],
    [-0.724, -0.526,  0.447], [ 0.276, -0.851,  0.447], [ 0.724,  0.526, -0.447], [-0.276,  0.851, -0.447], 
    [-0.894,  0.000, -0.447], [-0.276, -0.851, -0.447], [ 0.724, -0.526, -0.447], [ 0.000,  0.000, -1.000] ]
icoCol = [ [1.0, 0.0, 0.0], [0.0, 0.0, 1.0], [1.0, 1.0, 0.0], [0.0, 1.0, 0.0], [1.0, 0.5, 0.0], [1.0, 0.0, 1.0] ]
icoIndices = [
    2,  0,  1,  3,  0,  2,  4,  0,  3,  5,  0,  4,  1,  0,  5, 11,  7,  6, 11,  8,  7, 11,  9,  8, 11, 10,  9, 11,  6, 10, 
    1,  6,  2,  2,  7,  3,  3,  8,  4,  4,  9,  5,  5, 10,  1,  2,  6,  7,  3,  7,  8,  4,  8,  9,  5,  9, 10,  1, 10,  6  ]
icoPosData = []
for inx in icoIndices:
    for inx_s in range(0, 3):
        icoPosData.append( icoPts[inx][inx_s] )
icoColData = []
for inx in range(0, len(icoPosData) // 9):
    inx_col = inx % len(icoCol)
    for inx_p in range(0, 3):
        for inx_s in range(0, 3):
                icoColData.append( icoCol[inx_col][inx_s] )
icoVAObj = CreateVAO( [ (3, icoPosData), (3, icoColData) ] )

# load, compile and link shader
shaderProgram = LinkProgram( [
        CompileShader( 'mvp.vert', GL_VERTEX_SHADER ), 
        CompileShader( 'mvp.frag', GL_FRAGMENT_SHADER )
    ] )
projectionMatLocation = glGetUniformLocation(shaderProgram, "projectionMat44")
viewMatLocation = glGetUniformLocation(shaderProgram, "viewMat44")
modelMatLocation = glGetUniformLocation(shaderProgram, "modelMat44")

# start main loop
startTime = time()
glutMainLoop()
```

<br/><hr/>

<a href="https://stackexchange.com/users/7322082/rabbid76"><img src="https://stackexchange.com/users/flair/7322082.png" width="208" height="58" alt="profile for Rabbid76 on Stack Exchange, a network of free, community-driven Q&amp;A sites" title="profile for Rabbid76 on Stack Exchange, a network of free, community-driven Q&amp;A sites" /></a>
