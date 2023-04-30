import { SSAOBlurMaterial } from './ssaoMaterialsAndShaders';
import { DepthNormalRenderTarget } from '../three/depthNormalRenderTarget'
import {
    SSAOParameters,
    SSAORenderTargets
} from './ssaoRenderTargets';
import { CopyMaterial } from '../three/shaderUtility'
import {
    CameraUpdate,
} from './threeUtility';
import {
    RenderOverrideVisibility,
    RenderPass,
} from './renderPass';
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
    protected depthNormalRenderTarget: DepthNormalRenderTarget;
    protected ssaoRenderTargets: SSAORenderTargets;
    protected _colorTexture?: THREE.FramebufferTexture;
    private _copyMaterial?: CopyMaterial;
    private _renderOverrideVisibility: RenderOverrideVisibility = new RenderOverrideVisibility(true);
    private cameraUpdate: CameraUpdate = new CameraUpdate();

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
        this.depthNormalRenderTarget = new DepthNormalRenderTarget({
            width, height, samples, 
            renderPass: this.renderPass,
            renderOverrideVisibility: this._renderOverrideVisibility,
            textureFilter: SSAOBlurMaterial.optimized ? THREE.LinearFilter : THREE.NearestFilter
        });
        this.ssaoRenderTargets = new SSAORenderTargets(this.depthNormalRenderTarget, { ...parameters, width, height, samples, renderPass: this.renderPass });
        this.ssaoParameters = this.ssaoRenderTargets.ssaoParameters;
    }

    dispose() {
        this.depthNormalRenderTarget.dispose();
        this.ssaoRenderTargets.dispose();
        this._copyMaterial?.dispose();
        this._colorTexture?.dispose();
    }

    public setSize(width: number, height: number) {
        this.width = width;
        this.height = height;
        this.depthNormalRenderTarget.setSize(width, height);
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
        const needsUpdate = this.ssaoParameters.alwaysUpdate || this.needsUpdate || this.cameraUpdate.changed(this.camera);
        this.needsUpdate = false;
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
        this.depthNormalRenderTarget.render(renderer, this.scene, camera);
        if (this.isSSAOEnabled) {
            this.ssaoRenderTargets.render(renderer, this.scene, camera);
        }
    }

    protected renderToTarget(renderer: THREE.WebGLRenderer, writeBuffer: THREE.WebGLRenderTarget): void {
        if (!this.renderToScreen) {
            this.renderPass.renderScreenSpace(renderer, this.getCopyMaterial({texture: this.colorTexture, blending: THREE.NoBlending}), this.renderToScreen ? null : writeBuffer);
        }
        this.renderPass.renderScreenSpace(renderer, this.getCopyMaterial({texture: this.ssaoRenderTargets.blurRenderTarget.texture, blending: THREE.CustomBlending}), this.renderToScreen ? null : writeBuffer);
    }
}
