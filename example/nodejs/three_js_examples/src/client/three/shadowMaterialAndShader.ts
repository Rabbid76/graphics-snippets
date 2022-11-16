import * as THREE from 'three';

const glslShadowVertexShader =
`varying vec2 vUv;
void main() {
    vUv = uv;
    gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
}`;

const glslShadowFragmentShader =
`uniform sampler2D tDepth;
uniform sampler2D tShadow;
uniform vec2 resolution;
uniform float cameraNear;
uniform float cameraFar;
uniform mat4 cameraProjectionMatrix;
uniform mat4 cameraInverseProjectionMatrix;
uniform mat4 cameraWorldMatrix;
uniform vec2 shadowMapResolution;
uniform mat4 shadowProjectionMatrix;
uniform mat4 shadowViewMatrix;
uniform float intensity;

varying vec2 vUv;

#include <packing>

float getDepth(const in vec2 screenPosition) {
    return texture2D(tDepth, screenPosition).x;
}

float accuratePerspectiveDepthToViewZ(float depth, float n, float f) {
    float z_ndc = 2.0 * depth - 1.0;
    float z_eye = 2.0 * n * f / (f + n - z_ndc * (f - n));
    return z_eye;
}

float getLinearDepth(const in vec2 screenPosition) {
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

float getShadowDepth(vec3 cameraUVDepth) {
    #if INVERSE_SHADOW_MAP == 1
        return 1.0 - texture2D(tShadow, cameraUVDepth.xy).x * step(max(cameraUVDepth.x, cameraUVDepth.y), 1.0) * step(0.0, min(cameraUVDepth.x, cameraUVDepth.y));
    #else
        return mix(1.0, texture2D(tShadow, cameraUVDepth.xy).x, step(max(cameraUVDepth.x, cameraUVDepth.y), 1.0) * step(0.0, min(cameraUVDepth.x, cameraUVDepth.y)));
    #endif
}

float getInverseShadow(vec3 cameraUVDepth) {
    float shadowDepth = getShadowDepth(cameraUVDepth);
    return step(shadowDepth + 0.005, cameraUVDepth.z) * step(shadowDepth+0.0001, 1.0);
}

void main() {

    float depth = getDepth(vUv);
    float viewZ = getViewZ(depth);
    vec3 viewPosition = getViewPosition(vUv, depth, viewZ);
    vec4 cameraMapNDC = shadowProjectionMatrix * shadowViewMatrix * cameraWorldMatrix * vec4(viewPosition, 1.0);
    cameraMapNDC.xyz /= cameraMapNDC.w;
    vec3 cameraUVDepth = cameraMapNDC.xyz * 0.5 + 0.5;
    #if BLUR == 1
        float inverseShadow = 0.0;
        vec2 offset = 1.0 / shadowMapResolution;
        inverseShadow += getInverseShadow(cameraUVDepth + vec3(-offset.x, -offset.y, 0.0));
        inverseShadow += getInverseShadow(cameraUVDepth + vec3(-offset.x, 0.0, 0.0));
        inverseShadow += getInverseShadow(cameraUVDepth + vec3(-offset.x, offset.y, 0.0));
        inverseShadow += getInverseShadow(cameraUVDepth + vec3(0.0, -offset.y, 0.0));
        inverseShadow += getInverseShadow(cameraUVDepth);
        inverseShadow += getInverseShadow(cameraUVDepth + vec3(0.0, offset.y, 0.0));
        inverseShadow += getInverseShadow(cameraUVDepth + vec3(offset.x, -offset.y, 0.0));
        inverseShadow += getInverseShadow(cameraUVDepth + vec3(offset.x, 0.0, 0.0));
        inverseShadow += getInverseShadow(cameraUVDepth + vec3(offset.x, offset.y, 0.0));
        inverseShadow /= 9.0;
    #else
        float inverseShadow = getInverseShadow(cameraUVDepth);
    #endif
    
    //float shadow = 1.0 - intensity * inverseShadow;
    float shadow = 1.0 - intensity * inverseShadow * (1.0 - step(1.0, depth));

    float linearDepth = getLinearDepth(vUv);
    gl_FragColor = vec4(vec3(shadow), 1.0);
}`;



export class ShadowMaterial extends THREE.ShaderMaterial {
    public static optimized: boolean = true;
    private static shader: any = {
        uniforms: {
            // @ts-ignore
            tDepth: { value: null },
            // @ts-ignore
            tShadow: { value: null },
            cameraNear: { value: 0.1 },
            cameraFar: { value: 1 },
            resolution: { value: new THREE.Vector2() },
            cameraProjectionMatrix: { value: new THREE.Matrix4() },
            cameraInverseProjectionMatrix: { value: new THREE.Matrix4() },
            cameraWorldMatrix: { value: new THREE.Matrix4() },
            shadowMapResolution: { value: new THREE.Vector2() },
            shadowProjectionMatrix: { value: new THREE.Matrix4() },
            shadowViewMatrix: { value: new THREE.Matrix4() },
            intensity: { value: 1 },
        },
        defines: {
            PERSPECTIVE_CAMERA: 1,
            INVERSE_SHADOW_MAP: 0,
            BLUR: 1,
        },
        vertexShader: glslShadowVertexShader,
        fragmentShader: glslShadowFragmentShader
    };
    
    constructor(parameters?: any) {
        super({
            defines: Object.assign({}, ShadowMaterial.shader.defines),
            uniforms: THREE.UniformsUtils.clone(ShadowMaterial.shader.uniforms),
            vertexShader: ShadowMaterial.shader.vertexShader,
            fragmentShader: ShadowMaterial.shader.fragmentShader,
            blending: THREE.NoBlending
        });
        this.update(parameters)
    }

    update(parameters?: any) : ShadowMaterial {
        if (parameters?.depthTexture) {
            this.uniforms.tDepth.value = parameters?.depthTexture
        }
        if (parameters?.shadowTexture) {
            this.uniforms.tShadow.value = parameters?.shadowTexture
        }
        if (parameters?.width || parameters?.height) {
            const width = parameters?.width ?? this.uniforms.resolution.value.x;
            const height = parameters?.height ?? this.uniforms.resolution.value.y;
            this.uniforms.resolution.value.set(width, height);
        }        
        if (parameters?.camera) {
            const camera = parameters?.camera as THREE.OrthographicCamera || parameters?.camera as THREE.PerspectiveCamera;
            this.uniforms.cameraNear.value = camera.near;
            this.uniforms.cameraFar.value = camera.far;
            this.uniforms.cameraProjectionMatrix.value.copy(camera.projectionMatrix);
            this.uniforms.cameraInverseProjectionMatrix.value.copy(camera.projectionMatrixInverse);
            this.uniforms.cameraWorldMatrix.value.copy(camera.matrixWorld);
        }
        if (parameters?.shadowMapResolution) {
            this.uniforms.shadowMapResolution.value.set(parameters.shadowMapResolution.x, parameters.shadowMapResolution.y);
        }
        if (parameters?.shadowCamera) {
            const shadowCamera = parameters?.shadowCamera as THREE.OrthographicCamera || parameters?.shadowCamera as THREE.PerspectiveCamera;
            this.uniforms.shadowProjectionMatrix.value.copy(shadowCamera.projectionMatrix);
            this.uniforms.shadowViewMatrix.value.copy(shadowCamera.matrixWorldInverse);
        }
        if (parameters?.intensity) {
            this.uniforms.intensity.value = parameters?.intensity
        }
        return this;
    }
}