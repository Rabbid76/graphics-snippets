import {
  ShadowAndAoRenderMaterial,
  ShadowAndAoBlurMaterial,
} from './shadow-and-ao-materials-and-shader';
import { DepthAndNormalTextures } from './depth-normal-render-targets';
import { RenderPass } from './render-util';
import {
  Camera,
  DataTexture,
  MathUtils,
  RGFormat,
  RepeatWrapping,
  Scene,
  ShaderMaterial,
  Texture,
  Vector3,
  WebGLRenderer,
  WebGLRenderTarget,
} from 'three';

export interface ShadowAndAoParameters {
  aoAndSoftShadowEnabled: boolean;
  aoAlwaysUpdate: boolean;
  aoKernelRadius: number;
  aoDepthBias: number;
  aoMaxDistance: number;
  aoMaxDepth: number;
  aoIntensity: number;
  aoFadeout: number;
  shadowRadius: number;
  shadowIntensity: number;
}

export class ShadowAndAoRenderTargets {
  public static uniformKernelDistribution: boolean = true;
  public static uniformNoiseDistribution: boolean = true;
  public shadowAndAoParameters: ShadowAndAoParameters;
  private width: number;
  private height: number;
  private samples: number;
  private aoTargetSamples: number = 0;
  private depthAndNormalTextures: DepthAndNormalTextures;
  private _noiseTexture?: DataTexture;
  private _aoKernel: Vector3[] = [];
  private _shKernel: Vector3[] = [];
  private _passRenderMaterial?: ShadowAndAoRenderMaterial;
  private _blurRenderMaterial?: ShadowAndAoBlurMaterial;
  private _passRenderTarget?: WebGLRenderTarget;
  private _blurRenderTarget?: WebGLRenderTarget;
  private renderPass: RenderPass;

  public get passRenderTarget(): WebGLRenderTarget {
    this._passRenderTarget =
      this._passRenderTarget ??
      new WebGLRenderTarget(this.width, this.height, {
        samples: this.aoTargetSamples,
        format: RGFormat,
      });
    return this._passRenderTarget;
  }

  public get blurRenderTarget(): WebGLRenderTarget {
    this._blurRenderTarget =
      this._blurRenderTarget ??
      new WebGLRenderTarget(this.width, this.height, {
        samples: this.aoTargetSamples,
        format: RGFormat,
      });
    return this._blurRenderTarget;
  }

  public get passRenderMaterial(): ShadowAndAoRenderMaterial {
    this._passRenderMaterial =
      this._passRenderMaterial ??
      new ShadowAndAoRenderMaterial({
        normalTexture: this.depthAndNormalTextures.getNormalTexture(),
        depthTexture: this.depthAndNormalTextures.getDepthTexture(),
        noiseTexture: this.noiseTexture,
        aoKernel: this.aoKernel,
        shKernel: this.shKernel,
      });
    return this._passRenderMaterial.update({
      width: this.width,
      height: this.height,
    });
  }

  public get blurRenderMaterial(): ShadowAndAoBlurMaterial {
    this._blurRenderMaterial =
      this._blurRenderMaterial ??
      new ShadowAndAoBlurMaterial({
        depthTexture: this.depthAndNormalTextures.getDepthTexture(),
      });
    return this._blurRenderMaterial?.update({
      width: this.width,
      height: this.height,
      texture: this.passRenderTarget.texture,
    });
  }

  private get noiseTexture(): DataTexture {
    this._noiseTexture =
      this._noiseTexture ?? this.generateRandomKernelRotations();
    return this._noiseTexture;
  }

  private get aoKernel(): Vector3[] {
    if (!this._aoKernel.length) {
      this._aoKernel = this.generateAoSampleKernel();
    }
    return this._aoKernel;
  }

  private get shKernel(): Vector3[] {
    if (!this._shKernel.length) {
      this._shKernel = this.generateShSampleKernel();
    }
    return this._shKernel;
  }

  constructor(
    depthAndNormalTextures: DepthAndNormalTextures,
    parameters?: any
  ) {
    this.shadowAndAoParameters = this.getShadowAndAoParameters(parameters);
    this.width = parameters?.width ?? 1024;
    this.height = parameters?.height ?? 1024;
    this.samples = parameters?.samples ?? 0;
    this.depthAndNormalTextures = depthAndNormalTextures;
    this.renderPass = parameters?.renderPass ?? new RenderPass();
  }

  private getShadowAndAoParameters(parameters?: any): ShadowAndAoParameters {
    return {
      aoAndSoftShadowEnabled: parameters?.enabled ?? true,
      aoAlwaysUpdate: parameters?.aoAlwaysUpdate ?? false,
      aoKernelRadius: parameters?.aoKernelRadius ?? 0.03,
      aoDepthBias: parameters?.aoDepthBias ?? 0.0005,
      aoMaxDistance: parameters?.aoMaxDistance ?? 0.1,
      aoMaxDepth: parameters?.aoMaxDepth ?? 0.99,
      aoIntensity: parameters?.aoIntensity ?? 1,
      aoFadeout: parameters?.aoFadeout ?? 1,
      shadowRadius: parameters?.shadowRadius ?? 0.15,
      shadowIntensity: parameters?.shadowIntensity ?? 0.25,
    };
  }

  public dispose() {
    this._noiseTexture?.dispose();
    this._passRenderMaterial?.dispose();
    this._blurRenderMaterial?.dispose();
    this._passRenderTarget?.dispose();
    this._blurRenderTarget?.dispose();
  }

  public setSize(width: number, height: number) {
    this.width = width;
    this.height = height;
    this._passRenderMaterial?.update({
      width: this.width,
      height: this.height,
    });
    this._blurRenderMaterial?.update({
      width: this.width,
      height: this.height,
    });
    this._passRenderTarget?.setSize(this.width, this.height);
    this._blurRenderTarget?.setSize(this.width, this.height);
  }

  public render(renderer: WebGLRenderer, scene: Scene, camera: Camera): void {
    this.renderPass.renderScreenSpace(
      renderer,
      this.updateSSAOMaterial(camera),
      this.passRenderTarget
    );
    this.renderPass.renderScreenSpace(
      renderer,
      this.updateBlurMaterial(camera),
      this.blurRenderTarget
    );
  }

  public updateSSAOMaterial(camera: Camera): ShaderMaterial {
    return this.passRenderMaterial.update({
      camera,
      ...this.shadowAndAoParameters,
    });
  }

  public updateBlurMaterial(camera: Camera): ShaderMaterial {
    return this.blurRenderMaterial.update({
      camera,
      aoKernelRadius: this.shadowAndAoParameters.aoKernelRadius,
      shadowRadius: this.shadowAndAoParameters.shadowRadius,
    });
  }

  public updateShadowTexture(shadowTexture: Texture) {
    if (this._passRenderMaterial) {
      this._passRenderMaterial.uniforms.tShadow.value = shadowTexture;
    }
  }

  public updateKernel() {
    if (this._noiseTexture) {
      this._noiseTexture = this.generateRandomKernelRotations();
    }
    if (this._aoKernel) {
      this._aoKernel = this.generateAoSampleKernel();
    }
    if (this._shKernel) {
      this._shKernel = this.generateShSampleKernel();
    }
    if (this._passRenderMaterial) {
      this._passRenderMaterial.uniforms.tNoise.value = this._noiseTexture;
      this._passRenderMaterial.uniforms.aoKernel.value = this._aoKernel;
      this._passRenderMaterial.uniforms.shKernel.value = this._shKernel;
    }
  }

  private generateAoSampleKernel(): Vector3[] {
    const kernelSize = ShadowAndAoRenderMaterial.kernelSize;
    return ShadowAndAoRenderTargets.uniformKernelDistribution
      ? this.uniformQuadraticSampleKernel(kernelSize)
      : this.randomQuadraticSampleKernel(kernelSize);
  }

  private generateShSampleKernel(): Vector3[] {
    const kernelSize = ShadowAndAoRenderMaterial.kernelSize;
    return ShadowAndAoRenderTargets.uniformKernelDistribution
      ? this.uniformQuadraticSampleKernel(kernelSize)
      : this.randomQuadraticSampleKernel(kernelSize);
  }

  private randomQuadraticSampleKernel(kernelSize: number): Vector3[] {
    const kernel: Vector3[] = [];
    for (let i = 0; i < kernelSize; i++) {
      const sample = new Vector3();
      sample.x = Math.random() * 2 - 1;
      sample.y = Math.random() * 2 - 1;
      sample.z = Math.random();
      sample.normalize();
      let scale = i / kernelSize;
      scale = MathUtils.lerp(0.1, 1, scale * scale);
      sample.multiplyScalar(scale);
      kernel.push(sample);
    }
    return kernel;
  }

  private uniformQuadraticSampleKernel(kernelSize: number): Vector3[] {
    const kernel: Vector3[] = [];
    const altitudeCount = Math.floor(kernelSize / 8);
    const altitudeStep = Math.PI / 2 / altitudeCount;
    for (let kernelIndex = 0; kernelIndex < kernelSize; kernelIndex++) {
      const altitudeIndex = kernelIndex % altitudeCount;
      const azimuthIndex = Math.floor(kernelIndex / altitudeCount);
      const azimuth =
        (Math.PI * 2 * azimuthIndex) / 8 +
        altitudeIndex * (Math.PI + (Math.PI * 2) / 11);
      //const altitude = altitudeStep * altitudeIndex;
      const altitude =
        altitudeStep * altitudeIndex +
        altitudeStep * (0.75 - 0.5 / azimuthIndex);
      //const altitude = Math.pow(azimuthIndex * 8 + altitudeIndex + 1, 2) * Math.PI / 2;
      const sample = new Vector3();
      sample.x = Math.cos(azimuth) * Math.cos(altitude);
      sample.y = Math.sin(azimuth) * Math.cos(altitude);
      sample.z = Math.sin(altitude);
      sample.normalize();
      let scale = kernelIndex / kernelSize;
      scale = MathUtils.lerp(0.1, 1, scale * scale);
      sample.multiplyScalar(scale);
      kernel.push(sample);
    }
    return kernel;
  }

  private generateRandomKernelRotations(): DataTexture {
    const width = 4;
    const height = 4;
    const noiseSize = width * height;
    const data = new Uint8Array(noiseSize * 4);
    for (let inx = 0; inx < noiseSize; ++inx) {
      const inx1 = inx % 2;
      const inx2 = inx / Math.floor(noiseSize / 2);
      const iAng = inx1 * Math.floor(noiseSize / 2) + inx2;
      const angle = (2 * Math.PI * iAng) / noiseSize;
      const randomVec = new Vector3(
        ShadowAndAoRenderTargets.uniformNoiseDistribution
          ? Math.cos(angle)
          : Math.random() * 2 - 1,
        ShadowAndAoRenderTargets.uniformNoiseDistribution
          ? Math.sin(angle)
          : Math.random() * 2 - 1,
        0
      ).normalize();
      data[inx * 4] = (randomVec.x * 0.5 + 0.5) * 255;
      data[inx * 4 + 1] = (randomVec.y * 0.5 + 0.5) * 255;
      data[inx * 4 + 2] = 127;
      data[inx * 4 + 3] = 0;
    }
    const noiseTexture = new DataTexture(data, width, height);
    noiseTexture.wrapS = RepeatWrapping;
    noiseTexture.wrapT = RepeatWrapping;
    noiseTexture.needsUpdate = true;
    return noiseTexture;
  }
}
