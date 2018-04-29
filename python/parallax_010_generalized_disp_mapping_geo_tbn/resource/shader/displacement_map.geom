#version 450

layout( triangles ) in;
layout( triangle_strip, max_vertices = 15 ) out;

in TVertexData
{
    vec3 world_pos;
    vec3 world_nv;
    vec3 col;
    vec2 uv;
} inData[];

out TGeometryData
{
    vec3  pos;
    vec3  nv;
    vec3  tv;
    vec3  bv;
    vec3  col;
    vec3  uvh;
    vec4  d;
    float clip;
} outData;

uniform vec4 u_clipPlane;
uniform mat4 u_viewMat44;
uniform mat4 u_projectionMat44;

uniform float u_displacement_scale;

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

    vec3 world_pos_up[3];
    for (int i = 0; i < 3; ++ i)
        world_pos_up[i] = inData[i].world_pos + inData[i].world_nv * u_displacement_scale;

    vec3 view_nv[3];
    vec3 view_pos[3];
    vec3 view_pos_up[3];
    for (int i = 0; i < 3; ++ i)
    {
        vec4 viewPos   = u_viewMat44 * vec4(inData[i].world_pos, 1.0);
        view_nv[i]     = normalize(mat3(u_viewMat44) * inData[i].world_nv);
        view_pos[i]    = viewPos.xyz;
        view_pos_up[i] = view_pos[i] + view_nv[i] * u_displacement_scale;
        //view_pos_up[i] = (u_viewMat44 * vec4(world_pos_up[i], 1.0)).xyz;
    }

    // tangent space
    // Followup: Normal Mapping Without Precomputed Tangents [http://www.thetenthplanet.de/archives/1180]
    vec3  dp1  = view_pos[1].xyz - view_pos[0].xyz;
    vec3  dp2  = view_pos[2].xyz - view_pos[0].xyz;
    vec2  duv1 = inData[1].uv.xy - inData[0].uv.xy;
    vec2  duv2 = inData[2].uv.xy - inData[0].uv.xy;

    vec3 nv[3];
    vec3 tv[3];
    vec3 bv[3];
    for ( int i=0; i < 3; ++i )
    {
        vec3 dp2perp = cross(dp2, view_nv[i]); 
        vec3 dp1perp = cross(view_nv[i], dp1);
        
        nv[i] = view_nv[i] * u_displacement_scale;
        tv[i] = dp2perp * duv1.x + dp1perp * duv2.x;
        bv[i] = dp2perp * duv1.y + dp1perp * duv2.y;
    }

    
    // distance to opposite planes
    float d[3];
    float d_up[3];
    float d_opp[3];
    float d_opp_up[3];
    float d_top[3];
    for ( int i0=0; i0 < 3; ++i0 )
    {
      d[i0]    = length(view_pos[i0].xyz);
      d_up[i0] = length(view_pos_up[i0].xyz);

      int i1 = (i0+1) % 3; 
      int i2 = (i0+2) % 3; 
      vec3 edge    = view_pos[i2].xyz - view_pos[i1].xyz;
      vec3 edge_up = view_pos_up[i2].xyz - view_pos_up[i1].xyz;
      vec3 up      = view_nv[i1].xyz + view_nv[i2].xyz;

      // intersect the view ray trough a corner point of the prism (with triangular base)
      // with the opposite side face of the prism
      //
      // d = dot(P0 - R0, N) / dot(D, N)
      //
      // R0 : point on the ray
      // D  : direction of the ray
      // P0 : point on the plane
      // N  : norma vector of the plane
      // d  :: distance from R0 to the intersection with the plane along D
      
      //vec3  R0      = vec3(view_pos[i0].xy, 0.0); // for orthographic projection
      //vec3  D       = vec3(0.0, 0.0, -1.0); // for orthographic projection
      vec3  R0      = vec3(0.0); // for persepctive projection
      vec3  D       = normalize(view_pos[i0].xyz); // for persepctive projection
      vec3  N       = normalize(cross(edge, up));
      vec3  P0      = (view_pos[i1].xyz+view_pos[i2].xyz)/2.0;
      d_opp[i0]     = dot(P0 - R0, N) / dot(D, N);

      //vec3  R0_up   = vec2(view_pos_up[i0].xyz, 0.0); // for orthographic projection
      //vec3  D_up  = vec3(0.0, 0.0, -1.0); // for orthographic projection
      vec3  R0_up   = vec3(0.0); // for persepctive projection 
      vec3  D_up    = normalize(view_pos_up[i0].xyz); // for persepctive projection
      vec3  N_up    = normalize(cross(edge_up, up));
      vec3  P0_up   = (view_pos_up[i1].xyz+view_pos_up[i2].xyz)/2.0;
      d_opp_up[i0]  = dot(P0_up - R0_up, N_up) / dot(D_up, N_up);

      //vec3  N_top   = view_nv[i0];
      vec3  N_top   = normalize(view_nv[0]+view_nv[1]+view_nv[2]);
      vec3  P0_top  = (view_pos_up[0].xyz + view_pos_up[1].xyz + view_pos_up[2].xyz)/3.0;
      d_top[i0]     = dot(P0_top - R0, N_top) / dot(D, N_top);
    }

    vec4 clipPlane = vec4(normalize(u_clipPlane.xyz), u_clipPlane.w);

    for ( int i=0; i < 3; ++i )
    {
        outData.nv   = nv[i];
        outData.tv   = tv[i];
        outData.bv   = bv[i];
        outData.pos  = view_pos[i];
        outData.col  = inData[i].col;
        outData.uvh  = vec3(inData[i].uv, 0.0);
        outData.d    = vec4( i==0 ? d_opp[i] : d[i], i==1 ? d_opp[i] : d[i], i==2 ? d_opp[i] : d[i], d_top[i] );
        outData.clip = dot(vec4(inData[i].world_pos, 1.0), clipPlane);
        gl_Position  = u_projectionMat44 * vec4( outData.pos, 1.0 );
        EmitVertex();
    }
    EndPrimitive();

    vec3 cpt_tri = (view_pos[0] + view_pos[1] + view_pos[2]) / 3.0;
    for ( int i0=0; i0 < 3; ++i0 )
    {
        int i1 = (i0+1) % 3;
        int i2 = (i0+2) % 3; 

        vec3 cpt_edge    = (view_pos[i0] + view_pos[i1]) / 2.0;
        vec3 dir_to_edge = cpt_edge - cpt_tri; // direction from thge center of the triangle to the edge

        vec3 edge    = view_pos[i1] - view_pos[i0];
        vec3 nv_edge  = nv[i0] + nv[i1];
        vec3 nv_side = cross(edge, nv_edge); // normal vector of a side of the prism
        nv_side *= sign(dot(nv_side, dir_to_edge)); // orentate the normal vector out of the center of the triangle

        // a front face is a side of the prism, where the normal vector is directed against the view vector
        float frontface = sign(dot(cpt_edge, -nv_side));

        float d_opp0, d_opp1, d_opp_up0, d_opp_up1;
        if ( frontface > 0.0 )
        {
            d_opp0    = max(d[i0], d_opp[i0]);
            d_opp1    = max(d[i1], d_opp[i1]);
            d_opp_up0 = max(d_up[i0], d_opp_up[i0]);
            d_opp_up1 = max(d_up[i1], d_opp_up[i1]);
        }
        else
        {
            d_opp0    = min(d[i0], d_opp[i0]);
            d_opp1    = min(d[i1], d_opp[i1]);
            d_opp_up0 = min(d_up[i0], d_opp_up[i0]);
            d_opp_up1 = min(d_up[i1], d_opp_up[i1]);
        }

        outData.nv   = nv[i0];
        outData.tv   = tv[i0];
        outData.bv   = bv[i0];
        outData.pos  = view_pos[i0];
        outData.col  = inData[i0].col;
        outData.uvh  = vec3(inData[i0].uv, 0.0);
        outData.d    = vec4(d_opp0, d[i0], frontface, d_top[i0]);
        outData.clip = dot(vec4(inData[i0].world_pos, 1.0), clipPlane);
        gl_Position  = u_projectionMat44 * vec4( outData.pos, 1.0 );
        EmitVertex();

        outData.nv   = nv[i1];
        outData.tv   = tv[i1];
        outData.bv   = bv[i1];
        outData.pos  = view_pos[i1];
        outData.col  = inData[i1].col;
        outData.uvh  = vec3(inData[i1].uv, 0.0);
        outData.d    = vec4(d[i1], d_opp1, frontface, d_top[i1]);
        outData.clip = dot(vec4(inData[i1].world_pos, 1.0), clipPlane);
        gl_Position  = u_projectionMat44 * vec4( outData.pos, 1.0 );
        EmitVertex();

        outData.nv   = nv[i0];
        outData.tv   = tv[i0];
        outData.bv   = bv[i0];
        outData.pos  = view_pos_up[i0];
        outData.col  = inData[i0].col;
        outData.uvh  = vec3(inData[i0].uv, 1.0);
        outData.d    = vec4(d_opp_up0, d_up[i0], frontface, 0.0);
        outData.clip = dot(vec4(world_pos_up[i0], 1.0), clipPlane);
        gl_Position  = u_projectionMat44 * vec4( outData.pos, 1.0 );
        EmitVertex();

        outData.nv   = nv[i1];
        outData.tv   = tv[i1];
        outData.bv   = bv[i1];
        outData.pos  = view_pos_up[i1];
        outData.col  = inData[i1].col;
        outData.uvh  = vec3(inData[i1].uv, 1.0);
        outData.d    = vec4(d_up[i1], d_opp_up1, frontface, 0.0);
        outData.clip = dot(vec4(world_pos_up[i1], 1.0), clipPlane);
        gl_Position  = u_projectionMat44 * vec4( outData.pos, 1.0 );
        EmitVertex();

        EndPrimitive();
    }
}