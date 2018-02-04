#version 450

layout (location = 0) in vec2 inPos;

out TVertexData
{
    vec2 pos;
} outData;

void main()
{
    outData.pos.xy  = inPos.xy;
    gl_Position = vec4( inPos, 0.0, 1.0 );
}