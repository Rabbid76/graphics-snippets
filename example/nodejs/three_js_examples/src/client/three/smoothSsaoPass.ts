import {
    renderOverride,
    RenderOverrideVisibility
} from './threeUtility';
import {
    SSAOParameters,
    SSAORenderTargets
} from './ssaoRenderTargets';
import * as THREE from 'three';
import {
    Pass,
    FullScreenQuad
} from 'three/examples/jsm/postprocessing/Pass';
import { CopyShader } from 'three/examples/jsm/shaders/CopyShader.js';

export class SmoothSSAOPass extends Pass {
    public ssaoParameters: SSAOParameters;
    public needsUpdate: boolean = true;
    private width: number;
    private height: number;
    private sceneRenderer: THREE.WebGLRenderer;
    private scene: THREE.Scene;
    private camera: THREE.Camera;
    protected ssaoRenderTargets: SSAORenderTargets;
    protected colorTexture?: THREE.FramebufferTexture;
    private screenSpaceQuad?: FullScreenQuad;
    private copyMaterial?: THREE.ShaderMaterial;
    private _renderOverrideVisibility: RenderOverrideVisibility = new RenderOverrideVisibility();
    private _originalClearColor = new THREE.Color();
    private lastCameraProjection: THREE.Matrix4 | undefined;
    private lastCameraWorld: THREE.Matrix4 | undefined;

    constructor(sceneRenderer: THREE.WebGLRenderer, scene: THREE.Scene, camera: THREE.Camera, width: number, height: number, samples: number) {
        super();
        this.scene = scene;
        this.camera = camera;
        this.width = width;
        this.height = height;
        this.sceneRenderer = sceneRenderer;
        this.ssaoRenderTargets = new SSAORenderTargets(width, height, samples);
        this.ssaoParameters = this.ssaoRenderTargets.ssaoParameters;
    }

    dispose() {
        this.ssaoRenderTargets.dispose();
        this.copyMaterial?.dispose();
        this.screenSpaceQuad?.dispose();
        this.colorTexture?.dispose();
    }

    public setSize(width: number, height: number) {
        this.width = width;
        this.height = height;
        this.ssaoRenderTargets.setSize(width, height);
        this._renderOverrideVisibility = new RenderOverrideVisibility();
        if (this.colorTexture) {
            this.colorTexture.dispose();
            this.colorTexture = undefined;
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
        const colorTexture = this.getColorTexture();
        this.sceneRenderer.copyFramebufferToTexture(new THREE.Vector2(), colorTexture);
        this.sceneRenderer.clear();
    }

    protected ssaoIsEnabled(): boolean {
        return this.ssaoParameters.enabled;
    }

    private renderSSAO(renderer: THREE.WebGLRenderer, camera: THREE.Camera): void {
        const depthNormalTarget = this.ssaoRenderTargets.getDepthNormalRenderTarget();
        const normalMaterial = this.ssaoRenderTargets.getNormalRenderMaterial();
        this.renderOverrideVisibility(renderer, normalMaterial, depthNormalTarget, 0x7777ff, 1.0);

        if (this.ssaoIsEnabled()) {
            const ssaoMaterial = this.ssaoRenderTargets.updateSSAOMaterial(camera);
            const ssaoRenderTarget = this.ssaoRenderTargets.getSSSAORenderTarget();
            this.renderScreenSpacePass(renderer, ssaoMaterial, ssaoRenderTarget);
        }

        if (this.ssaoIsEnabled()) {
            const blurMaterial = this.ssaoRenderTargets.getBlurRenderMaterial();
            const blurRenderTarget = this.ssaoRenderTargets.getBlurRenderTarget();
            this.renderScreenSpacePass(renderer, blurMaterial, blurRenderTarget);
        }
    }

    protected renderToTarget(renderer: THREE.WebGLRenderer, writeBuffer: THREE.WebGLRenderTarget): void {
        const copyMaterial = this.getCopyMaterial();
        if (!this.renderToScreen) {
            copyMaterial.uniforms.tDiffuse.value = this.colorTexture;
            copyMaterial.blending = THREE.NoBlending;
            this.renderScreenSpacePass(renderer, copyMaterial, this.renderToScreen ? null : writeBuffer);
        }
        copyMaterial.uniforms.tDiffuse.value = this.ssaoRenderTargets.blurRenderTarget?.texture;
        copyMaterial.blending = THREE.CustomBlending;
        this.renderScreenSpacePass(renderer, copyMaterial, this.renderToScreen ? null : writeBuffer);
    }

    protected renderScreenSpacePass(renderer: THREE.WebGLRenderer, passMaterial: THREE.Material, renderTarget: THREE.WebGLRenderTarget | null, clearColor?: any, clearAlpha?: any): void {
        renderer.getClearColor(this._originalClearColor);
        const originalClearAlpha = renderer.getClearAlpha();
        const originalAutoClear = renderer.autoClear;

        renderer.setRenderTarget(renderTarget);

        // setup pass state
        renderer.autoClear = false;
        if ((clearColor !== undefined) && (clearColor !== null)) {
            renderer.setClearColor(clearColor);
            renderer.setClearAlpha(clearAlpha || 0.0);
            renderer.clear();
        }

        const screenSpaceQuad = this.getScreenSpaceQuad();
        screenSpaceQuad.material = passMaterial;
        screenSpaceQuad.render(renderer);

        renderer.autoClear = originalAutoClear;
        renderer.setClearColor(this._originalClearColor);
        renderer.setClearAlpha(originalClearAlpha);
    }

    private renderOverrideVisibility(renderer: THREE.WebGLRenderer, overrideMaterial: THREE.Material, renderTarget: THREE.WebGLRenderTarget | null, clearColor: any, clearAlpha: any): void {
        this._renderOverrideVisibility.render(this.scene, () => {
            renderOverride(renderer, this.scene, this.camera, overrideMaterial, renderTarget, clearColor, clearAlpha);
        });
    }

    private getScreenSpaceQuad(): FullScreenQuad {
        if (!this.screenSpaceQuad) {
            this.screenSpaceQuad = new FullScreenQuad(undefined);
        }
        return this.screenSpaceQuad;
    }

    private getColorTexture(): THREE.FramebufferTexture {
        if (!this.colorTexture) {
            this.colorTexture = new THREE.FramebufferTexture(this.width, this.height, THREE.RGBAFormat);
        }
        return this.colorTexture;
    }

    protected getCopyMaterial(): THREE.ShaderMaterial {
        if (!this.copyMaterial) {
            this.copyMaterial = new THREE.ShaderMaterial( {
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
            } );
        }
        return this.copyMaterial;
    }
}
