# Stackoverflow #

Put a texture on the model and use a texture matrix in OGL 4.0 GLSL
[https://stackoverflow.com/documentation/glsl/5480/getting-started-with-glsl/31873/put-a-texture-on-the-model-and-use-a-texture-matrix-in-ogl-4-0-glsl#t=201707171618120589366]


# GLSL documentation : Put a texture on the model and use a texture matrix in OGL 4.0 GLSL #

A simple OGL 4.0 GLSL shader program that shows how to map a 2D texture on a mesh.
The program is executed with a phyton script. To run the script, PyOpenGL and NumPy must be installed.

The texture matrix defines how the texture is mapped on the mesh. 
By manipulating the texture matrix, the texture can be displaced, scaled and rotated.


### Vertex shader ###
*tex.vert*

    #version 400

    layout (location = 0) in vec3 inPos;
    layout (location = 1) in vec2 inTex;
    
    out vec2 vertTex;
    
    uniform mat4 u_projectionMat44;
    uniform mat4 u_viewMat44;
    uniform mat4 u_modelMat44;
    uniform mat4 u_textureMat44;
    
    void main()
    {
        vertTex = ( u_textureMat44 * vec4( inTex, 0.0, 1.0 ) ).st;
        vec4 modolPos = u_modelMat44 * vec4( inPos, 1.0 );
        vec4 viewPos = u_viewMat44 * modolPos;
        gl_Position = u_projectionMat44 * viewPos;
    }

### Fragment shader ###
*tex.frag*

    #version 400
    
    in vec2 vertTex;
    
    out vec4 fragColor;
    
    uniform sampler2D u_texture;
    
    void main()
    {
        vec4 texCol = texture( u_texture, vertTex.st );
        fragColor = vec4( texCol.rgb, 1.0 );
    }

### Python script ###

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
        prjMat = Perspective(90.0, wndW/wndH, 0.5, 100.0) 
        # set up view matrix
        viewMat = Translate( np.matrix(np.identity(4), copy=False, dtype='float32'), np.array( [0.0, 0.0, -15.0] ) )
        viewMat = RotateView( viewMat, [30.0, CalcAng( currentTime, 60.0 ), 0.0] )
        
        # set up tetrahedron model matrix
        cubeModelMat = np.matrix(np.identity(4), copy=False, dtype='float32')
        cubeModelMat = RotateX( cubeModelMat, -90.0 )
        cubeModelMat = Scale( cubeModelMat, np.repeat( 5.0, 3 ) )
        
        # set up texture matrix
        texMat = np.matrix(np.identity(4), copy=False, dtype='float32')
        deltaT = Fract( (currentTime - startTime) / 28.0 ) * 28.0
        if deltaT < 7.0 or deltaT >= 21.0:
            texMat = Scale( texMat, np.repeat( CalcMove(currentTime, 7.0, [1.0, 2.0]), 3 ) )
        if deltaT >= 7.0 and deltaT < 14.0 or deltaT >= 21.0:
            transAng = math.radians( CalcAng(currentTime, 7.0) )
            texMat = Translate( texMat, np.array( [math.sin(transAng)*0.5, math.cos(transAng)*0.5-0.5, 0.0] ) )
        if deltaT >= 14.0:
            texMat = RotateZ( texMat, CalcAng(currentTime, 7.0) )
        
        # set up attributes and shader program
        glEnable( GL_DEPTH_TEST )
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT )
        glUseProgram( shaderProgram )
        glUniformMatrix4fv( projectionMatLocation, 1, GL_FALSE, prjMat )
        glUniformMatrix4fv( viewMatLocation, 1, GL_FALSE, viewMat )
        glUniformMatrix4fv( textureMatLocation, 1, GL_FALSE, texMat )
        glUniform1i( textureLocation, 0 )
        
        # draw cube
        glUniformMatrix4fv( modelMatLocation, 1, GL_FALSE, cubeModelMat )
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
    
    # define cube vertex array opject
    cubePts = [
        [-1.0, -1.0,  1.0], [ 1.0, -1.0,  1.0], [ 1.0,  1.0,  1.0], [-1.0,  1.0,  1.0],
        [-1.0, -1.0, -1.0], [ 1.0, -1.0, -1.0], [ 1.0,  1.0, -1.0], [-1.0,  1.0, -1.0] ]
    cubePosData = []
    for inx in [ 0, 1, 2, 3, 1, 5, 6, 2, 5, 4, 7, 6, 4, 0, 3, 7, 3, 2, 6, 7, 1, 0, 4, 5 ]:
        for inx_s in range(0, 3): cubePosData.append( cubePts[inx][inx_s] )
    cubeTexData = []
    for inx in range(0, 6):
        for texCoord in [-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, 0.5]: cubeTexData.append( texCoord )
    cubeIndices = []
    for inx in range(0, 6):
        for inx_s in [0, 1, 2, 0, 2, 3]: cubeIndices.append( inx * 4 + inx_s )
    cubeVAObj = CreateVAO( [ (3, cubePosData), (2, cubeTexData) ] )
    cubeInxArr = np.array( cubeIndices, dtype='uint' )
    
    # load, compile and link shader
    shaderProgram = LinkProgram( [
            CompileShader( 'python/ogl4tex/tex.vert', GL_VERTEX_SHADER ), 
            CompileShader( 'python/ogl4tex/tex.frag', GL_FRAGMENT_SHADER )
        ] )
    projectionMatLocation = glGetUniformLocation(shaderProgram, "u_projectionMat44")
    viewMatLocation = glGetUniformLocation(shaderProgram, "u_viewMat44")
    modelMatLocation = glGetUniformLocation(shaderProgram, "u_modelMat44")
    textureMatLocation = glGetUniformLocation(shaderProgram, "u_textureMat44")
    textureLocation = glGetUniformLocation(shaderProgram, "u_texture")
    
    # create texture
    texCX, texCY = 128, 128
    texPlan = np.zeros( texCX * texCY * 4, dtype=np.uint8 )
    for inx_x in range(0, texCX):
        for inx_y in range(0, texCY):
            val_x = math.sin( math.pi * 6.0 * inx_x / texCX )
            val_y = math.sin( math.pi * 6.0 * inx_y / texCY )
            inx_tex = inx_y * texCX * 4 + inx_x * 4
            texPlan[inx_tex + 0] = int( 128 + 127 * val_x )
            texPlan[inx_tex + 1] = 63
            texPlan[inx_tex + 2] = int( 128 + 127 * val_y )
            texPlan[inx_tex + 3] = 255
    glActiveTexture( GL_TEXTURE0 )
    texObj = glGenTextures( 1  )
    glBindTexture( GL_TEXTURE_2D, texObj )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texCX, texCY, 0, GL_RGBA, GL_UNSIGNED_BYTE, texPlan)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT) 
    
    # start main loop
    startTime = time()
    glutMainLoop()