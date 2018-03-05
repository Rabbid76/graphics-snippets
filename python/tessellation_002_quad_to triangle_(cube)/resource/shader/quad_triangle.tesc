#version 400

layout( vertices=3 ) out;

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
        gl_TessLevelOuter[0] = 1.0;
        gl_TessLevelOuter[1] = 1.0;
        gl_TessLevelOuter[2] = 1.0;
        gl_TessLevelInner[0] = 1.0;
    }
}