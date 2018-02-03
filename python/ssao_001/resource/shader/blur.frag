#version 450

out vec4 fragColor;

in TVertexData
{
    vec2 pos;
} inData;


uniform sampler2D u_samplerColor;
uniform sampler2D u_samplerSSAO;
uniform vec2      u_viewportsize;
uniform float     u_color_mix;

float SSAO44( in sampler2D ssaoSampler, in vec2 texC )
{
    vec2 texOffs = 1.0 / u_viewportsize;
    float ssao = texture2D( ssaoSampler, texC.st ).x;  
    for ( int inxX = -1; inxX < 3; ++ inxX )
    {
        for ( int inxY = -1; inxY < 3; ++ inxY )
            ssao += texture2D( ssaoSampler, texC.st + texOffs * vec2( int(inxX), int(inxY) ) ).x;
    }
    return ssao / 16.0;
}

void main()
{
    vec2  texC   = inData.pos.st * 0.5 + 0.5;
    vec4  texCol = texture2D( u_samplerColor, texC );
    float ssao   = SSAO44( u_samplerSSAO, texC );
    vec3  color  = mix(vec3(1.0), texCol.rgb, u_color_mix);
    fragColor    = vec4( mix( vec3(0.75, 0.7, 0.8), color * ssao, texCol.a ), 1.0 );
}