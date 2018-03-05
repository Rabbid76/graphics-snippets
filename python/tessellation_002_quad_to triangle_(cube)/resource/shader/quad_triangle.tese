#version 450

layout(triangles, equal_spacing, ccw) in;

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
    // See [Tessellation Triangles](https://www.khronos.org/opengl/wiki/Tessellation#Triangles)
    // Tessellation Evaluation Shader:
    //     - vertices = 4
    //     - gl_TessLevelOuter = float[4](1.0, 1.0, 2.0, 0.0);
    //    -  gl_TessLevelInner = float[2](1.0, 0.0);   

    int inx = 2;
    if ( gl_TessCoord.z > 0.75 )
        inx = 0;  
    else if ( gl_TessCoord.y > 0.75 )
        inx = 1;  
    else if ( gl_TessCoord.x > 0.75 )
        inx = 3;  

    vec3 pos = inData[inx].pos;
    vec3 nv  = inData[inx].nv;
    vec3 col = inData[inx].col;  

    outData.pos = pos;
    outData.nv  = nv;
    outData.col = col;

    gl_Position = u_projectionMat44 * vec4( outData.pos, 1.0 );
}