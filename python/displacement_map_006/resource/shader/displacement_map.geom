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
    vec3  vsPos1;
    float vsPos_rel01;
    vec3  vsNV;
    vec3  vsTV;
    float vsBVsign;
    vec3  col;
    vec3  uv0;
    vec3  uv1;
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

        // primitive face normal vectors of the top and bottom primitive
        vec3 v_AB0    = vsPosMin[1].xyz - vsPosMin[0].xyz; 
        vec3 v_AC0    = vsPosMin[2].xyz - vsPosMin[0].xyz;
        vec3 face0_nv = normalize(cross(v_AB0, v_AC0));
        vec3 v_AB1    = vsPosMax[1].xyz - vsPosMax[0].xyz; 
        vec3 v_AC1    = vsPosMax[2].xyz - vsPosMax[0].xyz;
        vec3 face1_nv = normalize(cross(v_AB1, v_AC1));

        // Calculate intersection points and intersection texture coordinates and
        // calculate the the texture coordinate range on the view rays:
        //   1. Bottom plane
        //     1.1 Define a ray by the origin of the view and the corner point of the primitve on the top plane.
        //     1.2. Get the intersection point of the ray and the bottom plane.
        //     1.3. Calculate the distance relation between the top corner point and the bootom intersection point.
        //     1.4. Calculate the texture coordinates according to the barycentric coordinates. 
        //   2. Top plane
        //     1.1 Define a ray by the origin of the view and the corner point of the primitve on the bottom plane.
        //     1.2. Get the intersection point of the ray and the top plane.
        //     1.3. Calculate the distance relation between the top intersection point the bottom corner point and.
        //     1.4. Calculate the texture coordinates according to the barycentric coordinates. 
        //   3. Find the inner primitive points and texturcoordinates
        const int i_top = 3;
        vec3  pos_X[6];
        float dist_rel[6];
        vec3  b_coord[6];
        vec2  uv0[6];
        vec2  uv1[6];
        for (int i=0; i<3; ++i)
        {
            // bottom plane intersection

            // Define the ray trough the top corner point. The origin of the ray is (0,0,0) in view space
            vec3 ray1_dir = normalize(vsPosMax[i].xyz);

            // Intersect the ray and the bottom plane
            float dist0  = dot(vsPosMin[i].xyz, face0_nv) / dot(ray1_dir, face0_nv);
            pos_X[i] = ray1_dir * dist0;
     
            // Distance relation between the top corner point and the bootom intersection point
            dist_rel[i] = length(pos_X[i]) / length(vsPosMax[i].xyz); 
       
            // Calculate the barycentric coordinates
            vec3 v_AX0  = pos_X[i] - vsPosMin[0].xyz; 
            b_coord[i] = Barycentric(v_AB0, v_AC0, v_AX0);

            // Calcualte the texture coordinates
            uv0[i]         = b_coord[i].x * inData[0].uv + b_coord[i].y * inData[1].uv + b_coord[i].z * inData[2].uv;
            uv0[i_top + i] = inData[i].uv; 
            

            // top plane intersection

            // Define the ray trough the bottom corner point. The origin of the ray is (0,0,0) in view space
            vec3 ray0_dir = normalize(vsPosMin[i].xyz);

            // Intersect the ray and the top plane
            float dist1      = dot(vsPosMax[i].xyz, face1_nv) / dot(ray0_dir, face1_nv);
            pos_X[i_top + i] = ray0_dir * dist1;

            // Distance relation between the top corner point and the bootom intersection point
            dist_rel[i_top + i] = length(vsPosMin[i].xyz) / length(pos_X[i_top + i]); 

            // Calculate the barycentric coordinates
            vec3 v_AX1  = pos_X[i_top + i] - vsPosMax[0].xyz; 
            b_coord[i_top + i] = Barycentric(v_AB1, v_AC1, v_AX1);

            // Calcualte the texture coordinates
            uv1[i]         = inData[i].uv;
            uv1[i_top + i] = b_coord[i_top + i].x * inData[0].uv + b_coord[i_top + i].y * inData[1].uv + b_coord[i_top + i].z * inData[2].uv;
        }

        float dist_w = (dist_rel[0]+dist_rel[1]+dist_rel[2])/3.0;

        int i_in[3];
        int i_out[3];
        vec3  pos_in[3];
        vec3  pos_out[3];
        float topIsOuter[3];
        for (int i=0; i<3; ++i)
        {
            // inner points and coordiantes
            //topIsOuter[i] = 
            //  step(0.0, b_coord[i_top+i].x) * step(0.0, b_coord[i_top+i].y) * step(0.0, b_coord[i_top+i].z) *
            //  step(b_coord[i_top+i].x, 1.0) * step(b_coord[i_top+i].y, 1.0) * step(b_coord[i_top+i].z, 1.0);
            //topIsOuter[i] = step(b_coord[i_top+i][i], 1.0);
            topIsOuter[i] = 1.0;
            //topIsOuter[i] = dist_w >= 1.0 ? 1.0 : 0.0;
            i_in[i]       = ( topIsOuter[i] > 0.5 ) ? 3 : 0;
            i_out[i]      = ( topIsOuter[i] > 0.5 ) ? 0 : 3;
            pos_in[i]     = ( topIsOuter[i] > 0.5 ) ? pos_X[i_top + i] : vsPosMax[i].xyz;
            //pos_out[i]    = ( topIsOuter[i] > 0.5 ) ? vsPosMax[i].xyz : pos_X[i];
            pos_out[i]    = ( topIsOuter[i] > 0.5 ) ? vsPosMax[i].xyz : pos_X[i_top + i];
        }

        // TODO $$$ normal vector by barycentric coordinates

        // main primitive
        //if ( dist_w > 1.0)
        //if ( dist_w < 1.0)
        {

        for (int i=0; i<3; ++i)
        {
            outData.vsPos1      = pos_in[i].xyz;
            outData.vsPos_rel01 = dist_rel[i_in[i] + i];
            outData.uv0         = vec3(uv0[i_in[i] + i], 0.0);
            outData.uv1         = vec3(uv1[i_in[i] + i], 1.0);
            outData.vsNV        = normalMat * normalize(inData[i].nv);
            outData.col         = inData[i].col;
            gl_Position         = u_projectionMat44 * vec4(pos_in[i].xyz, 1.0);
            EmitVertex();
        }
        EndPrimitive();       
        
        for (int i_edge=0; i_edge<3; ++i_edge)
        {
            for (int i_pt=0; i_pt<2; ++i_pt )
            {
                //int i = (i_edge+1-i_pt) % 3;
                int i = (i_edge+i_pt) % 3;
                //int i = (dist_w >= 1.0) ? ((i_edge+i_pt) % 3) : ((i_edge+1-i_pt) % 3);
                
                outData.vsPos1      = pos_out[i].xyz;
                //outData.vsPos1      = vsPosMax[i].xyz;
                //outData.vsPos_rel01 = 1.0;
                //outData.uv0         = vec3(inData[i].uv, topIsOuter[i]);
                //outData.uv1         = vec3(inData[i].uv, topIsOuter[i]);
                outData.vsPos_rel01 = dist_rel[i_out[i] + i];
                outData.uv0         = vec3(uv0[i_out[i] + i], topIsOuter[i]);
                outData.uv1         = vec3(uv1[i_out[i] + i], topIsOuter[i]);
                outData.vsNV        = normalMat * normalize(inData[i].nv);
                outData.col         = inData[i].col;
                gl_Position         = u_projectionMat44 * vec4(pos_out[i].xyz, 1.0);
                //gl_Position         = u_projectionMat44 * vec4(vsPosMax[i].xyz, 1.0);
                EmitVertex();
            }

            for (int i_pt=0; i_pt<2; ++i_pt )
            {
                //int i = (i_edge+1-i_pt) % 3;
                int i = (i_edge+i_pt) % 3;
                //int i = (dist_w >= 1.0) ? ((i_edge+i_pt) % 3) : ((i_edge+1-i_pt) % 3);
                
                outData.vsPos1      = pos_in[i].xyz;
                outData.vsPos_rel01 = dist_rel[i_in[i] + i];
                outData.uv0         = vec3(uv0[i_in[i] + i], 0.0);
                outData.uv1         = vec3(uv1[i_in[i] + i], 1.0);
                outData.vsNV        = normalMat * normalize(inData[i].nv);
                outData.col         = inData[i].col;
                gl_Position         = u_projectionMat44 * vec4(pos_in[i].xyz, 1.0);
                EmitVertex();
            }
        }
        EndPrimitive();
    
        }
    }
}