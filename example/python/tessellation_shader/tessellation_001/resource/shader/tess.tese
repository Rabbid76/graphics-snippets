
#version 400

layout(triangles, equal_spacing, ccw) in;

in TVertexData
{
    vec3 pos;
    vec3 nv;
} inData[];

out TTessData
{
    vec3  pos;
    vec3  nv;
    float height;
} outData;

uniform mat4 u_projectionMat44;

void main()
{
    float sideLen[3] = float[3]
    (
        length( inData[1].pos - inData[0].pos ),
        length( inData[2].pos - inData[1].pos ),
        length( inData[0].pos - inData[2].pos )
    );
    float s = ( sideLen[0] + sideLen[1] + sideLen[2] ) / 2.0;
    float rad = sqrt( (s - sideLen[0]) * (s - sideLen[1]) * (s - sideLen[2]) / s );

    vec3 cpt = ( inData[0].pos + inData[1].pos + inData[2].pos ) / 3.0;
    vec3 pos = inData[0].pos * gl_TessCoord.x + inData[1].pos * gl_TessCoord.y + inData[2].pos * gl_TessCoord.z;
    vec3 nv  = normalize( inData[0].nv * gl_TessCoord.x + inData[1].nv * gl_TessCoord.y + inData[2].nv * gl_TessCoord.z );

    float cptDist      = length( cpt - pos );
    float sizeRelation = 1.0 - min( rad, cptDist ) / rad; 
    float height       = pow( sizeRelation, 2.0 );

    outData.pos    = pos + nv * height * rad;
    outData.nv     = mix( nv, normalize( pos - cpt ), height );
    outData.height = height;

    gl_Position = u_projectionMat44 * vec4( outData.pos, 1.0 );
}