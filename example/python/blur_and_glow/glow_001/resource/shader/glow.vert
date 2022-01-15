#version 400

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNV;
layout (location = 2) in vec3 inCol;

out vec3 vertPos;
out vec3 vertNV;
out vec3 vertCol;

uniform mat4 u_projectionMat44;
uniform mat4 u_modelViewMat44;

void main()
{
    vertNV      = mat3( u_modelViewMat44 ) * inNV;
    vertCol     = inCol;
    vec4 pos    = u_modelViewMat44 * vec4( inPos, 1.0 );
    vertPos     = pos.xyz / pos.w;
    gl_Position = u_projectionMat44 * pos;
}