# Stackoverflow #

First OGL 4.0 GLSL shader progra
[https://stackoverflow.com/documentation/glsl/5480/getting-started-with-glsl/31813/first-ogl-4-0-glsl-shader-program#t=201707171653368709123]


# GLSL documentation : First OGL 4.0 GLSL shader program #

A simple OGL 4.0 GLSL shader program with vertex position and color attribute.
The program is executed with a phyton script. To run the script, PyOpenGL must be installed.

A shader program consists at least of a vertex shader and a fragmant shader (exception of computer shaders).
The 1st shader stage is the vertex shader and the last shader stage is the fragment shader (In between, optional further stages are possible, which are not further described here).


## Vertex shader ##
*first.vet*

The vertex shader processes the vertices and associated attributes specified by the drawing command.
The vertex shader processes vertices from the input stream and can manipulate it in any desired way
A vertex shader receives one single vertex from the input stream and generates one single vertex to the output vertex stream.

In our example we draw a single triangle, so the vertex shader is executed 3 times, once for each corner point of the triangle. In this case the input to the vertex shader is the vertex position `in vec3 inPos` and the color attribute `in vec3 inCol`. The color attributes is passed to the next shader stage (`out vec3 vertCol`).

    #version 400
    
    layout (location = 0) in vec3 inPos;
    layout (location = 1) in vec3 inCol;
    
    out vec3 vertCol;
    
    void main()
    {
        vertCol = inCol;
        gl_Position = vec4( inPos, 1.0 );
    }

## Fragment shader ##
*first.frag*

In this example, the fragment shader follows immediately after the vertex shader.
The vertex positions and attributes are interpolated within each face for each fragment.
The fragment shader is executed once for each fragment on the entire triangle and receives the color attribute from the frgment shader.
Since a triangle is drawn the color attribute is interpolated according to the barycentric coordinates of the fragment based on the drawn triangle.


    #version 400
    
    in vec3 vertCol;
    
    out vec4 fragColor;
    
    void main()
    {
        fragColor = vec4( vertCol, 1.0 );
    }

## Python script ##

The python script is just to compile, link and execute the shader program and to draw geometry.
It could be trivially rewritten in C or anything else. It  is not the part of this documentation to
which the greatest attention should be devoted. 

    from OpenGL.GL import *
    from OpenGL.GLUT import *
    from OpenGL.GLU import *
    from sys import *
    from array import array
                 
    # draw event
    def OnDraw(): 
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT )
        glUseProgram( shaderProgram )
        glBindVertexArray( vaObj )
        glDrawArrays( GL_TRIANGLES, 0, 3 )
        glutSwapBuffers()
    
    # read vertex shader program
    with open( 'first.vert', 'r' ) as vertFile:
        vertCode = vertFile.read()
    print( '\nvertex shader code:' )
    print( vertCode )
    
    # read fragment shader program
    with open( 'first.frag', 'r' ) as fragFile:
        fragCode = fragFile.read()
    print( '\nfragment shader code:' )
    print( fragCode )
    
    # initialize glut
    glutInit()
    
    # create window
    wndW = 800
    wndH = 600
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH)
    glutInitWindowPosition(0, 0)
    glutInitWindowSize(wndW, wndH)
    wndID = glutCreateWindow(b'OGL window') 
    glutDisplayFunc(OnDraw) 
    glutIdleFunc(OnDraw)
    
    # define triangle data
    posData = [ -0.636, -0.45, 0.0, 0.636, -0.45, 0.0, 0.0, 0.9, 0.0 ]
    colData = [ 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0 ]
    posAr = array( "f", posData )
    colAr = array( "f", colData )
    
    # create buffers
    posBuffer = glGenBuffers(1)
    glBindBuffer( GL_ARRAY_BUFFER, posBuffer )
    glBufferData( GL_ARRAY_BUFFER, posAr.tostring(), GL_STATIC_DRAW )
    colBuffer = glGenBuffers(1)
    glBindBuffer( GL_ARRAY_BUFFER, colBuffer )
    glBufferData( GL_ARRAY_BUFFER, colAr.tostring(), GL_STATIC_DRAW )
    
    # create vertex array opject
    vaObj = glGenVertexArrays( 1 )
    glBindVertexArray( vaObj )
    glEnableVertexAttribArray( 0 )
    glEnableVertexAttribArray( 1 )
    glBindBuffer( GL_ARRAY_BUFFER, posBuffer )
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, None )
    glBindBuffer( GL_ARRAY_BUFFER, colBuffer )
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, None )
    
    # compile vertex shader
    vertShader = glCreateShader( GL_VERTEX_SHADER )
    glShaderSource( vertShader, vertCode )
    glCompileShader( vertShader )
    result = glGetShaderiv( vertShader, GL_COMPILE_STATUS )
    if not (result):
        print( glGetShaderInfoLog( vertShader ) )
        sys.exit()
    
    # compile fragment shader
    fragShader = glCreateShader( GL_FRAGMENT_SHADER )
    glShaderSource( fragShader, fragCode )
    glCompileShader( fragShader )
    result = glGetShaderiv( fragShader, GL_COMPILE_STATUS )
    if not (result):
        print( glGetShaderInfoLog( fragShader ) )
        sys.exit()
    
    # link shader program
    shaderProgram = glCreateProgram()
    glAttachShader( shaderProgram, vertShader )
    glAttachShader( shaderProgram, fragShader )
    glLinkProgram( shaderProgram )
    result = glGetProgramiv( shaderProgram, GL_LINK_STATUS )
    if not (result):
        print( 'link error:' )
        print( glGetProgramInfoLog( shaderProgram ) )
        sys.exit()
    
    # start main loop
    glutMainLoop()