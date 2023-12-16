import {
    GBufferRenderTargets,
    GBufferTextures,
} from './gBufferRenderTarget';
import { CopyTransformMaterial } from './shaderUtility';
import {
    CameraUpdate
} from './threeUtility';
import {
    RenderOverrideVisibility,
    RenderPass,
} from './renderPass';
import {
    Camera,
    CustomBlending,
    DataTexture,
    LinearFilter,
    MathUtils,
    Matrix4,
    NearestFilter,
    NoBlending,
    OrthographicCamera,
    PerspectiveCamera,
    RepeatWrapping,
    RGFormat,
    Scene,
    ShaderMaterial,
    UniformsUtils,
    Vector3,
    WebGLCapabilities,
    WebGLRenderer,
    WebGLRenderTarget,
} from 'three';
import { Pass } from 'three/examples/jsm/postprocessing/Pass';
import { AOShader, AoAlgorithms, generateAoSampleKernelInitializer, generateMagicSquareNoise } from './aoShader';
import { PoissonDenoiseShader, generatePdSamplePointInitializer } from './pdShader';

export { AoAlgorithms } from './aoShader';

export interface AoParameters {
    [key: string]: any;
    aoEnabled: boolean;
    aoAlwaysUpdate: boolean;
    aoIntensity: number;
    algorithm: number;
    samples: number;
    clipRangeCheck: boolean;
    nvAlignedSamples: boolean;
    screenSpaceRadius: boolean;
    radius: number;
    distanceExponent: number;
    thickness: number;
    distanceFallOff: number
    bias: number;
    scale: number;
    pdSamples: number,
    pdLumaPhi: number,
    pdDepthPhi: number,
    pdNormalPhi: number,
    pdRadius: number,
}
  
export class SsaoRenderPass extends Pass {
    public parameters: AoParameters;
    public needsUpdate: boolean = true;
    public scene?: Scene;
    public camera?: Camera;
    private width: number;
    private height: number;
    private samples: number;
    private capabilities?: WebGLCapabilities;
    protected renderPass: RenderPass = new RenderPass();
    private _sharedGBufferRenderTarget?: GBufferRenderTargets;
    private _gBufferRenderTarget?: GBufferRenderTargets;
    public aoRenderTargets: AoRenderTargets;
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
          capabilities: this.capabilities,
        });
      return this._gBufferRenderTarget;
    }
  
    public get finalRenderTarget(): WebGLRenderTarget {
      return this.aoRenderTargets.blurRenderTarget;
    }
  
    constructor(
      width: number,
      height: number,
      samples: number,
      parameters?: any
    ) {
      super();
      if (parameters?.gBufferRenderTarget) {
        this._sharedGBufferRenderTarget = parameters?.gBufferRenderTarget;
      }
      this.camera = parameters?.camera;
      this.scene = parameters?.scene;
      this.width = width;
      this.height = height;
      this.samples = samples;
      this.capabilities = parameters?.capabilities;
      this.aoRenderTargets = new AoRenderTargets(
        this.gBufferRenderTarget,
        { ...parameters, width, height, samples, renderPass: this.renderPass }
      );
      this.parameters = this.aoRenderTargets.aoParameters;
      this._copyMaterial = new CopyTransformMaterial();
      this.updateCopyMaterial();
    }
  
    dispose() {
      this.gBufferRenderTarget?.dispose();
      this.aoRenderTargets.dispose();
      this._copyMaterial?.dispose();
    }
  
    public setSize(width: number, height: number) {
      this.width = width;
      this.height = height;
      this._gBufferRenderTarget?.setSize(width, height);
      this.aoRenderTargets.setSize(width, height);
      this._renderOverrideVisibility = new RenderOverrideVisibility();
      this.updateCopyMaterial();
      this.needsUpdate = true;
    }
  
    public updateParameters(parameters: any) {
      for (let propertyName in parameters) {
        if (this.parameters.hasOwnProperty(propertyName)) {
          this.parameters[propertyName] = parameters[propertyName];
          this.aoRenderTargets.parametersNeedsUpdate = true;
        }
      }
    }
  
    public updateBounds(aoScale: number) {
      this.aoRenderTargets.updateBounds(aoScale);
    }

    public render(renderer: WebGLRenderer, writeBuffer: WebGLRenderTarget): void {
        this.renderSSAO(renderer, writeBuffer);
    }
  
    public renderSSAO(
      renderer: WebGLRenderer,
      writeBuffer?: WebGLRenderTarget,
    ): void {
      if (!this.parameters.aoEnabled) {
        return;
      }
      if (this.camera && this.scene && this.evaluateIfUpdateIsNeeded(this.camera)) {
        this.renderSsaoPass(renderer, this.scene, this.camera);
      }
      this.renderToTarget(renderer, writeBuffer);
    }
  
    private evaluateIfUpdateIsNeeded(camera: Camera): boolean {
      (camera as OrthographicCamera | PerspectiveCamera).updateProjectionMatrix();
      const needsUpdate =
        this.parameters.aoAlwaysUpdate ||
        this.needsUpdate ||
        (camera != null && this.cameraUpdate.changed(camera));
      this.needsUpdate = false;
      return needsUpdate;
    }
  
    private updateCopyMaterial(intensityScale: number = 1) {
      const aoIntensity = this.parameters.aoIntensity * intensityScale;
      this._copyMaterial.update({
        texture: this.finalRenderTarget.texture,
        blending: CustomBlending,
        colorTransform: new Matrix4().set(
          aoIntensity, 0, 0, 1 - aoIntensity,
          0, 0, 0, 1,
          0, 0, 0, 1,
          0, 0, 0, 1
        ),
        multiplyChannels: 1,
      });
    }
  
    private renderSsaoPass(
      renderer: WebGLRenderer,
      scene: Scene,
      camera: Camera
    ): void {
      this.gBufferRenderTarget.render(renderer, scene, camera);
      this.aoRenderTargets.render(renderer, scene, camera);
    }
  
    protected renderToTarget(
      renderer: WebGLRenderer,
      writeBuffer?: WebGLRenderTarget,
    ): void {
      this.updateCopyMaterial(1);
      this.renderPass.renderScreenSpace(
        renderer,
        this._copyMaterial,
        renderer.getRenderTarget()
      );
    }
}
  
export class AoRenderTargets {
    public aoParameters: AoParameters;
    public parametersNeedsUpdate: boolean = true;
    private width: number;
    private height: number;
    private samples: number;
    private aoScale: number = 1;
    private aoTargetSamples: number = 0;
    private depthAndNormalTextures: GBufferTextures;
    private _aoNoiseTexture?: DataTexture;
    private _pdNoiseTexture?: DataTexture;
    private _passRenderMaterial?: AoRenderMaterial;
    private _blurRenderMaterial?: AoBlurMaterial;
    private _passRenderTarget?: WebGLRenderTarget;
    private _blurRenderTarget?: WebGLRenderTarget;
    private renderPass: RenderPass;
  
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
  
    private get passRenderMaterial(): AoRenderMaterial {
      this._passRenderMaterial =
        this._passRenderMaterial ??
        new AoRenderMaterial({
          normalTexture: this.depthAndNormalTextures.getGBufferTexture(),
          depthTexture: this.depthAndNormalTextures.getTextureWithDepth(),
          noiseTexture: this.aoNoiseTexture,
          normalVectorType: 
            this.depthAndNormalTextures.isFloatGBufferWithRgbNormalAlphaDepth() ? 2 : 1,
          depthValueSource: 
            this.depthAndNormalTextures.isFloatGBufferWithRgbNormalAlphaDepth() ? 1 : 0,
        });
      return this._passRenderMaterial;
    }
  
    private get blurRenderMaterial(): AoBlurMaterial {
      this._blurRenderMaterial =
        this._blurRenderMaterial ??
        new AoBlurMaterial({
          normalTexture: this.depthAndNormalTextures.getGBufferTexture(),
          depthTexture: this.depthAndNormalTextures.getDepthBufferTexture(),
          noiseTexture: this.pdNoiseTexture,
          floatGBufferRgbNormalAlphaDepth: 
            this.depthAndNormalTextures.isFloatGBufferWithRgbNormalAlphaDepth(),
        });
      return this._blurRenderMaterial;
    }
  
    private get aoNoiseTexture(): DataTexture {
      this._aoNoiseTexture = this._aoNoiseTexture ?? generateMagicSquareNoise();
      return this._aoNoiseTexture;
    }

    private get pdNoiseTexture(): DataTexture {
      this._pdNoiseTexture = this._pdNoiseTexture ?? generateUniformKernelRotations();
      return this._pdNoiseTexture;
    }
  
    constructor(
      depthAndNormalTextures: GBufferTextures,
      parameters?: any
    ) {
      this.aoParameters = this.getAoParameters(parameters);
      this.width = parameters?.width ?? 1024;
      this.height = parameters?.height ?? 1024;
      this.samples = parameters?.samples ?? 0;
      this.depthAndNormalTextures = depthAndNormalTextures;
      this.renderPass = parameters?.renderPass ?? new RenderPass();
    }
  
    private getAoParameters(parameters?: any): AoParameters {
      return {
        aoEnabled: parameters?.enabled ?? true,
        aoAlwaysUpdate: true,
        aoIntensity: 1,
        algorithm: AoAlgorithms.SSAO,
        aoSamples: 16,
        clipRangeCheck: true,
        nvAlignedSamples: true,
        screenSpaceRadius: false,
        radius: 0.25,
        distanceExponent: 2,
        thickness: 10,
        distanceFallOff: 1,
        bias: 0.001,
        scale: 1,
        pdLumaPhi: 10.,
				pdDepthPhi: 2.,
				pdNormalPhi: 3.,
				pdRadius: 4.,
				pdSamples: 16,
        ...parameters,
      };
    }
  
    public dispose() {
      this._aoNoiseTexture?.dispose();
      this._pdNoiseTexture?.dispose();
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
  
    public updateBounds(aoScale: number) {
      this.aoScale = aoScale;
      this.parametersNeedsUpdate = true;
    }
  
    public render(renderer: WebGLRenderer, scene: Scene, camera: Camera): void {
      this.renderPass.renderScreenSpace(
        renderer,
        this.updateSSAOMaterial(camera),
        this.passRenderTarget,
        0xffffff,
        1.0,
      );
      this.renderPass.renderScreenSpace(
        renderer,
        this.updateBlurMaterial(camera),
        this.blurRenderTarget
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
        passRenderMaterial.updateDefines({
          ...this.aoParameters,
        });
        passRenderMaterial.updateSettings({
          ...this.aoParameters,
          radius:
            this.aoParameters.radius * this.aoScale,
          thickness:
            this.aoParameters.thickness * this.aoScale,
        });
        this.blurRenderMaterial.updateDefines({
          ...this.aoParameters,
        });
        this.blurRenderMaterial.updateSettings({
          ...this.aoParameters,
        });
      }
      return passRenderMaterial;
    }
  
    public updateBlurMaterial(camera: Camera): ShaderMaterial {
      const blurRenderMaterial = this.blurRenderMaterial;
      blurRenderMaterial.updateDependencies({
        width: this.width,
        height: this.height,
        texture: this.passRenderTarget.texture,
        camera,
      });
      blurRenderMaterial.updateDependencies({
        camera,
        aoKernelRadius:
          this.aoParameters.aoKernelRadius * this.aoScale,
        aoMaxDistance:
          this.aoParameters.aoMaxDistance * this.aoScale,
      });
      return blurRenderMaterial;
    }
  }
  
export class AoRenderMaterial extends ShaderMaterial {
    
    constructor(parameters?: any) {
      super({
        defines: Object.assign({...AOShader.defines}),
        uniforms: UniformsUtils.clone(AOShader.uniforms),
        vertexShader: AOShader.vertexShader,
        fragmentShader: AOShader.fragmentShader,
        blending: NoBlending,
        depthTest: false,
			  depthWrite: false,
      });
      this.update(parameters);
    }
  
    public update(parameters?: any): AoRenderMaterial {
      this.updateDefines(parameters);
      this.updateDependencies(parameters);
      this.updateSettings(parameters);
      return this;
    }

    public updateDefines(parameters?: any) {
      if (parameters?.algorithm !== undefined) {
        this.defines.AO_ALGORITHM = parameters?.algorithm;
        this.needsUpdate = true;
      }
      if (parameters?.normalVectorType !== undefined) {
        this.defines.NORMAL_VECTOR_TYPE = parameters?.normalVectorType;
        this.needsUpdate = true;
      }
      if (parameters?.depthValueSource !== undefined) {
        this.defines.DEPTH_SWIZZLING = parameters?.depthValueSource === 1 ? 'w' : 'x';
        this.needsUpdate = true;
      }
      if (parameters?.nvAlignedSamples !== undefined) {
        this.defines.NV_ALIGNED_SAMPLES = parameters?.nvAlignedSamples ? 1 : 0;
        this.needsUpdate = true;
      }
      if (parameters?.screenSpaceRadius !== undefined) {
        this.defines.SCREEN_SPACE_RADIUS = parameters?.screenSpaceRadius ? 1 : 0;
        this.needsUpdate = true;
      }
      if (parameters?.screenSpaceRadiusScale !== undefined) {
        this.defines.SCREEN_SPACE_RADIUS_SCALE = parameters?.screenSpaceRadiusScale;
        this.needsUpdate = true;
      }
      if (parameters?.aoSamples !== undefined) {
        this.defines.SAMPLES = parameters?.aoSamples;
        this.defines.SAMPLE_VECTORS = generateAoSampleKernelInitializer( parameters?.aoSamples );
        this.needsUpdate = true;
      }
    }
  
    public updateDependencies(parameters?: any) {
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
        this.uniforms.cameraProjectionMatrixInverse.value.copy(
          camera.projectionMatrixInverse
        );
        this.uniforms.cameraWorldMatrix.value.copy(camera.matrixWorld);
      }
      if (parameters?.sampleKernel !== undefined) {
        this.uniforms.sampleKernel.value = parameters?.sampleKernel;
      }
    }
  
    public updateSettings(parameters?: any) {
      if (parameters?.radius !== undefined) {
        this.uniforms.radius.value = parameters?.radius;
      }
      if (parameters?.distanceExponent !== undefined) {
        this.uniforms.distanceExponent.value = parameters?.distanceExponent;
      }
      if (parameters?.thickness !== undefined) {
        this.uniforms.thickness.value = parameters?.thickness;
      }
      if (parameters?.distanceFallOff !== undefined) {
        this.uniforms.distanceFallOff.value = parameters?.distanceFallOff;
      }
      if (parameters?.bias !== undefined) {
        this.uniforms.bias.value = parameters?.bias;
      }
      if (parameters?.scale !== undefined) {
        this.uniforms.scale.value = parameters?.scale;
      }
    }
  } 
  

  export class AoBlurMaterial extends ShaderMaterial {
    constructor(parameters?: any) {
      super({
        defines: Object.assign({...PoissonDenoiseShader.defines}),
        uniforms: UniformsUtils.clone(PoissonDenoiseShader.uniforms),
        vertexShader: PoissonDenoiseShader.vertexShader,
        fragmentShader: PoissonDenoiseShader.fragmentShader,
        blending: NoBlending,
      });
      this.update(parameters);
    }
  
    public update(parameters?: any): AoBlurMaterial {
      this.updateDefines(parameters);
      this.updateDependencies(parameters);
      this.updateSettings(parameters);
      return this;
    }

    public updateDefines(parameters?: any) {
      if (parameters?.pdSamples !== undefined) {
        this.defines.SAMPLES = parameters?.pdSamples;
        this.defines.SAMPLE_VECTORS = generatePdSamplePointInitializer( parameters?.pdSamples, 2, 1 );
        this.needsUpdate = true;
      }
    }
  
    public updateDependencies(parameters?: any) {
      if (parameters?.texture !== undefined) {
        this.uniforms.tDiffuse.value = parameters?.texture;
      }
      if (parameters?.depthTexture !== undefined) {
        this.uniforms.tDepth.value = parameters?.depthTexture;
      }
      if (parameters?.normalTexture !== undefined) {
        this.uniforms.tNormal.value = parameters?.normalTexture;
      }
      if (parameters?.noiseTexture !== undefined) {
        this.uniforms.tNoise.value = parameters?.noiseTexture;
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
        this.uniforms.cameraProjectionMatrixInverse.value.copy(
          camera.projectionMatrixInverse
        );
      }
    }
  
    public updateSettings(parameters?: any) {
      if (parameters?.pdLumaPhi !== undefined) {
        this.uniforms.lumaPhi.value = parameters?.pdLumaPhi;
      }
      if (parameters?.pdDepthPhi !== undefined) {
        this.uniforms.depthPhi.value = parameters?.pdDepthPhi;
      }
      if (parameters?.pdNormalPhi !== undefined) {
        this.uniforms.normalPhi.value = parameters?.pdNormalPhi;
      }
      if (parameters?.pdRadius !== undefined) {
        this.uniforms.radius.value = parameters?.pdRadius;
      }
    }
}
  
export const spiralQuadraticSampleKernel = (kernelSize: number): Vector3[] => {
    const kernel: Vector3[] = [];
    for (let kernelIndex = 0; kernelIndex < kernelSize; kernelIndex++) {
      const spiralAngle = kernelIndex * Math.PI * (3 - Math.sqrt(5));
      const z = 0.99 - (kernelIndex / (kernelSize - 1)) * 0.8;
      const radius = Math.sqrt(1 - z * z);
      const x = Math.cos(spiralAngle) * radius;
      const y = Math.sin(spiralAngle) * radius;
      const scaleStep = 8;
      const scaleRange = Math.floor(kernelSize / scaleStep);
      const scaleIndex =
        Math.floor(kernelIndex / scaleStep) +
        (kernelIndex % scaleStep) * scaleRange;
      let scale = 1 - scaleIndex / kernelSize;
      scale = MathUtils.lerp(0.1, 1, scale * scale);
      kernel.push(new Vector3(x * scale, y * scale, z * scale));
    }
    return kernel;
};
  
export const generateUniformKernelRotations = (): DataTexture => {
    const width = 4;
    const height = 4;
    const noiseSize = width * height;
    const data = new Uint8Array(noiseSize * 4);
    for (let inx = 0; inx < noiseSize; ++inx) {
      const iAng = Math.floor(inx / 2) + (inx % 2) * 8;
      const angle = (2 * Math.PI * iAng) / noiseSize;
      const randomVec = new Vector3(
        Math.cos(angle),
        Math.sin(angle),
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
};