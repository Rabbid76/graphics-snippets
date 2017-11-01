#version 400

layout( vertices=3 ) out;

in TVertexData
{
    vec3 pos;
    vec3 nv;
} inData[];

out TVertexData
{
    vec3 pos;
    vec3 nv;
} outData[];

void main()
{
    outData[gl_InvocationID].pos = inData[gl_InvocationID].pos;
    outData[gl_InvocationID].nv  = inData[gl_InvocationID].nv;
  
    if ( gl_InvocationID == 0 )
    {
        gl_TessLevelOuter[0] = 10.0;
        gl_TessLevelOuter[1] = 10.0;
        gl_TessLevelOuter[2] = 10.0;
        gl_TessLevelInner[0] = 10.0;
    }
}