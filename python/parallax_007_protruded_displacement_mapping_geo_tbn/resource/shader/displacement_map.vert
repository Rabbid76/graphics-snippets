#version 400

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
} out_data;

uniform mat4 u_projectionMat44;
uniform mat4 u_viewMat44;
uniform mat4 u_modelMat44;

void main()
{
    mat4 mv_mat   = u_viewMat44 * u_modelMat44;
    vec4 viewPos  = mv_mat * vec4(inPos, 1.0);
    out_data.pos  = viewPos.xyz / viewPos.w;
    out_data.nv   = normalize( mat3( mv_mat ) * inNV );
    out_data.col  = inCol;
    out_data.uv   = inUV;
    //gl_Position = u_projectionMat44 * viewPos;
}