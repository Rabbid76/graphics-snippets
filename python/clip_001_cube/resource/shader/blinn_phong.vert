#version 400

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNV;
layout (location = 2) in vec3 inCol;

out vec3 vertPos;
out vec3 vertNV;
out vec3 vertCol;

uniform mat4 u_projectionMat44;
uniform mat4 u_viewMat44;
uniform mat4 u_modelMat44;
uniform vec4 u_clipPlane;

void main()
{
    vec3 modelNV       = mat3( u_modelMat44 ) * normalize( inNV );
    vertNV             = mat3( u_viewMat44 ) * modelNV;
    vertCol            = inCol;
    vec4 modelPos      = u_modelMat44 * vec4( inPos, 1.0 );
    vec4 viewPos       = u_viewMat44 * modelPos;
    vertPos            = viewPos.xyz / viewPos.w;
    gl_Position        = u_projectionMat44 * viewPos;
    vec4 clipPlane     = vec4(normalize(u_clipPlane.xyz), u_clipPlane.w);
    gl_ClipDistance[0] = dot(modelPos, clipPlane);
}