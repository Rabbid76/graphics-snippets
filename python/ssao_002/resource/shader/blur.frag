#version 450

out vec4 fragColor;

in TVertexData
{
    vec2 pos;
} inData;


layout (binding = 2) uniform sampler2D u_samplerColor;
layout (binding = 4) uniform sampler2D u_samplerSSAO;
uniform vec2   u_viewportsize;
uniform float  u_ssao_scale;
uniform float  u_color_mix;
uniform float  u_blur_mix;

float SSAO44( in sampler2D ssaoSampler, in vec2 texC )
{
    vec2 texOffs = 1.0 / (u_ssao_scale * u_viewportsize);
    float ssao = texture( ssaoSampler, texC.st ).x;  
    for ( int inxX = -1; inxX < 3; ++ inxX )
    {
        for ( int inxY = -1; inxY < 3; ++ inxY )
            ssao += texture( ssaoSampler, texC.st + texOffs * vec2( float(inxX), float(inxY) ) ).x;
    }
    return ssao / 16.0;
}

void main()
{
    vec2  texC   = inData.pos.st * 0.5 + 0.5;
    vec4  texCol = texture( u_samplerColor, texC );
    float ssao   = mix( texture( u_samplerSSAO, texC.st ).x, SSAO44( u_samplerSSAO, texC ), u_blur_mix );
    vec3  color  = mix( vec3(1.0), texCol.rgb, u_color_mix );
    fragColor    = vec4( mix( vec3(0.75, 0.7, 0.8), color * ssao, texCol.a ), 1.0 );
}