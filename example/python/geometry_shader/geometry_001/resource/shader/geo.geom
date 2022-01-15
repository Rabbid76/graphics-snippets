#version 400

layout( invocations = 3 ) in;
layout( points ) in;
layout( triangle_strip, max_vertices = 72 ) out;

in TVertexData
{
    mat3 orientationMat;
} inData[];

out TGeometryData
{
    vec3 pos;
    vec3 nv;
    vec3 col;
} outData;

uniform mat4 u_projectionMat44;
uniform mat4 u_viewMat44;
uniform mat4 u_modelMat44;

void NewVertex( in vec3 pt, in mat4 transMat )
{
    vec4 viewPos = transMat * vec4( pt, 1.0 );
    outData.pos = viewPos.xyz / viewPos.w;
    gl_Position = u_projectionMat44 * viewPos;
    EmitVertex();
}

const int circumferenceTile = 16;

void main()
{
    vec4 origin = gl_in[0].gl_Position;
    origin /= origin.w;
    mat4 orintationMat = mat4( vec4( inData[0].orientationMat[0], 0.0 ),
                               vec4( inData[0].orientationMat[1], 0.0 ),
                               vec4( inData[0].orientationMat[2], 0.0 ),
                               origin );
    mat4 modelViewMat = u_viewMat44 * u_modelMat44 * orintationMat;
    mat3 normalMat = mat3( modelViewMat );
  
    outData.col = vec3( 0.5, 0.7, 0.6 );

    if ( gl_InvocationID == 0 ) // top of the cylinder
    {
        outData.nv  = normalMat * vec3(0.0, 0.0, 1.0);
        for ( int inx = 0; inx < circumferenceTile; inx += 2 )
        {
            float ang1 = 2.0 * 3.14159 * float(inx) / float(circumferenceTile);
            float ang2 = 2.0 * 3.14159 * float(inx+1) / float(circumferenceTile);
            vec2 actPt1 = vec2( sin(ang1), cos(ang1) );
            vec2 actPt2 = vec2( sin(ang2), cos(ang2) );
       
            NewVertex( vec3(actPt1.xy, 1.0), modelViewMat );
            NewVertex( vec3(0.0, 0.0, 1.0), modelViewMat );
            NewVertex( vec3(actPt2.xy, 1.0), modelViewMat );
        }
        NewVertex( vec3(0.0, 0.0, 1.0), modelViewMat );
        NewVertex( vec3(0.0, 1.0, 1.0), modelViewMat );
        EndPrimitive();
    }

    if ( gl_InvocationID == 1 ) // bottom of the cylinder  
    {
        outData.nv  = normalMat * vec3(0.0, 0.0, -1.0);    
        vec2 prevPt = vec2( 0.0, 1.0 );
        for ( int inx = circumferenceTile; inx >= 0; inx -= 2 )
        {
            float ang1 = 2.0 * 3.14159 * float(inx) / float(circumferenceTile);
            float ang2 = 2.0 * 3.14159 * float(inx-1) / float(circumferenceTile);
            vec2 actPt1 = vec2( sin(ang1), cos(ang1) );
            vec2 actPt2 = vec2( sin(ang2), cos(ang2) );    
            
            NewVertex( vec3(actPt1.xy, -1.0), modelViewMat );
            NewVertex( vec3(0.0, 0.0, -1.0), modelViewMat );
            NewVertex( vec3(actPt2.xy, -1.0), modelViewMat );
        }
        NewVertex( vec3(0.0, 0.0, -1.0), modelViewMat );
        NewVertex( vec3(0.0, 1.0, -1.0), modelViewMat );
        EndPrimitive();
    }

    if ( gl_InvocationID == 2 ) // hull of the cylinder
    {
        vec2 prevPt = vec2( 0.0, 1.0 );
        for ( int inx = 1; inx <= circumferenceTile; ++ inx )
        {
            float ang = 2.0 * 3.14159 * float(inx) / float(circumferenceTile);
            vec2 actPt = vec2( sin(ang), cos(ang) );
            
            outData.nv = normalMat * vec3(prevPt, 0.0);
            NewVertex( vec3(prevPt.xy, -1.0), modelViewMat );
            outData.nv = normalMat * vec3(actPt, 0.0);
            NewVertex( vec3(actPt.xy, -1.0), modelViewMat );
            outData.nv = normalMat * vec3(prevPt, 0.0);
            NewVertex( vec3(prevPt.xy, 1.0), modelViewMat );
            outData.nv = normalMat * vec3(actPt, 0.0);
            NewVertex( vec3(actPt.xy, 1.0), modelViewMat );
            
            prevPt = actPt;
        }
        EndPrimitive();
    }
}