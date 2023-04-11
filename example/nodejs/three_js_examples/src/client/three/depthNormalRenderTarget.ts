import {
    RenderOverrideVisibility,
    RenderPass,
} from './threeUtility';
import * as THREE from 'three';

export interface DepthAndNormalTextures {
    get normalTexture(): THREE.Texture;
    get depthTexture(): THREE.Texture;
} 

export class DepthNormalRenderTarget implements DepthAndNormalTextures {
    private depthNormalScale = 1;
    private targetMinificationTextureFilter: THREE.TextureFilter;
    private targetMagnificationTextureFilter: THREE.MagnificationTextureFilter;
    private width: number;
    private height: number;
    private samples: number;
    private _normalRenderMaterial?: THREE.MeshNormalMaterial;
    private _depthNormalRenderTarget?: THREE.WebGLRenderTarget;
    private renderPass: RenderPass;
    private renderOverrideVisibility: RenderOverrideVisibility;

    public get normalTexture(): THREE.Texture { return this.depthNormalRenderTarget.texture; }
    public get depthTexture(): THREE.Texture { return this.depthNormalRenderTarget.depthTexture; }

    public get normalRenderMaterial(): THREE.MeshNormalMaterial {
        this._normalRenderMaterial ??= new THREE.MeshNormalMaterial({blending: THREE.NoBlending})
        return this._normalRenderMaterial;
    }

    public get depthNormalRenderTarget(): THREE.WebGLRenderTarget {
        if (!this._depthNormalRenderTarget) {
            const depthTexture = new THREE.DepthTexture(this.width * this.depthNormalScale, this.height * this.depthNormalScale);
            depthTexture.format = THREE.DepthStencilFormat;
            depthTexture.type = THREE.UnsignedInt248Type;
            this._depthNormalRenderTarget = new THREE.WebGLRenderTarget(this.width * this.depthNormalScale, this.height * this.depthNormalScale, {
                minFilter: this.targetMinificationTextureFilter,
                magFilter: this.targetMagnificationTextureFilter,
                depthTexture
            });
        }
        return this._depthNormalRenderTarget;
    }

    constructor(parameters?: any) {
        this.depthNormalScale = parameters?.depthNormalScale ?? 1;
        this.targetMinificationTextureFilter = parameters?.textureFilter ?? THREE.NearestFilter;
        this.targetMagnificationTextureFilter = parameters?.textureFilter ?? THREE.NearestFilter;
        this.width = parameters?.width ?? 1024;
        this.height = parameters?.height ?? 1024;
        this.samples = parameters?.samples ?? 0;
        this.renderPass = parameters?.renderPass ?? new RenderPass();
        this.renderOverrideVisibility = parameters?.renderOverrideVisibility ?? new RenderOverrideVisibility(true);
    }

    public dispose() {
        this._normalRenderMaterial?.dispose();
        this._depthNormalRenderTarget?.dispose();
    }

    public setSize(width: number, height: number) {
        this.width = width;
        this.height = height;
        this._depthNormalRenderTarget?.setSize(this.width * this.depthNormalScale, this.height * this.depthNormalScale);
    }

    public render(renderer: THREE.WebGLRenderer, scene: THREE.Scene, camera: THREE.Camera): void {
        this.renderOverrideVisibility.render(scene, () => {
            this.renderPass.renderWithOverrideMaterial(renderer, scene, camera, this.normalRenderMaterial, this.depthNormalRenderTarget, 0x7777ff, 1.0);
        });
    }
}
