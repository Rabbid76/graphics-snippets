import { RenderOverrideVisibility, RenderPass } from './renderPass';
import {
  Camera,
  DepthStencilFormat,
  DepthTexture,
  FloatType,
  LinearFilter,
  MagnificationTextureFilter,
  MeshNormalMaterial,
  NearestFilter,
  NoBlending,
  OrthographicCamera,
  PerspectiveCamera,
  Scene,
  ShaderMaterial,
  Texture,
  TextureFilter,
  UniformsUtils,
  UnsignedInt248Type,
  WebGLRenderer,
  WebGLRenderTarget,
} from 'three';

export interface GBufferTextures {
  isFloatGBufferWithRgbNormalAlphaDepth(): boolean;
  getGBufferTexture(): Texture;
  getDepthBufferTexture(): Texture;
  getTextureWithDepth(): Texture;
}

export interface GBufferParameters {
  [key: string]: any;
  depthNormalScale: number;
}

export class GBufferRenderTargets implements GBufferTextures {
  public parameters: GBufferParameters;
  public readonly floatGBufferRgbNormalAlphaDepth: boolean = false;
  private targetMinificationTextureFilter: TextureFilter;
  private targetMagnificationTextureFilter: MagnificationTextureFilter;
  private width: number;
  private height: number;
  private samples: number;
  private _normalRenderMaterial?: MeshNormalMaterial | NormalAndDepthRenderMaterial;
  private _depthNormalRenderTarget?: WebGLRenderTarget;
  private _antialiasedNormalRenderTarget?: WebGLRenderTarget;
  private renderPass: RenderPass;
  private renderOverrideVisibility: RenderOverrideVisibility;
  private shared: boolean;
  public needsUpdate: boolean = true;

  public isFloatGBufferWithRgbNormalAlphaDepth(): boolean {
    return this.floatGBufferRgbNormalAlphaDepth;
  }
  public getGBufferTexture(): Texture {
    return this.normalTexture;
  }
  public getDepthBufferTexture(): Texture {
    return this.depthTexture;
  }

  public getTextureWithDepth(): Texture {
    return this.floatGBufferRgbNormalAlphaDepth
      ? this.normalTexture
      : this.depthTexture;
  }

  private get normalTexture(): Texture {
    return this.depthNormalRenderTarget.texture;
  }
  private get depthTexture(): Texture {
    return this.depthNormalRenderTarget.depthTexture;
  }

  public get normalRenderMaterial(): MeshNormalMaterial | NormalAndDepthRenderMaterial {
    this._normalRenderMaterial =
      this._normalRenderMaterial ??
      (this.floatGBufferRgbNormalAlphaDepth 
        ? new NormalAndDepthRenderMaterial({ 
            blending: NoBlending,
            floatBufferType: true,
            linearDepth: false,
          })
        : new MeshNormalMaterial({ blending: NoBlending }));
    return this._normalRenderMaterial;
  }

  public get depthNormalRenderTarget(): WebGLRenderTarget {
    if (!this._depthNormalRenderTarget) {
      if (this.floatGBufferRgbNormalAlphaDepth) {
        this._depthNormalRenderTarget = new WebGLRenderTarget(
          this.width * this.parameters.depthNormalScale,
          this.height * this.parameters.depthNormalScale,
          {
            minFilter: this.targetMinificationTextureFilter,
            magFilter: this.targetMagnificationTextureFilter,
            type: FloatType, 
            samples: this.samples,
          }
        );
      } else {
        const depthTexture = new DepthTexture(
          this.width * this.parameters.depthNormalScale,
          this.height * this.parameters.depthNormalScale
        );
        depthTexture.format = DepthStencilFormat;
        depthTexture.type = UnsignedInt248Type;
        this._depthNormalRenderTarget = new WebGLRenderTarget(
          this.width * this.parameters.depthNormalScale,
          this.height * this.parameters.depthNormalScale,
          {
            minFilter: this.targetMinificationTextureFilter,
            magFilter: this.targetMagnificationTextureFilter,
            depthTexture,
          }
        );
      }
    }
    return this._depthNormalRenderTarget;
  }

  public get antialiasedNormalRenderTarget(): WebGLRenderTarget {
    this._antialiasedNormalRenderTarget =
      this._antialiasedNormalRenderTarget ??
      new WebGLRenderTarget(this.width, this.height, {
        magFilter: LinearFilter,
        minFilter: LinearFilter,
      });
    return this._antialiasedNormalRenderTarget;
  }

  constructor(parameters?: any) {
    this.floatGBufferRgbNormalAlphaDepth = parameters?.capabilities?.isWebGL2 ?? false;
    this.parameters = {
      depthNormalScale: parameters?.depthNormalScale ?? 1,
      normalBufferFxaa: parameters?.normalBufferFxaa ?? false,
    };
    this.targetMinificationTextureFilter =
      parameters?.textureMinificationFilter ?? NearestFilter;
    this.targetMagnificationTextureFilter =
      parameters?.textureMagnificationFilter ?? NearestFilter;
    this.width = parameters?.width ?? 1024;
    this.height = parameters?.height ?? 1024;
    this.samples = parameters?.samples ?? 0;
    this.shared = parameters?.shared ?? false;
    this.renderPass = parameters?.renderPass ?? new RenderPass();
    this.renderOverrideVisibility =
      parameters?.renderOverrideVisibility ??
      new RenderOverrideVisibility(true);
  }

  public dispose() {
    this._normalRenderMaterial?.dispose();
    this._depthNormalRenderTarget?.dispose();
    this._antialiasedNormalRenderTarget?.dispose();
  }

  public setSize(width: number, height: number) {
    this.width = width;
    this.height = height;
    this._depthNormalRenderTarget?.setSize(
      this.width * this.parameters.depthNormalScale,
      this.height * this.parameters.depthNormalScale
    );
    this._antialiasedNormalRenderTarget?.setSize(this.width, this.height);
  }

  public render(renderer: WebGLRenderer, scene: Scene, camera: Camera): void {
    if (this.shared && !this.needsUpdate) {
      return;
    }
    this.needsUpdate = false;
    const normalMaterial = this.normalRenderMaterial;
    if (normalMaterial instanceof NormalAndDepthRenderMaterial) {
      normalMaterial.update({camera});
    }
    this.renderOverrideVisibility.render(scene, () => {
      this.renderPass.renderWithOverrideMaterial(
        renderer,
        scene,
        camera,
        normalMaterial,
        this.depthNormalRenderTarget,
        0x7777ff,
        1.0
      );
    });
  }
}

const glslNormalAndDepthVertexShader = `varying vec3 vNormal;
#if LINEAR_DEPTH == 1
    varying float vZ;  
#endif

  void main() {
      vNormal = normalMatrix * normal;
      vec4 viewPosition = modelViewMatrix * vec4(position, 1.0);
      #if LINEAR_DEPTH == 1
          vZ = viewPosition.z;  
      #endif
      gl_Position = projectionMatrix * viewPosition;
  }`;

const glslNormalAndDepthFragmentShader = `varying vec3 vNormal;
#if LINEAR_DEPTH == 1
  varying float vZ;  
  uniform float cameraNear;
  uniform float cameraFar;
#endif

  void main() {
      #if FLOAT_BUFFER == 1
          vec3 normal = normalize(vNormal);
      #else
          vec3 normal = normalize(vNormal) * 0.5 + 0.5;
      #endif
      #if LINEAR_DEPTH == 1
          float depth = (-vZ - cameraNear) / (cameraFar - cameraNear);
      #else
          float depth = gl_FragCoord.z;
      #endif
      gl_FragColor = vec4(normal, depth);
  }`;

export class NormalAndDepthRenderMaterial extends ShaderMaterial {
  private static normalAndDepthShader: any = {
    uniforms: {
      cameraNear: { value: 0.1 },
      cameraFar: { value: 1 },
    },
    defines: {
      FLOAT_BUFFER: 0,
      LINEAR_DEPTH: 0,
    },
    vertexShader: glslNormalAndDepthVertexShader,
    fragmentShader: glslNormalAndDepthFragmentShader,
  };

  constructor(parameters?: any) {
    super({
      defines: Object.assign(
        {
          ...NormalAndDepthRenderMaterial.normalAndDepthShader.defines,
          FLOAT_BUFFER: parameters?.floatBufferType ? 1 : 0,
          LINEAR_DEPTH: parameters?.linearDepth ? 1 : 0,
        },
      ),
      uniforms: UniformsUtils.clone(
        NormalAndDepthRenderMaterial.normalAndDepthShader.uniforms
      ),
      vertexShader: NormalAndDepthRenderMaterial.normalAndDepthShader.vertexShader,
      fragmentShader:
      NormalAndDepthRenderMaterial.normalAndDepthShader.fragmentShader,
      blending: parameters?.blending ?? NoBlending,
    });
    this.update(parameters);
  }

  public update(parameters?: any): NormalAndDepthRenderMaterial {
    if (parameters?.camera !== undefined) {
      const camera =
        (parameters?.camera as OrthographicCamera) ||
        (parameters?.camera as PerspectiveCamera);
      this.uniforms.cameraNear.value = camera.near;
      this.uniforms.cameraFar.value = camera.far;
    }
    return this;
  }
}