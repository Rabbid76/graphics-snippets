<!-- TOC -->

- [Changing the geometry with tessellation shaders](#changing-the-geometry-with-tessellation-shaders)
  - [Vertex shader](#vertex-shader)
  - [Tessellation control shader](#tessellation-control-shader)
  - [Tessellation evaluation shader](#tessellation-evaluation-shader)
  - [Fragment shader](#fragment-shader)
    - [Python script](#python-script)

<!-- /TOC -->

# Changing the geometry with tessellation shaders

A simple OGL 4.0 GLSL shader program that shows that shows how to add details with tessellation shader to the geometry.
The program is executed with a python script. To run the script, PyOpenGL and NumPy must be installed.

The basic mesh in this example is an icosahedron that consists of 20 triangles. The tessellation control shader defines how each triangle is divided into a set of many small parts.
When tessellating a triangle, the generated data are barycentric coordinates based on the original triangle. The tessellation evaluation shader generates new geometry from the data obtained in this way. In this example, each triangle gets a peak in the middle, which rises outward from the center of the icosahedron. In this way a much more complex geometry is generated than the original icosahedron.

![preview](image/tessellation_001.gif)

## Vertex shader

*tess.vert*

```glsl
#version 400

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNV;

out TVertexData
{
    vec3 pos;
    vec3 nv;
} outData;

uniform mat4 u_projectionMat44;
uniform mat4 u_modelViewMat44;
uniform mat3 u_normalMat33;

void main()
{
    vec4 viewPos = u_modelViewMat44 * vec4( inPos, 1.0 );

    outData.pos = viewPos.xyz / viewPos.w;
    outData.nv  = u_normalMat33 * normalize( inNV );

    gl_Position = u_projectionMat44 * viewPos;
}
```

## Tessellation control shader

*tess.tctrl*

```glsl
#version 400

layout( vertices=3 ) out;

in TVertexData
{
    vec3 pos;
    vec3 nv;
} inData[];

out TVertexData
{
    vec3 pos;
    vec3 nv;
} outData[];

void main()
{
    outData[gl_InvocationID].pos = inData[gl_InvocationID].pos;
    outData[gl_InvocationID].nv  = inData[gl_InvocationID].nv;

    if ( gl_InvocationID == 0 )
    {
        gl_TessLevelOuter[0] = 10.0;
        gl_TessLevelOuter[1] = 10.0;
        gl_TessLevelOuter[2] = 10.0;
        gl_TessLevelInner[0] = 10.0;
    }
}
```

## Tessellation evaluation shader

*tess.teval*

```glsl
#version 400

layout(triangles, equal_spacing, ccw) in;

in TVertexData
{
    vec3 pos;
    vec3 nv;
} inData[];

out TTessData
{
    vec3  pos;
    vec3  nv;
    float height;
} outData;

uniform mat4 u_projectionMat44;

void main()
{
    float sideLen[3] = float[3]
    (
        length( inData[1].pos - inData[0].pos ),
        length( inData[2].pos - inData[1].pos ),
        length( inData[0].pos - inData[2].pos )
    );
    float s = ( sideLen[0] + sideLen[1] + sideLen[2] ) / 2.0;
    float rad = sqrt( (s - sideLen[0]) * (s - sideLen[1]) * (s - sideLen[2]) / s );

    vec3 cpt = ( inData[0].pos + inData[1].pos + inData[2].pos ) / 3.0;
    vec3 pos = inData[0].pos * gl_TessCoord.x + inData[1].pos * gl_TessCoord.y + inData[2].pos * gl_TessCoord.z;
    vec3 nv  = normalize( inData[0].nv * gl_TessCoord.x + inData[1].nv * gl_TessCoord.y + inData[2].nv * gl_TessCoord.z );

    float cptDist      = length( cpt - pos );
    float sizeRelation = 1.0 - min( rad, cptDist ) / rad;
    float height       = pow( sizeRelation, 2.0 );

    outData.pos    = pos + nv * height * rad;
    outData.nv     = mix( nv, normalize( pos - cpt ), height );
    outData.height = height;

    gl_Position = u_projectionMat44 * vec4( outData.pos, 1.0 );
}
```

## Fragment shader

*tess.frag*

```glsl
#version 400

in TTessData
{
    vec3  pos;
    vec3  nv;
    float height;
} inData;

out vec4 fragColor;

uniform sampler2D u_texture;

uniform UB_material
{
    float u_roughness;
    float u_fresnel0;
    vec4  u_color;
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

float Fresnel_Schlick( in float theta );
vec3 LightModel( in vec3 esPt, in vec3 esPtNV, in vec3 col, in vec4 specularTint, in float roughness, in float fresnel0 );

void main()
{
    vec3 col = mix( u_color.rgb, vec3( 1.0, 1.0, 1.0 ), inData.height );
    vec3 lightCol = LightModel( inData.pos, inData.nv, col, u_specularTint, u_roughness, u_fresnel0 );
    fragColor = vec4( clamp( lightCol, 0.0, 1.0 ), 1.0 );
}

float Fresnel_Schlick( in float theta )
{
    float m = clamp( 1.0 - theta, 0.0, 1.0 );
    float m2 = m * m;
    return m2 * m2 * m; // pow( m, 5.0 )
}

vec3 LightModel( in vec3 esPt, in vec3 esPtNV, in vec3 col, in vec4 specularTint, in float roughness, in float fresnel0 )
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
                    max( 0.0, k_specular ) * mix( col.rgb, specularTint.rgb, specularTint.a ) * u_lightSource.specular.rgb;
    return lightColor;
}
```

### Python script

```python
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
import numpy
from time import time
import math
import sys


def Cross( a, b ): return ( a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0], 0.0 )
def Dot( a, b ): return a[0]*b[0] + a[1]*b[1] + a[2]*b[2]
def Normalize( v ): 
    len = math.sqrt( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] )
    return (v[0] / len, v[1] / len, v[2] / len)

def IdentityMat44(): return numpy.matrix(numpy.identity(4), copy=False, dtype='float32')

class Camera:
    def __init__(self):
        self.pos    = (0, -1.6, 0.5)
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
    def LookAt(self):
        mz = Normalize( (self.pos[0]-self.target[0], self.pos[1]-self.target[1], self.pos[2]-self.target[2]) ) # inverse line of sight
        mx = Normalize( Cross( self.up, mz ) )
        my = Normalize( Cross( mz, mx ) )
        tx =  Dot( mx, self.pos )
        ty =  Dot( my, self.pos )
        tz = -Dot( mz, self.pos )
        return numpy.matrix( [ [mx[0], my[0], mz[0], 0], [mx[1], my[1], mz[1], 0], [mx[2], my[2], mz[2], 0], [tx, ty, tz, 1] ] )

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

# shader program object
class ShaderProgram:
    def __init__( self, shaderList ):
        shaderObjs = []
        for sh_info in shaderList: shaderObjs.append( self.CompileShader(sh_info[0], sh_info[1] ) )
        self.LinkProgram( shaderObjs )
        strLengthData = numpy.zeros(1, dtype=int)
        arraysizeData = numpy.zeros(1, dtype=int)
        typeData = numpy.zeros(1, dtype='uint32')
        nameData = numpy.chararray(glGetProgramiv( self.__prog, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH ) + 1)
        self.__attributeLocation = {}
        for i_attr in range(0, glGetProgramiv( self.__prog, GL_ACTIVE_ATTRIBUTES )):
            glGetActiveAttrib( self.__prog, i_attr, nameData.size-1, strLengthData, arraysizeData, typeData, nameData.data )
            name = nameData.tostring()[:strLengthData[0]]
            self.__attributeLocation[name] = glGetAttribLocation( self.__prog, name )
            print( "attribute  %-30s at loaction %d" % (name, self.__attributeLocation[name]) )
        nameData = numpy.chararray(glGetProgramiv( self.__prog, GL_ACTIVE_UNIFORM_MAX_LENGTH ) + 1)
        self.__unifomLocation = {}
        for i_attr in range(0, glGetProgramiv( self.__prog, GL_ACTIVE_UNIFORMS )):
            glGetActiveUniform( self.__prog, i_attr, nameData.size-1, strLengthData, arraysizeData, typeData, nameData.data )
            name = nameData.tostring()[:strLengthData[0]]
            self.__unifomLocation[name] = glGetUniformLocation( self.__prog, name )
            print( "uniform    %-30s at loaction %d" % (name, self.__unifomLocation[name]) )
        # glGetFragDataLocation( self.__prog, name )
    def Prog( self ):
        return self.__prog
    def Use(self):
        glUseProgram( self.__prog )
    def UniformLocation( self, name ):
        return self.__unifomLocation[name]
    def SetUniformI1( self, name, val ):
        if name in self.__unifomLocation: glUniform1i( self.__unifomLocation[name], val )
    def SetUniformF1( self, name, arr ):
        if name in self.__unifomLocation: glUniform1fv( self.__unifomLocation[name], 1, numpy.array( arr, dtype="float" ) )
    def SetUniformF2( self, name, arr ):
        if name in self.__unifomLocation: glUniform2fv( self.__unifomLocation[name], 1, numpy.array( arr, dtype="float" ) )
    def SetUniformF3( self, name, arr ):
        if name in self.__unifomLocation: glUniform3fv( self.__unifomLocation[name], 1, numpy.array( arr, dtype="float" ) )
    def SetUniformF4( self, name, arr ):
        if name in self.__unifomLocation: glUniform4fv( self.__unifomLocation[name], 1, numpy.array( arr, dtype="float" ) )
    def SetUniformM33( self, name, mat ):
        if name in self.__unifomLocation: glUniformMatrix3fv( self.__unifomLocation[name], 1, GL_FALSE, mat )
    def SetUniformM44( self, name, mat ):
        if name in self.__unifomLocation: glUniformMatrix4fv( self.__unifomLocation[name], 1, GL_FALSE, mat )
    # read shader program and compile shader
    def CompileShader(self, sourceFileName, shaderStage):
        with open( sourceFileName, 'r' ) as sourceFile:
            sourceCode = sourceFile.read()
        nameMap = { GL_COMPUTE_SHADER: 'compute', GL_VERTEX_SHADER: 'vertex', GL_TESS_CONTROL_SHADER: 'tessellation control', GL_TESS_EVALUATION_SHADER: 'tessellation evaluation',     GL_GEOMETRY_SHADER: 'geometry', GL_FRAGMENT_SHADER: 'fragment' }
        print( '\n%s shader code:' % nameMap.get( shaderStage, '' ) )
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
    def LinkProgram(self, shaderObjs):
        self.__prog = glCreateProgram()
        for shObj in shaderObjs: glAttachShader( self.__prog, shObj )
        glLinkProgram( self.__prog )
        result = glGetProgramiv( self.__prog, GL_LINK_STATUS )
        if not ( result ):
            print( 'link error:' )
            print( glGetProgramInfoLog( self.__prog ) )
            sys.exit()

# vertex array object
class VAObject:
    def __init__( self, dataArrays, indices = [], type = GL_TRIANGLES, patch_vertices = 3 ):
        self.__obj = glGenVertexArrays( 1 )
        self.__noOfIndices = len( indices )
        self.__indexArr = numpy.array( indices, dtype='uint' )
        self.__type = type
        self.__patch_vertices = patch_vertices
        self.__vertexSize = []
        self.__dataLength = []
        self.__noOfBuffers = len( dataArrays )
        self.__buffers = glGenBuffers( self.__noOfBuffers )
        glBindVertexArray( self.__obj )
        for i_buffer in range( 0, self.__noOfBuffers ):
            vertexSize, dataArr = dataArrays[i_buffer]
            self.__vertexSize.append( vertexSize )
            self.__dataLength.append( len( dataArr ) )
            glBindBuffer( GL_ARRAY_BUFFER, self.__buffers if self.__noOfBuffers == 1 else self.__buffers[i_buffer] )
            glBufferData( GL_ARRAY_BUFFER, numpy.array( dataArr, dtype='float32' ), GL_STATIC_DRAW )
            glEnableVertexAttribArray( i_buffer )
            glVertexAttribPointer( i_buffer, self.__vertexSize[i_buffer], GL_FLOAT, GL_FALSE, 0, None )
        self.__iBuffer = glGenBuffers( 1 )
        glBindBuffer( GL_ARRAY_BUFFER, 0 )
        if self.__noOfIndices > 0:
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self.__iBuffer )
            glBufferData( GL_ELEMENT_ARRAY_BUFFER, self.__indexArr, GL_STATIC_DRAW )
        glBindVertexArray( 0 )
    def DrawArray(self):
        glBindVertexArray( self.__obj )
        if self.__type == GL_PATCHES:
            glPatchParameteri( GL_PATCH_VERTICES, self.__patch_vertices )
        glDrawArrays( GL_PATCHES, 0, self.__dataLength[0] )
        glBindVertexArray( 0 )
    def Draw(self):
        if self.__noOfIndices == 0:
            self.DrawArray()
            return
        glBindVertexArray( self.__obj )
        #for i_buffer in range( 0, self.__noOfBuffers ):
        #    glBindBuffer( GL_ARRAY_BUFFER, self.__buffers if self.__noOfBuffers == 1 else self.__buffers[i_buffer] )
        #    glEnableVertexAttribArray( i_buffer )
        #    glVertexAttribPointer( i_buffer, self.__vertexSize[i_buffer], GL_FLOAT, GL_FALSE, 0, None )
        #glDrawElements( self.__type, self.__noOfIndices, GL_UNSIGNED_INT, self.__indexArr )
        if self.__type == GL_PATCHES:
            glPatchParameteri( GL_PATCH_VERTICES, self.__patch_vertices )
        glDrawElements( self.__type, self.__noOfIndices, GL_UNSIGNED_INT, None )
        glBindVertexArray( 0 )

# glut window
class Window:
    def __init__( self, cx, cy ):
        self.__vpsize = ( cx, cy )
        glutSetOption( GLUT_MULTISAMPLE, 8 )
        glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH | GLUT_MULTISAMPLE )
        glutInitWindowPosition( 0, 0 )
        glutInitWindowSize( self.__vpsize[0], self.__vpsize[1] )
        self.__id = glutCreateWindow( b'OGL window' ) 
        glutDisplayFunc( self.OnDraw ) 
        glutIdleFunc( self.OnDraw )
    def Run( self ):
        self.__startTime = time()
        glutMainLoop()

    # draw event
    def OnDraw(self):
        self.__vpsize = ( glutGet( GLUT_WINDOW_WIDTH ), glutGet( GLUT_WINDOW_HEIGHT ) )
        currentTime = time()
        # set up camera
        camera = Camera()
        camera.vp = self.__vpsize
        # set up projection matrix
        prjMat = camera.Perspective() 
        # set up view matrix
        viewMat = camera.LookAt()

        # set up light source
        lightSourceBuffer.BindDataFloat(b'u_lightSource.dir', TransformVec4([-1.0, -1.0, -5.0, 0.0], viewMat) )

        # set up icosahedron model matrix
        modelMat = IdentityMat44()
        modelMat = RotateX( modelMat, self.CalcAng( currentTime, 13.0 ) )
        modelMat = RotateY( modelMat, self.CalcAng( currentTime, 17.0 ) )

        # set up attributes and shader program
        glEnable( GL_DEPTH_TEST )
        glClearColor( 1, 1, 1, 0 )
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT )
        progTess.Use()
        progTess.SetUniformM44( b"u_projectionMat44", prjMat )
        lightSourceBuffer.BindToTarget()

        # draw icosahedron
        icoMaterialBuffer.BindToTarget()
        modelViewMat = Multiply(viewMat, modelMat)
        progTess.SetUniformM44( b"u_modelViewMat44", modelViewMat )
        progTess.SetUniformM33( b"u_normalMat33", ToMat33(modelViewMat) )
        icoVAO.Draw()

        glutSwapBuffers()

    def Fract( self, val ): return val - math.trunc(val)
    def CalcAng( self, currentTime, intervall ): return self.Fract( (currentTime - self.__startTime) / intervall ) * 2.0 * math.pi

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
        intData = numpy.zeros(1, dtype=int)
        glGetActiveUniformBlockiv(self.shaderProg, self.index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, intData)
        self.count = intData[0]
        self.indices = numpy.zeros(self.count, dtype=int)
        glGetActiveUniformBlockiv(self.shaderProg, self.index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, self.indices)
        self.offsets = numpy.zeros(self.count, dtype=int)
        glGetActiveUniformsiv(self.shaderProg, self.count, self.indices, GL_UNIFORM_OFFSET, self.offsets)
        strLengthData = numpy.zeros(1, dtype=int)
        arraysizeData = numpy.zeros(1, dtype=int)
        typeData = numpy.zeros(1, dtype='uint32')
        nameData = numpy.chararray(self.maxUniformNameLen+1)
        self.namemap = {}
        self.dataSize = 0 
        for inx in range(0, len(self.indices)):
            glGetActiveUniform( self.shaderProg, self.indices[inx], self.maxUniformNameLen, strLengthData, arraysizeData, typeData, nameData.data )
            name = nameData.tostring()[:strLengthData[0]]
            self.namemap[name] = inx
            self.dataSize = max(self.dataSize, self.offsets[inx] + arraysizeData * 16) 
        glUniformBlockBinding(self.shaderProg, self.index, self.bindingPoint)
        print('\nuniform block %s size:%4d' % (self.name, self.dataSize))
        for uName in self.namemap:
            print( '    %-40s index:%2d    offset:%4d' % (uName, self.indices[self.namemap[uName]], self.offsets[self.namemap[uName]]) ) 

# representation of a uniform block buffer
class UniformBlockBuffer:
    def __init__(self, ub):
        self.namemap = ub.namemap
        self.offsets = ub.offsets
        self.bindingPoint = ub.bindingPoint
        self.object = glGenBuffers(1)
        self.dataSize = ub.dataSize
        glBindBuffer(GL_UNIFORM_BUFFER, self.object)
        dataArray = numpy.zeros(self.dataSize//4, dtype='float32')
        glBufferData(GL_UNIFORM_BUFFER, self.dataSize, dataArray, GL_DYNAMIC_DRAW)
    def BindToTarget(self):
        glBindBuffer(GL_UNIFORM_BUFFER, self.object)
        glBindBufferBase(GL_UNIFORM_BUFFER, self.bindingPoint, self.object)
    def BindDataFloat(self, name, dataArr):
        glBindBuffer(GL_UNIFORM_BUFFER, self.object)
        dataArray = numpy.array(dataArr, dtype='float32')
        glBufferSubData(GL_UNIFORM_BUFFER, self.offsets[self.namemap[name]], len(dataArr)*4, dataArray)

def AddToBuffer( buffer, data, count=1 ): 
    for inx_c in range(0, count):
        for inx_s in range(0, len(data)): buffer.append( data[inx_s] ) 

# initialize glut
glutInit()
# create window
wnd = Window( 800, 600 )

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
for inx_nv in range(0, len(icoIndices) // 3):
    nv = [0.0, 0.0, 0.0]
    for inx_p in range(0, 3):
        for inx_s in range(0, 3): nv[inx_s] += icoPts[ icoIndices[inx_nv*3 + inx_p] ][inx_s]
    AddToBuffer( icoNVData, nv, 3 )
icoVAO = VAObject( [ (3, icoPosData), (3, icoNVData)], [], GL_PATCHES, 3 )

# load, compile and link shader
progTess = ShaderProgram( 
    [ ('ogl/tessellation/tess.vert', GL_VERTEX_SHADER),
        ('ogl/tessellation/tess.tesc', GL_TESS_CONTROL_SHADER),
        ('ogl/tessellation/tess.tese', GL_TESS_EVALUATION_SHADER),
        ('ogl/tessellation/tess.frag', GL_FRAGMENT_SHADER ) ] )
# linke uniform blocks
ubMaterial = UniformBlock(progTess.Prog(), "UB_material")
ubLightSource = UniformBlock(progTess.Prog(), "UB_lightSource")
ubMaterial.Link(1)
ubLightSource.Link(2)

# create uniform block buffers
lightSourceBuffer = UniformBlockBuffer(ubLightSource)
lightSourceBuffer.BindDataFloat(b'u_lightSource.ambient', [0.2, 0.2, 0.2, 1.0])
lightSourceBuffer.BindDataFloat(b'u_lightSource.diffuse', [0.2, 0.2, 0.2, 1.0])
lightSourceBuffer.BindDataFloat(b'u_lightSource.specular', [1.0, 1.0, 1.0, 1.0])

icoMaterialBuffer = UniformBlockBuffer(ubMaterial)
icoMaterialBuffer.BindDataFloat(b'u_roughness', [0.45])
icoMaterialBuffer.BindDataFloat(b'u_fresnel0', [0.4])
icoMaterialBuffer.BindDataFloat(b'u_color', [0.6, 0.5, 0.8, 1.0])
icoMaterialBuffer.BindDataFloat(b'u_specularTint',[1.0, 0.5, 0.5, 0.8])

# start main loop
wnd.Run()
```

<br/><hr/>

<a href="https://stackexchange.com/users/7322082/rabbid76"><img src="https://stackexchange.com/users/flair/7322082.png" width="208" height="58" alt="profile for Rabbid76 on Stack Exchange, a network of free, community-driven Q&amp;A sites" title="profile for Rabbid76 on Stack Exchange, a network of free, community-driven Q&amp;A sites" /></a>
