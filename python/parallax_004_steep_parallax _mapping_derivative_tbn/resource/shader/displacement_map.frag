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

float CalculateHeight( in vec2 texCoords )
{
    float height = texture( u_displacement_map, texCoords ).x;
    return clamp( height, 0.0, 1.0 );
}

vec4 CalculateNormal( in vec2 texCoords )
{
#if defined(NORMAL_MAP_TEXTURE)
    float height = CalculateHeight( texCoords );
    vec3  tempNV = texture( u_normal_map, texCoords ).xyz * 2.0 / 1.0;
    return vec4( normalize( tempNV ), height );
#else
    vec2 texOffs = 1.0 / textureSize( u_displacement_map, 0 ).xy;
    vec2 scale   = 1.0 / texOffs;
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

// Parallax Occlusion Mapping in GLSL [http://sunandblackcat.com/tipFullView.php?topicid=28]
vec3 SteepParallax( in float frontFace, in vec3 texDir3D, in vec2 texCoord )
{   
    vec2  quality_range   = u_parallax_quality;
    float quality         = mix( quality_range.x, quality_range.y, 1.0 - pow(abs(normalize(texDir3D).z),2.0) );
    float numSteps        = clamp( quality * 50.0, 1.0, 50.0 );
    int   numBinarySteps  = int( clamp( quality * 10.0, 1.0, 7.0 ) );
    
    float surf_sign       = frontFace;
    float back_face       = step(0.0, -surf_sign); 
    vec2  texStep         = surf_sign * texDir3D.xy / abs(texDir3D.z); // (z is negative) the direction vector points downwards int tangent-space
    vec2  texC            = texCoord.st + surf_sign * texStep + back_face * texStep.xy; 
    float mapHeight       = 1.0;
    float bestBumpHeight  = mapHeight;
    float bumpHeightStep  = 1.0 / numSteps;
    
    for ( int i = 0; i < int( numSteps ); ++ i )
    {
        mapHeight = back_face + surf_sign * CalculateHeight( texC.xy - bestBumpHeight * texStep.xy );
        if ( mapHeight >= bestBumpHeight )
            break;
        bestBumpHeight -= bumpHeightStep;   
    }
    bestBumpHeight -= bumpHeightStep * clamp( ( bestBumpHeight - mapHeight ) / bumpHeightStep, 0.0, 1.0 );
    mapHeight       = bestBumpHeight;
    texC           -= mapHeight * texStep;
        
    return vec3( texC.xy, mapHeight );
}

void main()
{
    vec3  objPosEs     = in_data.pos;
    vec3  objNormalEs  = in_data.nv;
    vec2  texCoords    = in_data.uv.st;
    
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
    vec3  N           = normalize( objNormalEs );
    vec3  dp1         = dFdx( objPosEs );
    vec3  dp2         = dFdy( objPosEs );
    vec2  duv1        = dFdx( texCoords );
    vec2  duv2        = dFdy( texCoords );
    vec3  dp2perp     = cross(dp2, N); 
    vec3  dp1perp     = cross(N, dp1);
    vec3  T           = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3  B           = dp2perp * duv1.y + dp1perp * duv2.y;   
    float invmax      = inversesqrt(max(dot(T, T), dot(B, B)));
    mat3  tbnMat      = mat3(T * invmax, B * invmax, N * u_displacement_scale);
   
    vec3  texDir3D     = normalize( inverse( tbnMat ) * objPosEs );
    float frontFace    = gl_FrontFacing ? 1.0 : -1.0; // TODO $$$ sign(dot(N,objPosEs));
    vec3  newTexCoords = abs(u_displacement_scale) < 0.001 ? vec3(texCoords.st, 0.0) : SteepParallax( frontFace, texDir3D, texCoords.st );
    texCoords.st       = newTexCoords.xy;
    vec4  normalVec    = CalculateNormal( texCoords ); 
    tbnMat[2].xyz     *= (gl_FrontFacing ? 1.0 : -1.0) * N / u_displacement_scale;
    vec3  nvMappedEs   = normalize( tbnMat * normalVec.xyz );

    //vec3 color = in_data.col;
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