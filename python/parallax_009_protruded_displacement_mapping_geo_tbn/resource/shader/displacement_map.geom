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

out TGeometryData
{
    vec3 pos;
    vec3 nv;
    vec3 tv;
    vec3 bv;
    vec3 col;
    vec2 uv;
} outData;

uniform mat4 u_projectionMat44;

void main()
{
    // tangent space
    //vec3  p_dA       = vsPos[1].xyz - vsPos[0].xyz;
    //vec3  p_dB       = vsPos[2].xyz - vsPos[0].xyz;
    //vec2  tc_dA      = inData[1].uv - inData[0].uv;
    //vec2  tc_dB      = inData[2].uv - inData[0].uv;
    //float texDet     = determinant( mat2( tc_dA, tc_dB ) );
    //outData.vsTV     = ( tc_dB.y * p_dA - tc_dA.y * p_dB ) / texDet;
    //outData.vsBVsign = sign(texDet);

    // tangent space
    // Followup: Normal Mapping Without Precomputed Tangents [http://www.thetenthplanet.de/archives/1180]
    vec3  dp1  = inData[1].pos.xyz - inData[0].pos.xyz;
    vec3  dp2  = inData[2].pos.xyz - inData[0].pos.xyz;
    vec2  duv1 = inData[1].uv.xy - inData[0].uv.xy;
    vec2  duv2 = inData[2].uv.xy - inData[0].uv.xy;

    vec3 tv[3];
    vec3 bv[3];
    for ( int i=0; i < 3; ++i )
    {
        vec3 dp2perp = cross(dp2, inData[i].nv); 
        vec3 dp1perp = cross(inData[i].nv, dp1);
        tv[i] = dp2perp * duv1.x + dp1perp * duv2.x;
        bv[i] = dp2perp * duv1.y + dp1perp * duv2.y;
    }

    // distance to edge
    for ( int i=0; i < 3; ++i )
    {
      // X  =  R0 + D * dot(PA - R0, N) / dot(D, N)
    }

    for ( int i=0; i < 3; ++i )
    {
        outData.nv  = inData[i].nv;
        outData.tv  = tv[i];
        outData.bv  = bv[i];
        outData.pos = inData[i].pos;
        outData.col = inData[i].col;
        outData.uv  = inData[i].uv;
        gl_Position = u_projectionMat44 * vec4( outData.pos, 1.0 );
        EmitVertex();
    }
    EndPrimitive();
}