#version 450

//#define NORMAL_MAP_TEXTURE
#define NORMAL_MAP_QUALITY 1

//#define TBN_BY_DERIVATIONS

in TGeometryData
{
    vec3  vsPos1;
    float vsPos_rel01;
    vec3  vsNV;
    vec3  vsTV;
    float vsBVsign;
    vec3  col;
    vec3  uv0;
    vec3  uv1;
} inData;

layout (location = 0) out vec4 fragColor;

uniform mat4 u_projectionMat44;

uniform vec3  u_lightDir;
uniform float u_ambient;
uniform float u_diffuse;
uniform float u_specular;
uniform float u_shininess;

uniform sampler2D u_texture;
uniform sampler2D u_displacement_map;
uniform float     u_displacement_scale;
uniform vec2      u_parallax_quality;

#ifdef NORMAL_MAP_TEXTURE
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

vec3 SteepParallax( in float bottom_rel, in vec3 texDir3D, in vec3 texC0, in vec3 texC1 )
{
  float mapHeight;
  float maxBumpHeight = u_displacement_scale;
  vec2  quality_range = u_parallax_quality;
  float frontface     = step(1.0, bottom_rel);
  float facesign      = frontface * 2.0 - 1.0;
  vec2  texCoord      = mix(texC1.xy, texC0.xy, frontface);
  if ( maxBumpHeight > 0.0 && texDir3D.z < 0.9994 )
  {
    float quality         = mix( quality_range.x, quality_range.y , gl_FragCoord.z * gl_FragCoord.z );
    float numSteps        = clamp( quality * mix( 5.0, 10.0 * clamp( 1.0 + 30.0 * maxBumpHeight, 1.0, 4.0 ), 1.0 - abs(texDir3D.z) ), 1.0, 50.0 );
    int   numBinarySteps  = int( clamp( quality * 5.1, 1.0, 7.0 ) );
    vec2  texStep         = (texC1.xy - texC0.xy) * facesign;
    float bumpHeightStep  = 1.0 / numSteps;
    float bestBumpHeight  = 1.0;
    mapHeight             = 1.0;
    for ( int i = 0; i < int( numSteps ); ++ i )
    {
        mapHeight = (1.0-frontface) + facesign * CalculateHeight( texCoord.xy + bestBumpHeight * texStep.xy );
        if ( mapHeight >= bestBumpHeight )
            break;
        bestBumpHeight -= bumpHeightStep;
    }
    bestBumpHeight += bumpHeightStep;
    for ( int i = 0; i < numBinarySteps; ++ i )
    {
        bumpHeightStep *= 0.5;
        bestBumpHeight -= bumpHeightStep;
        mapHeight       = (1.0-frontface) + facesign * CalculateHeight( texCoord.xy + bestBumpHeight * texStep.xy );
        bestBumpHeight += ( bestBumpHeight < mapHeight ) ? bumpHeightStep : 0.0;
    }
    bestBumpHeight -= bumpHeightStep * clamp( ( bestBumpHeight - mapHeight ) / bumpHeightStep, 0.0, 1.0 );
    texCoord       += bestBumpHeight * texStep;
  }
  else 
    mapHeight = CalculateHeight( texCoord.xy );
  return vec3(texCoord.xy, clamp(mapHeight, texC0.z, texC1.z));
}

void main()
{
    vec3  objPosEs1   = inData.vsPos1;
    vec3  objPosEs0   = objPosEs1 * inData.vsPos_rel01;
    vec3  objNormalEs = inData.vsNV;
    vec3  texCoords1  = inData.uv1.xyz;
    vec3  texCoords0  = inData.uv0.xyz;
    vec3  normalEs    = ( gl_FrontFacing ? 1.0 : -1.0 ) * normalize( objNormalEs );

#ifdef TBN_BY_DERIVATIONS     
    vec3  p_dx         = dFdx( objPosEs1 );
    vec3  p_dy         = dFdy( objPosEs1 );
    vec2  tc_dx        = dFdx( texCoords1.xy );
    vec2  tc_dy        = dFdy( texCoords1.xy );
    float texDet       = determinant( mat2( tc_dx, tc_dy ) );
    vec3  tangentVec   = ( tc_dy.y * p_dx - tc_dx.y * p_dy ) / texDet;
    float binormalSign = sign(texDet);
#else
    vec3  tangentVec   = inData.vsTV;
    float binormalSign = inData.vsBVsign;
#endif    
    vec3  tangentEs    = normalize( tangentVec - normalEs * dot(tangentVec, normalEs ) );
    mat3  tbnMat       = mat3( tangentEs, binormalSign * cross( normalEs, tangentEs ), normalEs );
    //mat3  tbnMat       = mat3( -tangentEs, -binormalSign * cross( normalEs, tangentEs ), normalEs ); // inverse steep parallax
   
    //vec3  texDir3D     = normalize( inverse( tbnMat ) * objPosEs1 );
    vec3  texDir3D     = normalize( transpose( tbnMat ) * objPosEs1 ); // `transpose` can be used instead of `inverse` for orthogonal 3*3 matrices 
    vec3  texCoordsMap = SteepParallax( inData.vsPos_rel01, texDir3D, texCoords0.xyz, texCoords1.xyz );
    vec4  normalVec    = CalculateNormal( texCoordsMap.xy ); 
    vec3  nvMappedEs   = normalize( tbnMat * normalVec.xyz );
    vec3  posMappedEs  = mix(objPosEs0, objPosEs1, texCoordsMap.z);
    vec4  posMappedPrj = u_projectionMat44 * vec4(posMappedEs, 1.0);

    //vec3 color = inData.col;
    vec3 color = texture( u_texture, texCoordsMap.xy ).rgb;

    // ambient part
    vec3 lightCol = u_ambient * color;

    // diffuse part
    vec3  normalV = normalize( nvMappedEs );
    vec3  lightV  = normalize( -u_lightDir );
    float NdotL   = max( 0.0, dot( normalV, lightV ) );
    lightCol     += NdotL * u_diffuse * color;
    
    // specular part
    vec3  eyeV      = normalize( -objPosEs1 );
    vec3  halfV     = normalize( eyeV + lightV );
    float NdotH     = max( 0.0, dot( normalV, halfV ) );
    float kSpecular = ( u_shininess + 2.0 ) * pow( NdotH, u_shininess ) / ( 2.0 * 3.14159265 );
    lightCol       += kSpecular * u_specular * color;

    gl_FragDepth = (posMappedPrj.z/posMappedPrj.w) * 0.5 + 0.5;
    fragColor    = vec4(lightCol.rgb, 1.0 );
}