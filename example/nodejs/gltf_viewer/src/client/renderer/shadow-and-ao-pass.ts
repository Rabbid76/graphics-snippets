import {
  DepthNormalRenderTarget,
  DepthAndNormalTextures,
} from './depth-normal-render-targets';
import { CopyTransformMaterial } from './shader-utility';
import {
  CameraUpdate,
  RenderOverrideVisibility,
  RenderPass,
} from './render-utility';
import {
  Camera,
  CustomBlending,
  DataTexture,
  MathUtils,
  Matrix4,
  NearestFilter,
  NoBlending,
  OrthographicCamera,
  PerspectiveCamera,
  RGFormat,
  RepeatWrapping,
  Scene,
  ShaderMaterial,
  Texture,
  UniformsUtils,
  Vector2,
  Vector3,
  WebGLRenderer,
  WebGLRenderTarget,
} from 'three';

export interface ShadowAndAoParameters {
  [key: string]: any;
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
  public parameters: ShadowAndAoParameters;
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
    this.parameters = this.shadowAndAoRenderTargets.shadowAndAoParameters;
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

  public updateParameters(parameters: any) {
    for (let propertyName in parameters) {
      if (this.parameters.hasOwnProperty(propertyName)) {
        this.parameters[propertyName] = parameters[propertyName];
      }
    }
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
    if (!this.parameters.aoAndSoftShadowEnabled) {
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
      this.parameters.aoAlwaysUpdate ||
      this.needsUpdate ||
      (camera != null && this.cameraUpdate.changed(camera));
    this.needsUpdate = false;
    return needsUpdate;
  }

  private updateCopyMaterial(intensityScale: number = 1) {
    const aoIntensity = this.parameters.aoIntensity * intensityScale;
    const shIntensity = this.parameters.shadowIntensity * intensityScale;
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
      aoIntensity: parameters?.aoIntensity ?? 0.5,
      aoFadeout: parameters?.aoFadeout ?? 1,
      shadowRadius: parameters?.shadowRadius ?? 0.1,
      shadowIntensity: parameters?.shadowIntensity ?? 0.3,
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

const glslShadowAndAoVertexShader = `varying vec2 vUv;
  void main() {
      vUv = uv;
      gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
  }`;

const glslShadowAndAoFragmentShader = `uniform sampler2D tShadow;
  uniform sampler2D tNormal;
  uniform sampler2D tDepth;
  uniform sampler2D tNoise;
  uniform vec2 resolution;
  uniform float cameraNear;
  uniform float cameraFar;
  uniform mat4 cameraProjectionMatrix;
  uniform mat4 cameraInverseProjectionMatrix;
  uniform vec3 aoKernel[KERNEL_SIZE];
  uniform float aoKernelRadius;
  uniform float aoDepthBias; // avoid artifacts caused by neighbour fragments with minimal depth difference
  uniform float aoMaxDistance; // avoid the influence of fragments which are too far away
  uniform float aoMaxDepth;
  uniform float aoIntensity;
  uniform float aoFadeout;
  uniform vec3 shKernel[KERNEL_SIZE];
  uniform float shKernelRadius;
  
  varying vec2 vUv;
  
  #include <packing>
  
  float getDepth(const in vec2 screenPosition) {
      return texture2D(tDepth, screenPosition).x;
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
      return unpackRGBToNormal(texture2D(tNormal, screenPosition).xyz);
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
          if (aoIntensity > 0.01) {
              vec3 aoSampleVector = kernelMatrix * aoKernel[i]; 
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
          
          vec3 shSampleVector = kernelMatrix * shKernel[i]; // reorient sample vector in view space
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
  
      aoOcclusion = clamp(aoOcclusion / float(KERNEL_SIZE) * (1.0 + aoFadeout), 0.0, 1.0);
      shOcclusion = clamp(shOcclusion / (shSamples + 1.0), 0.0, 1.0);
      gl_FragColor = vec4(clamp(1.0 - aoOcclusion, 0.0, 1.0), shOcclusion, 0.0, 1.0);
  }`;

export class ShadowAndAoRenderMaterial extends ShaderMaterial {
  public static kernelSize: number = 64;
  private static shader: any = {
    uniforms: {
      tShadow: { value: null as Texture | null },
      tNormal: { value: null as Texture | null },
      tDepth: { value: null as Texture | null },
      tNoise: { value: null as Texture | null },
      cameraNear: { value: 0.1 },
      cameraFar: { value: 1 },
      resolution: { value: new Vector2() },
      cameraProjectionMatrix: { value: new Matrix4() },
      cameraInverseProjectionMatrix: { value: new Matrix4() },
      aoKernel: { value: null },
      aoKernelRadius: { value: 0.1 },
      aoDepthBias: { value: 0.002 },
      aoMaxDistance: { value: 0.05 },
      aoMaxDepth: { value: 0.99 },
      aoIntensity: { value: 1.0 },
      aoFadeout: { value: 0.0 },
      shKernel: { value: null },
      shKernelRadius: { value: 0.15 },
    },
    defines: {
      PERSPECTIVE_CAMERA: 1,
      KERNEL_SIZE: ShadowAndAoRenderMaterial.kernelSize,
    },
    vertexShader: glslShadowAndAoVertexShader,
    fragmentShader: glslShadowAndAoFragmentShader,
  };

  constructor(parameters?: any) {
    super({
      defines: Object.assign({}, ShadowAndAoRenderMaterial.shader.defines),
      uniforms: UniformsUtils.clone(ShadowAndAoRenderMaterial.shader.uniforms),
      vertexShader: ShadowAndAoRenderMaterial.shader.vertexShader,
      fragmentShader: ShadowAndAoRenderMaterial.shader.fragmentShader,
      blending: NoBlending,
    });
    this.update(parameters);
  }

  public update(parameters?: any): ShadowAndAoRenderMaterial {
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
    if (parameters?.aoKernel !== undefined) {
      this.uniforms.aoKernel.value = parameters?.aoKernel;
    }
    if (parameters?.shKernel !== undefined) {
      this.uniforms.shKernel.value = parameters?.shKernel;
    }
    this.updateSettings(parameters);
    return this;
  }

  private updateSettings(parameters?: any) {
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
  }
}

const glslShadowAndAoBlurVertexShader = `varying vec2 vUv;
  void main() {
      vUv = uv;
      gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
  }`;

const glslShadowAndAoBlurFragmentShader = `uniform sampler2D tDiffuse;
  uniform sampler2D tDepth;
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
      float fragCoordZ = texture2D(tDepth, screenPosition).x;
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
              float depthDelta = sampleDepth - referenceDepth;
              float deltaDistance = depthDelta * (cameraFar - cameraNear);
              vec2 w = step(abs(deltaDistance), vec2(aoKernelRadius, shKernelRadius));
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
      cameraNear: { value: 0.1 },
      cameraFar: { value: 1 },
      resolution: { value: new Vector2() },
      cameraProjectionMatrix: { value: new Matrix4() },
      cameraInverseProjectionMatrix: { value: new Matrix4() },
      aoKernelRadius: { value: 0.1 },
      shKernelRadius: { value: 0.1 },
    },
    defines: {
      PERSPECTIVE_CAMERA: 1,
      ACCURATE_VIEW_Z: 0,
    },
    vertexShader: glslShadowAndAoBlurVertexShader,
    fragmentShader: glslShadowAndAoBlurFragmentShader,
  };

  constructor(parameters?: any) {
    super({
      defines: Object.assign({}, ShadowAndAoBlurMaterial.shader.defines),
      uniforms: UniformsUtils.clone(ShadowAndAoBlurMaterial.shader.uniforms),
      vertexShader: ShadowAndAoBlurMaterial.shader.vertexShader,
      fragmentShader: ShadowAndAoBlurMaterial.shader.fragmentShader,
      blending: NoBlending,
    });
    this.update(parameters);
  }

  public update(parameters?: any): ShadowAndAoBlurMaterial {
    if (parameters?.depthTexture !== undefined) {
      this.uniforms.tDepth.value = parameters?.depthTexture;
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
    this.updateSettings(parameters);
    return this;
  }

  private updateSettings(parameters?: any) {
    if (parameters?.aoKernelRadius !== undefined) {
      this.uniforms.aoKernelRadius.value = parameters?.aoKernelRadius;
    }
    if (parameters?.shadowRadius !== undefined) {
      this.uniforms.shKernelRadius.value = parameters?.shadowRadius;
    }
  }
}
