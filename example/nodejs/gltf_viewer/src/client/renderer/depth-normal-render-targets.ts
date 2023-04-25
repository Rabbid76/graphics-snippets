import { RenderOverrideVisibility, RenderPass } from './render-utility';
import {
  Camera,
  DepthStencilFormat,
  DepthTexture,
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
import { FXAAShader } from 'three/examples/jsm/shaders/FXAAShader';

export interface DepthAndNormalTextures {
  getNormalTexture(): Texture;
  getDepthTexture(): Texture;
}

export interface DepthNormalBufferParameters {
  [key: string]: any;
  depthNormalScale: number;
  normalBufferFxaa: boolean;
}

export class DepthNormalRenderTarget {
  public parameters: DepthNormalBufferParameters;
  private targetMinificationTextureFilter: TextureFilter;
  private targetMagnificationTextureFilter: MagnificationTextureFilter;
  private width: number;
  private height: number;
  private samples: number;
  private _normalRenderMaterial?: MeshNormalMaterial;
  private _depthNormalRenderTarget?: WebGLRenderTarget;
  private _antialiasedNormalRenderTarget?: WebGLRenderTarget;
  private _fxaaRenderMaterial?: ShaderMaterial;
  private renderPass: RenderPass;
  private renderOverrideVisibility: RenderOverrideVisibility;
  private shared: boolean;
  public needsUpdate: boolean = true;

  public getNormalTexture(): Texture {
    return this.normalTexture;
  }
  public getDepthTexture(): Texture {
    return this.depthTexture;
  }

  public get normalTexture(): Texture {
    return this.parameters.normalBufferFxaa
      ? this.antialiasedNormalRenderTarget.texture
      : this.depthNormalRenderTarget.texture;
  }
  public get depthTexture(): Texture {
    return this.depthNormalRenderTarget.depthTexture;
  }

  public get normalRenderMaterial(): MeshNormalMaterial {
    this._normalRenderMaterial =
      this._normalRenderMaterial ??
      new MeshNormalMaterial({ blending: NoBlending });
    return this._normalRenderMaterial;
  }

  public get depthNormalRenderTarget(): WebGLRenderTarget {
    if (!this._depthNormalRenderTarget) {
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

  public get fxaaRenderMaterial(): ShaderMaterial {
    this._fxaaRenderMaterial =
      this._fxaaRenderMaterial ?? new ShaderMaterial(FXAAShader);
    this._fxaaRenderMaterial.uniforms.tDiffuse.value =
      this.depthNormalRenderTarget.texture;
    this._fxaaRenderMaterial.uniforms.resolution.value.set(
      1 / this.width,
      1 / this.height
    );
    return this._fxaaRenderMaterial;
  }

  constructor(parameters?: any) {
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
    this.renderOverrideVisibility.render(scene, () => {
      this.renderPass.renderWithOverrideMaterial(
        renderer,
        scene,
        camera,
        this.normalRenderMaterial,
        this.depthNormalRenderTarget,
        0x7777ff,
        1.0
      );
    });
    if (this.parameters.normalBufferFxaa) {
      this.renderPass.renderScreenSpace(
        renderer,
        this.fxaaRenderMaterial,
        this.antialiasedNormalRenderTarget
      );
    }
  }
}
