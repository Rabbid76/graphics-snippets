#version 450

//#define NORMAL_MAP_TEXTURE
#define NORMAL_MAP_QUALITY 1

//#define TBN_BY_DERIVATIONS

in TGeometryData
{
    vec3  vsPos;
    vec3  vsNV;
    vec3  vsTV;
    float tDet;
    vec3  col;
    vec2  uv;
} inData;

layout (location = 0) out vec4 fragColor;

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

vec3 SteepParallax( in vec3 texDir3D, in vec2 texCoord )
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
    texCoord.xy          += texDir * maxBumpHeight / 2.0;
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
    texCoord       -= bestBumpHeight * texStep;
  }
  else 
    mapHeight = CalculateHeight( texCoord.xy );
  return vec3( texCoord.xy, mapHeight );
}

void main()
{
    vec3  objPosEs     = inData.vsPos;
    vec3  objNormalEs  = inData.vsNV;
    vec2  texCoords    = inData.uv;
    vec3  normalEs     = ( gl_FrontFacing ? 1.0 : -1.0 ) * normalize( objNormalEs );

#ifdef TBN_BY_DERIVATIONS     
    vec3  p_dx         = dFdx( objPosEs );
    vec3  p_dy         = dFdy( objPosEs );
    vec2  tc_dx        = dFdx( texCoords );
    vec2  tc_dy        = dFdy( texCoords );
    float texDet       = determinant( mat2( tc_dx, tc_dy ) );
    vec3  tangentVec   = ( tc_dy.y * p_dx - tc_dx.y * p_dy );
#else
    float texDet       = inData.tDet;
    vec3  tangentVec   = inData.vsTV;
#endif    
    vec3  tangentEs    = normalize( tangentVec - normalEs * dot(tangentVec, normalEs ) );
    mat3  tbnMat       = mat3( sign( texDet ) * tangentEs, cross( normalEs, tangentEs ), normalEs );
   
    //vec3  texDir3D     = normalize( inverse( tbnMat ) * objPosEs );
    vec3  texDir3D     = normalize( transpose( tbnMat ) * objPosEs ); // `transpose` can be used instead of `inverse` for orthogonal 3*3 matrices 
    vec3  newTexCoords = SteepParallax( texDir3D, texCoords.st );
    texCoords.st       = newTexCoords.xy;
    vec4  normalVec    = CalculateNormal( texCoords ); 
    vec3  nvMappedEs   = normalize( tbnMat * normalVec.xyz );

    //vec3 color = inData.col;
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