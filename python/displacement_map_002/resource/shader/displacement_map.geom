#version 450

//layout( invocations = 3 ) in;
layout( triangles ) in;
layout( triangle_strip, max_vertices = 3 ) out;

in TVertexData
{
    vec3 pos;
    vec3 nv;
    vec3 col;
    vec2 uv;
} inData[];

out TGeometryData
{
    vec3  vsPos;
    vec3  vsNV;
    vec3  vsTV;
    float vsBVsign;
    vec3  col;
    vec2  uv;
} outData;

uniform mat4 u_projectionMat44;
uniform mat4 u_viewMat44;
uniform mat4 u_modelMat44;

void main()
{
    if ( gl_InvocationID == 0 )
    {
        // transformation matrices
        mat4 modelViewMat = u_viewMat44 * u_modelMat44;
        mat3 normalMat    = mat3(modelViewMat);

        // view space positions
        vec4 vsPos[3];
        for (int i=0; i<3; ++i)
            vsPos[i] = modelViewMat * vec4(inData[i].pos, 1.0);
        
        // tangent space
        vec3  p_dA       = vsPos[1].xyz - vsPos[0].xyz;
        vec3  p_dB       = vsPos[2].xyz - vsPos[0].xyz;
        vec2  tc_dA      = inData[1].uv - inData[0].uv;
        vec2  tc_dB      = inData[2].uv - inData[0].uv;
        float texDet     = determinant( mat2( tc_dA, tc_dB ) );
        outData.vsTV     = ( tc_dB.y * p_dA - tc_dA.y * p_dB ) / texDet;
        outData.vsBVsign = sign(texDet);

        // main primitive
        for (int i=0; i<3; ++i)
        {
            outData.vsPos = vsPos[i].xyz;
            outData.vsNV  = normalMat * normalize(inData[i].nv);
            outData.col   = inData[i].col;
            outData.uv    = inData[i].uv;
            gl_Position   = u_projectionMat44 * vsPos[i];
            EmitVertex();
        }
        EndPrimitive();
    }
}