const glslSSAODepthVertexShader =
`varying vec2 vUv;
void main() {
    vUv = uv;
    gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
}`;

const glslSSAODepthFragmentShader =
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
        return texture2D(tDepth, screenPosition).x;
    #endif
}

void main() {
    float depth = getLinearDepth(vUv);
    gl_FragColor = vec4(vec3(1.0 - depth), 1.0);
}`;

export const SSAODepthShader = {
    uniforms: {
        // @ts-ignore
        tDepth: { value: null },
        cameraNear: { value: 0.1 },
        cameraFar: { value: 1 },
    },
    defines: {
        PERSPECTIVE_CAMERA: 1
    },
    vertexShader: glslSSAODepthVertexShader,
    fragmentShader: glslSSAODepthFragmentShader
};

const glslSSAODepthNormalShader =
`varying vec3 vNormal;
varying vec2 vUv;
void main() {
    vNormal = normalMatrix * normal;
    vUv = uv;
    gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
}`;

const glslSSAODepthNormalFragmentShader =
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

export const SSAODepthNormalShader = {
    uniforms: {
        // @ts-ignore
        tDepth: { value: null },
        cameraNear: { value: 0.1 },
        cameraFar: { value: 1 },
    },
    defines: {
        PERSPECTIVE_CAMERA: 1
    },
    vertexShader: glslSSAODepthNormalShader,
    fragmentShader: glslSSAODepthNormalFragmentShader
};

