#version 400

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNV;

out TVertexData
{
    vec3 pos;
    vec3 nv;
} outData;

uniform mat4 u_projectionMat44;
uniform mat4 u_modelViewMat44;
uniform mat3 u_normalMat33;

void main()
{
    vec4 viewPos = u_modelViewMat44 * vec4( inPos, 1.0 );
    
    outData.pos = viewPos.xyz / viewPos.w;
    outData.nv  = u_normalMat33 * normalize( inNV );
    
    gl_Position = u_projectionMat44 * viewPos;
}