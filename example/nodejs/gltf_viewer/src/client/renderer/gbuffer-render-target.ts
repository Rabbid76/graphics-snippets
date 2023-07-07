import { RenderOverrideVisibility, RenderPass } from './render-utility';
import {
  ALPHA_RGBA,
  ALPHA_TRANSFORM,
  CopyTransformMaterial,
  DEFAULT_UV_TRANSFORM,
  NormalAndDepthRenderMaterial,
} from './shader-utility';
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
  Scene,
  ShaderMaterial,
  Texture,
  TextureFilter,
  UnsignedInt248Type,
  WebGLRenderer,
  WebGLRenderTarget,
} from 'three';

export interface GBufferTextures {
  get isFloatGBufferWithRgbNormalAlphaDepth(): boolean;
  get gBufferTexture(): Texture;
  get depthBufferTexture(): Texture;
  get textureWithDepthValue(): Texture;
}

export interface GBufferParameters {
  [key: string]: any;
  depthNormalScale: number;
}

export class GBufferRenderTargets implements GBufferTextures {
  public parameters: GBufferParameters;
  public readonly floatGBufferRgbNormalAlphaDepth: boolean = false;
  public copyToSeparateDepthBuffer: boolean = false;
  private targetMinificationTextureFilter: TextureFilter;
  private targetMagnificationTextureFilter: MagnificationTextureFilter;
  private width: number;
  private height: number;
  private samples: number;
  private _normalRenderMaterial?:
    | MeshNormalMaterial
    | NormalAndDepthRenderMaterial;
  private _depthNormalRenderTarget?: WebGLRenderTarget;
  private _separateDeptRenderTarget?: WebGLRenderTarget;
  private _antialiasedNormalRenderTarget?: WebGLRenderTarget;
  private renderPass: RenderPass;
  private renderOverrideVisibility: RenderOverrideVisibility;
  private copyMaterial?: CopyTransformMaterial;
  private shared: boolean;
  public needsUpdate: boolean = true;

  public get isFloatGBufferWithRgbNormalAlphaDepth(): boolean {
    return this.floatGBufferRgbNormalAlphaDepth;
  }
  public get gBufferTexture(): Texture {
    return this.depthNormalRenderTarget.texture;
  }
  public get depthBufferTexture(): Texture {
    return this.floatGBufferRgbNormalAlphaDepth
      ? this.separateDeptRenderTarget.texture
      : this.depthNormalRenderTarget.depthTexture;
  }

  public get textureWithDepthValue(): Texture {
    return this.floatGBufferRgbNormalAlphaDepth
      ? this.depthNormalRenderTarget.texture
      : this.depthNormalRenderTarget.depthTexture;
  }

  public get normalRenderMaterial():
    | MeshNormalMaterial
    | NormalAndDepthRenderMaterial {
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

  public get separateDeptRenderTarget(): WebGLRenderTarget {
    if (!this._separateDeptRenderTarget) {
      this._separateDeptRenderTarget = new WebGLRenderTarget(
        this.width * this.parameters.depthNormalScale,
        this.height * this.parameters.depthNormalScale,
        {
          minFilter: this.targetMinificationTextureFilter,
          magFilter: this.targetMagnificationTextureFilter,
          //format: RedFormat,
          type: FloatType,
          samples: 0,
        }
      );
    }
    return this._separateDeptRenderTarget;
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
    this.floatGBufferRgbNormalAlphaDepth =
      parameters?.capabilities?.isWebGL2 ?? false;
    this.parameters = {
      depthNormalScale: parameters?.depthNormalScale ?? 1,
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
      normalMaterial.update({ camera });
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
    if (
      this.floatGBufferRgbNormalAlphaDepth &&
      this.copyToSeparateDepthBuffer
    ) {
      this.copyDepthToSeparateDepthTexture(
        renderer,
        this.depthNormalRenderTarget
      );
    }
  }

  protected getCopyMaterial(parameters?: any): ShaderMaterial {
    this.copyMaterial ??= new CopyTransformMaterial();
    return this.copyMaterial.update(parameters);
  }

  private copyDepthToSeparateDepthTexture(
    renderer: WebGLRenderer,
    source: WebGLRenderTarget
  ) {
    this.renderPass.renderScreenSpace(
      renderer,
      this.getCopyMaterial({
        texture: source.texture,
        blending: NoBlending,
        colorTransform: ALPHA_TRANSFORM,
        colorBase: ALPHA_RGBA,
        multiplyChannels: 0,
        uvTransform: DEFAULT_UV_TRANSFORM,
      }),
      this.separateDeptRenderTarget
    );
  }
}
