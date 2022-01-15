#version 400

out vec4 fragColor;

in vec2 vertPos;

uniform sampler2D u_textureCol;
uniform vec2 u_textureSize;
uniform float u_sigma;
uniform int u_width;

float CalcGauss( float x, float sigma ) 
{
	float coeff = 1.0 / (2.0 * 3.14157 * sigma);
	float expon = -(x*x) / (2.0 * sigma);
	return (coeff*exp(expon));
}

void main()
{
    vec2 texC = vertPos.st * 0.5 + 0.5;
    vec4 texCol = texture( u_textureCol, texC );
    vec4 gaussCol = vec4( texCol.rgb, 1.0 );
    float stepX = 1.0 / u_textureSize.x;
    for ( int i = 1; i <= u_width; ++ i )
    {
        float weight = CalcGauss( float(i) / float(u_width), u_sigma );
        texCol = texture( u_textureCol, texC + vec2( 0.0, float(i) * stepX ) );
        gaussCol += vec4( texCol.rgb * weight, weight );
        texCol = texture( u_textureCol, texC - vec2( 0.0, float(i) * stepX ) );
        gaussCol += vec4( texCol.rgb * weight, weight );
    }
    vec3 hdrCol = 2.0 * gaussCol.xyz / gaussCol.w;
    vec3 mappedCol = vec3( 1.0 ) - exp( -hdrCol.rgb * 3.0 );

    fragColor = vec4( clamp( mappedCol.rgb, 0.0, 1.0 ), 1.0 );
}
