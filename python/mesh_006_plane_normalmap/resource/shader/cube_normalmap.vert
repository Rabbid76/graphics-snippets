#version 400

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNV;
layout (location = 2) in vec3 inTV;
layout (location = 3) in vec3 inCol;
layout (location = 4) in vec2 inUV;

out vec3 vertPos;
out vec3 vertNV;
out vec3 vertTV;
out vec3 vertCol;
out vec2 vertUV;

out vec3 L;
out vec3 V;

uniform mat4 u_projectionMat44;
uniform mat4 u_viewMat44;
uniform mat4 u_modelMat44;

uniform vec3  u_lightDir;

void main()
{
    vec3 modelNV  = mat3( u_modelMat44 ) * normalize( inNV );
    vertNV        = mat3( u_viewMat44 ) * modelNV;
    vec3 modelTV  = mat3( u_modelMat44 ) * normalize( inTV );
    vertTV        = mat3( u_viewMat44 ) * modelTV;
    vertCol       = inCol;
    vertUV        = inUV;
    vec4 modelPos = u_modelMat44 * vec4( inPos, 1.0 );
    vec4 viewPos  = u_viewMat44 * modelPos;
    vertPos       = viewPos.xyz / viewPos.w;
    gl_Position = u_projectionMat44 * viewPos;


    vec3 surfaceNormal = vertNV.xyz;
    vec3 norm = normalize(surfaceNormal);

    vec3 tang = normalize(vertTV.xyz);
    vec3 bitang = normalize(cross(norm, tang));

    mat3 toTangentSpace = mat3( tang, bitang, norm );

    vec3 mvPosition = viewPos.xyz;

    L=  toTangentSpace *  (-u_lightDir);
    //L =  -u_lightDir;

    V =  toTangentSpace *  (-mvPosition);
}