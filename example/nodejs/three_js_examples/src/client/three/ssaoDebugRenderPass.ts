import { SsaoRenderPass } from "./ssaoRenderPass";
import { 
    CopyTransformMaterial,
    LinearDepthRenderMaterial,
} from './shaderUtility'
import {
    Camera,
    Matrix4,
    NoBlending,
    ShaderMaterial,
    Vector4,
    WebGLRenderer,
} from 'three';

export class SsaoDebugRenderPass extends SsaoRenderPass {
    public debugOutput: string = 'off';
    private copyMaterial?: CopyTransformMaterial;
    private depthRenderMaterial?: LinearDepthRenderMaterial;

    public dispose(): void {
        super.dispose();
        this.depthRenderMaterial?.dispose();
        this.copyMaterial?.dispose();
      }

      public render(renderer: THREE.WebGLRenderer, writeBuffer: THREE.WebGLRenderTarget): void {
        if (this.debugOutput === 'color') {
            return;
        }
        super.render(renderer, writeBuffer);
    }
    protected renderToTarget(renderer: THREE.WebGLRenderer, writeBuffer: THREE.WebGLRenderTarget): void {
        if (this.debugOutput && this.debugOutput !== '' && this.debugOutput !== 'off') {
            this.renderDebug(renderer);
        } else {
            super.renderToTarget(renderer, writeBuffer);
        }
    }
    
      protected getCopyMaterial(parameters?: any): ShaderMaterial {
        this.copyMaterial = this.copyMaterial ?? new CopyTransformMaterial();
        return this.copyMaterial.update(parameters);
      }
    
      private getDepthRenderMaterial(camera: Camera): LinearDepthRenderMaterial {
        this.depthRenderMaterial =
          this.depthRenderMaterial ??
          new LinearDepthRenderMaterial({
            depthTexture: this.gBufferRenderTarget.getTextureWithDepth(),
            depthFilter: this.gBufferRenderTarget.isFloatGBufferWithRgbNormalAlphaDepth()
              ? new Vector4(0, 0, 0, 1)
              : new Vector4(1, 0, 0, 0),
          });
        return this.depthRenderMaterial.update({ camera });
      }
    
      public renderDebug(renderer: WebGLRenderer): void {
        switch (this.debugOutput) {
          default:
            break;
            case 'lineardepth':
                if (this.camera) {
                    this.renderPass.renderScreenSpace(
                        renderer,
                        this.getDepthRenderMaterial(this.camera),
                        null
                    );
                }
                break;
          case 'g-normal':
            if (this.gBufferRenderTarget.isFloatGBufferWithRgbNormalAlphaDepth()) {
              this.renderPass.renderScreenSpace(
                renderer,
                this.getCopyMaterial({
                  texture: this.gBufferRenderTarget?.getGBufferTexture(),
                  blending: NoBlending,
                  colorTransform: new Matrix4().set(
                    0.5, 0, 0, 0, 
                    0, 0.5, 0, 0, 
                    0, 0, 0.5, 0,
                    0, 0, 0, 0
                  ),
                  colorBase: new Vector4(0.5, 0.5, 0.5, 1),
                  multiplyChannels: 0,
                  uvTransform: CopyTransformMaterial.defaultUvTransform,
                }),
                null
              );
            } else {
              this.renderPass.renderScreenSpace(
                renderer,
                this.getCopyMaterial({
                  texture: this.gBufferRenderTarget?.getGBufferTexture(),
                  blending: NoBlending,
                  colorTransform: CopyTransformMaterial.rgbTransform,
                  colorBase: CopyTransformMaterial.alphaRGBA,
                  multiplyChannels: 0,
                  uvTransform: CopyTransformMaterial.defaultUvTransform,
                }),
                null
              );
            }
            break;
          case 'g-depth':
            if (this.gBufferRenderTarget.isFloatGBufferWithRgbNormalAlphaDepth()) {
              this.renderPass.renderScreenSpace(
                renderer,
                this.getCopyMaterial({
                  texture: this.gBufferRenderTarget?.getGBufferTexture(),
                  blending: NoBlending,
                  colorTransform: CopyTransformMaterial.alphaTransform,
                  colorBase: CopyTransformMaterial.alphaRGBA,
                  multiplyChannels: 0,
                  uvTransform: CopyTransformMaterial.defaultUvTransform,
                }),
                null
              );
            } else {
              this.renderPass.renderScreenSpace(
                renderer,
                this.getCopyMaterial({
                  texture: this.gBufferRenderTarget?.getDepthBufferTexture(),
                  blending: NoBlending,
                  colorTransform: CopyTransformMaterial.redTransform,
                  colorBase: CopyTransformMaterial.alphaRGBA,
                  multiplyChannels: 0,
                  uvTransform: CopyTransformMaterial.defaultUvTransform,
                }),
                null
              );
            }
            break;
          case 'ao':
            this.renderPass.renderScreenSpace(
              renderer,
              this.getCopyMaterial({
                texture:
                  this.aoRenderTargets.passRenderTarget
                    .texture,
                blending: NoBlending,
                colorTransform: CopyTransformMaterial.grayscaleTransform,
                colorBase: CopyTransformMaterial.zeroRGBA,
                multiplyChannels: 0,
                uvTransform: CopyTransformMaterial.defaultUvTransform,
              }),
              null
            );
            break;
          case 'aodenoise':
            this.renderPass.renderScreenSpace(
              renderer,
              this.getCopyMaterial({
                texture: this.finalRenderTarget.texture,
                blending: NoBlending,
                colorTransform: CopyTransformMaterial.grayscaleTransform,
                colorBase: CopyTransformMaterial.zeroRGBA,
                multiplyChannels: 0,
                uvTransform: CopyTransformMaterial.defaultUvTransform,
              }),
              null
            );
            break;
        }
      }
}