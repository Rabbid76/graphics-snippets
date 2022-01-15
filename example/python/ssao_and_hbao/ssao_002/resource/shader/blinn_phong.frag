#version 450

in TVertexData
{
    vec3 pos;
    vec3 nv;
    vec3 col;
} inData;

layout (location = 0) out vec4 outColor[2];
//layout (location = 1) out vec3 outNV;

uniform vec3  u_lightDir;
uniform float u_ambient;
uniform float u_diffuse;
uniform float u_specular;
uniform float u_shininess;

void main()
{
    vec3 color = inData.col;

    // ambient part
    vec3 lightCol = u_ambient * color;

    // diffuse part
    vec3  normalV = normalize( inData.nv );
    vec3  lightV  = normalize( -u_lightDir );
    float NdotL   = max( 0.0, dot( normalV, lightV ) );
    lightCol     += NdotL * u_diffuse * color;
    
    // specular part
    vec3  eyeV      = normalize( -inData.pos );
    vec3  halfV     = normalize( eyeV + lightV );
    float NdotH     = max( 0.0, dot( normalV, halfV ) );
    float kSpecular = ( u_shininess + 2.0 ) * pow( NdotH, u_shininess ) / ( 2.0 * 3.14159265 );
    lightCol       += kSpecular * u_specular * color;

    outColor[0] = vec4( lightCol.rgb, 1.0 );
    outColor[1] = vec4( normalV, 1.0 );
}