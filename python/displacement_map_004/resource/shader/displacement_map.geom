#version 450

//layout( invocations = 3 ) in;
layout( triangles ) in;
layout( triangle_strip, max_vertices = 15 ) out;

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
    vec3  vsPos_end;
    vec3  vsNV;
    vec3  vsTV;
    float vsBVsign;
    vec3  col;
    vec2  uv;
    vec2  uv_end;
} outData;

uniform mat4 u_projectionMat44;
uniform mat4 u_viewMat44;
uniform mat4 u_modelMat44;

uniform vec2 u_displacement_range;


vec3 Barycentric( in vec3 v_AB, in vec3 v_AC, in vec3 v_AX )
{
    float d00 = dot(v_AB, v_AB);
    float d01 = dot(v_AB, v_AC);
    float d11 = dot(v_AC, v_AC);
    float d20 = dot(v_AX, v_AB);
    float d21 = dot(v_AX, v_AC);
    float denom = d00 * d11 - d01 * d01; // determinant
    vec3  b_coord;
    b_coord.y = (d11 * d20 - d01 * d21) / denom;
    b_coord.z = (d00 * d21 - d01 * d20) / denom;
    b_coord.x = 1.0f - b_coord.y - b_coord.z;
    return b_coord;
}


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

        // view space normal vector
        vec3 vsNV[3];
        for (int i=0; i<3; ++i)
            vsNV[i] = normalMat * normalize(inData[i].nv);

        // view space minimum and maximum displaced position
        vec4 vsPosMin[3];
        vec4 vsPosMax[3];
        for (int i=0; i<3; ++i)
        {
            vsPosMin[i] = vec4(vsPos[i].xyz + vsNV[i] * u_displacement_range.x, 1.0);
            vsPosMax[i] = vec4(vsPos[i].xyz + vsNV[i] * u_displacement_range.y, 1.0);
        }

        // tangent space
        vec3  p_dA       = vsPos[1].xyz - vsPos[0].xyz;
        vec3  p_dB       = vsPos[2].xyz - vsPos[0].xyz;
        vec2  tc_dA      = inData[1].uv - inData[0].uv;
        vec2  tc_dB      = inData[2].uv - inData[0].uv;
        float texDet     = determinant( mat2( tc_dA, tc_dB ) );
        outData.vsTV     = ( tc_dB.y * p_dA - tc_dA.y * p_dB ) / texDet;
        outData.vsBVsign = sign(texDet);

        // primitive face normal vector
        vec3 face_nv = normalize(cross(p_dA, p_dB));

        // Calculate the the texture coordinate range on the view rays:
        //   1. Define a ray by the origin of the view and the corner point of the primitve on the top plane.
        //   2. Get teh intersection point of the ray and the bottom plane.
        //   3. Calculate the barycentric coordinates of the intersection points.
        //   4. Calculate the texture coordinates according to the barycentric coordinates.
        vec3 vs_pt_end[3];
        vec2 uv_end[3];
        for (int i=0; i<3; ++i)
        {
            // Define the ray. The origin of the ray is (0,0,0) in view space
            vec3 ray_dir = normalize(vsPosMax[i].xyz);

            // Intersect the ray and the bottom plane
            float dist   = dot(vsPosMin[i].xyz, face_nv) / dot(ray_dir, face_nv);
            vs_pt_end[i] = ray_dir * dist;

            // Calculate the barycentric coordinates
            vec3 v_AB = vsPosMin[1].xyz - vsPosMin[0].xyz; 
            vec3 v_AC = vsPosMin[2].xyz - vsPosMin[0].xyz;
            vec3 v_AX = vs_pt_end[i] - vsPosMin[0].xyz; 
            vec3 b_coord = Barycentric(v_AB, v_AC, v_AX);

            // Calcualte the texture coordinates
            uv_end[i] = b_coord.x * inData[0].uv + b_coord.y * inData[1].uv + b_coord.z * inData[2].uv;
        }

        // main primitive
        for (int i=0; i<3; ++i)
        {
            outData.vsPos     = vsPosMax[i].xyz;
            outData.vsPos_end = vs_pt_end[i];
            outData.vsNV      = normalMat * normalize(inData[i].nv);
            outData.col       = inData[i].col;
            outData.uv        = inData[i].uv;
            outData.uv_end    = uv_end[i];
            gl_Position       = u_projectionMat44 * vsPosMax[i];
            EmitVertex();
        }
        EndPrimitive();

        /*
        for (int i=0; i<3; ++i)
        {
            int i2 = (i+1) % 3; 

            outData.vsPos = vsPosMin[i].xyz;
            outData.vsNV  = normalMat * normalize(inData[i].nv);
            outData.col   = inData[i].col;
            outData.uv    = inData[i].uv;
            gl_Position   = u_projectionMat44 * vsPosMin[i];
            EmitVertex();

            outData.vsPos = vsPosMin[i2].xyz;
            outData.vsNV  = normalMat * normalize(inData[i2].nv);
            outData.col   = inData[i2].col;
            outData.uv    = inData[i2].uv;
            gl_Position   = u_projectionMat44 * vsPosMin[i2];
            EmitVertex();

            outData.vsPos = vsPosMax[i2].xyz;
            outData.vsNV  = normalMat * normalize(inData[i2].nv);
            outData.col   = inData[i2].col;
            outData.uv    = inData[i2].uv;
            gl_Position   = u_projectionMat44 * vsPosMax[i2];
            EmitVertex();

            outData.vsPos = vsPosMax[i].xyz;
            outData.vsNV  = normalMat * normalize(inData[i].nv);
            outData.col   = inData[i].col;
            outData.uv    = inData[i].uv;
            gl_Position   = u_projectionMat44 * vsPosMax[i];
            EmitVertex();

            EndPrimitive();
        }
        */
    }
}