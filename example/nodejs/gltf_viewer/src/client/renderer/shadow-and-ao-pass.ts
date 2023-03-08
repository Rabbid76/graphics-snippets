import { DepthNormalRenderTarget } from './depth-normal-render-targets';
import {
  ShadowAndAoParameters,
  ShadowAndAoRenderTargets,
} from './shadow-and-ao-render-targets';
import { CopyTransformMaterial } from './shader-utility';
import {
  CameraUpdate,
  RenderOverrideVisibility,
  RenderPass,
} from './render-util';
import {
  Camera,
  CustomBlending,
  Matrix4,
  NearestFilter,
  OrthographicCamera,
  PerspectiveCamera,
  Scene,
  Texture,
  WebGLRenderer,
} from 'three';

export { ShadowAndAoParameters } from './shadow-and-ao-render-targets';

export class ShadowAndAoPass {
  public static shadowTransform: Matrix4 = new Matrix4().set(
    0,
    1,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    0,
    0,
    1
  );
  public shadowAndAoParameters: ShadowAndAoParameters;
  public needsUpdate: boolean = true;
  private width: number;
  private height: number;
  private samples: number;
  protected renderPass: RenderPass = new RenderPass();
  private _sharedDepthNormalRenderTarget?: DepthNormalRenderTarget;
  private _depthNormalRenderTarget?: DepthNormalRenderTarget;
  public shadowAndAoRenderTargets: ShadowAndAoRenderTargets;
  private _copyMaterial: CopyTransformMaterial;
  private _renderOverrideVisibility: RenderOverrideVisibility =
    new RenderOverrideVisibility(true);
  private cameraUpdate: CameraUpdate = new CameraUpdate();

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
        textureFilter: NearestFilter,
      });
    return this._depthNormalRenderTarget;
  }

  constructor(
    width: number,
    height: number,
    samples: number,
    parameters?: any
  ) {
    if (parameters?.depthNormalRenderTarget) {
      this._sharedDepthNormalRenderTarget = parameters?.depthNormalRenderTarget;
    }
    this.width = width;
    this.height = height;
    this.samples = samples;
    this.shadowAndAoRenderTargets = new ShadowAndAoRenderTargets(
      this.depthNormalRenderTarget,
      { ...parameters, width, height, samples, renderPass: this.renderPass }
    );
    this.shadowAndAoParameters =
      this.shadowAndAoRenderTargets.shadowAndAoParameters;
    this._copyMaterial = new CopyTransformMaterial();
    this.updateCopyMaterial();
  }

  dispose() {
    this._depthNormalRenderTarget?.dispose();
    this.shadowAndAoRenderTargets.dispose();
    this._copyMaterial?.dispose();
  }

  public setSize(width: number, height: number) {
    this.width = width;
    this.height = height;
    this._depthNormalRenderTarget?.setSize(width, height);
    this.shadowAndAoRenderTargets.setSize(width, height);
    this._renderOverrideVisibility = new RenderOverrideVisibility();
    this.updateCopyMaterial();
    this.needsUpdate = true;
  }

  public updateShadowTexture(shadowTexture: Texture) {
    this.shadowAndAoRenderTargets.updateShadowTexture(shadowTexture);
  }

  public updateKernel() {
    this.shadowAndAoRenderTargets.updateKernel();
    this.needsUpdate = true;
  }

  public render(
    renderer: WebGLRenderer,
    scene: Scene,
    camera: Camera,
    intensityScale: number = 1
  ): void {
    if (!this.shadowAndAoParameters.aoAndSoftShadowEnabled) {
      return;
    }
    if (this.evaluateIfUpdateIsNeeded(camera)) {
      this.renderShadowAndAo(renderer, scene, camera);
    }
    this.renderToTarget(renderer, intensityScale);
  }

  private evaluateIfUpdateIsNeeded(camera: Camera): boolean {
    (camera as OrthographicCamera | PerspectiveCamera).updateProjectionMatrix();
    const needsUpdate =
      this.shadowAndAoParameters.aoAlwaysUpdate ||
      this.needsUpdate ||
      (camera != null && this.cameraUpdate.changed(camera));
    this.needsUpdate = false;
    return needsUpdate;
  }

  private updateCopyMaterial(intensityScale: number = 1) {
    const aoIntensity = this.shadowAndAoParameters.aoIntensity * intensityScale;
    const shIntensity =
      this.shadowAndAoParameters.shadowIntensity * intensityScale;
    this._copyMaterial.update({
      texture: this.shadowAndAoRenderTargets.blurRenderTarget.texture,
      blending: CustomBlending,
      colorTransform: new Matrix4().set(
        aoIntensity,
        0,
        0,
        1 - aoIntensity,
        0,
        shIntensity,
        0,
        1 - shIntensity,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        1
      ),
      multiplyChannels: 1,
    });
  }

  private renderShadowAndAo(
    renderer: WebGLRenderer,
    scene: Scene,
    camera: Camera
  ): void {
    this.depthNormalRenderTarget.render(renderer, scene, camera);
    this.shadowAndAoRenderTargets.render(renderer, scene, camera);
  }

  protected renderToTarget(
    renderer: WebGLRenderer,
    intensityScale: number = 1
  ): void {
    this.updateCopyMaterial(intensityScale);
    this.renderPass.renderScreenSpace(
      renderer,
      this._copyMaterial,
      renderer.getRenderTarget()
    );
  }
}
