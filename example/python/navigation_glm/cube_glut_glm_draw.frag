#version 460 core
//#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in TVertexData
{
    vec3 pos;
    vec3 nv;
    vec3 col;
} inData;

layout (location = 0) out vec4 fragColor;

layout(std430, binding = 2) buffer TLight
{
    vec4  u_lightDir;
    float u_ambient;
    float u_diffuse;
    float u_specular;
    float u_shininess;
} light_data;

void main()
{
    vec3 color = inData.col;

    // ambient part
    vec3 lightCol = light_data.u_ambient * color;
    vec3 normalV  = normalize( inData.nv );
    vec3 eyeV     = normalize( -inData.pos );
    vec3 lightV   = normalize( -light_data.u_lightDir.xyz );

    // diffuse part
    float NdotL   = max( 0.0, dot( normalV, lightV ) );
    lightCol     += NdotL * light_data.u_diffuse * color;

    // specular part
    vec3  halfV     = normalize( eyeV + lightV );
    float NdotH     = max( 0.0, dot( normalV, halfV ) );
    float kSpecular = ( light_data.u_shininess + 2.0 ) * pow( NdotH, light_data.u_shininess ) / ( 2.0 * 3.14159265 );
    lightCol       += kSpecular * light_data.u_specular * color;

    fragColor = vec4( lightCol.rgb, 1.0 );
}