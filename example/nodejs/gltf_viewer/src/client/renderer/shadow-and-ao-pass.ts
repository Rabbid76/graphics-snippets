import { GBufferRenderTargets, GBufferTextures } from './gbuffer-render-target';
import { CopyTransformMaterial } from './shader-utility';
import {
  CameraUpdate,
  DenoisePass,
  generateUniformKernelRotations,
  RenderOverrideVisibility,
  RenderPass,
  SceneVolume,
  spiralQuadraticSampleKernel,
} from './render-utility';
import {
  defaultPoisonDenoiseParameters,
  defaultSSAOEffectParameters,
  defaultHBAOEffectParameters,
  HBAOEffect,
  HBAOEffectParameters,
  PoissionDenoiseEffect,
  PoisonDenoiseParameters,
  SSAOEffect,
  SSAOEffectParameters,
} from './effect-pass';
import {
  Camera,
  CustomBlending,
  DataTexture,
  Matrix4,
  NearestFilter,
  NoBlending,
  OrthographicCamera,
  PerspectiveCamera,
  RGFormat,
  Scene,
  ShaderMaterial,
  Texture,
  UniformsUtils,
  Vector2,
  Vector3,
  WebGLRenderer,
  WebGLRenderTarget,
  LinearFilter,
} from 'three';
import { FXAAShader } from 'three/examples/jsm/shaders/FXAAShader';

export enum AmbientOcclusionType {
  NONE,
  SSAO,
  EffectsSSAO,
  EffectsHBAO,
}

export interface ShadowAndAoParameters {
  [key: string]: any;
  aoAndSoftShadowFxaa: boolean;
  aoKernelRadius: number;
  aoDepthBias: number;
  aoMaxDistance: number;
  aoMaxDepth: number;
  aoFadeout: number;
  shadowRadius: number;
  shadowIntensity: number;
}

export interface ShadowAndAoPassParameters {
  enabled: boolean;
  aoType: AmbientOcclusionType;
  aoIntensity: number;
  aoOnGround: boolean;
  enablePoissionDenoise: boolean;
  alwaysUpdate: boolean;
  shAndAo: ShadowAndAoParameters;
  ssao: SSAOEffectParameters;
  hbao: HBAOEffectParameters;
  poissionDenoise: PoisonDenoiseParameters;
}

export const defaultShadowAndAoParameters: ShadowAndAoParameters = {
  aoAndSoftShadowFxaa: true,
  aoKernelRadius: 0.25, //0.05,
  aoDepthBias: 0.0001,
  aoMaxDistance: 0.5, //0.1,
  aoMaxDepth: 0.99,
  aoFadeout: 0.5, //1,
  shadowRadius: 0.1,
  shadowIntensity: 0.3,
};

export const defaultPassParameters = {
  enabled: true,
  aoType: AmbientOcclusionType.SSAO,
  aoIntensity: 0.7,
  aoOnGround: true,
  enablePoissionDenoise: true,
  alwaysUpdate: false,
};

export class ShadowAndAoPass {
  public static shadowTransform: Matrix4 = new Matrix4().set(
    // eslint-disable-next-line prettier/prettier
    0, 1, 0, 0,
    // eslint-disable-next-line prettier/prettier
    0, 1, 0, 0,
    // eslint-disable-next-line prettier/prettier
    0, 1, 0, 0,
    // eslint-disable-next-line prettier/prettier
    0, 0, 0, 1
  );
  public parameters: ShadowAndAoPassParameters;
  public needsUpdate: boolean = true;
  private width: number;
  private height: number;
  private samples: number;
  protected renderPass: RenderPass = new RenderPass();
  private _sharedGBufferRenderTarget?: GBufferRenderTargets;
  private _gBufferRenderTarget?: GBufferRenderTargets;
  public shadowAndAoRenderTargets: ShadowAndAoRenderTargets;
  private shadowAndAoDenoisePass?: ShadowAndAoDenoisePass;
  private _ssaoEffect?: SSAOEffect;
  private _hbaoEffect?: HBAOEffect;
  public poissionDenoiseEffect?: PoissionDenoiseEffect;
  private _copyMaterial: CopyTransformMaterial;
  private _renderOverrideVisibility: RenderOverrideVisibility =
    new RenderOverrideVisibility(true);
  private cameraUpdate: CameraUpdate = new CameraUpdate();

  public get gBufferRenderTarget(): GBufferRenderTargets {
    if (this._sharedGBufferRenderTarget) {
      return this._sharedGBufferRenderTarget;
    }
    this._gBufferRenderTarget =
      this._gBufferRenderTarget ??
      new GBufferRenderTargets({
        width: this.width,
        height: this.height,
        samples: this.samples,
        renderPass: this.renderPass,
        renderOverrideVisibility: this._renderOverrideVisibility,
        textureFilter: NearestFilter,
      });
    return this._gBufferRenderTarget;
  }

  public get SSAOEffect(): SSAOEffect {
    if (!this._ssaoEffect) {
      this._ssaoEffect = new SSAOEffect(this.width, this.height, {
        floatBufferNormalTexture:
          this.gBufferRenderTarget.isFloatGBufferWithRgbNormalAlphaDepth,
        modulateRedChannel: true,
        ssaoParameters: this.parameters.ssao,
      });
    }
    return this._ssaoEffect;
  }

  public get HBAOEffect(): HBAOEffect {
    if (!this._hbaoEffect) {
      this._hbaoEffect = new HBAOEffect(this.width, this.height, {
        floatBufferNormalTexture:
          this.gBufferRenderTarget.isFloatGBufferWithRgbNormalAlphaDepth,
        modulateRedChannel: true,
        hbaoParameters: this.parameters.hbao,
      });
    }
    return this._hbaoEffect;
  }

  public get denoisePass(): DenoisePass {
    if (this.parameters.enablePoissionDenoise) {
      if (!this.poissionDenoiseEffect) {
        this.poissionDenoiseEffect = new PoissionDenoiseEffect(
          this.width,
          this.height,
          {
            inputTexture:
              this.shadowAndAoRenderTargets.passRenderTarget.texture,
            depthTexture: this.gBufferRenderTarget.depthBufferTexture,
            normalTexture: this.gBufferRenderTarget.gBufferTexture,
            floatBufferNormalTexture:
              this.gBufferRenderTarget.isFloatGBufferWithRgbNormalAlphaDepth,
            rgInputTexture: true,
            poisonDenoiseParameters: this.parameters.poissionDenoise,
          }
        );
      }
      return this.poissionDenoiseEffect;
    } else {
      if (!this.shadowAndAoDenoisePass) {
        this.shadowAndAoDenoisePass = new ShadowAndAoDenoisePass(
          this.shadowAndAoRenderTargets,
          this.gBufferRenderTarget,
          {
            width: this.width,
            height: this.height,
            samples: this.samples,
            renderPass: this.renderPass,
          }
        );
      }
      return this.shadowAndAoDenoisePass;
    }
  }

  public get denoiseRenderTargetTexture(): Texture | null {
    return this.denoisePass ? this.denoisePass.texture : null;
  }

  constructor(
    width: number,
    height: number,
    samples: number,
    parameters?: any
  ) {
    if (parameters?.gBufferRenderTarget) {
      this._sharedGBufferRenderTarget = parameters?.gBufferRenderTarget;
    }
    this.width = width;
    this.height = height;
    this.samples = samples;
    this.shadowAndAoRenderTargets = new ShadowAndAoRenderTargets(
      this.gBufferRenderTarget,
      { ...parameters, width, height, samples, renderPass: this.renderPass }
    );
    this.parameters = {
      enabled: parameters?.enabled ?? defaultPassParameters.enabled,
      aoType: parameters?.aoType ?? defaultPassParameters.aoType,
      aoIntensity: parameters?.aoIntensity ?? defaultPassParameters.aoIntensity,
      aoOnGround: parameters?.aoOnGround ?? defaultPassParameters.aoOnGround,
      enablePoissionDenoise:
        parameters?.enablePoissionDenoise ??
        defaultPassParameters.enablePoissionDenoise,
      alwaysUpdate:
        parameters?.alwaysUpdate ?? defaultPassParameters.alwaysUpdate,
      shAndAo: this.shadowAndAoRenderTargets.shadowAndAoParameters,
      ssao: {
        ...defaultSSAOEffectParameters,
      },
      hbao: {
        ...defaultHBAOEffectParameters,
      },
      poissionDenoise: {
        ...defaultPoisonDenoiseParameters,
        //radius: 8,
      },
    };
    this._copyMaterial = new CopyTransformMaterial();
    this.updateCopyMaterial();
  }

  dispose() {
    this.gBufferRenderTarget?.dispose();
    this.shadowAndAoRenderTargets.dispose();
    this.shadowAndAoDenoisePass?.dispose();
    this._ssaoEffect?.dispose();
    this._hbaoEffect?.dispose();
    this.poissionDenoiseEffect?.dispose();
    this._copyMaterial?.dispose();
  }

  public setSize(width: number, height: number) {
    this.width = width;
    this.height = height;
    this._gBufferRenderTarget?.setSize(width, height);
    this.shadowAndAoRenderTargets.setSize(width, height);
    this.shadowAndAoDenoisePass?.setSize(this.width, this.height);
    this._ssaoEffect?.setSize(width, height);
    this._hbaoEffect?.setSize(width, height);
    this.poissionDenoiseEffect?.setSize(this.width, this.height);
    this._renderOverrideVisibility = new RenderOverrideVisibility();
    this.updateCopyMaterial();
    this.needsUpdate = true;
  }

  public updateParameters(parameters: any) {
    if (parameters.enabled !== undefined) {
      this.parameters.enabled = parameters.enabled;
    }
    if (parameters.aoType !== undefined) {
      this.parameters.aoType = parameters.aoType;
    }
    if (parameters.aoIntensity !== undefined) {
      this.parameters.aoIntensity = parameters.aoIntensity;
    }
    if (parameters.aoOnGround !== undefined) {
      this.parameters.aoOnGround = parameters.aoOnGround;
    }
    if (parameters.enablePoissionDenoise !== undefined) {
      this.parameters.enablePoissionDenoise = parameters.enablePoissionDenoise;
    }
    if (parameters.alwaysUpdate !== undefined) {
      this.parameters.alwaysUpdate = parameters.alwaysUpdate;
    }
    if (parameters?.shAndAo) {
      for (let propertyName in parameters.shAndAo) {
        if (this.parameters.shAndAo.hasOwnProperty(propertyName)) {
          this.parameters.shAndAo[propertyName] =
            parameters.shAndAo[propertyName];
          this.shadowAndAoRenderTargets.parametersNeedsUpdate = true;
        }
      }
    }
    if (parameters?.ssao) {
      this._hbaoEffect?.updateParameters(parameters?.hbao);
    }
    if (parameters?.hbao) {
      this._ssaoEffect?.updateParameters(parameters?.hbao);
    }
    if (parameters?.poissionDenoise) {
      this.poissionDenoiseEffect?.updateParameters(parameters?.poissionDenoise);
    }
  }

  public updateBounds(sceneBounds: SceneVolume, shadowAndAoScale: number) {
    this.shadowAndAoRenderTargets.updateBounds(sceneBounds, shadowAndAoScale);
    this.shadowAndAoDenoisePass?.updateBounds(sceneBounds, shadowAndAoScale);
  }

  public updateShadowTexture(shadowTexture: Texture) {
    this.shadowAndAoRenderTargets.updateShadowTexture(shadowTexture);
  }

  public render(
    renderer: WebGLRenderer,
    scene: Scene,
    camera: Camera,
    intensityScale: number = 1
  ): void {
    this.shadowAndAoRenderTargets.aoEnabled =
      this.parameters.aoType === AmbientOcclusionType.SSAO;
    if (
      !this.parameters.enabled ||
      !(
        this.shadowAndAoRenderTargets.aoEnabled ||
        this.shadowAndAoRenderTargets.shadowEnabled
      )
    ) {
      return;
    }
    if (this.needsUpdate) {
      if (this._ssaoEffect) {
        this._ssaoEffect.needsUpdate = true;
      }
      if (this._hbaoEffect) {
        this._hbaoEffect.needsUpdate = true;
      }
      if (this.poissionDenoiseEffect) {
        this.poissionDenoiseEffect.needsUpdate = true;
      }
    }
    if (this.evaluateIfUpdateIsNeeded(camera)) {
      this.renderShadowAndAo(renderer, scene, camera);
    }
    this.renderToTarget(renderer, intensityScale);
  }

  private evaluateIfUpdateIsNeeded(camera: Camera): boolean {
    (camera as OrthographicCamera | PerspectiveCamera).updateProjectionMatrix();
    const needsUpdate =
      this.parameters.alwaysUpdate ||
      this.needsUpdate ||
      (camera != null && this.cameraUpdate.changed(camera));
    this.needsUpdate = false;
    return needsUpdate;
  }

  private updateCopyMaterial(intensityScale: number = 1) {
    const aoIntensity = this.parameters.aoIntensity * intensityScale;
    const shIntensity =
      this.shadowAndAoRenderTargets.shadowIntensity * intensityScale;
    this._copyMaterial.update({
      texture: this.denoiseRenderTargetTexture,
      blending: CustomBlending,
      colorTransform: new Matrix4().set(
        // eslint-disable-next-line prettier/prettier
        aoIntensity, 0, 0, 1 - aoIntensity,
        // eslint-disable-next-line prettier/prettier
        0, shIntensity, 0, 1 - shIntensity,
        // eslint-disable-next-line prettier/prettier
        0, 0, 0, 1,
        // eslint-disable-next-line prettier/prettier
        0, 0, 0, 1
      ),
      multiplyChannels: 1,
    });
  }

  private renderShadowAndAo(
    renderer: WebGLRenderer,
    scene: Scene,
    camera: Camera
  ): void {
    this.gBufferRenderTarget.render(renderer, scene, camera);
    this.shadowAndAoRenderTargets.render(renderer, camera);
    this.renderAoEffect(renderer, scene, camera);
    this.denoisePass.inputTexture =
      this.shadowAndAoRenderTargets.passRenderTarget.texture;
    this.denoisePass.render(renderer, camera);
  }

  private renderAoEffect(
    renderer: WebGLRenderer,
    scene: Scene,
    camera: Camera
  ): void {
    if (
      this.parameters.aoType !== AmbientOcclusionType.EffectsSSAO &&
      this.parameters.aoType !== AmbientOcclusionType.EffectsHBAO
    ) {
      return;
    }
    const depthTexture = this.gBufferRenderTarget.depthBufferTexture;
    const normalTexture = this.gBufferRenderTarget.gBufferTexture;
    const renderTarget = this.shadowAndAoRenderTargets.passRenderTarget;
    const autoClear = renderer.autoClear;
    renderer.autoClear = false;
    switch (this.parameters.aoType) {
      default:
        break;
      case AmbientOcclusionType.EffectsSSAO:
        {
          const ssaoEffect = this.SSAOEffect;
          ssaoEffect.depthTexture = depthTexture;
          ssaoEffect.normalTexture = normalTexture;
          ssaoEffect.render(renderer, camera, scene, renderTarget);
        }
        break;
      case AmbientOcclusionType.EffectsHBAO:
        {
          const hbaoEffect = this.HBAOEffect;
          hbaoEffect.depthTexture = depthTexture;
          hbaoEffect.normalTexture = normalTexture;
          hbaoEffect.render(renderer, camera, scene, renderTarget);
        }
        break;
    }
    renderer.autoClear = autoClear;
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

export class ShadowAndAoRenderTargets {
  public shadowAndAoParameters: ShadowAndAoParameters;
  public parametersNeedsUpdate: boolean = true;
  public aoEnabled: boolean = true;
  public shadowEnabled: boolean = true;
  private width: number;
  private height: number;
  private samples: number;
  private shadowAndAoScale: number = 1;
  private aoTargetSamples: number = 0;
  private depthAndNormalTextures: GBufferTextures;
  private _noiseTexture?: DataTexture;
  private _sampleKernel: Vector3[] = [];
  private _passRenderMaterial?: ShadowAndAoRenderMaterial;
  private _passRenderTarget?: WebGLRenderTarget;
  private renderPass: RenderPass;

  public get shadowIntensity(): number {
    return this.shadowEnabled ? this.shadowAndAoParameters.shadowIntensity : 0;
  }

  public get passRenderTarget(): WebGLRenderTarget {
    this._passRenderTarget =
      this._passRenderTarget ??
      new WebGLRenderTarget(this.width, this.height, {
        samples: this.aoTargetSamples,
        format: RGFormat,
        magFilter: LinearFilter,
        minFilter: LinearFilter,
      });
    return this._passRenderTarget;
  }

  private get passRenderMaterial(): ShadowAndAoRenderMaterial {
    this._passRenderMaterial =
      this._passRenderMaterial ??
      new ShadowAndAoRenderMaterial({
        normalTexture: this.depthAndNormalTextures.gBufferTexture,
        depthTexture: this.depthAndNormalTextures.depthBufferTexture,
        noiseTexture: this.noiseTexture,
        sampleKernel: this.sampleKernel,
        floatGBufferRgbNormalAlphaDepth:
          this.depthAndNormalTextures.isFloatGBufferWithRgbNormalAlphaDepth,
      });
    return this._passRenderMaterial;
  }

  private get noiseTexture(): DataTexture {
    this._noiseTexture = this._noiseTexture ?? generateUniformKernelRotations();
    return this._noiseTexture;
  }

  private get sampleKernel(): Vector3[] {
    if (!this._sampleKernel.length) {
      this._sampleKernel = spiralQuadraticSampleKernel(
        ShadowAndAoRenderMaterial.kernelSize
      );
    }
    return this._sampleKernel;
  }

  constructor(depthAndNormalTextures: GBufferTextures, parameters?: any) {
    this.shadowAndAoParameters = this.getShadowAndAoParameters(parameters);
    this.width = parameters?.width ?? 1024;
    this.height = parameters?.height ?? 1024;
    this.samples = parameters?.samples ?? 0;
    this.depthAndNormalTextures = depthAndNormalTextures;
    this.renderPass = parameters?.renderPass ?? new RenderPass();
  }

  private getShadowAndAoParameters(parameters?: any): ShadowAndAoParameters {
    return {
      ...defaultShadowAndAoParameters,
      ...parameters,
    };
  }

  public dispose() {
    this._noiseTexture?.dispose();
    this._passRenderMaterial?.dispose();
    this._passRenderTarget?.dispose();
  }

  public setSize(width: number, height: number) {
    this.width = width;
    this.height = height;
    this._passRenderMaterial?.update({
      width: this.width,
      height: this.height,
    });
  }

  public updateBounds(sceneBounds: SceneVolume, shadowAndAoScale: number) {
    this.shadowAndAoScale = shadowAndAoScale;
    this.parametersNeedsUpdate = true;
  }

  public render(renderer: WebGLRenderer, camera: Camera): void {
    this.renderPass.renderScreenSpace(
      renderer,
      this.updateSSAOMaterial(camera),
      this.passRenderTarget
    );
    this.parametersNeedsUpdate = false;
  }

  public updateSSAOMaterial(camera: Camera): ShaderMaterial {
    const passRenderMaterial = this.passRenderMaterial;
    passRenderMaterial.updateDependencies({
      width: this.width,
      height: this.height,
      camera,
    });
    if (this.parametersNeedsUpdate) {
      passRenderMaterial.updateSettings({
        ...this.shadowAndAoParameters,
        aoKernelRadius:
          this.shadowAndAoParameters.aoKernelRadius * this.shadowAndAoScale,
        aoMaxDistance:
          this.shadowAndAoParameters.aoMaxDistance * this.shadowAndAoScale,
        shadowRadius:
          this.shadowAndAoParameters.shadowRadius * this.shadowAndAoScale,
        aoIntensity: this.aoEnabled ? 1 : 0,
        shadowIntensity: this.shadowIntensity,
      });
    }
    return passRenderMaterial;
  }

  public updateShadowTexture(shadowTexture: Texture) {
    if (this._passRenderMaterial) {
      this._passRenderMaterial.uniforms.tShadow.value = shadowTexture;
    }
  }
}

export class ShadowAndAoDenoisePass implements DenoisePass {
  public _inputTexture: Texture | null = null;
  public shadowAndAoRenderTargets: ShadowAndAoRenderTargets;
  public parametersNeedsUpdate: boolean = true;
  private width: number;
  private height: number;
  private samples: number;
  private shadowAndAoScale: number = 1;
  private aoTargetSamples: number = 0;
  private depthAndNormalTextures: GBufferTextures;
  private _blurRenderMaterial?: ShadowAndAoBlurMaterial;
  private _fxaaRenderMaterial?: ShaderMaterial;
  private _blurRenderTarget?: WebGLRenderTarget;
  private _fxaaRenderTarget?: WebGLRenderTarget;
  private renderPass: RenderPass;

  public get shadowAndAoParameters(): ShadowAndAoParameters {
    return this.shadowAndAoRenderTargets.shadowAndAoParameters;
  }

  public set inputTexture(texture: Texture | null) {
    this._inputTexture = texture;
  }

  public get texture(): Texture {
    return this.finalRenderTarget.texture;
  }

  public get finalRenderTarget(): WebGLRenderTarget {
    return this.shadowAndAoParameters.aoAndSoftShadowFxaa
      ? this.fxaaRenderTarget
      : this.blurRenderTarget;
  }

  public get blurRenderTarget(): WebGLRenderTarget {
    this._blurRenderTarget =
      this._blurRenderTarget ??
      new WebGLRenderTarget(this.width, this.height, {
        samples: this.aoTargetSamples,
        format: RGFormat,
        magFilter: LinearFilter,
        minFilter: LinearFilter,
      });
    return this._blurRenderTarget;
  }

  public get fxaaRenderTarget(): WebGLRenderTarget {
    this._fxaaRenderTarget =
      this._fxaaRenderTarget ??
      new WebGLRenderTarget(this.width, this.height, {
        samples: this.aoTargetSamples,
        format: RGFormat,
        magFilter: LinearFilter,
        minFilter: LinearFilter,
      });
    return this._fxaaRenderTarget;
  }

  private get blurRenderMaterial(): ShadowAndAoBlurMaterial {
    this._blurRenderMaterial =
      this._blurRenderMaterial ??
      new ShadowAndAoBlurMaterial({
        normalTexture: this.depthAndNormalTextures.gBufferTexture,
        depthTexture: this.depthAndNormalTextures.depthBufferTexture,
        floatGBufferRgbNormalAlphaDepth:
          this.depthAndNormalTextures.isFloatGBufferWithRgbNormalAlphaDepth,
      });
    return this._blurRenderMaterial;
  }

  private get fxaaRenderMaterial(): ShaderMaterial {
    this._fxaaRenderMaterial =
      this._fxaaRenderMaterial ?? new ShaderMaterial(FXAAShader);
    this._fxaaRenderMaterial.uniforms.tDiffuse.value =
      this.blurRenderTarget.texture;
    this._fxaaRenderMaterial.uniforms.resolution.value.set(
      1 / this.width,
      1 / this.height
    );
    return this._fxaaRenderMaterial;
  }

  constructor(
    shadowAndAoRenderTargets: ShadowAndAoRenderTargets,
    depthAndNormalTextures: GBufferTextures,
    parameters?: any
  ) {
    this.shadowAndAoRenderTargets = shadowAndAoRenderTargets;
    this.depthAndNormalTextures = depthAndNormalTextures;
    this.width = parameters?.width ?? 1024;
    this.height = parameters?.height ?? 1024;
    this.samples = parameters?.samples ?? 0;
    this.renderPass = parameters?.renderPass ?? new RenderPass();
  }

  public dispose() {
    this._blurRenderMaterial?.dispose();
    this._fxaaRenderMaterial?.dispose();
    this._blurRenderTarget?.dispose();
    this._fxaaRenderTarget?.dispose();
  }

  public setSize(width: number, height: number) {
    this.width = width;
    this.height = height;
    this._blurRenderMaterial?.update({
      width: this.width,
      height: this.height,
    });
    this._blurRenderTarget?.setSize(this.width, this.height);
    this._fxaaRenderTarget?.setSize(this.width, this.height);
  }

  public updateBounds(sceneBounds: SceneVolume, shadowAndAoScale: number) {
    this.shadowAndAoScale = shadowAndAoScale;
    this.parametersNeedsUpdate = true;
  }

  public render(renderer: WebGLRenderer, camera: Camera): void {
    this.renderPass.renderScreenSpace(
      renderer,
      this.updateBlurMaterial(camera),
      this.blurRenderTarget
    );
    if (this.shadowAndAoParameters.aoAndSoftShadowFxaa) {
      this.renderPass.renderScreenSpace(
        renderer,
        this.fxaaRenderMaterial,
        this.fxaaRenderTarget
      );
    }
    this.parametersNeedsUpdate = false;
  }

  public updateBlurMaterial(camera: Camera): ShaderMaterial {
    const blurRenderMaterial = this.blurRenderMaterial;
    blurRenderMaterial.updateDependencies({
      width: this.width,
      height: this.height,
      texture: this._inputTexture,
      camera,
    });
    blurRenderMaterial.updateDependencies({
      camera,
      aoKernelRadius:
        this.shadowAndAoParameters.aoKernelRadius * this.shadowAndAoScale,
      aoMaxDistance:
        this.shadowAndAoParameters.aoMaxDistance * this.shadowAndAoScale,
      shadowRadius:
        this.shadowAndAoParameters.shadowRadius * this.shadowAndAoScale,
    });
    return blurRenderMaterial;
  }
}

const glslShadowAndAoVertexShader = `varying vec2 vUv;
  void main() {
      vUv = uv;
      gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
  }`;

const glslShadowAndAoFragmentShader = `uniform sampler2D tShadow;
  uniform sampler2D tNormal;
#if FLOAT_GBUFFER_RGB_NORMAL_ALPHA_DEPTH != 1 
  uniform sampler2D tDepth;
#endif  
  uniform sampler2D tNoise;
  uniform vec3 sampleKernel[KERNEL_SIZE];
  uniform vec2 resolution;
  uniform float cameraNear;
  uniform float cameraFar;
  uniform mat4 cameraProjectionMatrix;
  uniform mat4 cameraInverseProjectionMatrix;
  uniform float aoKernelRadius;
  uniform float aoDepthBias; // avoid artifacts caused by neighbour fragments with minimal depth difference
  uniform float aoMaxDistance; // avoid the influence of fragments which are too far away
  uniform float aoMaxDepth;
  uniform float aoIntensity;
  uniform float aoFadeout;
  uniform float shKernelRadius;
  uniform float shIntensity;
  
  varying vec2 vUv;
  
  #include <packing>
  
  float getDepth(const in vec2 screenPosition) {
      #if FLOAT_GBUFFER_RGB_NORMAL_ALPHA_DEPTH == 1
          return texture2D(tNormal, screenPosition).w;
      #else    
          return texture2D(tDepth, screenPosition).x;
      #endif
  }
  
  float getViewZ(const in float depth) {
      #if PERSPECTIVE_CAMERA == 1
          return perspectiveDepthToViewZ(depth, cameraNear, cameraFar);
      #else
          return orthographicDepthToViewZ(depth, cameraNear, cameraFar);
      #endif
  }
  
  vec3 getViewPosition(const in vec2 screenPosition, const in float depth, const in float viewZ) {
      float clipW = cameraProjectionMatrix[2][3] * viewZ + cameraProjectionMatrix[3][3];
      vec4 clipPosition = vec4( ( vec3( screenPosition, depth ) - 0.5 ) * 2.0, 1.0 );
      clipPosition *= clipW; // unprojection.
      return ( cameraInverseProjectionMatrix * clipPosition ).xyz;
  }
  
  vec3 getViewNormal(const in vec2 screenPosition) {
      #if FLOAT_GBUFFER_RGB_NORMAL_ALPHA_DEPTH == 1
          return texture2D(tNormal, screenPosition).xyz;
      #else
          return unpackRGBToNormal(texture2D(tNormal, screenPosition).xyz);
      #endif
  }
  
  void main() {
  
      float depth = getDepth(vUv);
      float viewZ = getViewZ(depth);
  
      vec3 viewPosition = getViewPosition(vUv, depth, viewZ);
      vec3 viewNormal = getViewNormal(vUv);
  
      vec2 noiseScale = resolution.xy / 4.0;
      vec3 random = texture2D(tNoise, vUv * noiseScale).xyz * 2.0 - 1.0;
  
      // compute matrix used to reorient a kernel vector
      vec3 tangent = normalize(random - viewNormal * dot(random, viewNormal));
      vec3 bitangent = cross(viewNormal, tangent);
      mat3 kernelMatrix = mat3(tangent, bitangent, viewNormal);
  
      float aoOcclusion = 0.0;
      float shOcclusion = texture2D(tShadow, vUv).r;
      float shSamples = 0.0;
      for (int i = 0; i < KERNEL_SIZE; i ++) {
          if (aoIntensity >= 0.01 && length(viewNormal) > 0.01) {
              vec3 aoSampleVector = kernelMatrix * sampleKernel[i]; 
              vec3 aoSamplePoint = viewPosition + aoSampleVector * aoKernelRadius; 
              vec4 aoSamplePointNDC = cameraProjectionMatrix * vec4(aoSamplePoint, 1.0); 
              aoSamplePointNDC /= aoSamplePointNDC.w;
              vec2 aoSamplePointUv = aoSamplePointNDC.xy * 0.5 + 0.5;
              float aoRealSampleDepth = getDepth(aoSamplePointUv);
              float aoSampleDeltaZ = getViewZ(aoRealSampleDepth) - aoSamplePoint.z;
              float aoRealDeltaZ =  viewZ - aoSamplePoint.z;
              float w_long = clamp((aoMaxDistance - max(aoRealDeltaZ, aoSampleDeltaZ)) / aoMaxDistance, 0.0, 1.0);
              float w_lat = clamp(1.0 - length(aoSampleVector.xy), 0.0, 1.0);
              aoOcclusion += 
                  step(aoRealSampleDepth, aoMaxDepth) * 
                  step(aoDepthBias, aoSampleDeltaZ / (cameraFar - cameraNear)) * 
                  step(aoSampleDeltaZ, aoMaxDistance) * mix(1.0, w_long * w_lat, aoFadeout);
          }
          if (shIntensity >= 0.01 && length(viewNormal) > 0.01) {
              vec3 shSampleVector = kernelMatrix * sampleKernel[i]; // reorient sample vector in view space
              vec3 shSamplePoint = viewPosition + shSampleVector * shKernelRadius; // calculate sample point
              vec4 shSamplePointNDC = cameraProjectionMatrix * vec4(shSamplePoint, 1.0); // project point and calculate NDC
              shSamplePointNDC /= shSamplePointNDC.w;
              vec2 shSamplePointUv = shSamplePointNDC.xy * 0.5 + 0.5; // compute uv coordinates
              vec3 shSampleNormal = getViewNormal(shSamplePointUv);
              float shDeltaZ = getViewZ(getDepth(shSamplePointUv)) - shSamplePoint.z;
              float w = step(abs(shDeltaZ), shKernelRadius) * max(0.0, dot(shSampleNormal, viewNormal));
              shSamples += w;
              shOcclusion += texture2D(tShadow, shSamplePointUv).r * w;
          }
      }
  
      aoOcclusion = clamp(aoOcclusion / float(KERNEL_SIZE) * (1.0 + aoFadeout), 0.0, 1.0);
      shOcclusion = clamp(shOcclusion / (shSamples + 1.0), 0.0, 1.0);
      gl_FragColor = vec4(clamp(1.0 - aoOcclusion, 0.0, 1.0), shOcclusion, 0.0, 1.0);
  }`;

export class ShadowAndAoRenderMaterial extends ShaderMaterial {
  public static kernelSize: number = 32;
  private static shader: any = {
    uniforms: {
      tShadow: { value: null as Texture | null },
      tNormal: { value: null as Texture | null },
      tDepth: { value: null as Texture | null },
      tNoise: { value: null as Texture | null },
      sampleKernel: { value: null },
      cameraNear: { value: 0.1 },
      cameraFar: { value: 1 },
      resolution: { value: new Vector2() },
      cameraProjectionMatrix: { value: new Matrix4() },
      cameraInverseProjectionMatrix: { value: new Matrix4() },
      aoKernelRadius: { value: 0.1 },
      aoDepthBias: { value: 0.001 },
      aoMaxDistance: { value: 0.05 },
      aoMaxDepth: { value: 0.99 },
      aoIntensity: { value: 1.0 },
      aoFadeout: { value: 0.0 },
      shKernelRadius: { value: 0.15 },
      shIntensity: { value: 1.0 },
    },
    defines: {
      PERSPECTIVE_CAMERA: 1,
      KERNEL_SIZE: ShadowAndAoRenderMaterial.kernelSize,
      FLOAT_GBUFFER_RGB_NORMAL_ALPHA_DEPTH: 0,
    },
    vertexShader: glslShadowAndAoVertexShader,
    fragmentShader: glslShadowAndAoFragmentShader,
  };

  constructor(parameters?: any) {
    super({
      defines: Object.assign({
        ...ShadowAndAoRenderMaterial.shader.defines,
        KERNEL_SIZE:
          parameters?.sampleKernel?.length ??
          ShadowAndAoRenderMaterial.kernelSize,
        FLOAT_GBUFFER_RGB_NORMAL_ALPHA_DEPTH:
          parameters?.floatGBufferRgbNormalAlphaDepth ? 1 : 0,
      }),
      uniforms: UniformsUtils.clone(ShadowAndAoRenderMaterial.shader.uniforms),
      vertexShader: ShadowAndAoRenderMaterial.shader.vertexShader,
      fragmentShader: ShadowAndAoRenderMaterial.shader.fragmentShader,
      blending: NoBlending,
    });
    this.update(parameters);
  }

  public update(parameters?: any): ShadowAndAoRenderMaterial {
    this.updateDependencies(parameters);
    this.updateSettings(parameters);
    return this;
  }

  public updateDependencies(parameters?: any) {
    if (parameters?.shadowTexture !== undefined) {
      this.uniforms.tShadow.value = parameters?.shadowTexture;
    }
    if (parameters?.normalTexture !== undefined) {
      this.uniforms.tNormal.value = parameters?.normalTexture;
    }
    if (parameters?.depthTexture !== undefined) {
      this.uniforms.tDepth.value = parameters?.depthTexture;
    }
    if (parameters?.noiseTexture !== undefined) {
      this.uniforms.tNoise.value = parameters?.noiseTexture;
    }
    if (parameters?.width || parameters?.height) {
      const width = parameters?.width ?? this.uniforms.resolution.value.x;
      const height = parameters?.width ?? this.uniforms.resolution.value.y;
      this.uniforms.resolution.value.set(width, height);
    }
    if (parameters?.camera !== undefined) {
      const camera =
        (parameters?.camera as OrthographicCamera) ||
        (parameters?.camera as PerspectiveCamera);
      this.uniforms.cameraNear.value = camera.near;
      this.uniforms.cameraFar.value = camera.far;
      this.uniforms.cameraProjectionMatrix.value.copy(camera.projectionMatrix);
      this.uniforms.cameraInverseProjectionMatrix.value.copy(
        camera.projectionMatrixInverse
      );
    }
    if (parameters?.sampleKernel !== undefined) {
      this.uniforms.sampleKernel.value = parameters?.sampleKernel;
    }
  }

  public updateSettings(parameters?: any) {
    if (parameters?.aoKernelRadius !== undefined) {
      this.uniforms.aoKernelRadius.value = parameters?.aoKernelRadius;
    }
    if (parameters?.aoDepthBias !== undefined) {
      this.uniforms.aoDepthBias.value = parameters?.aoDepthBias;
    }
    if (parameters?.aoMaxDistance !== undefined) {
      this.uniforms.aoMaxDistance.value = parameters?.aoMaxDistance;
    }
    if (parameters?.aoMaxDepth !== undefined) {
      this.uniforms.aoMaxDepth.value = parameters?.aoMaxDepth;
    }
    if (parameters?.aoIntensity !== undefined) {
      this.uniforms.aoIntensity.value = parameters?.aoIntensity;
    }
    if (parameters?.aoFadeout !== undefined) {
      this.uniforms.aoFadeout.value = parameters?.aoFadeout;
    }
    if (parameters?.shadowRadius !== undefined) {
      this.uniforms.shKernelRadius.value = parameters?.shadowRadius;
    }
    if (parameters?.shadowIntensity !== undefined) {
      this.uniforms.shIntensity.value = parameters?.shadowIntensity;
    }
  }
}

const glslShadowAndAoBlurVertexShader = `varying vec2 vUv;
  void main() {
      vUv = uv;
      gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
  }`;

const glslShadowAndAoBlurFragmentShader = `uniform sampler2D tDiffuse;
#if FLOAT_GBUFFER_RGB_NORMAL_ALPHA_DEPTH == 1 
  uniform sampler2D tNormal;
#else
    uniform sampler2D tDepth;
#endif
  uniform vec2 resolution;
  uniform float cameraNear;
  uniform float cameraFar;
  uniform mat4 cameraProjectionMatrix;
  uniform mat4 cameraInverseProjectionMatrix;
  uniform float aoKernelRadius;
  uniform float shKernelRadius;
  
  varying vec2 vUv;
  
  float perspectiveDepthToViewZ(const in float invClipZ, const in float near, const in float far) {
      #if PERSPECTIVE_CAMERA == 1    
          float z_ndc = 2.0 * invClipZ - 1.0;
          float z_eye = 2.0 * near * far / ((far - near) * z_ndc - far - near);
          return z_eye;
      #else
          return (near * far) / ((far - near) * invClipZ - far);
      #endif
  }
  
  float viewZToOrthographicDepth(const in float viewZ, const in float near, const in float far) {
      // viewZ is negative!
      return (viewZ + near) / (near - far);
  }
  
  float getLinearDepth(const in vec2 screenPosition) {
      #if FLOAT_GBUFFER_RGB_NORMAL_ALPHA_DEPTH == 1
          float fragCoordZ = texture2D(tNormal, screenPosition).w;
      #else    
          float fragCoordZ = texture2D(tDepth, screenPosition).x;
      #endif
      #if PERSPECTIVE_CAMERA == 1
          float viewZ = perspectiveDepthToViewZ(fragCoordZ, cameraNear, cameraFar);
          return viewZToOrthographicDepth(viewZ, cameraNear, cameraFar);
      #else
          return fragCoordZ;
      #endif
  }
  
  void main() {
      vec2 texelSize = 1.0 / resolution ;
      float referenceDepth = getLinearDepth(vUv);
      vec2 result = vec2(0.0);
      vec2 samples = vec2(0.0);
      for (int i = - 2; i <= 2; i++) {
          for (int j = - 2; j <= 2; j++) {
              vec2 offset = vec2(float(i), float(j)) * texelSize;
              float sampleDepth = getLinearDepth(vUv + offset);
              float depthDelta =  sampleDepth - referenceDepth;
              float deltaDistance = depthDelta * (cameraFar - cameraNear);
              vec2 w = step(vec2(abs(deltaDistance), abs(deltaDistance)), vec2(aoKernelRadius, shKernelRadius));
              samples += w;
              result += texture2D(tDiffuse, vUv + offset).rg * w;
          }
      }
      gl_FragColor = vec4(result.rg / samples.rg, 0.0, 1.0);
  }`;

export class ShadowAndAoBlurMaterial extends ShaderMaterial {
  private static shader: any = {
    uniforms: {
      tDiffuse: { value: null as Texture | null },
      tDepth: { value: null as Texture | null },
      tNormal: { value: null as Texture | null },
      cameraNear: { value: 0.1 },
      cameraFar: { value: 1 },
      resolution: { value: new Vector2() },
      cameraProjectionMatrix: { value: new Matrix4() },
      cameraInverseProjectionMatrix: { value: new Matrix4() },
      aoKernelRadius: { value: 0.1 },
      shKernelRadius: { value: 0.1 },
    },
    defines: {
      FLOAT_GBUFFER_RGB_NORMAL_ALPHA_DEPTH: 0,
      PERSPECTIVE_CAMERA: 1,
      ACCURATE_VIEW_Z: 0,
    },
    vertexShader: glslShadowAndAoBlurVertexShader,
    fragmentShader: glslShadowAndAoBlurFragmentShader,
  };

  constructor(parameters?: any) {
    super({
      defines: Object.assign({
        ...ShadowAndAoBlurMaterial.shader.defines,
        FLOAT_GBUFFER_RGB_NORMAL_ALPHA_DEPTH:
          parameters?.floatGBufferRgbNormalAlphaDepth ? 1 : 0,
      }),
      uniforms: UniformsUtils.clone(ShadowAndAoBlurMaterial.shader.uniforms),
      vertexShader: ShadowAndAoBlurMaterial.shader.vertexShader,
      fragmentShader: ShadowAndAoBlurMaterial.shader.fragmentShader,
      blending: NoBlending,
    });
    this.update(parameters);
  }

  public update(parameters?: any): ShadowAndAoBlurMaterial {
    this.updateDependencies(parameters);
    this.updateSettings(parameters);
    return this;
  }

  public updateDependencies(parameters?: any) {
    if (parameters?.depthTexture !== undefined) {
      this.uniforms.tDepth.value = parameters?.depthTexture;
    }
    if (parameters?.normalTexture !== undefined) {
      this.uniforms.tNormal.value = parameters?.normalTexture;
    }
    if (parameters?.texture !== undefined) {
      this.uniforms.tDiffuse.value = parameters?.texture;
    }
    if (parameters?.width || parameters?.height) {
      const width = parameters?.width ?? this.uniforms.resolution.value.x;
      const height = parameters?.height ?? this.uniforms.resolution.value.y;
      this.uniforms.resolution.value.set(width, height);
    }
    if (parameters?.camera !== undefined) {
      const camera =
        (parameters?.camera as OrthographicCamera) ||
        (parameters?.camera as PerspectiveCamera);
      this.uniforms.cameraNear.value = camera.near;
      this.uniforms.cameraFar.value = camera.far;
      this.uniforms.cameraProjectionMatrix.value.copy(camera.projectionMatrix);
      this.uniforms.cameraInverseProjectionMatrix.value.copy(
        camera.projectionMatrixInverse
      );
    }
  }

  public updateSettings(parameters?: any) {
    if (parameters?.aoKernelRadius !== undefined) {
      this.uniforms.aoKernelRadius.value = parameters?.aoKernelRadius;
    }
    if (parameters?.shadowRadius !== undefined) {
      this.uniforms.shKernelRadius.value = parameters?.shadowRadius;
    }
  }
}
