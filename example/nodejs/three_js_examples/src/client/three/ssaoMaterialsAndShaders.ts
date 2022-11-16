import * as THREE from 'three';

const glslSSAOVertexShader =
`varying vec2 vUv;
void main() {
    vUv = uv;
    gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
}`;

const glslSSAOFragmentShader =
`uniform sampler2D tDiffuse;
uniform sampler2D tNormal;
uniform sampler2D tDepth;
uniform sampler2D tNoise;
uniform vec3 kernel[ KERNEL_SIZE ];
uniform vec2 resolution;
uniform float cameraNear;
uniform float cameraFar;
uniform mat4 cameraProjectionMatrix;
uniform mat4 cameraInverseProjectionMatrix;
uniform float kernelRadius;
uniform float depthBias; // avoid artifacts caused by neighbour fragments with minimal depth difference
uniform float maxDistance; // avoid the influence of fragments which are too far away
uniform float maxDepth;
uniform float intensity;
uniform float fadeout;

varying vec2 vUv;

#include <packing>

float getDepth( const in vec2 screenPosition ) {
    return texture2D( tDepth, screenPosition ).x;
}

float accuratePerspectiveDepthToViewZ(float depth, float n, float f) {
    float z_ndc = 2.0 * depth - 1.0;
    float z_eye = 2.0 * n * f / (f + n - z_ndc * (f - n));
    return z_eye;
}

float getLinearDepth( const in vec2 screenPosition ) {
    #if PERSPECTIVE_CAMERA == 1
        float fragCoordZ = texture2D( tDepth, screenPosition ).x;
        float viewZ = perspectiveDepthToViewZ( fragCoordZ, cameraNear, cameraFar );
        return viewZToOrthographicDepth( viewZ, cameraNear, cameraFar );
    #else
        return texture2D( tDepth, screenPosition ).x;
    #endif
}

float getViewZ( const in float depth ) {
    #if PERSPECTIVE_CAMERA == 1
        return perspectiveDepthToViewZ( depth, cameraNear, cameraFar );
    #else
        return orthographicDepthToViewZ( depth, cameraNear, cameraFar );
    #endif
}

vec3 getViewPosition( const in vec2 screenPosition, const in float depth, const in float viewZ ) {
    float clipW = cameraProjectionMatrix[2][3] * viewZ + cameraProjectionMatrix[3][3];
    vec4 clipPosition = vec4( ( vec3( screenPosition, depth ) - 0.5 ) * 2.0, 1.0 );
    clipPosition *= clipW; // unprojection.
    return ( cameraInverseProjectionMatrix * clipPosition ).xyz;
}

vec3 getViewNormal( const in vec2 screenPosition ) {
    return unpackRGBToNormal( texture2D( tNormal, screenPosition ).xyz );
}

void main() {

    float depth = getDepth(vUv);
    float viewZ = getViewZ(depth);

    vec3 viewPosition = getViewPosition(vUv, depth, viewZ);
    vec3 viewNormal = getViewNormal(vUv);

    vec2 noiseScale = resolution.xy / 4.0;
    vec3 random = texture2D(tNoise, vUv * noiseScale).rrr;

    // compute matrix used to reorient a kernel vector
    vec3 tangent = normalize(random - viewNormal * dot(random, viewNormal));
    vec3 bitangent = cross(viewNormal, tangent);
    mat3 kernelMatrix = mat3(tangent, bitangent, viewNormal);

    float occlusion = 0.0;
    for (int i = 0; i < KERNEL_SIZE; i ++) {

        vec3 sampleVector = kernelMatrix * kernel[i]; // reorient sample vector in view space
        vec3 samplePoint = viewPosition + sampleVector * kernelRadius; // calculate sample point

        vec4 samplePointNDC = cameraProjectionMatrix * vec4(samplePoint, 1.0); // project point and calculate NDC
        samplePointNDC /= samplePointNDC.w;

        vec2 samplePointUv = samplePointNDC.xy * 0.5 + 0.5; // compute uv coordinates

        float realDepth = getLinearDepth(samplePointUv); // get linear depth from depth texture
        float sampleDepth = viewZToOrthographicDepth(samplePoint.z, cameraNear, cameraFar); // compute linear depth of the sample view Z value

        float depthDelta = sampleDepth - realDepth;
        float deltaDistance = depthDelta * (cameraFar - cameraNear);
        float w_long = clamp((maxDistance - deltaDistance) / maxDistance, 0.0, 1.0);
        //float w_long = 1.0 - smoothstep(maxDistance * 0.25, maxDistance, deltaDistance);
        float w_lat = clamp(1.0 - length(sampleVector.xy), 0.0, 1.0);
        occlusion += step(sampleDepth, maxDepth) * step(realDepth, maxDepth) * step(depthBias, depthDelta) * step(deltaDistance, maxDistance) * mix(1.0, w_long * w_lat, fadeout);
    }

    occlusion = clamp(occlusion / float(KERNEL_SIZE)  * intensity * (1.0 + fadeout), 0.0, 1.0);
    gl_FragColor = vec4( vec3( 1.0 - occlusion ), 1.0 );
}`;

export class SSAORenderMaterial extends THREE.ShaderMaterial {
    public static kernelSize: number = 32;
    private static shader: any = {
        uniforms: {
            // @ts-ignore
            tDiffuse: { value: null },
            // @ts-ignore
            tNormal: { value: null },
            // @ts-ignore
            tDepth: { value: null },
            // @ts-ignore
            tNoise: { value: null },
            // @ts-ignore
            kernel: { value: null },
            cameraNear: { value: 0.1 },
            cameraFar: { value: 1 },
            resolution: { value: new THREE.Vector2() },
            cameraProjectionMatrix: { value: new THREE.Matrix4() },
            cameraInverseProjectionMatrix: { value: new THREE.Matrix4() },
            kernelRadius: { value: 8 },
            depthBias: { value: 0.005 },
            maxDistance: { value: 0.05 },
            maxDepth: { value: 0.99 },
            intensity: { value: 1 },
            fadeout: { value: 0 },
        },
        defines: {
            PERSPECTIVE_CAMERA: 1,
            KERNEL_SIZE: this.kernelSize
        },
        vertexShader: glslSSAOVertexShader,
        fragmentShader: glslSSAOFragmentShader
    };

    constructor(parameters?: any) {
        super({
            defines: Object.assign({}, SSAORenderMaterial.shader.defines),
            uniforms: THREE.UniformsUtils.clone(SSAORenderMaterial.shader.uniforms),
            vertexShader: SSAORenderMaterial.shader.vertexShader,
            fragmentShader: SSAORenderMaterial.shader.fragmentShader,
            blending: THREE.NoBlending
        });
        this.update(parameters)
    }

    update(parameters?: any) : SSAORenderMaterial {
        if (parameters?.normalTexture !== undefined) {
            this.uniforms.tNormal.value = parameters?.normalTexture
        }
        if (parameters?.depthTexture !== undefined) {
            this.uniforms.tDepth.value = parameters?.depthTexture
        }
        if (parameters?.noiseTexture !== undefined) {
            this.uniforms.tNoise.value = parameters?.noiseTexture
        }
        if (parameters?.kernel !== undefined) {
            this.uniforms.kernel.value = parameters?.kernel
        }
        if (parameters?.width || parameters?.height) {
            const width = parameters?.width ?? this.uniforms.resolution.value.x;
            const height = parameters?.width ?? this.uniforms.resolution.value.y;
            this.uniforms.resolution.value.set(width, height);
        }
        if (parameters?.camera !== undefined) {
            const camera = parameters?.camera as THREE.OrthographicCamera || parameters?.camera as THREE.PerspectiveCamera;
            this.uniforms.cameraNear.value = camera.near;
            this.uniforms.cameraFar.value = camera.far;
            this.uniforms.cameraProjectionMatrix.value.copy(camera.projectionMatrix);
            this.uniforms.cameraInverseProjectionMatrix.value.copy(camera.projectionMatrixInverse);
        }  
        if (parameters?.kernelRadius !== undefined) {
            this.uniforms.kernelRadius.value = parameters?.kernelRadius
        }
        if (parameters?.depthBias !== undefined) {
            this.uniforms.depthBias.value = parameters?.depthBias
        }
        if (parameters?.maxDistance !== undefined) {
            this.uniforms.maxDistance.value = parameters?.maxDistance
        }
        if (parameters?.maxDepth !== undefined) {
            this.uniforms.maxDepth.value = parameters?.maxDepth
        }
        if (parameters?.intensity !== undefined) {
            this.uniforms.intensity.value = parameters?.intensity
        }
        if (parameters?.fadeout !== undefined) {
            this.uniforms.fadeout.value = parameters?.fadeout
        }
        return this;
    }
}

const glslSSAOBlurVertexShader =
`varying vec2 vUv;
void main() {
    vUv = uv;
    gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
}`;

const glslSSAOBlurFragmentShader =
`uniform sampler2D tDiffuse;
uniform vec2 resolution;
varying vec2 vUv;

void main() {
    vec2 texelSize = 1.0 / resolution ;
    float result = 0.0;
#if OPTIMIZED_BLUR == 1
    result += texture2D(tDiffuse, vUv + texelSize * vec2(-1.0, -1.0)).r;
    result += texture2D(tDiffuse, vUv + texelSize * vec2(-1.0, 1.0)).r;
    result += texture2D(tDiffuse, vUv + texelSize * vec2(1.0, -1.0)).r;
    result += texture2D(tDiffuse, vUv + texelSize * vec2(1.0, 1.0)).r;
    gl_FragColor = vec4(vec3(result / 4.0), 1.0);
#else
    for (int i = - 2; i <= 2; i++) {
        for (int j = - 2; j <= 2; j++) {
            vec2 offset = vec2(float(i), float(j)) * texelSize;
            result += texture2D(tDiffuse, vUv + offset).r;
        }
    }
    gl_FragColor = vec4(vec3(result / 25.0), 1.0);
#endif
}`;

export class SSAOBlurMaterial extends THREE.ShaderMaterial {
    public static optimized: boolean = true;
    private static shader: any = {
        uniforms: {
            // @ts-ignore
            tDiffuse: { value: null },
            resolution: { value: new THREE.Vector2() }
        },
        defines: {
            OPTIMIZED_BLUR: SSAOBlurMaterial.optimized ? 1 : 0
        },
        vertexShader: glslSSAOBlurVertexShader,
        fragmentShader: glslSSAOBlurFragmentShader
    };
    
    constructor(parameters?: any) {
        super({
            defines: Object.assign({}, SSAOBlurMaterial.shader.defines),
            uniforms: THREE.UniformsUtils.clone(SSAOBlurMaterial.shader.uniforms),
            vertexShader: SSAOBlurMaterial.shader.vertexShader,
            fragmentShader: SSAOBlurMaterial.shader.fragmentShader,
            blending: THREE.NoBlending
        });
        this.update(parameters)
    }

    update(parameters?: any) : SSAORenderMaterial {
        if (parameters?.texture !== undefined) {
            this.uniforms.tDiffuse.value = parameters?.texture
        }
        if (parameters?.width || parameters?.height) {
            const width = parameters?.width ?? this.uniforms.resolution.value.x;
            const height = parameters?.height ?? this.uniforms.resolution.value.y;
            this.uniforms.resolution.value.set(width, height);
        }        
        return this;
    }
}