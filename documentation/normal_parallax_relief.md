
# Normal, Parallax and Relief mapping


Textured cube<br/>
[![no_parallax](image/parallax_mapping/parallax_001_no_parallax_mapping_1.png)][1]
[![no_parallax](image/parallax_mapping/parallax_001_no_parallax_mapping_2.png)][1]

Normal Mapping<br/>
[![no_parallax](image/parallax_mapping/parallax_002_normal_mapping_1.png)][2]
[![no_parallax](image/parallax_mapping/parallax_002_normal_mapping_2.png)][2]

Offset Limiting<br/>
[![no_parallax](image/parallax_mapping/parallax_003_offset_limiting_1.png)][3]
[![no_parallax](image/parallax_mapping/parallax_003_offset_limiting_2.png)][3]

Steep Parallax Mapping<br/>
[![no_parallax](image/parallax_mapping/parallax_004_steep_parallax_mapping_derivative_tbn_1.png)][4]
[![no_parallax](image/parallax_mapping/parallax_004_steep_parallax_mapping_derivative_tbn_2.png)][4]

Parallax Occlusion Mapping<br/>
[![no_parallax](image/parallax_mapping/parallax_005_parallax_occlusion_mapping_derivative_tbn_1.png)][5]
[![no_parallax](image/parallax_mapping/parallax_005_parallax_occlusion_mapping_derivative_tbn_2.png)][5]


<br/><hr/>
## Displacement map (Height map)

![height map](../resource/texture/example_1_heightmap.bmp)

TODO


<br/><hr/>
## Normal map

TODO

### Normal vector from height map 

**High performance, low quality**

    vec4 CalculateNormal( in vec2 texCoords )
    {
        vec2 texOffs = 1.0 / textureSize( u_displacement_map, 0 ).xy;
        vec2 scale   = u_displacement_scale / texOffs;

        vec4  heights = textureGather( u_displacement_map, texCoords, 0 );
        vec2  deltaH  = vec2(dot(heights, vec4(1.0, -1.0, -1.0, 1.0)), dot(heights, vec4(-1.0, -1.0, 1.0, 1.0)));
        float h_mid   = heights.w; 

        return vec4( normalize( vec3( deltaH * scale, 1.0 ) ), h_mid );
    }

**Medium performance, medium quality**

    vec4 CalculateNormal( in vec2 texCoords )
    {
        vec2 texOffs = 1.0 / textureSize( u_displacement_map, 0 ).xy;
        vec2 scale   = u_displacement_scale / texOffs;

        float h_mid  = texture( u_displacement_map, texCoords.st ).r;
        float h_xa   = texture( u_displacement_map, texCoords.st + texOffs * vec2(-1.0,  0.0) ).r;
        float h_xb   = texture( u_displacement_map, texCoords.st + texOffs * vec2( 1.0,  0.0) ).r;
        float h_ya   = texture( u_displacement_map, texCoords.st + texOffs * vec2( 0.0, -1.0) ).r;
        float h_yb   = texture( u_displacement_map, texCoords.st + texOffs * vec2( 0.0,  1.0) ).r;
        vec2  deltaH = vec2(h_xa-h_xb, h_ya-h_yb);  

        return vec4( normalize( vec3( deltaH * scale, 1.0 ) ), h_mid );
    }

**High quality, still acceptable performance**

    vec4 CalculateNormal( in vec2 texCoords )
    {
        vec2 texOffs = 1.0 / textureSize( u_displacement_map, 0 ).xy;
        vec2 scale   = u_displacement_scale / texOffs;

        float hx[9];
        hx[0] = texture( u_displacement_map, texCoords.st + texOffs * vec2(-1.0, -1.0) ).r;
        hx[1] = texture( u_displacement_map, texCoords.st + texOffs * vec2( 0.0, -1.0) ).r;
        hx[2] = texture( u_displacement_map, texCoords.st + texOffs * vec2( 1.0, -1.0) ).r;
        hx[3] = texture( u_displacement_map, texCoords.st + texOffs * vec2(-1.0,  0.0) ).r;
        hx[4] = texture( u_displacement_map, texCoords.st ).r;
        hx[5] = texture( u_displacement_map, texCoords.st + texOffs * vec2( 1.0, 0.0) ).r;
        hx[6] = texture( u_displacement_map, texCoords.st + texOffs * vec2(-1.0, 1.0) ).r;
        hx[7] = texture( u_displacement_map, texCoords.st + texOffs * vec2( 0.0, 1.0) ).r;
        hx[8] = texture( u_displacement_map, texCoords.st + texOffs * vec2( 1.0, 1.0) ).r;
        vec2  deltaH = vec2(hx[0]-hx[2] + 2.0*(hx[3]-hx[5]) + hx[6]-hx[8], hx[0]-hx[6] + 2.0*(hx[1]-hx[7]) + hx[2]-hx[8]); 
        float h_mid  = hx[4]; 

        return vec4( normalize( vec3( deltaH * scale, 1.0 ) ), h_mid );
    }

<br/><hr/>
## Normal Mapping

    in TVertexData
    {
        vec3 pos;
        vec3 nv;
        vec3 col;
        vec2 uv;
    } in_data;

    out vec4 fragColor;

    uniform vec3  u_lightDir;
    uniform float u_ambient;
    uniform float u_diffuse;
    uniform float u_specular;
    uniform float u_shininess;

    uniform sampler2D u_texture;
    uniform sampler2D u_displacement_map;
    uniform float     u_displacement_scale;
    uniform vec2      u_parallax_quality;

    float CalculateHeight( in vec2 texCoords )
    {
        float height = texture( u_displacement_map, texCoords ).x;
        return clamp( height, 0.0, 1.0 );
    }

    // no parallax, pass through
    vec3 Parallax( in vec3 texDir3D, in vec2 texCoord )
    {
        return vec3(texCoord.xy, 0.0);
    }

    void main()
    {
        vec3  objPosEs     = in_data.pos;
        vec3  objNormalEs  = in_data.nv;
        vec2  texCoords    = in_data.uv.st;
        vec3  normalEs     = ( gl_FrontFacing ? 1.0 : -1.0 ) * normalize( objNormalEs );
        
        // (co-)tangent space
        vec3  N           = normalEs;
        vec3  dp1         = dFdx( objPosEs );
        vec3  dp2         = dFdy( objPosEs );
        vec2  duv1        = dFdx( texCoords );
        vec2  duv2        = dFdy( texCoords );
        vec3  dp2perp     = cross(dp2, normalEs); 
        vec3  dp1perp     = cross(normalEs, dp1);
        vec3  T           = dp2perp * duv1.x + dp1perp * duv2.x;
        vec3  B           = dp2perp * duv1.y + dp1perp * duv2.y;   
        float invmax      = inversesqrt(max(dot(T, T), dot(B, B)));
        mat3  tbnMat      = mat3(T * invmax, B * invmax, N);
      
        // parallax mapping
        vec3  texDir3D     = normalize( inverse( tbnMat ) * objPosEs );
        vec3  newTexCoords = NoParallax( texDir3D, texCoords.st );
        texCoords.st       = newTexCoords.xy;
        vec4  normalVec    = CalculateNormal( texCoords ); 
        vec3  nvMappedEs   = normalize( tbnMat * normalVec.xyz );

        // texture color
        vec3 color = texture( u_texture, texCoords.st ).rgb;

        // ambient part
        vec3 lightCol = u_ambient * color;

        // diffuse part
        vec3  normalV = normalize( nvMappedEs );
        vec3  lightV  = normalize( -u_lightDir );
        float NdotL   = max( 0.0, dot( normalV, lightV ) );
        lightCol     += NdotL * u_diffuse * color;
        
        // specular part
        vec3  eyeV      = normalize( -objPosEs );
        vec3  halfV     = normalize( eyeV + lightV );
        float NdotH     = max( 0.0, dot( normalV, halfV ) );
        float kSpecular = ( u_shininess + 2.0 ) * pow( NdotH, u_shininess ) / ( 2.0 * 3.14159265 );
        lightCol       += kSpecular * u_specular * color;

        fragColor = vec4( lightCol.rgb, 1.0 );
    }

<br/><hr/>
## Offset Limiting

    vec3 Parallax( in vec3 texDir3D, in vec2 texCoord )
    {
        float parallaxScale = 0.05;
        float mapHeight     = CalculateHeight( texCoord.st );
        vec2  texCoordOffst = parallaxScale * mapHeight * texDir3D.xy / texDir3D.z;
        return vec3(texCoord.xy + texCoordOffst.xy, mapHeight);
    }


<br/><hr/>
## Steep Prallax Mapping

The idea of advanced parallax algorithms like "Steep Parallax Mapping", is to displace the fragments of a surface by the corresponding heights from height map along its normal vectors.

At "Steep Parallax Mapping" the line of sight is transfomed to the (co-)tangent space, this is the reference system, where the uv-coordinates of a texture form the xy-plane and the z-axis points out of the texture.

Along the projection of the line of sight onto the texture, the heights of the height map define a 2 dimensional curve.

The *Steep Parallax* algorithm is used to identify a texel in the texture, by testing sample points along the line of sight.
A texel is found when the normal distance of a sample point to the surface, is less than or equal to the *height* of the texel below the point.

![steep parallax](image/steep_prallax_1.png)

In this calculation, the scaling of the texture does not have to be taken into account since the texture coordinates and the height of the texture are scaled in equal space. 
This is different when calculating the depth (e.g. depth-dependent shadow).
The change in depth is the absolute distance between the intersection of the line of sight with the surface and the identified sampling point on the line of sight.
The absolute distance changes proportionally with the reciprocal scaling factor of the texture.

    vec3 Parallax( in vec3 texDir3D, in vec2 texCoord )
    {   
        float mapHeight;
        float maxBumpHeight = u_displacement_scale;
        vec2  quality_range = u_parallax_quality;
        if ( maxBumpHeight > 0.0 && texDir3D.z < 0.9994 )
        {
            float quality         = mix( quality_range.x, quality_range.y , gl_FragCoord.z * gl_FragCoord.z );
            float numSteps        = clamp( quality * mix( 5.0, 10.0 * clamp( 1.0 + 30.0 * maxBumpHeight, 1.0, 4.0 ), 1.0 - abs(texDir3D.z) ), 1.0, 50.0 );
            int   numBinarySteps  = int( clamp( quality * 5.1, 1.0, 7.0 ) );
            vec2  texDir          = -texDir3D.xy / texDir3D.z;
            //texCoord.xy          += texDir * maxBumpHeight / 2.0;
            texCoord.xy          += texDir * maxBumpHeight;
            vec2  texStep         = texDir * maxBumpHeight;
            float bumpHeightStep  = 1.0 / numSteps;
            mapHeight             = 1.0;
            float bestBumpHeight  = 1.0;
            for ( int i = 0; i < int( numSteps ); ++ i )
            {
                mapHeight = CalculateHeight( texCoord.xy - bestBumpHeight * texStep.xy );
                if ( mapHeight >= bestBumpHeight )
                    break;
                bestBumpHeight -= bumpHeightStep;
            }
            bestBumpHeight += bumpHeightStep * (1.0 - clamp( ( bestBumpHeight - mapHeight ) / bumpHeightStep, 0.0, 1.0 ));
            mapHeight       = bestBumpHeight;
            texCoord       -= mapHeight * texStep;
        }
        else 
            mapHeight = CalculateHeight( texCoord.xy );
        return vec3( texCoord.xy, mapHeight );
    }


<br/><hr/>
## Prallax Occlusion Mapping

    vec3 ParallaxOcclusion( in vec3 texDir3D, in vec2 texCoord )
    {   
        float mapHeight;
        float maxBumpHeight = u_displacement_scale;
        vec2  quality_range = u_parallax_quality;
        if ( maxBumpHeight > 0.0 && texDir3D.z < 0.9994 )
        {
            float quality         = mix( quality_range.x, quality_range.y , gl_FragCoord.z * gl_FragCoord.z );
            float numSteps        = clamp( quality * mix( 5.0, 10.0 * clamp( 1.0 + 30.0 * maxBumpHeight, 1.0, 4.0 ), 1.0 - abs(texDir3D.z) ), 1.0, 50.0 );
            int   numBinarySteps  = int( clamp( quality * 5.1, 1.0, 7.0 ) );
            vec2  texDir          = -texDir3D.xy / texDir3D.z;
            //texCoord.xy          += texDir * maxBumpHeight / 2.0;
            texCoord.xy          += texDir * maxBumpHeight;
            vec2  texStep         = texDir * maxBumpHeight;
            float bumpHeightStep  = 1.0 / numSteps;
            mapHeight             = 1.0;
            float bestBumpHeight  = 1.0;
            for ( int i = 0; i < int( numSteps ); ++ i )
            {
                mapHeight = CalculateHeight( texCoord.xy - bestBumpHeight * texStep.xy );
                if ( mapHeight >= bestBumpHeight )
                    break;
                bestBumpHeight -= bumpHeightStep;
            }
            bestBumpHeight += bumpHeightStep;
            for ( int i = 0; i < numBinarySteps; ++ i )
            {
                bumpHeightStep *= 0.5;
                bestBumpHeight -= bumpHeightStep;
                mapHeight       = CalculateHeight( texCoord.xy - bestBumpHeight * texStep.xy );
                bestBumpHeight += ( bestBumpHeight < mapHeight ) ? bumpHeightStep : 0.0;
            }
            bestBumpHeight -= bumpHeightStep * clamp( ( bestBumpHeight - mapHeight ) / bumpHeightStep, 0.0, 1.0 );
            mapHeight       = bestBumpHeight;
            texCoord       -= mapHeight * texStep;
        }
        else 
            mapHeight = CalculateHeight( texCoord.xy );
        return vec3( texCoord.xy, mapHeight );
    }

<br/><hr/>
## Cone Step Mapping

![cone step](image/parallax_cone_step.png)

    X  =  P + R * t  =  Q + S * u

    t  =  dot(Q-P, vec2(S.y, -S.x)) / dot(R, vec2(S.y, -S.x))  =  determinant(mat2(Q-P, S)) / determinant(mat2(R, S))
    u  =  dot(Q-P, vec2(R.y, -R.x)) / dot(R, vec2(S.y, -S.x))  =  determinant(mat2(Q-P, R)) / determinant(mat2(R, S))

<br/><hr/>
# TODO

## Reliefmepping with geometry shader

Reliefmapping on a prism, which is defined by the contur o the triangle primitive  and the minimum and maximum height of the height map.
A ray from the view postion to each of the 6 cormers of the prism can be defined. 
The maximum distance for sampling the ray and sarching intersections with the height field, can be limited by 4 planes.
The first 2 lanes ar given by the minimum and maximum haight of the height field. If the ray goes out of this bounds, the sampling can be canceled. No intersection with the height field is found and the view ray hits the prism outside of the silhouette of the height field.
A prism with a triangular base has at most to backfac body surfaces. This 2 surfaces can be identified by the point with the largest distance to the eye position. If the edge trough this point is no silhouette, then there are 2 backface surfaces. For each of the 6 prism corner points the distance to the intersection point with the surfaces along the view ray can be calcualted. This distances can be interpolated for each fragment. The maximum sample distance is given by the closest distance. If the disatance is exceeded, then the view ray hits the prism outside of the silhouette of the height field. This also solves the issue of surfaces at the backface of the prism, because the closest distance will be 0, which means that the ray doesn't hit the height field. The only issue left is to map the distance from the vertex ccordiante space to the texture coordiante space. This can be solved by transforming the vertex points to the (co-)tangent space and measuring the distance in the (co-)tangent space.

This algorithm should well fit with cone step mapping.


  [1]: https://rabbid76.github.io/graphics-snippets/html/technique/parallax_001_no_parallax_mapping.html
  [2]: https://rabbid76.github.io/graphics-snippets/html/technique/parallax_002_normal_mapping.html
  [3]: https://rabbid76.github.io/graphics-snippets/html/technique/parallax_003_offset_limiting.html
  [4]: https://rabbid76.github.io/graphics-snippets/html/technique/parallax_004_steep_parallax_mapping_derivative_tbn.html
  [5]: https://rabbid76.github.io/graphics-snippets/html/technique/parallax_005_parallax_occlusion_mapping_derivative_tbn.html
