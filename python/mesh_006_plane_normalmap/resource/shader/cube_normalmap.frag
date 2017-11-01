#version 400

in vec3 vertPos;
in vec3 vertNV;
in vec3 vertTV;
in vec3 vertCol;
in vec2 vertUV;

in vec3 L;
in vec3 V;

out vec4 fragColor;

//uniform vec3  u_lightDir;
uniform float u_ambient;
uniform float u_diffuse;
uniform float u_specular;
uniform float u_shininess;

uniform sampler2D u_texture;
uniform sampler2D u_normalmap;


void main()
{
    vec4 normalMapValue = 2.0 * texture(u_normalmap, vertUV) * 2.0 - 1.0;
    vec3 unitNormal = normalize(normalMapValue.rgb);
    vec3 unitVectorToCamera = normalize(V);
    vec3 unitLightVector = normalize(L);
    
    //vec3 color = vertCol;
    vec3 color = texture( u_texture, vertUV.st ).rgb;

    vec3 NV = unitNormal;
    //vec3 NV = vertNV

    vec3 LD = unitLightVector;
    //vec3 LD = -u_lightDir;

    // ambient part
    vec3 lightCol = u_ambient * color;

    // diffuse part
    vec3  normalV = normalize( NV );
    vec3  lightV  = normalize( LD );
    float NdotL   = max( 0.0, dot( normalV, lightV ) );
    lightCol     += NdotL * u_diffuse * color;
    
    // specular part
    vec3  eyeV      = normalize( -vertPos );
    vec3  halfV     = normalize( eyeV + lightV );
    float NdotH     = max( 0.0, dot( normalV, halfV ) );
    float kSpecular = ( u_shininess + 2.0 ) * pow( NdotH, u_shininess ) / ( 2.0 * 3.14159265 );
    lightCol       += kSpecular * u_specular * color;

    fragColor = vec4( lightCol.rgb, 1.0 );
}