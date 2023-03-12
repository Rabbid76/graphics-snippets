import { RenderOverrideVisibility, RenderPass } from './render-utility';
import {
  Camera,
  DepthStencilFormat,
  DepthTexture,
  MeshNormalMaterial,
  NearestFilter,
  NoBlending,
  Scene,
  Texture,
  TextureFilter,
  UnsignedInt248Type,
  WebGLRenderer,
  WebGLRenderTarget,
} from 'three';

export interface DepthAndNormalTextures {
  getNormalTexture(): Texture;
  getDepthTexture(): Texture;
}

export class DepthNormalRenderTarget {
  private depthNormalScale = 1;
  private targetTextureFilter: TextureFilter;
  private width: number;
  private height: number;
  private samples: number;
  private _normalRenderMaterial?: MeshNormalMaterial;
  private _depthNormalRenderTarget?: WebGLRenderTarget;
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
    return this.depthNormalRenderTarget.texture;
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
        this.width * this.depthNormalScale,
        this.height * this.depthNormalScale
      );
      depthTexture.format = DepthStencilFormat;
      depthTexture.type = UnsignedInt248Type;
      this._depthNormalRenderTarget = new WebGLRenderTarget(
        this.width * this.depthNormalScale,
        this.height * this.depthNormalScale,
        {
          minFilter: this.targetTextureFilter,
          magFilter: this.targetTextureFilter,
          depthTexture,
        }
      );
    }
    return this._depthNormalRenderTarget;
  }

  constructor(parameters?: any) {
    this.depthNormalScale = parameters?.depthNormalScale ?? 1;
    this.targetTextureFilter = parameters?.textureFilter ?? NearestFilter;
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
  }

  public setSize(width: number, height: number) {
    this.width = width;
    this.height = height;
    this._depthNormalRenderTarget?.setSize(
      this.width * this.depthNormalScale,
      this.height * this.depthNormalScale
    );
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
  }
}
