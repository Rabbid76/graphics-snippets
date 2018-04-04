#version 450

layout( triangles ) in;
layout( triangle_strip, max_vertices = 3 ) out;

in TVertexData
{
    vec3 pos;
    vec3 nv;
    vec3 col;
    vec2 uv;
} inData[];

out TVertexData
{
    vec3 pos;
    vec3 nv;
    vec3 col;
    vec2 uv;
} outData;

uniform mat4 u_projectionMat44;

void main()
{
    for ( int i=0; i < 3; ++i )
    {
        outData.pos = inData[i].pos;
        outData.nv  = inData[i].nv;
        outData.col = inData[i].col;
        outData.uv  = inData[i].uv;
        gl_Position = u_projectionMat44 * vec4( outData.pos, 1.0 );
        EmitVertex();
    }
    EndPrimitive();
}