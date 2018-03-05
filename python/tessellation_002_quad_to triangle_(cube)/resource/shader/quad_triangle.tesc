#version 450

layout( vertices = 4 ) out;

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
} outData[];

void main()
{
    outData[gl_InvocationID].pos = inData[gl_InvocationID].pos;
    outData[gl_InvocationID].nv  = inData[gl_InvocationID].nv;
    outData[gl_InvocationID].col = inData[gl_InvocationID].col;

    if ( gl_InvocationID == 0 )
    {
        gl_TessLevelOuter = float[4](2.0, 1.0, 1.0, 0.0);
        gl_TessLevelInner = float[2](1.0, 0.0);
    }
}