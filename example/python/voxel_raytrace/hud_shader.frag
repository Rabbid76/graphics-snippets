#version 460 core
//#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in TVertexData
{
    vec2 pos;
} inData;

layout (location = 2) uniform vec4 u_color;

layout (location = 0) out vec4 fragColor;

void main()
{
    fragColor = u_color;
}