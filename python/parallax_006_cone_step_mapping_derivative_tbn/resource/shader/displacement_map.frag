#version 400

//#define NORMAL_MAP_TEXTURE
#define NORMAL_MAP_QUALITY 1

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

#if defined(NORMAL_MAP_TEXTURE)
uniform sampler2D u_normal_map;
#endif

vec2 GetHeightAndCone( in vec2 texCoords )
{
    vec2 h_and_c = texture( u_displacement_map, texCoords ).rg;
    return clamp( h_and_c, 0.0, 1.0 );
}

vec4 CalculateNormal( in vec2 texCoords )
{
#if defined(NORMAL_MAP_TEXTURE)
    float height = GetHeight( texCoords );
    vec3  tempNV = texture( u_normal_map, texCoords ).xyz * 2.0 / 1.0;
    return vec4( normalize( tempNV ), height );
#else
    vec2 texOffs = 1.0 / textureSize( u_displacement_map, 0 ).xy;
    vec2 scale   = u_displacement_scale / texOffs;
#if NORMAL_MAP_QUALITY > 1
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
#elif NORMAL_MAP_QUALITY > 0
    float h_mid  = texture( u_displacement_map, texCoords.st ).r;
    float h_xa   = texture( u_displacement_map, texCoords.st + texOffs * vec2(-1.0,  0.0) ).r;
    float h_xb   = texture( u_displacement_map, texCoords.st + texOffs * vec2( 1.0,  0.0) ).r;
    float h_ya   = texture( u_displacement_map, texCoords.st + texOffs * vec2( 0.0, -1.0) ).r;
    float h_yb   = texture( u_displacement_map, texCoords.st + texOffs * vec2( 0.0,  1.0) ).r;
    vec2  deltaH = vec2(h_xa-h_xb, h_ya-h_yb); 
#else
    vec4  heights = textureGather( u_displacement_map, texCoords, 0 );
    vec2  deltaH  = vec2(dot(heights, vec4(1.0, -1.0, -1.0, 1.0)), dot(heights, vec4(-1.0, -1.0, 1.0, 1.0)));
    float h_mid   = heights.w; 
#endif
    return vec4( normalize( vec3( deltaH * scale, 1.0 ) ), h_mid );
#endif 
}

// the super fast version
// (change number of iterations at run time)
float intersect_cone_fixed(in vec2 dp, in vec3 ds)
{
    // the "not Z" component of the direction vector
    // (requires that the vector ds was normalized!)
    float iz = sqrt(1.0-ds.z*ds.z);
    // my starting location (is at z=1,
    // and moving down so I don't have
    // to invert height maps)
    // texture lookup (and initialized to starting location)
    vec4 t;
    // scaling distance along vector ds
    float sc;
    // the ds.z component is positive!
    // (headed the wrong way, since
    // I'm using heightmaps)
    // find the initial location and height
    t=texture(u_displacement_map,dp);
    // right, I need to take one step.
    // I use the current height above the texture,
    // and the information about the cone-ratio
    // to size a single step. So it is fast and
    // precise! (like a coneified version of
    // "space leaping", but adapted from voxels)
    sc = (1.0-ds.z*sc-t.r) / (ds.z + iz/(t.g*t.g));
    // and repeat a few (4x) times
    t=texture(u_displacement_map,dp+ds.xy*sc);
    sc += (1.0-ds.z*sc-t.r) / (ds.z + iz/(t.g*t.g));
    t=texture(u_displacement_map,dp+ds.xy*sc);
    sc += (1.0-ds.z*sc-t.r) / (ds.z + iz/(t.g*t.g));
    t=texture(u_displacement_map,dp+ds.xy*sc);
    sc += (1.0-ds.z*sc-t.r) / (ds.z + iz/(t.g*t.g));
    t=texture(u_displacement_map,dp+ds.xy*sc);
    sc += (1.0-ds.z*sc-t.r) / (ds.z + iz/(t.g*t.g));
    // and another five!
    t=texture(u_displacement_map,dp+ds.xy*sc);
    sc += (1.0-ds.z*sc-t.r) / (ds.z + iz/(t.g*t.g));
    t=texture(u_displacement_map,dp+ds.xy*sc);
    sc += (1.0-ds.z*sc-t.r) / (ds.z + iz/(t.g*t.g));
    t=texture(u_displacement_map,dp+ds.xy*sc);
    sc += (1.0-ds.z*sc-t.r) / (ds.z + iz/(t.g*t.g));
    t=texture(u_displacement_map,dp+ds.xy*sc);
    sc += (1.0-ds.z*sc-t.r) / (ds.z + iz/(t.g*t.g));
    t=texture(u_displacement_map,dp+ds.xy*sc);
    sc += (1.0-ds.z*sc-t.r) / (ds.z + iz/(t.g*t.g));
    // return the vector length needed to hit the height-map
    return (sc);
}

// (and you can do LOD by changing "conesteps" based on size/distance, etc.)
float intersect_cone_loop(in vec2 dp, in vec3 ds)
{
    float maxBumpHeight = u_displacement_scale;

    const int conesteps = 10; // ???
    // the "not Z" component of the direction vector
    // (requires that the vector ds was normalized!)
    float iz = sqrt(1.0-ds.z*ds.z);
    // my starting location (is at z=1,
    // and moving down so I don't have
    // to invert height maps)
    // texture lookup (and initialized to starting location)
    vec4 t;
    // scaling distance along vector ds
    float sc=0.0;
    //t=texture2D(stepmap,dp);
    //return (max(0.0,-(t.b-0.5)*ds.x-(t.a-0.5)*ds.y));
    // the ds.z component is positive!
    // (headed the wrong way, since
    // I'm using heightmaps)
    // adaptive (same speed as it averages the same # steps)
    //for (int i = int(float(conesteps)*(0.5+iz)); i > 0; --i)
    // fixed
    for (int i = conesteps; i > 0; --i)
    {
        // find the new location and height
        t=texture(u_displacement_map,dp+ds.xy*sc);
        t.r = t.r * maxBumpHeight;
        t.g = t.g / maxBumpHeight; 

        // right, I need to take one step.
        // I use the current height above the texture,
        // and the information about the cone-ratio
        // to size a single step. So it is fast and
        // precise! (like a coneified version of
        // "space leaping", but adapted from voxels)
        sc += (1.0-ds.z*sc-t.r) / (ds.z + iz/(t.g*t.g));
    }
    // return the vector length needed to hit the height-map
    return (sc);
}

// slowest, but best quality
float intersect_cone_exact(in vec2 dp, in vec3 ds)
{
    vec2 texsize = textureSize( u_displacement_map, 0 );

    // minimum feature size parameter
    float w = 1.0 / max(texsize.x, texsize.y);
    // the "not Z" component of the direction vector
    // (requires that the vector ds was normalized!)
    float iz = sqrt(1.0-ds.z*ds.z);
    // my starting location (is at z=1,
    // and moving down so I don't have
    // to invert height maps)
    // texture lookup
    vec4 t;
    // scaling distance along vector ds
    float sc=0.0;
    // the ds.z component is positive!
    // (headed the wrong way, since
    // I'm using heightmaps)
    // find the starting location and height
    t=texture(u_displacement_map,dp);
    while (1.0-ds.z*sc > t.r)
    {
        // right, I need to take one step.
        // I use the current height above the texture,
        // and the information about the cone-ratio
        // to size a single step. So it is fast and
        // precise! (like a coneified version of
        // "space leaping", but adapted from voxels)
        sc += w + (1.0-ds.z*sc-t.r) / (ds.z + iz/(t.g*t.g));
        // find the new location and height
        t=texture(u_displacement_map,dp+ds.xy*sc);
    }
    // back off one step
    sc -= w;
    // return the vector length needed to hit the height-map
    return (sc);
}

// Parallax Occlusion Mapping in GLSL [http://sunandblackcat.com/tipFullView.php?topicid=28]
vec3 ConeStep( in vec3 texDir3D, in vec2 texCoord )
{   
  float mapHeight;
  float maxBumpHeight = u_displacement_scale;
  vec2  quality_range = u_parallax_quality;
  if ( maxBumpHeight > 0.0 && texDir3D.z < 0.9994 )
  {
    float quality         = mix( quality_range.x, quality_range.y , gl_FragCoord.z * gl_FragCoord.z );
    float numSteps        = clamp( quality * mix( 5.0, 10.0 * clamp( 1.0 + 30.0 * maxBumpHeight, 1.0, 4.0 ), 1.0 - abs(texDir3D.z) ), 1.0, 50.0 );
    int   numBinarySteps  = int( clamp( quality * 5.1, 1.0, 7.0 ) );
    vec2  texDir          = texDir3D.xy / texDir3D.z;
    //texCoord.xy          -= texDir * maxBumpHeight / 2.0;
    texCoord.xy          -= texDir * maxBumpHeight;
    vec2  texStep         = texDir * maxBumpHeight;
    float bumpHeightStep  = 1.0 / numSteps;
    mapHeight             = 1.0;
    float bestBumpHeight  = 1.0;
    float lastStep        = bumpHeightStep;

    // [Determinante](https://de.wikipedia.org/wiki/Determinante)
    // A x B = A.x * B.y - A.y * B.x = dot(A, vec2(B.y,-B.x)) = det(mat2(A,B))

    // [How do you detect where two line segments intersect?](https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect)
    vec2 R = normalize(vec2(length(texDir3D.xy), texDir3D.z)); 
    vec2 P = R / texDir3D.z; 

    for ( int i = 0; i < int( numSteps ); ++ i )
    {
      vec2 sample_tex = texCoord.xy + bestBumpHeight * texStep.xy;
  
      vec2 h_and_c = GetHeightAndCone( sample_tex );
      mapHeight = h_and_c.x;
      if ( mapHeight >= bestBumpHeight )
        break;
      
      vec2 Q = vec2(length(sample_tex), 0.0);
      vec2 S = normalize(vec2(h_and_c.y/maxBumpHeight, 1.0));

      float u = dot(P-Q, vec2(R.y, -R.x)) / dot(R, vec2(S.y,-S.x));    
      float tex_h = u * S.y;
      float cone_step = abs(bestBumpHeight - tex_h);

      lastStep        = max(cone_step, bumpHeightStep);
      bestBumpHeight -= lastStep;
    }
    bestBumpHeight += lastStep * (1.0 - clamp( ( lastStep - mapHeight ) / lastStep, 0.0, 1.0 )); // TODO $$$ skip
    mapHeight       = bestBumpHeight;
    texCoord       += mapHeight * texStep;
  }
  else 
    mapHeight = GetHeightAndCone( texCoord.xy ).x;
  return vec3( texCoord.xy, mapHeight );
}

void main()
{
    vec3  objPosEs     = in_data.pos;
    vec3  objNormalEs  = in_data.nv;
    vec2  texCoords    = in_data.uv.st;
    vec3  normalEs     = ( gl_FrontFacing ? 1.0 : -1.0 ) * normalize( objNormalEs );
    
    // orthonormal tangent space matrix
    //vec3  p_dx         = dFdx( objPosEs );
    //vec3  p_dy         = dFdy( objPosEs );
    //vec2  tc_dx        = dFdx( texCoords );
    //vec2  tc_dy        = dFdy( texCoords );
    //float texDet       = determinant( mat2( tc_dx, tc_dy ) );
    //vec3  tangentVec   = ( tc_dy.y * p_dx - tc_dx.y * p_dy ) / abs( texDet );
    //vec3  tangentEs    = normalize( tangentVec - normalEs * dot(tangentVec, normalEs ) );
    //mat3  tbnMat       = mat3( sign( texDet ) * tangentEs, cross( normalEs, tangentEs ), normalEs );

    // Followup: Normal Mapping Without Precomputed Tangents [http://www.thetenthplanet.de/archives/1180]
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
   
    vec3  texDir3D     = normalize( inverse( tbnMat ) * objPosEs );
    vec3  newTexCoords = ConeStep( texDir3D, texCoords.st );
    texCoords.st       = newTexCoords.xy;
    vec4  normalVec    = CalculateNormal( texCoords ); 
    vec3  nvMappedEs   = normalize( tbnMat * normalVec.xyz );

    //vec3 color = vertCol;
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