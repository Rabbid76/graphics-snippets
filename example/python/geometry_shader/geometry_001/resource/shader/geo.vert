#version 400

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;

out TVertexData
{
    mat3 orientationMat;
} outData;

void main()
{
    vec3 normal   = normalize( inNormal );
    vec3 tangent  = normalize( inTangent );
    vec3 binormal = cross( tangent, normal );
    
    outData.orientationMat = mat3( normal, cross( binormal, normal ), binormal );
    gl_Position = vec4( inPos, 1.0 );
}