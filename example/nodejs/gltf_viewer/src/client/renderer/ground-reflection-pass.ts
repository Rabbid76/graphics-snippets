import { CopyTransformMaterial } from './shader-utility';
import { RenderPass } from './render-utility';
import { HorizontalBlurShader } from 'three/examples/jsm/shaders/HorizontalBlurShader.js';
import { VerticalBlurShader } from 'three/examples/jsm/shaders/VerticalBlurShader.js';
import {
  Camera,
  Matrix4,
  PerspectiveCamera,
  RGBAFormat,
  Scene,
  ShaderMaterial,
  WebGLRenderer,
  WebGLRenderTarget,
} from 'three';

export interface GroundReflectionParameters {
  enabled: boolean;
  intensity: number;
  brightness: number;
  blur: number;
  groundLevel: number;
  renderTargetDownScale: number;
}

export class GroundReflectionPass {
  private width: number;
  private height: number;
  public parameters: GroundReflectionParameters;
  private _reflectionRenderTarget?: WebGLRenderTarget;
  private _blurRenderTarget?: WebGLRenderTarget;
  private _copyMaterial: CopyTransformMaterial;
  private renderPass: RenderPass;
  private horizontalBlurMaterial: ShaderMaterial;
  private verticalBlurMaterial: ShaderMaterial;

  public get reflectionRenderTarget(): WebGLRenderTarget {
    this._reflectionRenderTarget =
      this._reflectionRenderTarget ??
      new WebGLRenderTarget(
        this.width / this.parameters.renderTargetDownScale,
        this.height / this.parameters.renderTargetDownScale,
        {
          samples: 1,
          format: RGBAFormat,
        }
      );
    return this._reflectionRenderTarget;
  }

  public get blurRenderTarget(): WebGLRenderTarget {
    this._blurRenderTarget =
      this._blurRenderTarget ??
      new WebGLRenderTarget(
        this.width / this.parameters.renderTargetDownScale,
        this.height / this.parameters.renderTargetDownScale,
        {
          samples: 1,
          format: RGBAFormat,
        }
      );
    return this._blurRenderTarget;
  }

  constructor(width: number, height: number, parameters: any) {
    this.width = width;
    this.height = height;
    this.parameters = {
      enabled: parameters.enabled ?? false,
      intensity: parameters.intensity ?? 0.25,
      brightness: parameters.brightness ?? 1.0,
      blur: parameters.blur ?? 5.0,
      groundLevel: parameters.groundLevel ?? 0,
      renderTargetDownScale: parameters.renderTargetDownScale ?? 4,
    };
    this._copyMaterial = new CopyTransformMaterial({}, false);
    this.updateCopyMaterial();
    this.renderPass = parameters?.renderPass ?? new RenderPass();
    this.horizontalBlurMaterial = new ShaderMaterial(HorizontalBlurShader);
    this.horizontalBlurMaterial.depthTest = false;
    this.verticalBlurMaterial = new ShaderMaterial(VerticalBlurShader);
    this.verticalBlurMaterial.depthTest = false;
  }

  dispose() {
    this._reflectionRenderTarget?.dispose();
    this._blurRenderTarget?.dispose();
    this._copyMaterial.dispose();
  }

  public setSize(width: number, height: number) {
    this.width = width;
    this.height = height;
    this._reflectionRenderTarget?.setSize(
      this.width / this.parameters.renderTargetDownScale,
      this.height / this.parameters.renderTargetDownScale
    );
    this._blurRenderTarget?.setSize(
      this.width / this.parameters.renderTargetDownScale,
      this.height / this.parameters.renderTargetDownScale
    );
  }

  public updateParameters(parameters: any) {
    if (parameters.enabled !== undefined) {
      this.parameters.enabled = parameters.enabled;
    }
    if (parameters.intensity !== undefined) {
      this.parameters.intensity = parameters.intensity;
    }
    if (parameters.brightness !== undefined) {
      this.parameters.brightness = parameters.brightness;
    }
    if (parameters.blur !== undefined) {
      this.parameters.blur = parameters.blur;
    }
    if (parameters.groundLevel !== undefined) {
      this.parameters.groundLevel = parameters.groundLevel;
    }
    if (parameters.renderTargetDownScale !== undefined) {
      this.parameters.renderTargetDownScale = parameters.renderTargetDownScale;
    }
  }
  private updateCopyMaterial() {
    const intensity = this.parameters.intensity;
    const brightness = this.parameters.brightness;
    this._copyMaterial.update({
      texture: this.reflectionRenderTarget.texture,
      colorTransform: new Matrix4().set(
        brightness,
        0,
        0,
        0,
        0,
        brightness,
        0,
        0,
        0,
        0,
        brightness,
        0,
        0,
        0,
        0,
        intensity
      ),
      multiplyChannels: 0,
      uvTransform: CopyTransformMaterial.flipYuvTransform,
    });
  }

  public render(renderer: WebGLRenderer, scene: Scene, camera: Camera): void {
    if (!this.parameters.enabled || !(camera instanceof PerspectiveCamera)) {
      return;
    }
    this.renderGroundReflection(
      renderer,
      scene,
      camera,
      this.reflectionRenderTarget
    );
    if (this.parameters.blur > 0) {
      this.blurReflection(renderer);
    }
    this.updateCopyMaterial();
    this.renderPass.renderScreenSpace(
      renderer,
      this._copyMaterial,
      renderer.getRenderTarget()
    );
  }

  private renderGroundReflection(
    renderer: WebGLRenderer,
    scene: Scene,
    camera: Camera,
    renderTarget: WebGLRenderTarget | undefined
  ) {
    const groundReflectionCamera = this.createGroundReflectionCamera(camera);
    const renderTargetBackup = renderer.getRenderTarget();
    if (renderTarget) {
      renderer.setRenderTarget(renderTarget);
    }
    renderer.render(scene, groundReflectionCamera);
    if (renderTarget) {
      renderer.setRenderTarget(renderTargetBackup);
    }
  }

  private createGroundReflectionCamera(camera: Camera): Camera {
    const groundReflectionCamera = camera.clone() as PerspectiveCamera;
    groundReflectionCamera.position.set(
      camera.position.x,
      -camera.position.y + 2 * this.parameters.groundLevel,
      camera.position.z
    );
    //groundReflectionCamera.lookAt(0, 2 * groundLevel, 0);
    groundReflectionCamera.rotation.set(
      -camera.rotation.x,
      camera.rotation.y,
      -camera.rotation.z
    );
    //groundReflectionCamera.scale.set(1, -1, 1);
    groundReflectionCamera.updateMatrixWorld();
    groundReflectionCamera.updateProjectionMatrix();
    return groundReflectionCamera;
  }

  public blurReflection(renderer: WebGLRenderer): void {
    const renderTargetBackup = renderer.getRenderTarget();
    this.horizontalBlurMaterial.uniforms.h.value =
      (this.parameters.blur * 1) / this.width;
    this.horizontalBlurMaterial.uniforms.tDiffuse.value =
      this.reflectionRenderTarget.texture;
    renderer.setRenderTarget(this.blurRenderTarget);
    this.renderPass.renderScreenSpace(
      renderer,
      this.horizontalBlurMaterial,
      renderer.getRenderTarget()
    );
    this.verticalBlurMaterial.uniforms.v.value =
      (this.parameters.blur * 1) / this.height;
    this.verticalBlurMaterial.uniforms.tDiffuse.value =
      this.blurRenderTarget.texture;
    renderer.setRenderTarget(this.reflectionRenderTarget);
    this.renderPass.renderScreenSpace(
      renderer,
      this.verticalBlurMaterial,
      renderer.getRenderTarget()
    );
    renderer.setRenderTarget(renderTargetBackup);
  }
}
