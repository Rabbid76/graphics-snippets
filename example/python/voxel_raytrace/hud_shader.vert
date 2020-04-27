#version 460 core
//#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 inPos;

layout (location = 0) out TVertexData
{
    vec2 pos;
} outData;

out gl_PerVertex {
    vec4 gl_Position;
};

layout (location = 0) uniform mat4 u_projection;
layout (location = 1) uniform mat4 u_modelview;

void main()
{
    outData.pos = inPos;
    gl_Position = u_projection * u_modelview * vec4(outData.pos, 0.0, 1.0);
}