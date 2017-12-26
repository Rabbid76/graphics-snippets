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
    //vec3  uv0;
    //vec3  uv1;
    noperspective vec3  uv0;
    noperspective vec3  uv1;
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



void main()
{
    vec3  objPosEs1   = inData.vsPos1;
    vec3  objPosEs0   = objPosEs1 * inData.vsPos_rel01;
    vec3  objNormalEs = inData.vsNV;
    vec3  texCoords0  = inData.uv0.xyz;
    vec3  texCoords1  = inData.uv1.xyz;
    vec3  normalEs    = ( gl_FrontFacing ? 1.0 : -1.0 ) * normalize( objNormalEs ); // TODO $$$

#ifdef TBN_BY_DERIVATIONS     
    vec3  p_dx         = dFdx( objPosEs1 );
    vec3  p_dy         = dFdy( objPosEs1 );
    vec2  tc_dx        = dFdx( texCoords0.xy );
    vec2  tc_dy        = dFdy( texCoords0.xy );
    float texDet       = determinant( mat2( tc_dx, tc_dy ) );
    vec3  tangentVec   = ( tc_dy.y * p_dx - tc_dx.y * p_dy ) / texDet;
    float binormalSign = sign(texDet);
#else
    vec3  tangentVec   = inData.vsTV;
    float binormalSign = inData.vsBVsign;
#endif    
    vec3  tangentEs    = normalize( tangentVec - normalEs * dot(tangentVec, normalEs ) );
    mat3  tbnMat       = mat3( tangentEs, binormalSign * cross( normalEs, tangentEs ), normalEs );
    
    vec3  texCoordsMap = mix(texCoords0, texCoords1, 0.0/*texCoords0.z*/);
    vec3  nvMappedEs   = normalize( normalEs );
    vec3  posMappedEs  = objPosEs1;
    
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

    fragColor    = vec4(lightCol.rgb, 1.0 );
}