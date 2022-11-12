import { SmoothSSAOPass } from './smoothSsaoPass';
import { SSAODepthShader } from './depthAndNormalMaterialsAndShaders';
import * as THREE from 'three';

export const enum SmoothSSAODebugOutput {
    None,
    Color,
    Depth,
    Normal,
    SSAO,
    SSAOBlur,
}

export class SmoothSSAODebugPass extends SmoothSSAOPass {
    public debugEffect: SmoothSSAODebugOutput;
    private depthRenderMaterial?: THREE.ShaderMaterial;

    constructor(sceneRenderer: THREE.WebGLRenderer, scene: THREE.Scene, camera: THREE.Camera, width: number, height: number, samples: number, parameters?: any) {
        super(sceneRenderer, scene, camera, width, height, samples, parameters);
        this.debugEffect = parameters?.debugEffect ?? SmoothSSAODebugOutput.None;
    }

    public dispose(): void {
        super.dispose();
        this.depthRenderMaterial?.dispose();
    }

    public setSize(width: number, height: number) {
        super.setSize(width, height);
    }

    public render(renderer: THREE.WebGLRenderer, writeBuffer: THREE.WebGLRenderTarget): void {
        if (this.debugEffect === SmoothSSAODebugOutput.Color) {
            return;
        }
        if (this.debugEffect === SmoothSSAODebugOutput.None && !this.ssaoParameters.enabled) {
            return;
        }
        super.render(renderer, writeBuffer);
    }

    protected ssaoIsEnabled(): boolean {
        return (this.ssaoParameters.enabled && this.debugEffect === SmoothSSAODebugOutput.None) ||
            this.debugEffect === SmoothSSAODebugOutput.SSAO ||
            this.debugEffect === SmoothSSAODebugOutput.SSAOBlur;
    }

    protected renderToTarget(renderer: THREE.WebGLRenderer, writeBuffer: THREE.WebGLRenderTarget): void {
        switch (this.debugEffect) {
            case SmoothSSAODebugOutput.Depth: {
                const getDepthRenderMaterial = this.getDepthRenderMaterial();
                this.renderScreenSpacePass(renderer, getDepthRenderMaterial, this.renderToScreen ? null : writeBuffer);
                break;
            }
            case SmoothSSAODebugOutput.Normal: {
                const copyMaterial = this.getCopyMaterial();
                copyMaterial.uniforms.tDiffuse.value = this.ssaoRenderTargets.depthNormalRenderTarget?.texture;
                copyMaterial.blending = THREE.NoBlending;
                this.renderScreenSpacePass(renderer, copyMaterial, this.renderToScreen ? null : writeBuffer);
                break;
            }
            case SmoothSSAODebugOutput.SSAO: {
                const copyMaterial = this.getCopyMaterial();
                copyMaterial.uniforms.tDiffuse.value = this.ssaoRenderTargets.ssaoRenderTarget?.texture;
                copyMaterial.blending = THREE.NoBlending;
                this.renderScreenSpacePass(renderer, copyMaterial, this.renderToScreen ? null : writeBuffer);
                break;
            }
            case SmoothSSAODebugOutput.SSAOBlur: {
                const copyMaterial = this.getCopyMaterial();
                copyMaterial.uniforms.tDiffuse.value = this.ssaoRenderTargets.blurRenderTarget?.texture;
                copyMaterial.blending = THREE.NoBlending;
                this.renderScreenSpacePass(renderer, copyMaterial, this.renderToScreen ? null : writeBuffer);
                break;
            }
            default:
            case SmoothSSAODebugOutput.None: {
                super.renderToTarget(renderer, writeBuffer);
                break;
            }
        }
    }

    private getDepthRenderMaterial(): THREE.ShaderMaterial {
        if (!this.depthRenderMaterial) {
            this.depthRenderMaterial = new THREE.ShaderMaterial( {
                defines: Object.assign({}, SSAODepthShader.defines),
                uniforms: THREE.UniformsUtils.clone(SSAODepthShader.uniforms),
                vertexShader: SSAODepthShader.vertexShader,
                fragmentShader: SSAODepthShader.fragmentShader,
                blending: THREE.NoBlending
            } );
            const depthNormalTarget = this.ssaoRenderTargets.getDepthNormalRenderTarget();
            this.depthRenderMaterial.uniforms.tDepth.value = depthNormalTarget.depthTexture;
        }
        // @ts-ignore
        this.depthRenderMaterial.uniforms.cameraNear.value = this.camera.near;
        // @ts-ignore
        this.depthRenderMaterial.uniforms.cameraFar.value = this.camera.far;
        return this.depthRenderMaterial;
    }
}
