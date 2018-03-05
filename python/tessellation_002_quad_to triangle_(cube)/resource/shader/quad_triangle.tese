
#version 400

layout(triangles, equal_spacing, ccw) in;

in TTessData
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
    vec3 pos = inData[0].pos * gl_TessCoord.x + inData[1].pos * gl_TessCoord.y + inData[2].pos * gl_TessCoord.z;
    vec3 nv  = inData[0].nv  * gl_TessCoord.x + inData[1].nv  * gl_TessCoord.y + inData[2].nv  * gl_TessCoord.z;
    vec3 col = inData[0].col * gl_TessCoord.x + inData[1].col * gl_TessCoord.y + inData[2].col * gl_TessCoord.z;

    outData.pos = pos;
    outData.nv  = nv;
    outData.col = col;

    gl_Position = u_projectionMat44 * vec4( outData.pos, 1.0 );
}