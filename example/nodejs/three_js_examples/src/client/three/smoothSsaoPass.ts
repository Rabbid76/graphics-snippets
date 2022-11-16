import {
    SSAOParameters,
    SSAORenderTargets
} from './ssaoRenderTargets';
import { CopyMaterial } from '../three/shaderUtility'
import {
    RenderOverrideVisibility,
    RenderPass,
} from './threeUtility';
import * as THREE from 'three';
import { Pass } from 'three/examples/jsm/postprocessing/Pass';

export class SmoothSSAOPass extends Pass {
    public ssaoParameters: SSAOParameters;
    public needsUpdate: boolean = true;
    private width: number;
    private height: number;
    private sceneRenderer: THREE.WebGLRenderer;
    private scene: THREE.Scene;
    private camera: THREE.Camera;
    protected renderPass: RenderPass = new RenderPass();
    protected ssaoRenderTargets: SSAORenderTargets;
    protected _colorTexture?: THREE.FramebufferTexture;
    private _copyMaterial?: CopyMaterial;
    private _renderOverrideVisibility: RenderOverrideVisibility = new RenderOverrideVisibility(true);
    private lastCameraProjection: THREE.Matrix4 | undefined;
    private lastCameraWorld: THREE.Matrix4 | undefined;

    protected get isSSAOEnabled(): boolean {
        return this.ssaoParameters.enabled;
    }

    private get colorTexture(): THREE.FramebufferTexture {
        if (!this._colorTexture) {
            this._colorTexture = new THREE.FramebufferTexture(this.width, this.height, THREE.RGBAFormat);
        }
        return this._colorTexture;
    }

    protected getCopyMaterial(parameters?: any): THREE.ShaderMaterial {
        this._copyMaterial ??= new CopyMaterial();
        return this._copyMaterial.update(parameters);
    }

    constructor(sceneRenderer: THREE.WebGLRenderer, scene: THREE.Scene, camera: THREE.Camera, width: number, height: number, samples: number, parameters?: any) {
        super();
        this.scene = scene;
        this.camera = camera;
        this.width = width;
        this.height = height;
        this.sceneRenderer = sceneRenderer;
        this.ssaoRenderTargets = new SSAORenderTargets(width, height, samples, parameters);
        this.ssaoParameters = this.ssaoRenderTargets.ssaoParameters;
    }

    dispose() {
        this.ssaoRenderTargets.dispose();
        this._copyMaterial?.dispose();
        this._colorTexture?.dispose();
    }

    public setSize(width: number, height: number) {
        this.width = width;
        this.height = height;
        this.ssaoRenderTargets.setSize(width, height);
        this._renderOverrideVisibility = new RenderOverrideVisibility();
        if (this._colorTexture) {
            this._colorTexture.dispose();
            this._colorTexture = undefined;
        }
        this.needsUpdate = true;
    }

    public prepareRender(scene: THREE.Scene, camera: THREE.Camera) {
        this.scene = scene;
        this.camera = camera;
        // @ts-ignore
        this.camera.updateProjectionMatrix();
    }

    public updateSSAO() {
        this.ssaoRenderTargets.updateSSAOKernel();
        this.needsUpdate = true;
    }

    public render(renderer: THREE.WebGLRenderer, writeBuffer: THREE.WebGLRenderTarget): void {
        const ssaoNeedsUpdate = this.evaluateIfSSAOUpdateIsNeeded();
        this.prepareRenderToFramebuffer();
        if (ssaoNeedsUpdate) {
            this.renderSSAO(renderer, this.camera);
        }
        this.renderToTarget(renderer, writeBuffer);
    }

    private evaluateIfSSAOUpdateIsNeeded(): boolean {
        const needsUpdate = this.ssaoParameters.alwaysUpdate || this.needsUpdate ||
            !this.lastCameraProjection?.equals(this.camera.projectionMatrix) || !this.lastCameraWorld?.equals(this.camera.matrixWorld);
        this.needsUpdate = false;
        this.lastCameraProjection = this.camera.projectionMatrix.clone();
        this.lastCameraWorld = this.camera.matrixWorld.clone();
        return needsUpdate;
    }

    private prepareRenderToFramebuffer(): void {
        if (this.renderToScreen) {
            return;
        }
        this.sceneRenderer.copyFramebufferToTexture(new THREE.Vector2(), this.colorTexture);
        this.sceneRenderer.clear();
    }

    private renderSSAO(renderer: THREE.WebGLRenderer, camera: THREE.Camera): void {
        const depthNormalTarget = this.ssaoRenderTargets.depthNormalRenderTarget;
        const normalMaterial = this.ssaoRenderTargets.normalRenderMaterial;
        this.renderOverrideVisibility(renderer, normalMaterial, depthNormalTarget, 0x7777ff, 1.0);

        if (this.isSSAOEnabled) {
            const ssaoMaterial = this.ssaoRenderTargets.updateSSAOMaterial(camera);
            const ssaoRenderTarget = this.ssaoRenderTargets.ssaoRenderTarget;
            this.renderPass.renderScreenSpace(renderer, ssaoMaterial, ssaoRenderTarget);
        }

        if (this.isSSAOEnabled) {
            const blurMaterial = this.ssaoRenderTargets.blurRenderMaterial;
            const blurRenderTarget = this.ssaoRenderTargets.blurRenderTarget;
            this.renderPass.renderScreenSpace(renderer, blurMaterial, blurRenderTarget);
        }
    }

    protected renderToTarget(renderer: THREE.WebGLRenderer, writeBuffer: THREE.WebGLRenderTarget): void {
        if (!this.renderToScreen) {
            this.renderPass.renderScreenSpace(renderer, this.getCopyMaterial({texture: this.colorTexture, blending: THREE.NoBlending}), this.renderToScreen ? null : writeBuffer);
        }
        this.renderPass.renderScreenSpace(renderer, this.getCopyMaterial({texture: this.ssaoRenderTargets.blurRenderTarget.texture, blending: THREE.CustomBlending}), this.renderToScreen ? null : writeBuffer);
    }

    private renderOverrideVisibility(renderer: THREE.WebGLRenderer, overrideMaterial: THREE.Material, renderTarget: THREE.WebGLRenderTarget | null, clearColor: any, clearAlpha: any): void {
        this._renderOverrideVisibility.render(this.scene, () => {
            this.renderPass.renderWithOverrideMaterial(renderer, this.scene, this.camera, overrideMaterial, renderTarget, clearColor, clearAlpha);
        });
    }
}
