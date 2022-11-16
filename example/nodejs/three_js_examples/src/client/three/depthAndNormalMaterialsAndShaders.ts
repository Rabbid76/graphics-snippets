import * as THREE from 'three';

const glslLinearDepthVertexShader =
`varying vec2 vUv;
void main() {
    vUv = uv;
    gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
}`;

const glslLinearDepthFragmentShader =
`uniform sampler2D tDepth;
uniform float cameraNear;
uniform float cameraFar;
varying vec2 vUv;

#include <packing>

float getLinearDepth(const in vec2 screenPosition) {
    #if PERSPECTIVE_CAMERA == 1
        float fragCoordZ = texture2D(tDepth, screenPosition).x;
        float viewZ = perspectiveDepthToViewZ(fragCoordZ, cameraNear, cameraFar);
        return viewZToOrthographicDepth(viewZ, cameraNear, cameraFar);
    #else
        return texture2D(tDepth, screenPosition).x;c
    #endif
}

void main() {
    float depth = getLinearDepth(vUv);
    gl_FragColor = vec4(vec3(1.0 - depth), 1.0);
}`;

export class LinearDepthRenderMaterial extends THREE.ShaderMaterial{
    private static linearDepthShader: any = {
        uniforms: {
            // @ts-ignore
            tDepth: { value: null },
            cameraNear: { value: 0.1 },
            cameraFar: { value: 1 },
        },
        defines: {
            PERSPECTIVE_CAMERA: 1
        },
        vertexShader: glslLinearDepthVertexShader,
        fragmentShader: glslLinearDepthFragmentShader
    };

    constructor(parameters?: any) {
        super({
            defines: Object.assign({}, LinearDepthRenderMaterial.linearDepthShader.defines),
            uniforms: THREE.UniformsUtils.clone(LinearDepthRenderMaterial.linearDepthShader.uniforms),
            vertexShader: LinearDepthRenderMaterial.linearDepthShader.vertexShader,
            fragmentShader: LinearDepthRenderMaterial.linearDepthShader.fragmentShader,
            blending: THREE.NoBlending
        });
        this.update(parameters);
    }

    public update(parameters?: any): LinearDepthRenderMaterial {
        if (parameters?.depthTexture !== undefined) {
            this.uniforms.tDepth.value = parameters?.depthTexture;
        }
        if (parameters?.camera !== undefined) {
            const camera = parameters?.camera as THREE.OrthographicCamera || parameters?.camera as THREE.PerspectiveCamera;
            this.uniforms.cameraNear.value = camera.near;
            this.uniforms.cameraFar.value = camera.far;
        }   
        return this;
    }
}

const glslLinearDepthNormalVertexShader =
`varying vec3 vNormal;
varying vec2 vUv;
void main() {
    vNormal = normalMatrix * normal;
    vUv = uv;
    gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
}`;

const glslLinearDepthNormalFragmentShader =
`uniform sampler2D tDepth;
uniform float cameraNear;
uniform float cameraFar;
varying vec3 vNormal;
varying vec2 vUv;

#include <packing>

float getLinearDepth(const in vec2 screenPosition) {
    #if PERSPECTIVE_CAMERA == 1
        float fragCoordZ = texture2D(tDepth, screenPosition).x;
        float viewZ = perspectiveDepthToViewZ(fragCoordZ, cameraNear, cameraFar);
        return viewZToOrthographicDepth(viewZ, cameraNear, cameraFar);
    #else
        return texture2D(tDepth, screenPosition).x;
    #endif
}

void main() {
    float depth = getLinearDepth(vUv);
    gl_FragColor = vec4(vNormal, 1.0 - depth);
}`;

export const linearDepthNormalShader = {
    uniforms: {
        // @ts-ignore
        tDepth: { value: null },
        cameraNear: { value: 0.1 },
        cameraFar: { value: 1 },
    },
    defines: {
        PERSPECTIVE_CAMERA: 1
    },
    vertexShader: glslLinearDepthNormalVertexShader,
    fragmentShader: glslLinearDepthNormalFragmentShader
};

