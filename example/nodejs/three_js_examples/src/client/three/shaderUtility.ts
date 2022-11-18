import * as THREE from 'three';

const CopyShader = {
    uniforms: {
        tDiffuse: { value: null },
        opacity: { value: 1.0 },
        brightness: { value: 0.0 }
    },
    vertexShader: /* glsl */`
        varying vec2 vUv;

        void main() {
            vUv = uv;
            gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
        }`,
    fragmentShader: /* glsl */`
        uniform float opacity;
        uniform float brightness;
        uniform sampler2D tDiffuse;
        varying vec2 vUv;

        void main() {
            gl_FragColor = texture2D(tDiffuse, vUv);
            gl_FragColor.rgb = mix(gl_FragColor.rgb, vec3(1.0), brightness);
            gl_FragColor.a *= opacity;
        }`
};

export class CopyMaterial extends THREE.ShaderMaterial {
    constructor(parameters?: any) {
        super({
            uniforms: THREE.UniformsUtils.clone(CopyShader.uniforms),
            vertexShader: CopyShader.vertexShader,
            fragmentShader: CopyShader.fragmentShader,
            transparent: true,
            depthTest: false,
            depthWrite: false,
            blendSrc: THREE.DstColorFactor,
            blendDst: THREE.ZeroFactor,
            blendEquation: THREE.AddEquation,
            blendSrcAlpha: THREE.DstAlphaFactor,
            blendDstAlpha: THREE.ZeroFactor,
            blendEquationAlpha: THREE.AddEquation
        });
        this.update(parameters);
    }

    update(parameters?: any): CopyMaterial {
        if (parameters?.texture !== undefined) {
            this.uniforms.tDiffuse.value = parameters?.texture;
        }
        if (parameters?.opacity !== undefined) {
            this.uniforms.opacity.value = parameters?.opacity;
        }
        if (parameters?.brightness !== undefined) {
            this.uniforms.brightness.value = parameters?.brightness;
        }
        if (parameters?.blending !== undefined) {
            this.blending = parameters?.blending;
        }
        if (parameters?.blendSrc !== undefined) {
            this.blendSrc = parameters?.blendSrc;
        }
        if (parameters?.blendDst !== undefined) {
            this.blendDst = parameters?.blendDst;
        }
        if (parameters?.blendEquation !== undefined) {
            this.blendEquation = parameters?.blendEquation;
        }
        if (parameters?.blendSrcAlpha !== undefined) {
            this.blendSrcAlpha = parameters?.blendSrcAlpha;
        }
        if (parameters?.blendDstAlpha !== undefined) {
            this.blendDstAlpha = parameters?.blendDstAlpha;
        }
        if (parameters?.blendEquationAlpha !== undefined) {
            this.blendEquationAlpha = parameters?.blendEquationAlpha;
        }
        return this;
    }
}