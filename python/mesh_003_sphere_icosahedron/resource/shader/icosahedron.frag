#version 400

in vec3 vertPos;
in vec3 vertNV;
//in vec3 vertCol;

out vec4 fragColor;

uniform vec4  u_color;
uniform vec3  u_lightDir;
uniform float u_ambient;
uniform float u_diffuse;
uniform float u_specular;
uniform float u_shininess;

void main()
{
    vec3 color = u_color.rgb;

    // ambient part
    vec3 lightCol = u_ambient * color;

    // diffuse part
    vec3  normalV = normalize( vertNV );
    vec3  lightV  = normalize( -u_lightDir );
    float NdotL   = max( 0.0, dot( normalV, lightV ) );
    lightCol     += NdotL * u_diffuse * color;
    
    // specular part
    vec3  eyeV      = normalize( -vertPos );
    vec3  halfV     = normalize( eyeV + lightV );
    float NdotH     = max( 0.0, dot( normalV, halfV ) );
    float kSpecular = ( u_shininess + 2.0 ) * pow( NdotH, u_shininess ) / ( 2.0 * 3.14159265 );
    lightCol       += kSpecular * u_specular * color;

    fragColor = vec4( 1.0, lightCol.gb, u_color.a );
}