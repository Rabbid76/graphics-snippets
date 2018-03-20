#version 450

in TVertexData
{
    vec3 pos;
    vec3 nv;
} inData;

out vec4 fragColor;

uniform vec3  u_lightDir;
uniform float u_ambient;
uniform float u_diffuse;
uniform float u_specular;
uniform float u_shininess;

layout (binding = 1) uniform sampler1D u_color_tex;

void main()
{
    vec4 color = texelFetch(u_color_tex, gl_PrimitiveID, 0);
    
    // ambient part
    vec3 lightCol = u_ambient * color.rgb;
    vec3 normalV  = normalize( inData.nv );
    vec3 eyeV     = normalize( -inData.pos );
    vec3 lightV   = normalize( -u_lightDir );

    // diffuse part
    float NdotL   = max( 0.0, dot( normalV, lightV ) );
    lightCol     += NdotL * u_diffuse * color.rgb;
    
    // specular part
    vec3  halfV     = normalize( eyeV + lightV );
    float NdotH     = max( 0.0, dot( normalV, halfV ) );
    float kSpecular = ( u_shininess + 2.0 ) * pow( NdotH, u_shininess ) / ( 2.0 * 3.14159265 );
    lightCol       += kSpecular * u_specular * color.rgb;

    fragColor = vec4( lightCol.rgb, 1.0 );
}