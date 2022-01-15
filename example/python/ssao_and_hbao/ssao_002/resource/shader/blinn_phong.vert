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

uniform mat4 u_projectionMat44;
uniform mat4 u_viewMat44;
uniform mat4 u_modelMat44;

void main()
{
    vec3 modelNV  = mat3( u_modelMat44 ) * normalize( inNV );
    outData.nv    = mat3( u_viewMat44 ) * modelNV;
    outData.col   = inCol;
    vec4 modelPos = u_modelMat44 * vec4( inPos, 1.0 );
    vec4 viewPos  = u_viewMat44 * modelPos;
    outData.pos   = viewPos.xyz / viewPos.w;
    gl_Position   = u_projectionMat44 * viewPos;
}