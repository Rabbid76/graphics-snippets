#version 450

layout(quads, equal_spacing, ccw) in;
//layout(quads, ccw) in;

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

    float i_quad = dot( vec2(1.0, 2.0), gl_TessCoord.xy );
    int   inx    = inx_map[int(round(i_quad))];

    vec3 pos = inData[inx].pos;
    vec3 nv  = inData[inx].nv;
    vec3 col = inData[inx].col;  

    outData.pos = pos;
    outData.nv  = nv;
    outData.col = col;

    gl_Position = u_projectionMat44 * vec4( outData.pos, 1.0 );
}

/*!

Alternative solution with triangles:

See [Tessellation Triangles](https://www.khronos.org/opengl/wiki/Tessellation#Triangles)

Tessellation Evaluation Shader:

    layout( vertices = 4 ) out;

    gl_TessLevelOuter = float[4](2.0, 1.0, 1.0, 0.0);
    gl_TessLevelInner = float[2](1.0, 0.0);   


Tessellation Contral Shader:

either

    int inx = 0;
    if ( gl_TessCoord.y > 0.75 )
        inx = 1;  
    else if ( gl_TessCoord.x > 0.75 )
        inx = 2;  
    else if ( gl_TessCoord.z > 0.75 )
        inx = 3;

or

    float fi  = dot(vec3(2.0, 1.0, 3.0), step(vec3(0.75), gl_TessCoord.xyz));
    int   inx = int(fi + 0.5);

*/