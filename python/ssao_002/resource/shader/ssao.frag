#version 450

out float outSSAO;

in TVertexData
{
    vec2 pos;
} inData;

layout (binding = 1) uniform sampler2D u_samplerDepth;
layout (binding = 3) uniform sampler2D u_samplerNV;
layout (binding = 5) uniform sampler2D u_samplerSSAOKernel;
layout (binding = 6) uniform sampler2D u_samplerSSAONoise;
uniform vec2  u_viewportsize;
uniform float u_ssao_scale;
uniform vec2  u_depthrange;
uniform mat4  u_projectionMat44;
uniform int   u_kernel_size;
uniform float u_radius;

float Depth( in sampler2D depthSampler, in vec2 texC )
{
    float depthVal = texture( depthSampler, texC.st ).x;  
    return depthVal;
}

float DepthToZ( in float depth )
{
    float near  = u_depthrange.x; // distance to near plane (absolute value)
    float far   = u_depthrange.y; // distance to far plane (absolute value)
    float z_ndc = 2.0 * depth - 1.0;
    float z_eye = 2.0 * near * far / (far + near - z_ndc * (far - near));
    return -z_eye;
}

vec3 GetNormalFromDepth( in float depth, in vec2 vUV )
{    
    vec2 offsetX = vec2(u_ssao_scale/u_viewportsize.x, 0.0);
    vec2 offsetY = vec2(0.0, u_ssao_scale/u_viewportsize.y);
    
    float depthOffsetX = Depth(u_samplerDepth, vUV + offsetX); // Horizontal neighbour
    float depthOffsetY = Depth(u_samplerDepth, vUV + offsetY); // Vertical neighbour
    vec3 normal = vec3(0.0);
    
    vec3 pX = vec3(offsetX, depthOffsetX - depth);
    vec3 pY = vec3(offsetY, depthOffsetY - depth);
    normal = cross(pY, pX);
    normal.z = abs(normal.z); // We want normal.z positive
    
    return normalize(normal); // [-1,1]
}

void main()
{
    float depthBias = 1.0e-5;
    float power     = 1.0;

    float aspect    = u_viewportsize.x / u_viewportsize.y;
    vec2  texC      = inData.pos.st * 0.5 + 0.5;
    vec2  offset    = u_ssao_scale / u_viewportsize.xy;
    float fragDepth = Depth( u_samplerDepth, texC );
    
    //vec3  fragNV    = GetNormalFromDepth( fragDepth, texC );

    vec3  fragNV    = texture( u_samplerNV, texC ).xyz;

    /*
    vec3  fragNVm   = texture( u_samplerNV, texC ).xyz;
    vec3  fragNVx0  = texture( u_samplerNV, texC - vec2(offset.x, 0.0) ).xyz;
    vec3  fragNVx1  = texture( u_samplerNV, texC + vec2(offset.x, 0.0) ).xyz;
    vec3  fragNVy0  = texture( u_samplerNV, texC - vec2(0.0, offset.y) ).xyz;
    vec3  fragNVy1  = texture( u_samplerNV, texC + vec2(0.0, offset.y) ).xyz;
    vec3  fragNV    = normalize(fragNVm + fragNVx0 + fragNVx1 + fragNVy0 + fragNVy1);
    */

    float ambientOcclusion = 1.0;
    float alpha = 0.0;
    if (fragDepth > 0.0)
    {
        vec2 noiseScale   = u_viewportsize.xy / 4.0;
        vec3 randomVal    = texture( u_samplerSSAONoise, texC.st * noiseScale ).xyz;
        vec3 randomVec    = randomVal.xyz * 2.0 - 1.0;
        vec3 tangent      = normalize( randomVec - fragNV * dot( randomVec, fragNV ) );
        mat3 TBN          = mat3( tangent, cross( fragNV, tangent ), fragNV );

        //vec3 fragPosition = vCornerPos * abs( DepthToZ(fragDepth) / u_depthrange.y );
        //vec3 fragPosition = vec3( inData.pos.xy * u_tanFOV_2 * vec2(aspect, 1.0 ), -1.0 ) * abs(DepthToZ(fragDepth));
        vec3 fragPosition = vec3( inData.pos.x / u_projectionMat44[0][0], inData.pos.y / u_projectionMat44[1][1], -1.0 ) * abs(DepthToZ(fragDepth));
    
        float occlusion = 0.0;
        for (int inx = 0; inx < u_kernel_size; inx++)
        {
            //vec3 samplePosition = fragPosition + u_radius * TBN * kernelSamples[i];
            vec4 sampleVal = texture( u_samplerSSAOKernel, vec2( float( inx ) / 32.0, 0.0 ) );
            vec3 samplePosition = fragPosition + u_radius * TBN * sampleVal.xyz * sampleVal.w;
    
            // Project sample position from view space to screen space:
            vec4 offset = u_projectionMat44 * vec4(samplePosition, 1.0);
            offset.xy /= offset.w; // Perspective division -> [-1,1]
            offset.xy = offset.xy * 0.5 + 0.5; // [-1,1] -> [0,1]
    
            // Get current sample depth:
            float sampleZ = DepthToZ( Depth(u_samplerDepth, offset.xy) );
    
            // Range check and accumulate if fragment contributes to occlusion:
            float rangeCheck = step( abs(fragPosition.z - sampleZ), u_radius );
            occlusion += step( samplePosition.z - sampleZ, -depthBias ) * rangeCheck;
        }
        // Inversion
        ambientOcclusion = 1.0 - (occlusion / 32.0);
        ambientOcclusion = pow(ambientOcclusion, power);
        alpha = 1.0;
    }
    outSSAO = ambientOcclusion;
}