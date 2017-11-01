# Matrix definitions and operations

## Matrix definitons

### 3*3 matrix

A 3*3 matrix looks like this:
   
      c0  c1  c2             x   y   z 
    [ Xx  Yx  Zx ]        [  0   4   8 ]     
    [ Xy  Yy  Zy ]        [  1   5   9 ]     
    [ Xz  Yz  Zz ]        [  2   6  10 ]

And the memory image of a 3*3 matrix like this:

    [ Xx, Xy, Xz, Yx, Yy, Yz, Zx, Zy, Zz, Tx, Ty, Tz ]

Initializing a `mat3 m33` in GLSL:

    mat3 m33 = mat3(
        vec3( Xx, Xy, Xz ),
        vec3( Yx, Xy, Yz ),
        vec3( Zx  Zy  Zz ),
        vec3( Tx, Ty, Tz ) );

Addressing the columns of a `mat3 m33;` in GLSL:

    vec3 x = m33[0].xyz;
    vec3 y = m33[1].xyz;
    vec3 z = m33[2].xyz;    

### 4*4 matrix

A 4*4 matrix looks like this:

      c0  c1  c2  c3            c0  c1  c2  c3
    [ Xx  Yx  Zx  Tx ]        [  0   4   8  12 ]     
    [ Xy  Yy  Zy  Ty ]        [  1   5   9  13 ]     
    [ Xz  Yz  Zz  Tz ]        [  2   6  10  14 ]     
    [  0   0   0   1 ]        [  3   7  11  15 ] 

And the memory image of a 4*4 matrix looks like this:

    [ Xx, Xy, Xz, 0, Yx, Yy, Yz, 0, Zx, Zy, Zz, 0, Tx, Ty, Tz, 1 ]

Initializing a `mat4 m44` in GLSL:

    mat4 m44 = mat4(
        vec4( Xx, Xy, Xz, 0.0),
        vec4( Yx, Xy, Yz, 0.0),
        vec4( Zx  Zy  Zz, 0.0),
        vec4( Tx, Ty, Tz, 1.0) );

Addressing the columns of a `mat4 m44;` in GLSL:

    vec4 c0 = m44[0].xyzw;
    vec4 c1 = m44[1].xyzw;
    vec4 c2 = m44[2].xyzw;
    vec4 c3 = m44[3].xyzw;
   

<br/><hr/>
## Deprecated OpenGL compatibility mode matrix stack

In OpenGL there is one matrix stack for each matrix mode (See [**`glMatrixMode`**][4]). The matrix modes are `GL_MODELVIEW`, `GL_PROJECTION`, and `GL_TEXTURE`.

Multiplication: See the documentation of [**`glMultMatrix`**][5]:
>`glMultMatrix` multiplies the current matrix with the one specified using `m`, and replaces the current matrix with the product.

Translation: See the documentation of [**`glTranslate`**][6]:
>`glTranslate` produces a translation by `x y z` . The current matrix (see `glMatrixMode`) is multiplied by this translation matrix, with the product replacing the current matrix.

Rotation: See the documentation of [**`glRotate`**][7]:
>`glRotate` produces a rotation of angle degrees around the vector `x y z` . The current matrix (see `glMatrixMode`) is multiplied by a rotation matrix with the product replacing the current matrix.

Scaling: See the documentation of [**`glScale`**][8]:
>`glScale`produces a nonuniform scaling along the `x`, `y`, and `z` axes. The three parameters indicate the desired scale factor along each of the three axes.
The current matrix (see `glMatrixMode`) is multiplied by this scale matrix.


<br/><hr/>
## Matrix operations

### Matrix translation

C++:

    void glTranslatef( GLfloat x, GLfloat y, GLfloat z )
    {
        float translation[]{
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            x,    y,    z,    1.0f };

        multiplyMatrix(&modelViewMatrix[0], translation, &modelViewMatrix[0]);
    }

Python:

    def Translate(matA, trans):
        matB = numpy.copy(matA)
        for i in range(0, 4): matB[3,i] = matA[0,i] * trans[0] + matA[1,i] * trans[1] + matA[2,i] * trans[2] + matA[3,i] 
        return matB

### Matrix rotation

C++:

    void glRotatef( GLfloat angle, GLfloat x, GLfloat y, GLfloat z )
    {
        float radians = angle * M_PI/180;
        float c = cos(radians);
        float s = sin(radians);
        
        float rotation[16]{
           x*x*(1.0f-c)+c,   x*y*(1.0f-c)-z*s, x*z*(1.0f-c)+y*s, 0.0f,
           y*x*(1.0f-c)+z*s, y*y*(1.0f-c)+c,   y*z*(1.0f-c)-x*s, 0.0f,
           z*x*(1.0f-c)-y*s  z*y*(1.0f-c)+x*s, z*z*(1.0f-c)+c,   0.0f,
           0.0f,             0.0f,             0.0f,             1.0f };

        multiplyMatrix(&rotationMatrix[0], rotation, &rotationMatrix[0]);
    }  

Python:

    def Rotate(matA, angRad, axis):
        matB = numpy.copy(matA)
        s, c = math.sin(angRad), math.cos(angRad)
        x, y, z = axis[0], axis[1], axis[2] 
        return numpy.matrix( [
            [x*x*(1-c)+c,   x*y*(1-c)-z*s, x*z*(1-c)+y*s, 0],
            [y*x*(1-c)+z*s, y*y*(1-c)+c,   y*z*(1-c)-x*s, 0],
            [z*x*(1-c)-y*s  z*y*(1-c)+x*s, z*z*(1-c)+c,   0],
            [0,             0,             0,             1] ] )

### Matrix scaling

C++:

    void glScalef( GLfloat x, GLfloat y, GLfloat z )
    {
        float scaling[]{
            x,    0.0f, 0.0f, 0.0f,
            0.0f, y,    0.0f, 0.0f,
            0.0f, 0.0f, z,    0.0f,
            0.0f, 0.0f, 0.0f, 1.0f };

        multiplyMatrix(&modelViewMatrix[0], scaling, &modelViewMatrix[0]);
    }

Python:

    def Scale(matA, s):
        matB = numpy.copy(matA)
        for i0 in range(0, 3):
            for i1 in range(0, 4): matB[i0,i1] = matA[i0,i1] * s[i0] 
        return matB


### Matrix multiplication (concatenation)

A matrix multiplication `C = A * B` works like this:

    Matrix4x4 A, B, C;
    
    // C = A * B
    for ( int k = 0; k < 4; ++ k )
        for ( int j = 0; j < 4; ++ j )
            C[k][j] = A[0][l] * B[k][0] + A[1][j] * B[k][1] + A[2][j] * B[k][2] +  A[3][j] * B[k][3];

C++:

    void multiplyMatrix( float A[], float B[], float P[] )
    {
        float C[16];
        for ( int k = 0; k < 4; ++ k ) {
            for ( int l = 0; l < 4; ++ l ) {
                C[k*4+j] =
                    A[0*4+j] * B[k*4+0] +
                    A[1*4+j] * B[k*4+1] +
                    A[2*4+j] * B[k*4+2] +
                    A[3*4+j] * B[k*4+3];
            }
        }
        std::copy(C, C+16, P);
    }

Python:

    def Multiply(matA, matB):
        matC = numpy.copy(matA)
        for i0 in range(0, 4):
            for i1 in range(0, 4):
                matC[i0,i1] = matB[i0,0] * matA[0,i1] + matB[i0,1] * matA[1,i1] + matB[i0,2] * matA[2,i1] + matB[i0,3] * matA[3,i1]    
        return matC 


**A concatenation of 2 matrices (matrix multiplication) is not commutative:**<br/>
e.g. multiplcation of atranslation matrix and a rotation matrix aounf the Y-axis

The translation matrix looks like this:

    Matrix4x4 translate;

    translate[0] : ( 1,  0,  0,  0 )
    translate[1] : ( 0,  1,  0,  0 )
    translate[2] : ( 0,  0,  1,  0 )
    translate[3] : ( tx, ty, tz, 1 )

And the rotation matrix around Y-Axis looks like this:

    Matrix4x4  rotate;
    float      angle;

    rotate[0] : ( cos(angle),  0, sin(angle), 0 )
    rotate[1] : ( 0,           1, 0,          0 )
    rotate[2] : ( -sin(angle), 0, cos(angle), 0 )
    rotate[3] : ( 0,           0, 0,          1 )   

<br/>
The result of `translate * rotate` is this:

    model[0] : ( cos(angle),  0,  sin(angle), 0 )
    model[1] : ( 0,           1,  0,          0 )
    model[2] : ( -sin(angle), 0,  cos(angle), 0 )
    model[3] : ( tx,          ty, tz,         1 )

![translate * rotate](image/translate-rotate.svg)

<br/>
Note, the result of `rotate * translate` would be:

    model[0] : ( cos(angle),                     0,   sin(angle),                     0 )
    model[1] : ( 0,                              1,   0,                              0 )
    model[2] : ( -sin(angle),                    0,   cos(angle),                     0 )
    model[3] : ( cos(angle)*tx - sin(angle)*tx,  ty,  sin(angle)*tz + cos(angle)*tz,  1 )

![rotate * translate](image/rotate-translate.svg)


<br/><hr/>
## Resources

- [GLSL Programming/Vector and Matrix Operations][1]
- [Data Type (GLSL)][2]    
- [GLSL 4×4 Matrix Fields][3]
- [Matrix Translation in GLSL is infinitely stretched][9]
- [What is wrong with my matrix stack implementation (OpenGL ES 2.0)?][10]
- [three.js object translate and rotate based on object self coordinate system or world coordinate system][11]
- [Rotating a multipart object][12]
- [OpenGL move object and keep transformation][13]
- [Issues with Z-axis rotation matrix in glsl shader][14]
- [three.js object translate and rotate based on object self coordinate system or world coordinate system][15]
- [movement of rendered objects in opengl][16]



  [1]: https://en.wikibooks.org/wiki/GLSL_Programming/Vector_and_Matrix_Operations
  [2]: https://www.khronos.org/opengl/wiki/Data_Type_(GLSL)
  [3]: http://www.geeks3d.com/20141114/glsl-4x4-matrix-mat4-fields/
  [4]: https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glMatrixMode.xml
  [5]: https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glMultMatrix.xml
  [6]: https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glTranslate.xml
  [7]: https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glRotate.xml
  [8]: https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glScale.xml
  [9]: https://stackoverflow.com/questions/46763234/matrix-translation-in-glsl-is-infinitely-stretched/46763693#46763693
  [10]: https://stackoverflow.com/questions/46732918/what-is-wrong-with-my-matrix-stack-implementation-opengl-es-2-0/46733128#46733128
  [11]: https://stackoverflow.com/questions/46700593/three-js-object-translate-and-rotate-based-on-object-self-coordinate-system-or-w/46701675#46701675
  [12]: https://stackoverflow.com/questions/46720166/rotating-a-multipart-object/46722875#46722875
  [13]: https://stackoverflow.com/questions/46641995/opengl-move-object-and-keep-transformation/46650784#46650784
  [14]: https://stackoverflow.com/questions/6458051/issues-with-z-axis-rotation-matrix-in-glsl-shader/44986176#44986176
  [15]: https://stackoverflow.com/questions/46700593/three-js-object-translate-and-rotate-based-on-object-self-coordinate-system-or-w/46701675#46701675
  [16]: https://stackoverflow.com/questions/46634046/movement-of-rendered-objects-in-opengl/46634442#46634442


-----
- [Particles not oriented to the camera](https://stackoverflow.com/questions/45779313/particles-not-oriented-to-the-camera/45779696#45779696)
- [OpenGL screen coordinates to world coordinates](https://stackoverflow.com/questions/44965202/opengl-screen-coordinates-to-world-coordinates/45000237#45000237)
- [Is 4th row in model view projection the viewing position?](https://stackoverflow.com/questions/46637247/is-4th-row-in-model-view-projection-the-viewing-position/46639494#46639494)
- [OpenGL transforming objects with multiple rotations of Different axis](https://stackoverflow.com/questions/45091505/opengl-transforming-objects-with-multiple-rotations-of-different-axis/45095288#45095288)
- [Drawing cubes with stacked matrix](https://stackoverflow.com/questions/46238282/drawing-cubes-with-stacked-matrix)
- [Translation on square made of triangles in opengl](https://stackoverflow.com/questions/47004946/translation-on-square-made-of-triangles-in-opengl/47005569#47005569)
- [What is the difference between the order in which a mat4x4 is multiplied with a vec4?](https://stackoverflow.com/questions/46888117/what-is-the-difference-between-the-order-in-which-a-mat4x4-is-multiplied-with-a/46888465#46888465)

