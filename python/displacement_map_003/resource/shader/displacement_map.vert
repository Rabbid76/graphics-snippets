#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNV;
layout (location = 2) in vec3 inCol;
layout (location = 3) in vec2 inUV;

out TVertexData
{
    vec3 pos;
    vec3 nv;
    vec3 col;
    vec2 uv;
} outData;

uniform mat4 u_projectionMat44;
uniform mat4 u_viewMat44;
uniform mat4 u_modelMat44;

void main()
{
    outData.pos = inPos;
    outData.nv  = inNV;
    outData.col = inCol;
    outData.uv  = inUV;
}