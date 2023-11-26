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
    Texture,
    UniformsUtils,
    Vector2,
    Vector3,
    Vector4,
    WebGLCapabilities,
    WebGLRenderer,
    WebGLRenderTarget,
} from 'three';
import { FXAAShader } from 'three/examples/jsm/shaders/FXAAShader';
import { Pass } from 'three/examples/jsm/postprocessing/Pass';
  
export enum AoAlgorithms {
  SSAO = 0,
  SAO = 1,
  HBAO = 2,
  N8AO = 3,
  GTAO = 4,
};

export interface AoParameters {
    [key: string]: any;
    aoEnabled: boolean;
    aoFxaa: boolean;
    aoAlwaysUpdate: boolean;
    aoIntensity: number;
    algorithm: AoAlgorithms;
    samples: number;
    clipRangeCheck: boolean;
    distanceFallOff: boolean;
    nvAlignedSamples: boolean;
    screenSpaceRadius: boolean;
    radius: number;
    distanceExponent: number;
    thickness: number;
    bias: number;
    scale: number;
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
      return this.parameters.aoFxaa
        ? this.aoRenderTargets.fxaaRenderTarget
        : this.aoRenderTargets.blurRenderTarget;
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
    private _noiseTexture?: DataTexture;
    private _sampleKernel: Vector3[] = [];
    private _passRenderMaterial?: AoRenderMaterial;
    private _blurRenderMaterial?: AoBlurMaterial;
    private _fxaaRenderMaterial?: ShaderMaterial;
    private _passRenderTarget?: WebGLRenderTarget;
    private _blurRenderTarget?: WebGLRenderTarget;
    private _fxaaRenderTarget?: WebGLRenderTarget;
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
  
    private get passRenderMaterial(): AoRenderMaterial {
      this._passRenderMaterial =
        this._passRenderMaterial ??
        new AoRenderMaterial({
          normalTexture: this.depthAndNormalTextures.getGBufferTexture(),
          depthTexture: this.depthAndNormalTextures.getTextureWithDepth(),
          noiseTexture: this.noiseTexture,
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
          floatGBufferRgbNormalAlphaDepth: 
            this.depthAndNormalTextures.isFloatGBufferWithRgbNormalAlphaDepth(),
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
  
    private get noiseTexture(): DataTexture {
      this._noiseTexture = this._noiseTexture ?? generateUniformKernelRotations();
      return this._noiseTexture;
    }
  
    private get sampleKernel(): Vector3[] {
      if (!this._sampleKernel.length) {
        this._sampleKernel = spiralQuadraticSampleKernel(
          AoRenderMaterial.kernelSize
        );
      }
      return this._sampleKernel;
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
        aoFxaa: true,
        aoAlwaysUpdate: true,
        aoIntensity: 1,
        algorithm: AoAlgorithms.SSAO,
        aoSamples: 16,
        clipRangeCheck: true,
        distanceFallOff: true,
        nvAlignedSamples: true,
        screenSpaceRadius: false,
        radius: 0.5,
        distanceExponent: 1,
        thickness: 1,
        bias: 0.0001,
        scale: 1,
        ...parameters,
      };
    }
  
    public dispose() {
      this._noiseTexture?.dispose();
      this._passRenderMaterial?.dispose();
      this._blurRenderMaterial?.dispose();
      this._fxaaRenderMaterial?.dispose();
      this._passRenderTarget?.dispose();
      this._blurRenderTarget?.dispose();
      this._fxaaRenderTarget?.dispose();
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
      this._fxaaRenderTarget?.setSize(this.width, this.height);
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
      if (this.aoParameters.aoFxaa) {
        this.renderPass.renderScreenSpace(
          renderer,
          this.fxaaRenderMaterial,
          this.fxaaRenderTarget
        );
      }
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
  
  const glslAoVertexShader = `varying vec2 vUv;
  void main() {
      vUv = uv;
      gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
  }`;
  
  const glslAoFragmentShader = `varying vec2 vUv;
  uniform sampler2D tNormal;
  uniform sampler2D tDepth;
  uniform sampler2D tNoise;
  uniform vec2 resolution;
  uniform float cameraNear;
  uniform float cameraFar;
  uniform mat4 cameraProjectionMatrix;
  uniform mat4 cameraProjectionMatrixInverse;		
  uniform float radius;
  uniform float distanceExponent;
  uniform float thickness;
  uniform float bias;
  uniform float scale;
  
  #include <common>
  #include <packing>

  #ifndef FRAGMENT_OUTPUT
  #define FRAGMENT_OUTPUT vec4(vec3(ao), 1.)
  #endif

  const vec4 sampleKernel[SAMPLES] = SAMPLE_VECTORS;

  vec3 getViewPosition(const in vec2 screenPosition, const in float depth) {
    vec4 clipSpacePosition = vec4(vec3(screenPosition, depth) * 2.0 - 1.0, 1.0);
    vec4 viewSpacePosition = cameraProjectionMatrixInverse * clipSpacePosition;
    return viewSpacePosition.xyz / viewSpacePosition.w;
  }

  float getDepth(const vec2 uv) {
    #if DEPTH_VALUE_SOURCE == 1    
      return textureLod(tDepth, uv.xy, 0.0).a;
    #else
      return textureLod(tDepth, uv.xy, 0.0).r;
    #endif
  }

  float fetchDepth(const ivec2 uv) {
    #if DEPTH_VALUE_SOURCE == 1    
      return texelFetch(tDepth, uv.xy, 0).a;
    #else
      return texelFetch(tDepth, uv.xy, 0).r;
    #endif
  }

  float getViewZ(const in float depth) {
    #if PERSPECTIVE_CAMERA == 1
      return perspectiveDepthToViewZ(depth, cameraNear, cameraFar);
    #else
      return orthographicDepthToViewZ(depth, cameraNear, cameraFar);
    #endif
  }

  vec3 computeNormalFromDepth(const vec2 uv) {
          vec2 size = vec2(textureSize(tDepth, 0));
          ivec2 p = ivec2(uv * size);
          float c0 = fetchDepth(p);
          float l2 = fetchDepth(p - ivec2(2, 0));
          float l1 = fetchDepth(p - ivec2(1, 0));
          float r1 = fetchDepth(p + ivec2(1, 0));
          float r2 = fetchDepth(p + ivec2(2, 0));
          float b2 = fetchDepth(p - ivec2(0, 2));
          float b1 = fetchDepth(p - ivec2(0, 1));
          float t1 = fetchDepth(p + ivec2(0, 1));
          float t2 = fetchDepth(p + ivec2(0, 2));
          float dl = abs((2.0 * l1 - l2) - c0);
          float dr = abs((2.0 * r1 - r2) - c0);
          float db = abs((2.0 * b1 - b2) - c0);
          float dt = abs((2.0 * t1 - t2) - c0);
          vec3 ce = getViewPosition(uv, c0).xyz;
          vec3 dpdx = (dl < dr) ?  ce - getViewPosition((uv - vec2(1.0 / size.x, 0.0)), l1).xyz
                                : -ce + getViewPosition((uv + vec2(1.0 / size.x, 0.0)), r1).xyz;
          vec3 dpdy = (db < dt) ?  ce - getViewPosition((uv - vec2(0.0, 1.0 / size.y)), b1).xyz
                                : -ce + getViewPosition((uv + vec2(0.0, 1.0 / size.y)), t1).xyz;
          return normalize(cross(dpdx, dpdy));
  }

  vec3 getViewNormal(const vec2 uv) {
    #if NORMAL_VECTOR_TYPE == 2
      return normalize(textureLod(tNormal, uv, 0.).rgb);
    #elif NORMAL_VECTOR_TYPE == 1
      return unpackRGBToNormal(textureLod(tNormal, uv, 0.).rgb);
    #else
      return computeNormalFromDepth(uv);
    #endif
  }

  vec3 getSceneUvAndDepth(vec3 sampleViewPos) {
    vec4 sampleClipPos = cameraProjectionMatrix * vec4(sampleViewPos, 1.);
    vec2 sampleUv = sampleClipPos.xy / sampleClipPos.w * 0.5 + 0.5;
    float sampleSceneDepth = getDepth(sampleUv);
    return vec3(sampleUv, sampleSceneDepth);
  }

  float sinusToPlane(vec3 pointOnPlane, vec3 planeNormal, vec3 point) {
    vec3 delta = point - pointOnPlane;
    float sinV = dot(planeNormal, normalize(delta));
    return sinV;
  }

  float getFallOff(float delta, float falloffDistance) {
    #if DISTANCE_FALL_OFF == 1
      float fallOff = smoothstep(0., 1., 1. - abs(delta) / falloffDistance);
    #else
      float fallOff = step(abs(delta), falloffDistance);
    #endif
    return fallOff;
  }
  
  void main() {
    float depth = getDepth(vUv.xy);
    if (depth == 1.0) {
      discard;
      return;
    }
    vec3 viewPos = getViewPosition(vUv, depth);
    vec3 viewNormal = getViewNormal(vUv);
    
    vec2 noiseResolution = vec2(textureSize(tNoise, 0));
    vec2 noiseUv = vUv * resolution / noiseResolution;
    vec4 noiseTexel = textureLod(tNoise, noiseUv, 0.0);
    vec3 randomVec = noiseTexel.xyz * 2.0 - 1.0;

    #if NV_ALIGNED_SAMPLES == 1
        vec3 tangent = normalize(randomVec - viewNormal * dot(randomVec, viewNormal));
          vec3 bitangent = cross(viewNormal, tangent);
          mat3 kernelMatrix = mat3(tangent, bitangent, viewNormal);
    #else
      vec3 tangent = normalize(vec3(randomVec.xy, 0.));
      vec3 bitangent = vec3(-tangent.y, tangent.x, 0.);
      mat3 kernelMatrix = mat3(tangent, bitangent, vec3(0., 0., 1.));
    #endif

    float radiusToUse = radius;
    float distanceFalloffToUse = thickness;
    #if SCREEN_SPACE_RADIUS == 1
        // TODO: needs to be verified - optimization
      //float clipW = cameraProjectionMatrix[2][3] * viewPos.z + cameraProjectionMatrix[3][3];
      //float nearW = cameraProjectionMatrix[2][3] * -cameraNear + cameraProjectionMatrix[3][3];
      //float referenceW = cameraProjectionMatrix[2][3] * -(cameraNear + cameraFar)/2. + cameraProjectionMatrix[3][3];
      //float radiusScale = clipW / nearW;
      vec3 radiusDistPos = getViewPosition(vUv + vec2(radius * float(SCREEN_SPACE_RADIUS_SCALE), 0.0) / resolution, depth);
      float radiusScale = distance(viewPos, radiusDistPos);
      radiusToUse *= radiusScale;
      distanceFalloffToUse *= radiusScale;
    #endif

  #if AO_ALGORITHM == 4
    const int DIRECTIONS = 2;
    const int STEPS = SAMPLES / 2;
  #elif AO_ALGORITHM == 2
    const int STEPS = 4;
    const int DIRECTIONS = (SAMPLES + STEPS - 1) / STEPS;
  #else
    const int DIRECTIONS = SAMPLES;
    const int STEPS = 1;
  #endif

    float ao = 0.0, totalWeight = 0.0;
    for (int i = 0; i < DIRECTIONS; ++i) {

    #if AO_ALGORITHM == 4	
      // GTAO		
      vec2 horizons = vec2(-1.0);
      float phi = noiseTexel.x * PI + float(i) * PI_HALF;
      //vec3 dir = normalize(vec3(randomVec.xy, 0.));
      vec3 dir = vec3(cos(phi), sin(phi), 0.); 
      for (int j = 0; j < STEPS; ++j) {
        vec3 sampleViewOffset = dir.xyz * radiusToUse * pow(0.5 + noiseTexel.w * 0.5, distanceExponent) * float(j + 1) / float(SAMPLES / 2);
        vec3 sampleViewPos = viewPos + sampleViewOffset;
    #elif AO_ALGORITHM == 2
      // HBAO
      float angle = float(i) / float(DIRECTIONS) * 2. * PI;
      vec4 sampleViewDir = vec4(cos(angle), sin(angle), 0., 1.);
      float sinH = sampleViewDir.z;
      for (int j = 0; j < STEPS; ++j) {
        sampleViewDir.w *= float(j + 1) / float(STEPS);
        sampleViewDir.xyz = normalize(kernelMatrix * sampleViewDir.xyz);
        vec3 sampleViewPos = viewPos + sampleViewDir.xyz * radiusToUse * pow(sampleViewDir.w, distanceExponent);
    #else
        vec4 sampleViewDir = sampleKernel[i];
        sampleViewDir.xyz = normalize(kernelMatrix * sampleViewDir.xyz);
        vec3 sampleViewPos = viewPos + sampleViewDir.xyz * radiusToUse * pow(sampleViewDir.w, distanceExponent);
    #endif	

        vec3 sampleSceneUvDepth = getSceneUvAndDepth(sampleViewPos);
        vec3 sampleSceneViewPos = getViewPosition(sampleSceneUvDepth.xy, sampleSceneUvDepth.z);
        float sceneSampleDist = abs(sampleSceneViewPos.z);
        float sampleDist = abs(sampleViewPos.z);

        // TODO verify fall off for GTAO
        float fallOff = getFallOff(sceneSampleDist - sampleDist, distanceFalloffToUse);
        
      #if AO_ALGORITHM == 4
        // GTAO
        float sinS = sinusToPlane(viewPos, viewNormal, sampleSceneViewPos);
        horizons.x = max(horizons.x, sinS - fallOff);
        sampleSceneUvDepth = getSceneUvAndDepth(viewPos - sampleViewOffset);
        sampleSceneViewPos = getViewPosition(sampleSceneUvDepth.xy, sampleSceneUvDepth.z);
        fallOff = getFallOff(abs(sampleSceneViewPos.z) - sampleDist, distanceFalloffToUse);
        sinS = sinusToPlane(viewPos, viewNormal, sampleSceneViewPos);
        horizons.y = max(horizons.y, sinS - fallOff);	
      #elif AO_ALGORITHM == 3
        // N8AO
        float weight = dot(viewNormal, sampleViewDir.xyz);
        float occlusion = weight * step(sceneSampleDist + bias, sampleDist) / scale;
      #elif AO_ALGORITHM == 2
        // HBAO
        float weight = 1. / float(STEPS);
        float sinS = sinusToPlane(viewPos, viewNormal, sampleSceneViewPos);
        float occlusion = max(0., (sinS - sinH) / scale);
        sinH = max(sinH, sinS);
      #elif AO_ALGORITHM == 1
        // SAO
        vec3 viewDelta = sampleSceneViewPos - viewPos;
        float minResolution = 0.; // ?
        float scaledViewDist = length( viewDelta ) * scale;
        float weight = 1.;
        float occlusion = max(0., (dot(viewNormal, viewDelta) - minResolution) / scaledViewDist - bias) / (1. + scaledViewDist * scaledViewDist );
      #else
        // SSAO
        float weight = scale;
        float occlusion = step(sceneSampleDist + bias, sampleDist);
      #endif

  #if AO_ALGORITHM != 4

        occlusion *= fallOff;
        //weight *= fallOff;

        vec2 diff = (vUv - sampleSceneUvDepth.xy) * resolution;
        occlusion *= step(1., dot(diff, diff));
      
      #if CLIP_RANGE_CHECK == 1
        vec2 clipRangeCheck = step(0., sampleSceneUvDepth.xy) * step(sampleSceneUvDepth.xy, vec2(1.));
        occlusion *= clipRangeCheck.x * clipRangeCheck.y;
        weight *= clipRangeCheck.x * clipRangeCheck.y;
      #endif
      
        totalWeight += weight;
        ao += occlusion;
    #if AO_ALGORITHM == 2
      }
    #endif
    }
    ao /= totalWeight + 1. - step(0., totalWeight);
    ao = clamp(1. - ao, 0., 1.);

  #else
      }
      // GTAO
      // project normal vector in view direction / sample direction plane
      vec3 viewdir	= normalize(-viewPos.xyz);
      vec3 bitangent_	= normalize(cross(dir, viewdir));
      vec3 tangent_	= cross(viewdir, bitangent_);
      vec3 nx			= viewNormal - bitangent_ * dot(viewNormal, bitangent_);

      // calculate gamma
      float nnx		= length(nx);
      float invnnx	= 1.0 / (nnx + 1e-6);			// to avoid division with zero
      float cosxi		= dot(nx, tangent_) * invnnx;	// xi = gamma + PI_HALF    ? same as dot(viewNormal, tangent_)
      float gamma		= acos(cosxi) - PI_HALF;
      float cosgamma	= dot(nx, viewdir) * invnnx;
      float singamma2	= -2.0 * cosxi;					// cos(x + PI_HALF) = -sin(x)	

      // clamp to normal hemisphere
      horizons.x = gamma + max(-horizons.x - gamma, -PI_HALF);
      horizons.y = gamma + min(horizons.y - gamma, PI_HALF);

      //float nnxScale = 0.25;
      float nnxScale = 0.5 * scale;
      ao += nnx * nnxScale * dot(vec2(1.0), horizons * singamma2 + cosgamma - cos(2.0 * horizons - gamma));
    }
    ao /= float(DIRECTIONS);

  #endif	
    if (depth > 0.99) ao = 0.0;

    gl_FragColor = FRAGMENT_OUTPUT;
  }`;


  const generateAoSampleKernelInitializer = (samples: number) => {
    const poissonDisk = generateAoSamples( samples );
    let glslCode = 'vec4[SAMPLES](';
    for ( let i = 0; i < samples; i ++ ) {
      const sample = poissonDisk[ i ];
      glslCode += `vec4(${sample.x}, ${sample.y}, ${sample.z}, ${sample.w})`;
      if ( i < samples - 1 ) {
        glslCode += ',';
      }
    }
    glslCode += ')';
    return glslCode;
  }
  
const generateAoSamples = (samples: number) => {
    // https://github.com/Rabbid76/research-sampling-hemisphere
    const kernel = [];
    for ( let kernelIndex = 0; kernelIndex < samples; kernelIndex ++ ) {
      const spiralAngle = kernelIndex * Math.PI * ( 3 - Math.sqrt( 5 ) );
      const z = Math.sqrt( 0.99 - ( kernelIndex / ( samples - 1 ) ) * 0.98 );
      //const radius = Math.sqrt( 1 - z * z );
      const radius = 1 - z;
      const x = Math.cos( spiralAngle ) * radius;
      const y = Math.sin( spiralAngle ) * radius;
      const scaleStep = 8;
      const scaleRange = Math.floor( samples / scaleStep );
      const scaleIndex =
        Math.floor( kernelIndex / scaleStep ) +
        ( kernelIndex % scaleStep ) * scaleRange;
      let scale = 1 - scaleIndex / samples;
      scale = 0.1 + 0.9 * scale;
      kernel.push( new Vector4( x, y, z, scale ) );
    }
    return kernel;
  } 
  
export class AoRenderMaterial extends ShaderMaterial {
    public static kernelSize: number = 32;
    private static shader: any = {
      uniforms: {
        tNormal: { value: null },
        tDepth: { value: null },
        tNoise: { value: null },
        resolution: { value: new Vector2() },
        cameraNear: { value: null },
        cameraFar: { value: null },
        cameraProjectionMatrix: { value: new Matrix4() },
        cameraProjectionMatrixInverse: { value: new Matrix4() },
        radius: { value: 1. },
        distanceExponent: { value: 1. },
        thickness: { value: 1. },
        bias: { value: 0.001 },
        scale: { value: 1. },
      },
      defines: {
        PERSPECTIVE_CAMERA: 1,
        SAMPLES: 16,
        SAMPLE_VECTORS: generateAoSampleKernelInitializer( 16 ),
        NORMAL_VECTOR_TYPE: 1,
        // TODO DEPTH_VALUE_SWIZZLE
        DEPTH_VALUE_SOURCE: 1,
        AO_ALGORITHM: AoAlgorithms.SSAO,
        CLIP_RANGE_CHECK: 1,
        DISTANCE_FALL_OFF: 1,
        NV_ALIGNED_SAMPLES: 1,
        SCREEN_SPACE_RADIUS: 0,
        SCREEN_SPACE_RADIUS_SCALE: 100.0,
      },
      vertexShader: glslAoVertexShader,
      fragmentShader: glslAoFragmentShader,
    };
  
    constructor(parameters?: any) {
      super({
        defines: Object.assign(
          {
            ...AoRenderMaterial.shader.defines,
            KERNEL_SIZE:
              parameters?.sampleKernel?.length ??
              AoRenderMaterial.kernelSize,
              FLOAT_GBUFFER_RGB_NORMAL_ALPHA_DEPTH: parameters?.floatGBufferRgbNormalAlphaDepth ? 1 : 0,
          }
        ),
        uniforms: UniformsUtils.clone(AoRenderMaterial.shader.uniforms),
        vertexShader: AoRenderMaterial.shader.vertexShader,
        fragmentShader: AoRenderMaterial.shader.fragmentShader,
        blending: NoBlending,
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
        this.defines.DEPTH_VALUE_SOURCE = parameters?.depthValueSource;
        this.needsUpdate = true;
      }
      if (parameters?.clipRangeCheck !== undefined) {
        this.defines.CLIP_RANGE_CHECK = parameters?.clipRangeCheck ? 1 : 0;
        this.needsUpdate = true;
      }
      if (parameters?.distanceFallOff !== undefined) {
        this.defines.DISTANCE_FALL_OFF = parameters?.distanceFallOff ? 1 : 0;
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
      if (parameters?.bias !== undefined) {
        this.uniforms.bias.value = parameters?.bias;
      }
      if (parameters?.scale !== undefined) {
        this.uniforms.scale.value = parameters?.scale;
      }
    }
  } 
  
const glslAoBlurVertexShader = `varying vec2 vUv;
    void main() {
        vUv = uv;
        gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
    }`;
  
const glslAoBlurFragmentShader = `uniform sampler2D tDiffuse;
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
        float result = 0.0;
        float samples = 0.0;
        for (int i = - 2; i <= 2; i++) {
            for (int j = - 2; j <= 2; j++) {
                vec2 offset = vec2(float(i), float(j)) * texelSize;
                float sampleDepth = getLinearDepth(vUv + offset);
                float depthDelta =  sampleDepth - referenceDepth;
                float deltaDistance = depthDelta * (cameraFar - cameraNear);
                float w = step(abs(deltaDistance), aoKernelRadius);
                samples += w;
                result += texture2D(tDiffuse, vUv + offset).r * w;
            }
        }
        gl_FragColor = vec4(result / samples, 0.0, 0.0, 1.0);
    }`;
  
export class AoBlurMaterial extends ShaderMaterial {
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
      },
      defines: {
        FLOAT_GBUFFER_RGB_NORMAL_ALPHA_DEPTH: 0,
        PERSPECTIVE_CAMERA: 1,
        ACCURATE_VIEW_Z: 0,
      },
      vertexShader: glslAoBlurVertexShader,
      fragmentShader: glslAoBlurFragmentShader,
    };
  
    constructor(parameters?: any) {
      super({
        defines: Object.assign(
          {
            ...AoBlurMaterial.shader.defines,
            FLOAT_GBUFFER_RGB_NORMAL_ALPHA_DEPTH: parameters?.floatGBufferRgbNormalAlphaDepth ? 1 : 0,
          }
        ),
        uniforms: UniformsUtils.clone(AoBlurMaterial.shader.uniforms),
        vertexShader: AoBlurMaterial.shader.vertexShader,
        fragmentShader: AoBlurMaterial.shader.fragmentShader,
        blending: NoBlending,
      });
      this.update(parameters);
    }
  
    public update(parameters?: any): AoBlurMaterial {
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