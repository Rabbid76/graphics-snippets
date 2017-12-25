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
    noperspective vec3  uv0;
    noperspective vec3  uv1;
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
        //   2. Top plane
        //     1.1 Define a ray by the origin of the view and the corner point of the primitve on the bottom plane.
        //     1.2. Get the intersection point of the ray and the top plane.
        //     1.3. Calculate the distance relation between the top intersection point the bottom corner point and.
        //   3. Find the inner primitive points and texturcoordinates
        const int i_top = 3;
        vec3  pos_X[6];
        float dist_rel[6];
        vec3  b_c0[6] = vec3[6](
            vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0),
            vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0) ); 
        vec3  b_c1[6] = vec3[6](
            vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0),
            vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0) ); 
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
            b_c0[i] = Barycentric(vec3(v_AB0.xy,0.0), vec3(v_AC0.xy,0.0), vec3(v_AX0.xy,0.0));
            

            // top plane intersection

            // Define the ray trough the bottom corner point. The origin of the ray is (0,0,0) in view space
            vec3 ray0_dir = normalize(vsPosMin[i].xyz);

            // Intersect the ray and the top plane
            float dist1      = dot(vsPosMax[i].xyz, face1_nv) / dot(ray0_dir, face1_nv);
            pos_X[i_top + i] = ray0_dir * dist1;

            // Distance relation between the top corner point and the bootom intersection point
            dist_rel[i_top + i] = length(vsPosMin[i].xyz) / length(pos_X[i_top + i]); 

            // Calculate the barycentric coordinates
            vec3 v_AX1      = pos_X[i_top + i] - vsPosMax[0].xyz; 
            b_c1[i_top + i] = Barycentric(vec3(v_AB1.xy,0.0), vec3(v_AC1.xy,0.0), vec3(v_AX1.xy,0.0));
        }

        float dist_w = (dist_rel[0]+dist_rel[1]+dist_rel[2])/3.0;
        float fornt_face = step(1.0, dist_w);

        int i_in[3];
        int i_out[3];
        vec3  pos_in[3];
        vec3  pos_out[3];
        float topEdgeIsOuter[3] = float[3](1.0, 1.0, 1.0);
        for (int i=0; i<3; ++i)
        {
            // inner points and coordiantes
            int i2 = (i+1)%3;
            int i3 = (i+2)%3;
            //topEdgeIsOuter[i] = step(b_c0[i][i], 1.0) * step(0.0, b_c0[i][i3]) * step(b_c0[i2][i2], 1.0) * step(0.0, b_c0[i2][i3]); 

            // TODO $$$ topEdgeIsOuter by normal vector !!!
            
            i_in[i]       = ( topEdgeIsOuter[i] > 0.5 ) ? 3 : 0;
            i_out[i]      = ( topEdgeIsOuter[i] > 0.5 ) ? 0 : 3;
            pos_in[i]     = ( topEdgeIsOuter[i] > 0.5 ) ? pos_X[i_top + i] : vsPosMax[i].xyz;
            //pos_out[i]    = ( topEdgeIsOuter[i] > 0.5 ) ? vsPosMax[i].xyz : pos_X[i];
            //pos_out[i]    = ( topEdgeIsOuter[i] > 0.5 ) ? pos_X[i] : vsPosMin[i].xyz;
            pos_out[i]    = ( topEdgeIsOuter[i] > 0.5 ) ? vsPosMax[i].xyz : pos_X[i_top + i];
        }

        // TODO $$$ normal vector by barycentric coordinates

        // main primitive
        //if ( dist_w > 1.0)
        //if ( dist_w < 1.0)
        {

        for (int i=0; i<3; ++i)
        {
            int k = i_in[i] + i;

            outData.vsPos1      = pos_in[i].xyz;
            outData.vsPos_rel01 = dist_rel[k];
            outData.uv0         = vec3(b_c0[k].x * inData[0].uv + b_c0[k].y * inData[1].uv + b_c0[k].z * inData[2].uv, 0.0);
            outData.uv1         = vec3(b_c1[k].x * inData[0].uv + b_c1[k].y * inData[1].uv + b_c1[k].z * inData[2].uv, 1.0);
            outData.vsNV        = normalize(b_c0[k].x * vsNV[0] + b_c0[k].y * vsNV[1] + b_c0[k].z * vsNV[2]);
            outData.col         = inData[i].col;
            //gl_Position         = u_projectionMat44 * vec4(i_in[i] > 0 ? vsPosMin[i].xyz : vsPosMax[i].xyz, 1.0);
            vec3 map_pt = i_in[i] > 0 ? vsPosMin[i].xyz : pos_X[i].xyz;
            //if ( map_pt.z > -0.5 ) map_pt.xyz *= 0.5 / abs(map_pt.z);  
            gl_Position = u_projectionMat44 * vec4(map_pt.xyz, 1.0);
            EmitVertex();
        }
        EndPrimitive();      
        
        int minmax = 0;
        for (int i_edge=0; i_edge<3; ++i_edge)
        {
            //break;
            //if ( topEdgeIsOuter[i_edge] < 0.5 )
            //    break;

            int i1 = i_edge % 3;
            int i2 = (i_edge+1) % 3;
            //if ( dist_w >= 1.0 && b_c1[i_in[i1]+i1][i1] <= 1.0 && b_c1[i_in[i2]+i2][i2] <= 1.0 )
            //    break; 

            for (int i_pt=0; i_pt<2; ++i_pt )
            {
                int i = (i_edge+i_pt) % 3;
                int k = i_out[i] + i;
                
                outData.vsPos1      = pos_out[i].xyz;
                outData.vsPos_rel01 = dist_rel[k];
                outData.uv0         = vec3(b_c0[k].x * inData[0].uv + b_c0[k].y * inData[1].uv + b_c0[k].z * inData[2].uv, topEdgeIsOuter[i_edge] );
                outData.uv1         = vec3(b_c1[k].x * inData[0].uv + b_c1[k].y * inData[1].uv + b_c1[k].z * inData[2].uv, topEdgeIsOuter[i_edge] );
                outData.vsNV        = normalize(b_c1[k].x * vsNV[0] + b_c1[k].y * vsNV[1] + b_c1[k].z * vsNV[2]);
                outData.col         = inData[i].col;
                vec3 map_pt;
                if ( minmax == 0 )
                  map_pt = i_in[i] > 0 ? pos_X[i].xyz : vsPosMin[i].xyz;
                else
                  map_pt = i_in[i] > 0 ? vsPosMax[i].xyz : pos_X[i+3].xyz;
                //if ( map_pt.z > -0.5 ) map_pt.xyz *= 0.5 / abs(map_pt.z); 
                gl_Position = u_projectionMat44 * vec4(map_pt.xyz, 1.0);
                EmitVertex();
            }

            for (int i_pt=0; i_pt<2; ++i_pt )
            {
                int i = (i_edge+i_pt) % 3;
                int k = i_in[i] + i;
                
                outData.vsPos1      = pos_in[i].xyz;
                outData.vsPos_rel01 = dist_rel[k];
                outData.uv0         = vec3(b_c0[k].x * inData[0].uv + b_c0[k].y * inData[1].uv + b_c0[k].z * inData[2].uv, 0.0);
                outData.uv1         = vec3(b_c1[k].x * inData[0].uv + b_c1[k].y * inData[1].uv + b_c1[k].z * inData[2].uv, 1.0);
                outData.vsNV        = normalize(b_c0[k].x * vsNV[0] + b_c0[k].y * vsNV[1] + b_c0[k].z * vsNV[2]);
                outData.col         = inData[i].col;
                vec3 map_pt;
                if ( minmax == 0 )
                  map_pt = i_in[i] > 0 ? vsPosMin[i].xyz : pos_X[i].xyz;
                else
                  map_pt = i_in[i] > 0 ? pos_X[i+3].xyz : vsPosMax[i].xyz;
                //if ( map_pt.z > -0.5 ) map_pt.xyz *= 0.5 / abs(map_pt.z); 
                gl_Position = u_projectionMat44 * vec4(map_pt.xyz, 1.0);
                EmitVertex();
            }

            EndPrimitive();
        }
        
        }
    }
}