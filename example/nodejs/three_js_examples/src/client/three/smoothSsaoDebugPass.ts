import { SmoothSSAOPass } from './smoothSsaoPass';
import { LinearDepthRenderMaterial } from './shaderUtility';
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
    private _depthRenderMaterial?: LinearDepthRenderMaterial;

    // TODO property
    private get depthRenderMaterial(): LinearDepthRenderMaterial {
        this._depthRenderMaterial ??= new LinearDepthRenderMaterial({ 
            depthTexture: this.depthNormalRenderTarget.getDepthBufferTexture()
        });
        // @ts-ignore
        return this._depthRenderMaterial.update({camera: this.camera});
    }

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
                this.renderPass.renderScreenSpace(renderer, this.depthRenderMaterial, this.renderToScreen ? null : writeBuffer);
                break;
            }
            case SmoothSSAODebugOutput.Normal: {
                this.renderPass.renderScreenSpace(renderer, this.getCopyMaterial({texture: this.depthNormalRenderTarget?.getGBufferTexture(), blending: THREE.NoBlending}), this.renderToScreen ? null : writeBuffer);
                break;
            }
            case SmoothSSAODebugOutput.SSAO: {
                this.renderPass.renderScreenSpace(renderer, this.getCopyMaterial({texture: this.ssaoRenderTargets.ssaoRenderTarget.texture, blending: THREE.NoBlending}), this.renderToScreen ? null : writeBuffer);
                break;
            }
            case SmoothSSAODebugOutput.SSAOBlur: {
                this.renderPass.renderScreenSpace(renderer, this.getCopyMaterial({texture: this.ssaoRenderTargets.blurRenderTarget.texture, blending: THREE.NoBlending}), this.renderToScreen ? null : writeBuffer);
                break;
            }
            default:
            case SmoothSSAODebugOutput.None: {
                super.renderToTarget(renderer, writeBuffer);
                break;
            }
        }
    }
}
