[![StackOverflow](https://stackexchange.com/users/flair/7322082.png)](https://stackoverflow.com/users/5577765/rabbid76?tab=profile)

---

<!-- TOC -->

- [Creating geometry using a geometry shader](#creating-geometry-using-a-geometry-shader)
        - [Vertex shader](#vertex-shader)
        - [Geometry shader](#geometry-shader)
        - [Fragment shader](#fragment-shader)
        - [Pyhton script](#pyhton-script)

<!-- /TOC -->

# Creating geometry using a geometry shader

A simple OGL 4.0 GLSL shader program that shows the use of geometry shaders.
The program is executed with a python script. To run the script, PyOpenGL and NumPy must be installed.

In this example, the entire geometry (a cylinder) is generated in the geometry shader.

![preview](image/geometry_001.gif)

### Vertex shader

*geo.vert*

```glsl
#version 400

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;

out TVertexData
{
    mat3 orientationMat;
} outData;

void main()
{
    vec3 normal   = normalize( inNormal );
    vec3 tangent  = normalize( inTangent );
    vec3 binormal = cross( tangent, normal );

    outData.orientationMat = mat3( normal, cross( binormal, normal ), binormal );
    gl_Position = vec4( inPos, 1.0 );
}
```

### Geometry shader

*geo.geo*

```glsl
#version 400

layout( invocations = 3 ) in;
layout( points ) in;
layout( triangle_strip, max_vertices = 72 ) out;

in TVertexData
{
    mat3 orientationMat;
} inData[];

out TGeometryData
{
    vec3 pos;
    vec3 nv;
    vec3 col;
} outData;

uniform mat4 u_projectionMat44;
uniform mat4 u_viewMat44;
uniform mat4 u_modelMat44;

void NewVertex( in vec3 pt, in mat4 transMat )
{
    vec4 viewPos = transMat * vec4( pt, 1.0 );
    outData.pos = viewPos.xyz / viewPos.w;
    gl_Position = u_projectionMat44 * viewPos;
    EmitVertex();
}

const int circumferenceTile = 16;

void main()
{
    vec4 origin = gl_in[0].gl_Position;
    origin /= origin.w;
    mat4 orintationMat = mat4( vec4( inData[0].orientationMat[0], 0.0 ),
                                vec4( inData[0].orientationMat[1], 0.0 ),
                                vec4( inData[0].orientationMat[2], 0.0 ),
                                origin );
    mat4 modelViewMat = u_viewMat44 * u_modelMat44 * orintationMat;
    mat3 normalMat = mat3( modelViewMat );

    outData.col = vec3( 0.5, 0.7, 0.6 );

    if ( gl_InvocationID == 0 ) // top of the cylinder
    {
        outData.nv  = normalMat * vec3(0.0, 0.0, 1.0);
        for ( int inx = 0; inx < circumferenceTile; inx += 2 )
        {
            float ang1 = 2.0 * 3.14159 * float(inx) / float(circumferenceTile);
            float ang2 = 2.0 * 3.14159 * float(inx+1) / float(circumferenceTile);
            vec2 actPt1 = vec2( sin(ang1), cos(ang1) );
            vec2 actPt2 = vec2( sin(ang2), cos(ang2) );

            NewVertex( vec3(actPt1.xy, 1.0), modelViewMat );
            NewVertex( vec3(0.0, 0.0, 1.0), modelViewMat );
            NewVertex( vec3(actPt2.xy, 1.0), modelViewMat );
        }
        NewVertex( vec3(0.0, 0.0, 1.0), modelViewMat );
        NewVertex( vec3(0.0, 1.0, 1.0), modelViewMat );
        EndPrimitive();
    }

    if ( gl_InvocationID == 1 ) // bottom of the cylinder  
    {
        outData.nv  = normalMat * vec3(0.0, 0.0, -1.0);
        vec2 prevPt = vec2( 0.0, 1.0 );
        for ( int inx = circumferenceTile; inx >= 0; inx -= 2 )
        {
            float ang1 = 2.0 * 3.14159 * float(inx) / float(circumferenceTile);
            float ang2 = 2.0 * 3.14159 * float(inx-1) / float(circumferenceTile);
            vec2 actPt1 = vec2( sin(ang1), cos(ang1) );
            vec2 actPt2 = vec2( sin(ang2), cos(ang2) );

            NewVertex( vec3(actPt1.xy, -1.0), modelViewMat );
            NewVertex( vec3(0.0, 0.0, -1.0), modelViewMat );
            NewVertex( vec3(actPt2.xy, -1.0), modelViewMat );
        }
        NewVertex( vec3(0.0, 0.0, -1.0), modelViewMat );
        NewVertex( vec3(0.0, 1.0, -1.0), modelViewMat );
        EndPrimitive();
    }

    if ( gl_InvocationID == 2 ) // hull of the cylinder
    {
        vec2 prevPt = vec2( 0.0, 1.0 );
        for ( int inx = 1; inx <= circumferenceTile; ++ inx )
        {
            float ang = 2.0 * 3.14159 * float(inx) / float(circumferenceTile);
            vec2 actPt = vec2( sin(ang), cos(ang) );

            outData.nv = normalMat * vec3(prevPt, 0.0);
            NewVertex( vec3(prevPt.xy, -1.0), modelViewMat );
            outData.nv = normalMat * vec3(actPt, 0.0);
            NewVertex( vec3(actPt.xy, -1.0), modelViewMat );
            outData.nv = normalMat * vec3(prevPt, 0.0);
            NewVertex( vec3(prevPt.xy, 1.0), modelViewMat );
            outData.nv = normalMat * vec3(actPt, 0.0);
            NewVertex( vec3(actPt.xy, 1.0), modelViewMat );

            prevPt = actPt;
        }
        EndPrimitive();
    }
}
```

### Fragment shader

*geo.frag*

```glsl
#version 400

in TGeometryData
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

float Fresnel_Schlick( float theta )
{
    float m = clamp( 1.0 - theta, 0.0, 1.0 );
    float m2 = m * m;
    return m2 * m2 * m; // pow( m, 5.0 )
}

vec3 LightModel( vec3 esPt, vec3 esPtNV, vec3 col, vec4 specularTint, float roughness, float fresnel0 )
{
    vec3  esVLight      = normalize( -u_lightSource.dir.xyz );
    vec3  esVEye        = normalize( -esPt );
    vec3  halfVector    = normalize( esVEye + esVLight );
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
    // Schlick approximation
    float fresnel = fresnel0 + ( 1.0 - fresnel0 ) * Fresnel_Schlick( HdotL );
    // Beckmann distribution
    float distribution = max( 0.0, exp( ( NdotH2 - 1.0 ) / ( m2 * NdotH2 ) ) / ( 3.14159265 * m2 * NdotH2 * NdotH2 ) );
    // Torrance-Sparrow geometric term
    float geometric_att = min( 1.0, min( 2.0 * NdotH * NdotV_clamped / HdotL, 2.0 * NdotH * NdotL_clamped / HdotL ) );
    // Microfacet bidirectional reflectance distribution function 
    float k_specular = fresnel * distribution * geometric_att / ( 4.0 * NdotL_clamped * NdotV_clamped );

    vec3 lightColor = col.rgb * u_lightSource.ambient.rgb +
                    max( 0.0, k_diffuse ) * col.rgb * u_lightSource.diffuse.rgb +
                    max( 0.0, k_specular ) * mix( col.rgb, specularTint.rgb, specularTint.a ) *     u_lightSource.specular.rgb;
    return lightColor;
}

void main()
{
    vec3 lightCol = LightModel( inData.pos, inData.nv, inData.col, u_specularTint, u_roughness, u_fresnel0 );
    fragColor = vec4( clamp( lightCol, 0.0, 1.0 ), 1.0 );
}
```

### Pyhton script

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
    viewMat = np.matrix(np.identity(4), copy=False, dtype='float32')
    viewMat = Translate( viewMat, np.array( [0.0, 0.0, -12.0] ) )
    viewMat = RotateView( viewMat, [30.0, comeraRotAng if rotateCamera else 0.0, 0.0] )

    # set up light source
    lightSourceBuffer.BindDataFloat(b'u_lightSource.dir', TransformVec4([-0.1, 1.0, -5.0, 0.0], viewMat) )

    # set up the model matrix
    modelMat = np.matrix(np.identity(4), copy=False, dtype='float32')
    if not rotateCamera: modelMat = RotateY( modelMat, comeraRotAng )
    modelMat = Scale( modelMat, np.repeat( 4, 3 ) )
    #modelMat = Translate( modelMat, np.array( [0.0, 0.0, 1.0] ) )
    #modelMat = RotateY( modelMat, CalcAng( currentTime, 20.0 ) )
    modelMat = RotateX( modelMat, CalcAng( currentTime, 9.0 ) )

    # set up attributes and shader program
    glEnable( GL_DEPTH_TEST )
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT )
    glUseProgram( shaderProgram )
    glUniformMatrix4fv( projectionMatLocation, 1, GL_FALSE, prjMat )
    glUniformMatrix4fv( viewMatLocation, 1, GL_FALSE, viewMat )
    lightSourceBuffer.BindToTarget()

    # draw point
    materialBuffer.BindToTarget()
    glUniformMatrix4fv( modelMatLocation, 1, GL_FALSE, modelMat )
    glBindVertexArray( pointVAObj )
    glDrawArrays( GL_POINTS, 0, 1 )

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
    nameMap = { GL_VERTEX_SHADER: 'vertex', GL_GEOMETRY_SHADER: 'geometry', GL_FRAGMENT_SHADER: 'fragment' } 
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

# define location vertex array opject
pointVAObj = CreateVAO( [ (3, [0.0, 0.0, 0.0] ), (3, [0.0, 0.0, -1.0]), (3, [1.0, 0.0, 0.0]) ] )

# load, compile and link shader
shaderProgram = LinkProgram( [
        CompileShader( 'geo.vert', GL_VERTEX_SHADER ), 
        CompileShader( 'geo.geo', GL_GEOMETRY_SHADER ), 
        CompileShader( 'geo.frag', GL_FRAGMENT_SHADER )
    ] )
# get unifor locations
projectionMatLocation = glGetUniformLocation(shaderProgram, "u_projectionMat44")
viewMatLocation       = glGetUniformLocation(shaderProgram, "u_viewMat44")
modelMatLocation      = glGetUniformLocation(shaderProgram, "u_modelMat44")
# linke uniform blocks
ubMaterial = UniformBlock(shaderProgram, "UB_material")
ubLightSource = UniformBlock(shaderProgram, "UB_lightSource")
ubMaterial.Link(1)
ubLightSource.Link(2)

# create uniform block buffers
lightSourceBuffer = UniformBlockBuffer(ubLightSource)
lightSourceBuffer.BindDataFloat(b'u_lightSource.ambient', [0.2, 0.2, 0.2, 1.0])
lightSourceBuffer.BindDataFloat(b'u_lightSource.diffuse', [0.2, 0.2, 0.2, 1.0])
lightSourceBuffer.BindDataFloat(b'u_lightSource.specular', [1.0, 1.0, 1.0, 1.0])

materialBuffer = UniformBlockBuffer(ubMaterial)
materialBuffer.BindDataFloat(b'u_roughness', [0.5])
materialBuffer.BindDataFloat(b'u_fresnel0', [0.2])
materialBuffer.BindDataFloat(b'u_specularTint',[1.0, 0.5, 0.5, 0.8])

# start main loop
startTime = time()
glutMainLoop()
```

<br/><hr/>

<a href="https://stackexchange.com/users/7322082/rabbid76"><img src="https://stackexchange.com/users/flair/7322082.png" width="208" height="58" alt="profile for Rabbid76 on Stack Exchange, a network of free, community-driven Q&amp;A sites" title="profile for Rabbid76 on Stack Exchange, a network of free, community-driven Q&amp;A sites" /></a>
