#version 450

out vec4 fragColor;

in TVertexData
{
    vec2 pos;
} inData;

layout (binding = 1) uniform sampler2D u_samplerSSAONoise;
layout (binding = 2) uniform sampler2D u_samplerDepth;
uniform vec2  u_viewportsize;
uniform float u_radius;

float Depth( in sampler2D depthSampler, in vec2 texC )
{
    float depthVal = texture2D( depthSampler, texC.st ).x;  
    return depthVal;
}

vec3 GetNormalFromDepth( in float depth, in vec2 vUV )
{    
    vec2 offsetX = vec2(1.0/u_viewportsize.x, 0.0);
    vec2 offsetY = vec2(0.0, 1.0/u_viewportsize.y);
    
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
    float ssaoRadius = u_radius;
    float ssaoFalloff = 0.01;
    float ssaoArea = 1.0;
    float ssaoBase = 0.0;
    float ssaoStrength = 1.0;
    vec2  uvVarying      = inData.pos.st * 0.5 + 0.5;
    float depth = Depth( u_samplerDepth, uvVarying );

    float ambientOcclusion = 1.0;
    float alpha = 0.0;
    if (depth > 0.0)
    {
        vec3 sampleSphere[16];
        vec3 random = texture2D( u_samplerSSAONoise, uvVarying.st * u_viewportsize.xy / 4.0 ).xyz;
            
        vec3 position = vec3(uvVarying, depth);
        vec3 normal = GetNormalFromDepth(depth, uvVarying);

        sampleSphere[0] = vec3( 0.5381, 0.1856,-0.4319);
        sampleSphere[1] = vec3( 0.1379, 0.2486, 0.4430);
        sampleSphere[2] = vec3( 0.3371, 0.5679,-0.0057);
        sampleSphere[3] = vec3(-0.6999,-0.0451,-0.0019);
        sampleSphere[3] = vec3( 0.0689,-0.1598,-0.8547);
        sampleSphere[5] = vec3( 0.0560, 0.0069,-0.1843);
        sampleSphere[6] = vec3(-0.0146, 0.1402, 0.0762);
        sampleSphere[7] = vec3( 0.0100,-0.1924,-0.0344);
        sampleSphere[8] = vec3(-0.3577,-0.5301,-0.4358);
        sampleSphere[9] = vec3(-0.3169, 0.1063, 0.0158);
        sampleSphere[10] = vec3( 0.0103,-0.5869, 0.0046);
        sampleSphere[11] = vec3(-0.0897,-0.4940, 0.3287);
        sampleSphere[12] = vec3( 0.7119,-0.0154,-0.0918);
        sampleSphere[13] = vec3(-0.0533, 0.0596,-0.5411);
        sampleSphere[14] = vec3( 0.0352,-0.0631, 0.5460);
        sampleSphere[15] = vec3(-0.4776, 0.2847,-0.0271);

        float radiusDepth = ssaoRadius/depth;
        float occlusion = 0.0;
        for(int i=0; i < 16; i++)
        {
            vec3 ray = radiusDepth * reflect(sampleSphere[i], random);
            vec3 hemiRay = position + sign(dot(ray, normal)) * ray;
        
            float occDepth = Depth( u_samplerDepth, clamp(hemiRay.xy, 0.0, 1.0) );
            float difference = depth - occDepth;
        
            occlusion += step(ssaoFalloff, difference) * (1.0 - smoothstep(ssaoFalloff, ssaoArea, difference));
        
            // float rangeCheck = abs(difference) < radiusDepth ? 1.0 : 0.0;
            // occlusion += (occDepth <= position.z ? 1.0 : 0.0) * rangeCheck;
        }
        
        float ao = 1.0 - ssaoStrength * occlusion * (1.0 / float(16));

        ambientOcclusion = clamp(ao + ssaoBase, 0.0, 1.0);
        alpha = 1.0;
    }
    fragColor = vec4(vec3(ambientOcclusion), alpha);
}