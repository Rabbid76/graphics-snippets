#version 400

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNV;
layout (location = 2) in vec3 inCol;
layout (location = 3) in vec2 inUV;

out TVertexData
{
    vec3 world_pos;
    vec3 world_nv;
    vec3 col;
    vec2 uv;
} out_data;

uniform mat4 u_modelMat44;

void main()
{
    vec4 worldPos      = u_modelMat44 * vec4(inPos, 1.0);
    out_data.world_pos = worldPos.xyz / worldPos.w;
    out_data.world_nv  = normalize( mat3(u_modelMat44) * inNV );
    out_data.col       = inCol;
    out_data.uv        = inUV;
}