#version 400

in vec3 vertPos;
in vec3 vertNV;
in vec3 vertCol;

out vec4 fragColor;

void main()
{
    vec3 color = vertCol;
    
    float shininess = 10.0;
    vec3  normalV = normalize( vertNV );
    vec3  eyeV    = normalize( -vertPos );
    vec3  halfV   = normalize( eyeV + normalV );
    float NdotH   = max( 0.0, dot( normalV, halfV ) );
    float glowFac = ( shininess + 2.0 ) * pow( NdotH, shininess ) / ( 2.0 * 3.14159265 );
    
    fragColor = vec4( color.rgb * glowFac * 0.5, 1.0 );
}