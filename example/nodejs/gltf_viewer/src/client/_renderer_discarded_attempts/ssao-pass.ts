import { SSAOBlurMaterial } from './ssao-materials-and-shaders';
import { DepthNormalRenderTarget } from '../renderer/depth-normal-render-targets';
import { SSAOParameters, SSAORenderTargets } from './ssao-render-targets';
import { CopyTransformMaterial } from '../renderer/shader-utility';
import {
  CameraUpdate,
  RenderOverrideVisibility,
  RenderPass,
} from '../renderer/render-util';
import {
  Camera,
  CustomBlending,
  FramebufferTexture,
  LinearFilter,
  NearestFilter,
  NoBlending,
  OrthographicCamera,
  PerspectiveCamera,
  RGBAFormat,
  Scene,
  ShaderMaterial,
  Vector2,
  WebGLRenderer,
  WebGLRenderTarget,
} from 'three';
import { Pass } from 'three/examples/jsm/postprocessing/Pass';

export { SSAOParameters } from './ssao-render-targets';

export class SmoothSSAOPass extends Pass {
  public ssaoParameters: SSAOParameters;
  public needsUpdate: boolean = true;
  private width: number;
  private height: number;
  private samples: number;
  private sceneRenderer: WebGLRenderer;
  private scene: Scene | null = null;
  private camera: Camera | null = null;
  protected renderPass: RenderPass = new RenderPass();
  private _sharedDepthNormalRenderTarget?: DepthNormalRenderTarget;
  private _depthNormalRenderTarget?: DepthNormalRenderTarget;
  public ssaoRenderTargets: SSAORenderTargets;
  protected _colorTexture?: FramebufferTexture;
  private _copyMaterial?: CopyTransformMaterial;
  private _renderOverrideVisibility: RenderOverrideVisibility =
    new RenderOverrideVisibility(true);
  private cameraUpdate: CameraUpdate = new CameraUpdate();

  protected get isSSAOEnabled(): boolean {
    return this.ssaoParameters.enabled;
  }

  public get depthNormalRenderTarget(): DepthNormalRenderTarget {
    if (this._sharedDepthNormalRenderTarget) {
      return this._sharedDepthNormalRenderTarget;
    }
    this._depthNormalRenderTarget =
      this._depthNormalRenderTarget ??
      new DepthNormalRenderTarget({
        width: this.width,
        height: this.height,
        samples: this.samples,
        renderPass: this.renderPass,
        renderOverrideVisibility: this._renderOverrideVisibility,
        textureFilter: SSAOBlurMaterial.optimized
          ? LinearFilter
          : NearestFilter,
      });
    return this._depthNormalRenderTarget;
  }

  private get colorTexture(): FramebufferTexture {
    if (!this._colorTexture) {
      this._colorTexture = new FramebufferTexture(
        this.width,
        this.height,
        RGBAFormat
      );
    }
    return this._colorTexture;
  }

  protected getCopyMaterial(parameters?: any): ShaderMaterial {
    this._copyMaterial = this._copyMaterial ?? new CopyTransformMaterial();
    return this._copyMaterial.update(parameters);
  }

  constructor(
    sceneRenderer: WebGLRenderer,
    width: number,
    height: number,
    samples: number,
    parameters?: any
  ) {
    super();
    if (parameters?.depthNormalRenderTarget) {
      this._sharedDepthNormalRenderTarget = parameters?.depthNormalRenderTarget;
    }
    this.width = width;
    this.height = height;
    this.samples = samples;
    this.sceneRenderer = sceneRenderer;
    this.ssaoRenderTargets = new SSAORenderTargets(
      this.depthNormalRenderTarget,
      { ...parameters, width, height, samples, renderPass: this.renderPass }
    );
    this.ssaoParameters = this.ssaoRenderTargets.ssaoParameters;
  }

  dispose() {
    this._depthNormalRenderTarget?.dispose();
    this.ssaoRenderTargets.dispose();
    this._copyMaterial?.dispose();
    this._colorTexture?.dispose();
  }

  public setSize(width: number, height: number) {
    this.width = width;
    this.height = height;
    this._depthNormalRenderTarget?.setSize(width, height);
    this.ssaoRenderTargets.setSize(width, height);
    this._renderOverrideVisibility = new RenderOverrideVisibility();
    if (this._colorTexture) {
      this._colorTexture.dispose();
      this._colorTexture = undefined;
    }
    this.needsUpdate = true;
  }

  public prepareRender(scene: Scene, camera: Camera) {
    this.scene = scene;
    this.camera = camera;
    (
      this.camera as OrthographicCamera | PerspectiveCamera
    ).updateProjectionMatrix();
  }

  public updateSSAO() {
    this.ssaoRenderTargets.updateSSAOKernel();
    this.needsUpdate = true;
  }

  public render(renderer: WebGLRenderer, writeBuffer: WebGLRenderTarget): void {
    if (!this.ssaoParameters.enabled) {
      return;
    }
    const ssaoNeedsUpdate = this.evaluateIfSSAOUpdateIsNeeded();
    this.prepareRenderToFramebuffer();
    if (ssaoNeedsUpdate && this.camera) {
      this.renderSSAO(renderer, this.camera);
    }
    this.renderToTarget(renderer, writeBuffer);
  }

  private evaluateIfSSAOUpdateIsNeeded(): boolean {
    const needsUpdate =
      this.ssaoParameters.alwaysUpdate ||
      this.needsUpdate ||
      (this.camera != null && this.cameraUpdate.changed(this.camera));
    this.needsUpdate = false;
    return needsUpdate;
  }

  private prepareRenderToFramebuffer(): void {
    if (this.renderToScreen) {
      return;
    }
    this.sceneRenderer.copyFramebufferToTexture(
      new Vector2(),
      this.colorTexture
    );
    this.sceneRenderer.clear();
  }

  private renderSSAO(renderer: WebGLRenderer, camera: Camera): void {
    if (this.scene) {
      this.depthNormalRenderTarget.render(renderer, this.scene, camera);
      if (this.isSSAOEnabled) {
        this.ssaoRenderTargets.render(renderer, this.scene, camera);
      }
    }
  }

  protected renderToTarget(
    renderer: WebGLRenderer,
    writeBuffer: WebGLRenderTarget
  ): void {
    if (!this.renderToScreen) {
      this.renderPass.renderScreenSpace(
        renderer,
        this.getCopyMaterial({
          texture: this.colorTexture,
          blending: NoBlending,
          colorTransform: CopyTransformMaterial.defaultTransform,
        }),
        this.renderToScreen ? null : writeBuffer
      );
    }
    this.renderPass.renderScreenSpace(
      renderer,
      this.getCopyMaterial({
        texture: this.ssaoRenderTargets.blurRenderTarget.texture,
        blending: CustomBlending,
        colorTransform: CopyTransformMaterial.grayscaleTransform,
      }),
      this.renderToScreen ? null : writeBuffer
    );
  }
}
