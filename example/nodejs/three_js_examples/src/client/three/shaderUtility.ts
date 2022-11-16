import * as THREE from 'three';
import { CopyShader } from 'three/examples/jsm/shaders/CopyShader.js';

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
        if (parameters?.blending !== undefined) {
            this.blending = parameters?.blending;
        }
        return this;
    }
}