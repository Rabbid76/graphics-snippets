# Tangent space


## Relation system

In an [Right-Handed Coordinate System][3] (see further [Right-hand rule][5])

![right handed TBN](image/right_hand_tbn.svg)

the Binormal Vector is calcualted by:

    B = N x T

while in an [Left-Handed Coordinate System][4]

![left handed TBN](image/left_hand_tbn.svg)

the Binormal Vector is calcualted by:

    B = T x N

<br/>
In OpenGL is commonly used a [Right-Handed Coordinate System][3], but this depends on the specification and definitions which are chosen by the user.

If a system is mirrored it changes from one system to the other. This may occur if a texture is mirrored or looked at from the backface.


<br/><hr/>
## Tangent

### Precomputed tangent

One way to get the tangential space in the shader is to pass the tanget as an attribute to the shader, just like the normalvector.
For this the tangents have to be pre-computed and stored in the mesh data.


### Calculating the Tangent in the Fragment Shader


### Calculating the Tangent in a Geometry or Tesselation Shader


<br/><hr/>
## Resources

- [How to calculate Tangent and Binormal?][1]
- [Calculating Binormal Vector][2] 

  
  [1]: https://stackoverflow.com/questions/5255806/how-to-calculate-tangent-and-binormal/44901073#44901073
  [2]: https://stackoverflow.com/questions/47117083/calculating-binormal-vector/47121567#47121567
  [3]: http://mathworld.wolfram.com/Right-HandedCoordinateSystem.html
  [4]: http://mathworld.wolfram.com/Left-HandedCoordinateSystem.html
  [5]: https://en.wikipedia.org/wiki/Right-hand_rule
 

----------------

Let's assume that we have a **normalized normalvector** `n`, and we have the tangent `t` and the binormal` b` or we can calculate them from the derivations as follows:

    // derivations of the fragment position
    vec3 pos_dx = dFdx( fragPos );
    vec3 pos_dy = dFdy( fragPos );
    // derivations of the texture coordinate
    vec2 texC_dx = dFdx( texCoord );
    vec2 texC_dy = dFdy( texCoord );
    // tangent vector and binormal vector
    vec3 t = texC_dy.y * pos_dx - texC_dx.y * pos_dy;
    vec3 b = texC_dx.x * pos_dy - texC_dy.x * pos_dx;

Of course the an orthonormalized tangent space matrix can be calcualted by using the cross product,
but this would only work for right-hand systems. If a matrix was mirrored (left-hand system) it will turn to a right hand system.:

    t = cross( cross( n, t ), t ); // orthonormalization of the tangent vector
    b = cross( n, t );             // orthonormalization of the binormal vector may invert the binormal vector
    mat3 tbn = mat3( normalize(t), normalize(b), n );

In the code snippet above the binormal vector is reversed if the tangent space is a left-handed system.
To avoid this, the hard must be gone:

    t = cross( cross( n, t ), t ); // orthonormalization of the tangent vector
    b = cross( b, cross( b, n ) ); // orthonormalization of the binormal vectors to the normal vector 
    b = cross( cross( t, b ), t ); // orthonormalization of the binormal vectors to the tangent vector
    mat3 tbn = mat3( normalize(t), normalize(b), n );

A common way to orthogonalize any matrix is the [Gram?Schmidt process](https://en.wikipedia.org/wiki/Gram%E2%80%93Schmidt_process):

    t = t - n * dot( t, n ); // orthonormalization ot the tangent vectors
    b = b - n * dot( b, n ); // orthonormalization of the binormal vectors to the normal vector 
    b = b - t * dot( b, t ); // orthonormalization of the binormal vectors to the tangent vector
    mat3 tbn = mat3( normalize(t), normalize(b), n );
   
Another possibility is to use the determinant of the 2*2 matrix which results from the derivations of the texture coordinates `texC_dx`, `texC_dy`
to take the direction of the binormal vector into account. The idea is that the determinant of a orthogonal matrix is 1 and the determined one of a orthogonal mirror matrix -1.

The determinant can eihter be calcualted by the GLSL function `determinant( mat2( texC_dx, texC_dy )`
or it can be calcualated by it formula `texC_dx.x * texC_dy.y - texC_dy.x * texC_dx.y`.

For the calculation of the orthonormalized tangent space matrix, the binormal vector is no longer required and the calculation of the unit vector
(`normalize`) of the binormal vector can be evaded.

    float texDet = texC_dx.x * texC_dy.y - texC_dy.x * texC_dx.y;
    vec3 t = texC_dy.y * pos_dx - texC_dx.y * pos_dy;
    vec3 t = normalize( t - n * dot( t, n ) );
    vec3 b = cross( n, t );                      // b is normlized because n and t are orthonormalized unit vectors
    mat3 tbn = mat3( t, sign( texDet ) * b, n ); // take in account the direction of the binormal vector


-------------------


## Calculating Tangent Sapce in Fragment Shader

### `dFdx`, `dFdy`
- `dFdx`, `dFdy` [https://www.opengl.org/sdk/docs/man/html/dFdx.xhtml]
- Explanation of dFdx [http://stackoverflow.com/questions/16365385/explanation-of-dfdx]
- Derivatives I: Discontinuities and [http://hacksoflife.blogspot.co.at/2011/01/derivatives-i-discontinuities-and.html]

### Tangent-Binormal-Normal Matrix


<b>Variant 1</b>

    mat3 TBN( vec3 ptV, vec3 N, vec2 texCoord )
    {
      vec3  p_dx   = dFdx( ptV );
      vec3  p_dy   = dFdy( ptV );
      vec2  tc_dx  = dFdx( texCoord );
      vec2  tc_dy  = dFdy( texCoord );
      float tc_det = tc_dx.x * tc_dy.y - tc_dx.y * tc_dy.x; // determinant( mat2( tc_dx, tc_dy ) );
      
      vec3 vecT = normalize( tc_dy.y * p_dx - tc_dx.y * p_dy ) / abs( tc_det );
      vec3 T = normalize( vecT - N * dot( vecT, N ) );
      return mat3( ( tc_det < 0.0 ) ? -T : T, cross( N, T ), N );
    }

Calulate the partial derivative of Vertexcoordinate and Texturecoordinate by `dFdx`, `dFdy`.
Calculate the tangent with the \ref pagShaderGlossary_GramSchmidt.
Use the determinant of the UV matrix to correct the tangential space of mirrored textures.

<b>Variant 2</b>

    mat3 TBN( vec3 ptV, vec3 N, vec2 texCoord )
    {
      vec3  p_dx   = dFdx( ptV );
      vec3  p_dy   = dFdy( ptV );
      vec2  tc_dx  = dFdx( texCoord );
      vec2  tc_dy  = dFdy( texCoord );
      
      vec3 dp1perp = cross( N, p_dx );
      vec3 dp2perp = cross( p_dy, N );
      vec3 T = dp2perp * tc_dx.x + dp1perp * tc_dy.x;
      vec3 B = dp2perp * tc_dx.y + dp1perp * tc_dy.y;
      return mat3( normalize( T ), normalize( B ), N );
    }


### Inverse Tangent-Binormal-Normal Matrix
 
    mat3 inverseTBN( vec3 ptV, vec3 N, vec2 texCoord )
    {
      vec3  p_dx   = dFdx( ptV );
      vec3  p_dy   = dFdy( ptV );
      vec2  tc_dx  = dFdx( texCoord );
      vec2  tc_dy  = dFdy( texCoord );

      vec3 dp1perp = cross( N, p_dx );
      vec3 dp2perp = cross( p_dy, N );
      vec3 T = dp2perp * tc_dx.x + dp1perp * tc_dy.x;
      vec3 B = dp2perp * tc_dx.y + dp1perp * tc_dy.y;
      float invmax = inversesqrt( max( dot( T, T ), dot( B, B ) ) );
      return mat3( normalize( T * invmax ), normalize( B * invmax ), N );
    }


## Tangent Sapce in Geometry or Tesselation Shader


# Reference

- Followup: Normal Mapping Without Precomputed Tangents [http://www.thetenthplanet.de/archives/1180]
- Normal Mapping without Precomputed Tangent Space Vectors [http://www.geeks3d.com/20130122/normal-mapping-without-precomputed-tangent-space-vectors/]
- How to calculate Tangent and Binormal? [http://stackoverflow.com/questions/5255806/how-to-calculate-tangent-and-binormal]
- Tangent space [https://en.wikipedia.org/wiki/Tangent_space]