import {
    Camera,
    Color,
    Material,
    Object3D,
    Scene,
    WebGLRenderer,
    WebGLRenderTarget,
} from 'three';
import { FullScreenQuad } from 'three/examples/jsm/postprocessing/Pass';

export class RenderOverrideVisibility {
    private _visibilityCache = new Map();
    private background: any;
    private hideLinesAndPoints: boolean;
    public onIsObjectInvisible?: (object: any) => boolean;
  
    constructor(
      hideLinesAndPoints?: boolean,
      onIsObjectInvisible?: (object: any) => boolean,
      background?: any
    ) {
      this.background = background;
      this.hideLinesAndPoints = hideLinesAndPoints ?? false;
      this.onIsObjectInvisible = onIsObjectInvisible;
    }
  
    public updateVisibilityCache(scene?: Scene) {
      this._visibilityCache.clear();
      if (scene && (this.hideLinesAndPoints || this.onIsObjectInvisible)) {
        scene.traverse((object: any) => {
          if (this.hideLinesAndPoints && (object.isPoints || object.isLine)) {
            this._visibilityCache.set(object, object.visible);
          } else if (this.onIsObjectInvisible) {
            const invisible = this.onIsObjectInvisible(object);
            if (invisible) {
              this._visibilityCache.set(object, object.visible);
            }
          }
        });
      }
    }
  
    public render(scene: Scene, renderMethod: () => void) {
      this.updateVisibilityCache(scene);
      const sceneBackground = scene.background;
      if (this.background !== undefined) {
        scene.background = this.background;
      }
      this._visibilityCache.forEach((_visible: boolean, object: Object3D) => {
        object.visible = false;
      });
      renderMethod();
      this._visibilityCache.forEach((visible: boolean, object: Object3D) => {
        object.visible = visible;
      });
      if (this.background !== undefined) {
        scene.background = sceneBackground;
      }
    }
}

export class RenderPass {
    private originalClearColor = new Color();
    private originalClearAlpha: number = 0;
    private originalAutoClear: boolean = false;
    private originalRenderTarget: WebGLRenderTarget | null = null;
    private screenSpaceQuad = new FullScreenQuad(undefined);
  
    public renderWithOverrideMaterial(
      renderer: WebGLRenderer,
      scene: Scene,
      camera: Camera,
      overrideMaterial: Material | null,
      renderTarget: WebGLRenderTarget | null,
      clearColor?: any,
      clearAlpha?: any
    ): void {
      this.backup(renderer);
      this.prepareRenderer(renderer, renderTarget, clearColor, clearAlpha);
      const originalOverrideMaterial = scene.overrideMaterial;
      scene.overrideMaterial = overrideMaterial;
      renderer.render(scene, camera);
      scene.overrideMaterial = originalOverrideMaterial;
      this.restore(renderer);
    }
  
    public render(
      renderer: WebGLRenderer,
      scene: Scene,
      camera: Camera,
      renderTarget: WebGLRenderTarget | null,
      clearColor?: any,
      clearAlpha?: any
    ): void {
      this.backup(renderer);
      this.prepareRenderer(renderer, renderTarget, clearColor, clearAlpha);
      renderer.render(scene, camera);
      this.restore(renderer);
    }
  
    public renderScreenSpace(
      renderer: WebGLRenderer,
      passMaterial: Material,
      renderTarget: WebGLRenderTarget | null,
      clearColor?: any,
      clearAlpha?: any
    ): void {
      this.backup(renderer);
      this.prepareRenderer(renderer, renderTarget, clearColor, clearAlpha);
      this.screenSpaceQuad.material = passMaterial;
      this.screenSpaceQuad.render(renderer);
      this.restore(renderer);
    }
  
    private prepareRenderer(
      renderer: WebGLRenderer,
      renderTarget: WebGLRenderTarget | null,
      clearColor?: any,
      clearAlpha?: any
    ): void {
      renderer.setRenderTarget(renderTarget);
      renderer.autoClear = false;
      if (clearColor !== undefined && clearColor !== null) {
        renderer.setClearColor(clearColor);
        renderer.setClearAlpha(clearAlpha || 0.0);
        renderer.clear();
      }
    }
  
    private backup(renderer: WebGLRenderer): void {
      renderer.getClearColor(this.originalClearColor);
      this.originalClearAlpha = renderer.getClearAlpha();
      this.originalAutoClear = renderer.autoClear;
      this.originalRenderTarget = renderer.getRenderTarget();
    }
  
    private restore(renderer: WebGLRenderer): void {
      renderer.setClearColor(this.originalClearColor);
      renderer.setClearAlpha(this.originalClearAlpha);
      renderer.setRenderTarget(this.originalRenderTarget);
      renderer.autoClear = this.originalAutoClear;
    }
}
