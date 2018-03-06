#version 450

layout( lines_adjacency ) in;
layout( triangle_strip, max_vertices = 4 ) out;

in TVertexData
{
    vec3 pos;
    vec3 nv;
    vec3 col;
} inData[];

out TVertexData
{
    vec3 pos;
    vec3 nv;
    vec3 col;
} outData;

uniform mat4 u_projectionMat44;

void main()
{
    const int inx_map[4] = int[4](0, 1, 3, 2);
    for ( int i=0; i < 4; ++i )
    {
        outData.pos = inData[inx_map[i]].pos;
        outData.nv  = inData[inx_map[i]].nv;
        outData.col = inData[inx_map[i]].col;
        gl_Position = u_projectionMat44 * vec4( outData.pos, 1.0 );
        EmitVertex();
    }
    EndPrimitive();
}