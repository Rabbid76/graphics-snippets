#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNV;
layout (location = 2) in vec3 inCol;

out TVertexData
{
    vec3 pos;
    vec3 nv;
    vec3 col;
} outData;

void main()
{
    outData.pos = inPos;
    outData.nv  = normalize( inNV );
    outData.col = inCol;
}