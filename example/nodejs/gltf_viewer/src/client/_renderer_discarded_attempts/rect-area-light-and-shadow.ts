import { RectAreaLightHelper } from 'three/examples/jsm/helpers/RectAreaLightHelper.js';
import {
  CopyTransformMaterial,
  HorizontalBlurShadowShader,
  MixShadowShader,
  VerticalBlurShadowShader,
} from '../renderer/shader-utility';
import {
  CameraUpdate,
  RenderOverrideVisibility,
  RenderPass,
  setOrthographicViewVolumeFromBox,
} from '../renderer/render-utility';
import {
  IntegralShadowRenderMaterial,
  IntegralShadowBlurMaterial,
} from './integral-shadow-materials-and-shaders';
import { DepthNormalRenderTarget } from '../renderer/depth-normal-render-targets';
import { SimplexNoise } from 'three/examples/jsm/math/SimplexNoise.js';
import {
  AddEquation,
  Box3,
  Camera,
  CustomBlending,
  DataTexture,
  DirectionalLight,
  DoubleSide,
  FloatType,
  LinearFilter,
  LineBasicMaterial,
  Material,
  MathUtils,
  Matrix4,
  MeshPhongMaterial,
  NearestFilter,
  NoBlending,
  OneFactor,
  RectAreaLight,
  RedFormat,
  RepeatWrapping,
  Scene,
  ShaderMaterial,
  SrcColorFactor,
  Texture,
  Vector2,
  Vector3,
  WebGLRenderer,
  WebGLRenderTarget,
  ZeroFactor,
} from 'three';

interface LightSamplePositions {
  position: Vector3;
  target: Vector3;
}

export class RectAreaLightAndShadowWithDirectionalLight {
  public needsUpdate: boolean = false;
  private _shadowIntensity: number;
  private _rectAreaLight: RectAreaLight;
  private _directionalLight: DirectionalLight;
  private _rectLightHelper?: RectAreaLightHelper;
  private viewportSize: Vector2;
  private samples: number;
  public shadowRadius: number;
  public alwaysUpdate: boolean;
  public castShadow: boolean;
  public softShadow: boolean;
  public monteCarlo: boolean;
  private shadowMapSize: number;
  private renderPass: RenderPass;
  private shadowMaterial: Material;
  private mixShadowMaterial: ShaderMaterial;
  private horizontalBlurMaterial: ShaderMaterial;
  private verticalBlurMaterial: ShaderMaterial;
  private shadowRenderTargets: WebGLRenderTarget[];
  private currentTarget: number = 0;
  private copyMaterial: CopyTransformMaterial;
  private cameraUpdate: CameraUpdate = new CameraUpdate();
  private lightSamples: LightSamplePositions[] = [];
  private currentLightSample: number = 0;
  private _sharedDepthNormalRenderTarget?: DepthNormalRenderTarget;
  private _depthNormalRenderTarget?: DepthNormalRenderTarget;
  private _noiseTexture?: DataTexture;
  private _kernel: Vector3[] = [];
  private _shadowRenderMaterial?: IntegralShadowRenderMaterial;
  private _blurRenderMaterial?: IntegralShadowBlurMaterial;
  private _renderOverrideVisibility: RenderOverrideVisibility =
    new RenderOverrideVisibility(true);

  public get visible(): boolean {
    return this._rectAreaLight.visible;
  }
  public set visible(isVisible: boolean) {
    this._rectAreaLight.visible = isVisible;
  }
  public get shadowIntensity(): number {
    return this._shadowIntensity;
  }
  public set shadowIntensity(intensity: number) {
    this._shadowIntensity = intensity;
  }
  public get rectAreaLight(): RectAreaLight {
    return this._rectAreaLight;
  }
  public get shadowLight(): DirectionalLight {
    return this._directionalLight;
  }
  public get shadowCamera(): Camera {
    return this._directionalLight.shadow.camera;
  }
  public get rectLightHelper(): RectAreaLightHelper | undefined {
    return this._rectLightHelper;
  }
  public get shadowMapTexture(): Texture | undefined {
    return this._directionalLight.shadow.map?.texture;
  }
  public get shadowTexture(): Texture {
    return this.shadowRenderTargets[this.currentTarget].texture;
  }

  public get depthNormalRenderTarget(): DepthNormalRenderTarget {
    if (this._sharedDepthNormalRenderTarget) {
      return this._sharedDepthNormalRenderTarget;
    }
    this._depthNormalRenderTarget =
      this._depthNormalRenderTarget ??
      new DepthNormalRenderTarget({
        width: this.viewportSize.x,
        height: this.viewportSize.y,
        samples: this.samples,
        renderPass: this.renderPass,
        renderOverrideVisibility: this._renderOverrideVisibility,
        textureFilter: IntegralShadowBlurMaterial.optimized
          ? LinearFilter
          : NearestFilter,
      });
    return this._depthNormalRenderTarget;
  }

  public get shadowRenderMaterial(): IntegralShadowRenderMaterial {
    this._shadowRenderMaterial =
      this._shadowRenderMaterial ??
      new IntegralShadowRenderMaterial({
        normalTexture: this.depthNormalRenderTarget.getNormalTexture(),
        depthTexture: this.depthNormalRenderTarget.getDepthTexture(),
        noiseTexture: this.noiseTexture,
        kernel: this.kernel,
      });
    return this._shadowRenderMaterial.update({
      width: this.viewportSize.x,
      height: this.viewportSize.y,
    });
  }

  public get blurRenderMaterial(): IntegralShadowBlurMaterial {
    this._blurRenderMaterial =
      this._blurRenderMaterial ??
      new IntegralShadowBlurMaterial({
        depthTexture: this.depthNormalRenderTarget.getDepthTexture(),
      });
    return this._blurRenderMaterial;
  }

  private get noiseTexture(): DataTexture {
    this._noiseTexture =
      this._noiseTexture ?? this.generateRandomKernelRotations();
    return this._noiseTexture;
  }

  private get kernel(): Vector3[] {
    if (!this._kernel.length) {
      this._kernel = this.generateSampleKernel();
    }
    return this._kernel;
  }

  constructor(
    rectAreaLight: RectAreaLight,
    viewportSize: Vector2,
    parameters: any
  ) {
    this.viewportSize = new Vector2(viewportSize.x, viewportSize.y);
    this._shadowIntensity = parameters?.shadowIntensity ?? 0.25;
    this.shadowRadius = parameters?.shadowRadius ?? 0.05;
    this.samples = parameters?.samples ?? 0;
    this.shadowMapSize = parameters?.shadowMapSize ?? 1024;
    this.alwaysUpdate = parameters?.alwaysUpdate ?? false;
    this.castShadow = parameters?.castShadow ?? true;
    this.softShadow = parameters?.softShadow ?? true;
    this.monteCarlo = parameters?.monteCarlo ?? true;
    if (parameters?.depthNormalRenderTarget) {
      this._sharedDepthNormalRenderTarget = parameters?.depthNormalRenderTarget;
    }
    this._rectAreaLight = rectAreaLight;
    if (parameters?.addHelper) {
      this._rectLightHelper = new RectAreaLightHelper(this._rectAreaLight);
      (this._rectLightHelper.material as LineBasicMaterial).depthWrite = false;
      this._rectAreaLight.add(this._rectLightHelper);
    }
    this._directionalLight = new DirectionalLight(0xffffff, 2.5);
    this._directionalLight.position.set(
      this._rectAreaLight.position.x,
      this._rectAreaLight.position.y,
      this._rectAreaLight.position.z
    );
    this._directionalLight.lookAt(0, 0, 0);
    this._directionalLight.visible = false;
    this._directionalLight.castShadow = true;
    this._directionalLight.shadow.mapSize = new Vector2(
      this.shadowMapSize,
      this.shadowMapSize
    );
    this.renderPass = new RenderPass();
    //this.shadowMaterial = new MeshStandardMaterial({color: 0xffffff, envMapIntensity: 0, roughness: 1, side: DoubleSide});
    this.shadowMaterial = new MeshPhongMaterial({
      color: 0xffffff,
      shininess: 0,
      polygonOffsetFactor: 0,
      polygonOffsetUnits: 0,
      side: DoubleSide,
    });
    const samples = this.samples;
    this.shadowRenderTargets = [
      new WebGLRenderTarget(this.viewportSize.x, this.viewportSize.y, {
        samples,
        format: RedFormat,
      }),
      new WebGLRenderTarget(this.viewportSize.x, this.viewportSize.y, {
        samples,
        format: RedFormat,
      }),
      new WebGLRenderTarget(this.viewportSize.x, this.viewportSize.y, {
        samples,
        format: RedFormat,
      }),
    ];
    this.mixShadowMaterial = new ShaderMaterial(MixShadowShader);
    this.mixShadowMaterial.depthTest = false;
    this.horizontalBlurMaterial = new ShaderMaterial(
      HorizontalBlurShadowShader
    );
    this.horizontalBlurMaterial.depthTest = false;
    this.verticalBlurMaterial = new ShaderMaterial(VerticalBlurShadowShader);
    this.verticalBlurMaterial.depthTest = false;
    this.copyMaterial = new CopyTransformMaterial({
      blendSrc: ZeroFactor,
      blendDst: SrcColorFactor,
      blendEquation: AddEquation,
      blendSrcAlpha: ZeroFactor,
      blendDstAlpha: OneFactor,
      blendEquationAlpha: AddEquation,
    });
  }

  public dispose(): void {
    this._rectAreaLight.dispose();
    this._directionalLight.dispose();
    this.shadowMaterial.dispose();
    this.shadowRenderTargets.forEach((item) => item.dispose());
    this._depthNormalRenderTarget?.dispose();
  }

  addToScene(scene: Scene): void {
    scene.add(this.rectAreaLight);
    scene.add(this._directionalLight);
  }

  removeFromScene(scene: Scene): void {
    scene.remove(this.rectAreaLight);
    scene.remove(this._directionalLight);
  }

  public setSize(width: number, height: number): void {
    this.viewportSize = new Vector2(width, height);
    this.shadowRenderTargets.forEach((item) =>
      item.setSize(this.viewportSize.x, this.viewportSize.y)
    );
    this._depthNormalRenderTarget?.setSize(
      this.viewportSize.x,
      this.viewportSize.y
    );
  }

  public setLineOfSight(
    cameraPosition: Vector3,
    targetPosition: Vector3
  ): void {
    this.updateLightPositionAndTarget(cameraPosition, targetPosition);
    const viewSpacePositions = [
      new Vector3(0, 0, 0),
      new Vector3(
        -this.rectAreaLight.width / 6,
        -this.rectAreaLight.height / 3,
        0
      ),
      new Vector3(
        this.rectAreaLight.width / 3,
        this.rectAreaLight.height / 6,
        0
      ),
      new Vector3(
        -this.rectAreaLight.width / 3,
        this.rectAreaLight.height / 6,
        0
      ),
      new Vector3(
        this.rectAreaLight.width / 6,
        -this.rectAreaLight.height / 3,
        0
      ),
      new Vector3(-this.rectAreaLight.width / 2, 0, 0),
      new Vector3(this.rectAreaLight.width / 2, 0, 0),
      new Vector3(0, -this.rectAreaLight.height / 2, 0),
      new Vector3(0, this.rectAreaLight.height / 2, 0),
    ];
    this.lightSamples = viewSpacePositions.map((position) => {
      return {
        position: position.applyMatrix4(
          this._directionalLight.shadow.camera.matrixWorld
        ),
        target: targetPosition,
      };
    });
    this.currentLightSample = 0;
  }

  private updateLightPositionAndTarget(
    cameraPosition: Vector3,
    targetPosition: Vector3
  ): void {
    this._rectAreaLight.position.set(
      cameraPosition.x,
      cameraPosition.y,
      cameraPosition.z
    );
    this._rectAreaLight.lookAt(
      targetPosition.x,
      targetPosition.y,
      targetPosition.z
    );
    this.updateShadowPositionAndTarget(cameraPosition, targetPosition);
  }

  private updateShadowPositionAndTarget(
    cameraPosition: Vector3,
    targetPosition: Vector3
  ): void {
    this._directionalLight.position.set(
      cameraPosition.x,
      cameraPosition.y,
      cameraPosition.z
    );
    this._directionalLight.lookAt(
      targetPosition.x,
      targetPosition.y,
      targetPosition.z
    );
    this._directionalLight.shadow.camera.position.set(
      cameraPosition.x,
      cameraPosition.y,
      cameraPosition.z
    );
    this._directionalLight.shadow.camera.lookAt(
      targetPosition.x,
      targetPosition.y,
      targetPosition.z
    );
    //this._directionalLight.shadow.camera.near = 0.5;
    //this._directionalLight.shadow.camera.far = 20.0;
    this._directionalLight.updateMatrixWorld();
    this._directionalLight.shadow.camera.updateMatrixWorld();
  }

  public setShadowVolume(shadowVolumeBox: Box3): void {
    setOrthographicViewVolumeFromBox(
      this._directionalLight.shadow.camera,
      shadowVolumeBox
    );
  }

  public renderOnlyShadowMap(
    renderer: WebGLRenderer,
    scene: Scene,
    camera: Camera
  ): void {
    const needsUpdate =
      this.castShadow &&
      (this.needsUpdate ||
        this.alwaysUpdate ||
        this.cameraUpdate.changed(camera));
    if (!needsUpdate) {
      return;
    }
    this.needsUpdate = false;
    const isVisible = this.visible;
    this._directionalLight.visible = true;
    this._directionalLight.castShadow = true;
    this._rectAreaLight.visible = false;
    this.currentTarget = 0;
    this.renderShadowMap(renderer, scene, camera, this.shadowRenderTargets[0]);
    this.currentLightSample++;
    this._directionalLight.visible = false;
    this._directionalLight.castShadow = false;
    this._rectAreaLight.visible = isVisible;
  }

  public renderShadow(
    renderer: WebGLRenderer,
    scene: Scene,
    camera: Camera
  ): void {
    const needsUpdate =
      this.castShadow &&
      (this.needsUpdate ||
        this.alwaysUpdate ||
        this.cameraUpdate.changed(camera));
    if (!needsUpdate) {
      if (this.monteCarlo && this.currentLightSample === 5) {
        return;
      } else if (
        !this.softShadow ||
        this.currentLightSample > this.lightSamples.length
      ) {
        return;
      }
    } else {
      this.currentLightSample = 0;
    }
    this.needsUpdate = false;
    const isVisible = this.visible;
    this._directionalLight.visible = true;
    this._directionalLight.castShadow = true;
    this._rectAreaLight.visible = false;
    if (this.monteCarlo) {
      this.renderSoftShadowWithMonteCarloIntegration(renderer, scene, camera);
    } else {
      this.renderSoftShadowWithMultipleLightSources(renderer, scene, camera);
    }
    this.currentLightSample++;
    this._directionalLight.visible = false;
    this._directionalLight.castShadow = false;
    this._rectAreaLight.visible = isVisible;
  }

  private renderSoftShadowWithMonteCarloIntegration(
    renderer: WebGLRenderer,
    scene: Scene,
    camera: Camera
  ) {
    if (this.currentLightSample === 0) {
      this.currentTarget = 0;
      this.renderShadowMap(
        renderer,
        scene,
        camera,
        this.shadowRenderTargets[0]
      );
      this.depthNormalRenderTarget.render(renderer, scene, camera);
      this.renderPass.renderScreenSpace(
        renderer,
        this.updateShadowMaterial(camera, this.shadowRenderTargets[0]),
        this.shadowRenderTargets[1]
      );
      this.renderPass.renderScreenSpace(
        renderer,
        this.updateBlurShadowMaterial(camera, this.shadowRenderTargets[1]),
        this.shadowRenderTargets[2]
      );
      this.currentTarget = 2;
    } else {
      const source = this.currentTarget;
      this.currentTarget =
        (this.currentTarget + 1) % this.shadowRenderTargets.length;
      this.renderPass.renderScreenSpace(
        renderer,
        this.updateBlurShadowMaterial(camera, this.shadowRenderTargets[source]),
        this.shadowRenderTargets[this.currentTarget]
      );
    }
  }

  private renderSoftShadowWithMultipleLightSources(
    renderer: WebGLRenderer,
    scene: Scene,
    camera: Camera
  ) {
    if (this.currentLightSample === 0) {
      this.currentTarget = 0;
      this.renderShadowMap(
        renderer,
        scene,
        camera,
        this.shadowRenderTargets[0]
      );
    } else if (this.currentLightSample >= this.lightSamples.length) {
      this.renderSoftBlur(renderer);
    } else {
      this.updateShadowPositionAndTarget(
        this.lightSamples[this.currentLightSample].position,
        this.lightSamples[this.currentLightSample].target
      );
      this.currentTarget =
        this.currentLightSample === 1
          ? 1
          : (this.currentTarget + 1) % this.shadowRenderTargets.length;
      const prevShadowTarget =
        (this.currentTarget + 2) % this.shadowRenderTargets.length;
      const shadowTarget =
        (this.currentTarget + 1) % this.shadowRenderTargets.length;
      this.renderShadowMap(
        renderer,
        scene,
        camera,
        this.shadowRenderTargets[shadowTarget]
      );
      this.mixShadowMaterial.uniforms.tShadow1.value =
        this.shadowRenderTargets[prevShadowTarget].texture;
      this.mixShadowMaterial.uniforms.tShadow2.value =
        this.shadowRenderTargets[shadowTarget].texture;
      this.mixShadowMaterial.uniforms.shadowScale1.value =
        this.currentLightSample / (this.currentLightSample + 1);
      this.mixShadowMaterial.uniforms.shadowScale2.value =
        1 / (this.currentLightSample + 1);
      this.renderPass.renderScreenSpace(
        renderer,
        this.mixShadowMaterial,
        this.shadowRenderTargets[this.currentTarget]
      );
      this.updateShadowPositionAndTarget(
        this.lightSamples[0].position,
        this.lightSamples[0].target
      );
    }
  }

  private renderSoftBlur(renderer: WebGLRenderer) {
    const blur = 0.4;
    const blurShadowMin = 0;
    const blurShadowMax = 0.95;
    this.horizontalBlurMaterial.uniforms.tDiffuse.value =
      this.shadowRenderTargets[this.currentTarget].texture;
    this.horizontalBlurMaterial.uniforms.blur.value = (blur * 1) / 256;
    this.horizontalBlurMaterial.uniforms.shadowRange.value.x = blurShadowMin;
    this.horizontalBlurMaterial.uniforms.shadowRange.value.y = blurShadowMax;
    this.currentTarget =
      (this.currentTarget + 1) % this.shadowRenderTargets.length;
    this.renderPass.renderScreenSpace(
      renderer,
      this.horizontalBlurMaterial,
      this.shadowRenderTargets[this.currentTarget]
    );
    this.verticalBlurMaterial.uniforms.tDiffuse.value =
      this.shadowRenderTargets[this.currentTarget].texture;
    this.verticalBlurMaterial.uniforms.blur.value = (blur * 1) / 256;
    this.verticalBlurMaterial.uniforms.shadowRange.value.x = blurShadowMin;
    this.verticalBlurMaterial.uniforms.shadowRange.value.y = blurShadowMax;
    this.currentTarget =
      (this.currentTarget + 1) % this.shadowRenderTargets.length;
    this.renderPass.renderScreenSpace(
      renderer,
      this.verticalBlurMaterial,
      this.shadowRenderTargets[this.currentTarget]
    );
  }

  public renderShadowMap(
    renderer: WebGLRenderer,
    scene: Scene,
    camera: Camera,
    renderTarget: WebGLRenderTarget
  ): void {
    const materialCache = new Map();
    scene.traverse((object: any) => {
      if (
        object.isMesh &&
        object?.material &&
        !Array.isArray(object.material) &&
        !object.material.transparent
      ) {
        materialCache.set(object, object.material);
        object.material = this.shadowMaterial;
      }
    });
    this.renderPass.render(renderer, scene, camera, renderTarget, 0xffffff, 1);
    materialCache.forEach((material: any, object: any) => {
      object.material = material;
    });
  }

  public blendShadow(renderer: WebGLRenderer, overwriteTarget: boolean): void {
    if (this.castShadow) {
      const brightness = overwriteTarget ? 0 : 1 - this._shadowIntensity;
      this.renderPass.renderScreenSpace(
        renderer,
        this.copyMaterial.update({
          texture: this.shadowTexture,
          blending: overwriteTarget ? NoBlending : CustomBlending,
          colorTransform: new Matrix4().set(
            1 - brightness,
            0,
            0,
            brightness,
            1 - brightness,
            0,
            0,
            brightness,
            1 - brightness,
            0,
            0,
            brightness,
            0,
            0,
            0,
            1
          ),
        }),
        null
      );
    }
  }

  public updateShadowMaterial(
    camera: Camera,
    renderTarget: WebGLRenderTarget
  ): ShaderMaterial {
    return this.shadowRenderMaterial.update({
      camera,
      texture: renderTarget.texture,
      kernelRadius: this.shadowRadius,
    });
  }

  public updateBlurShadowMaterial(
    camera: Camera,
    renderTarget: WebGLRenderTarget
  ): ShaderMaterial {
    return this.blurRenderMaterial.update({
      width: this.viewportSize.x,
      height: this.viewportSize.y,
      camera,
      texture: renderTarget.texture,
      kernelRadius: this.shadowRadius,
    });
  }

  public updateShadowKernel() {
    if (this._noiseTexture) {
      this._noiseTexture = this.generateRandomKernelRotations();
    }
    if (this._kernel) {
      this._kernel = this.generateSampleKernel();
    }
    if (this._shadowRenderMaterial) {
      this._shadowRenderMaterial.uniforms.tNoise.value = this._noiseTexture;
      this._shadowRenderMaterial.uniforms.kernel.value = this._kernel;
    }
  }

  private generateSampleKernel(): Vector3[] {
    const kernelSize = IntegralShadowRenderMaterial.kernelSize;
    const kernel: Vector3[] = [];
    for (let i = 0; i < kernelSize; i++) {
      const sample = new Vector3();
      sample.x = Math.random() * 2 - 1;
      sample.y = Math.random() * 2 - 1;
      sample.z = Math.random();
      sample.normalize();
      let scale = i / kernelSize;
      scale = MathUtils.lerp(0.1, 1, scale);
      sample.multiplyScalar(scale);
      kernel.push(sample);
    }
    return kernel;
  }

  private generateRandomKernelRotations(): DataTexture {
    const width = 4;
    const height = 4;
    if (SimplexNoise === undefined) {
      console.error('Integral Shadow Pass: The pass relies on SimplexNoise.');
    }
    const simplex = new SimplexNoise();
    const size = width * height;
    const data = new Float32Array(size);
    for (let i = 0; i < size; i++) {
      const x = Math.random() * 2 - 1;
      const y = Math.random() * 2 - 1;
      const z = 0;
      data[i] = simplex.noise3d(x, y, z);
    }
    const noiseTexture = new DataTexture(
      data,
      width,
      height,
      RedFormat,
      FloatType
    );
    noiseTexture.wrapS = RepeatWrapping;
    noiseTexture.wrapT = RepeatWrapping;
    noiseTexture.needsUpdate = true;
    return noiseTexture;
  }
}
