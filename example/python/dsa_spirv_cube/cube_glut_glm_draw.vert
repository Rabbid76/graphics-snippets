#version 460 core
//#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNV;
layout (location = 2) in vec3 inCol;

layout (location = 0) out TVertexData
{
    vec3 pos;
    vec3 nv;
    vec3 col;
} outData;

struct TMVP
{
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout(std430, binding = 1) buffer MVP
{
    TMVP v[];
} mvp;

out gl_PerVertex {
    vec4 gl_Position;
};

void main()
{
    const int view_i = 0;

    mat4 mv_mat     = mvp.v[view_i].view * mvp.v[view_i].model;
    mat3 normal_mat = inverse(transpose(mat3(mv_mat))); 

    outData.nv   = normalize(normal_mat * inNV);
    outData.col  = inCol;
    vec4 viewPos = mv_mat * vec4(inPos, 1.0);
    outData.pos  = viewPos.xyz / viewPos.w;
    gl_Position  = mvp.v[view_i].proj * viewPos;
}