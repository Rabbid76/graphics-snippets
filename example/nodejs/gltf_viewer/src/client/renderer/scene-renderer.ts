import {
  CameraUpdate,
  getMaxSamples,
  RenderOverrideVisibility,
  RenderPass,
} from './render-util';
import { CopyTransformMaterial } from './shader-utility';
import { DepthNormalRenderTarget } from './depth-normal-render-targets';
import { GroundContactShadow } from './ground-contact-shadow';
import { LinearDepthRenderMaterial } from './depth-and-normal-materials-and-shaders';
import { OutLineRenderer, OutlineParameters } from './outline-renderer';
import { ScreenSpaceShadowMap } from './screen-space-shadow-map';
import { ShadowAndAoPass, ShadowAndAoParameters } from './shadow-and-ao-pass';
import { EffectComposer } from 'three/examples/jsm/postprocessing/EffectComposer';
import { Pass } from 'three/examples/jsm/postprocessing/Pass';
import {
  Camera,
  DoubleSide,
  MeshStandardMaterial,
  NearestFilter,
  NoBlending,
  Object3D,
  //PerspectiveCamera,
  RectAreaLight,
  Scene,
  ShaderMaterial,
  Vector2,
  WebGLRenderer,
  WebGLRenderTarget,
} from 'three';

export { GroundContactShadowParameters } from './ground-contact-shadow';
export { OutlineParameters } from './outline-renderer';
export { ShadowAndAoParameters } from './shadow-and-ao-render-targets';

class ScenePass extends Pass {
  private renderFunction: ((renderer: WebGLRenderer) => void) | null = null;

  constructor() {
    super();
  }

  public setRenderFunction(
    renderFunction: (renderer: WebGLRenderer) => void
  ): void {
    this.renderFunction = renderFunction;
  }

  public render(
    renderer: WebGLRenderer,
    writeBuffer: WebGLRenderTarget,
    _readBuffer: WebGLRenderTarget,
    _deltaTime: number,
    _maskActive: boolean
  ): void {
    renderer.setRenderTarget(this.renderToScreen ? null : writeBuffer);
    if (this.renderFunction) {
      renderer.clear();
      this.renderFunction(renderer);
    }
  }
}

export class SceneRenderer {
  public debugOutput = 'off';
  public outputEncoding = '';
  public toneMapping = '';
  private deferredShadowAndAoFrame = 0;
  private shadowAndAoFadeInFrames = 0;
  private noUpdateNeededCount = 0;
  private cameraUpdate: CameraUpdate = new CameraUpdate();
  public renderer: WebGLRenderer;
  private width: number = 0;
  private height: number = 0;
  private maxSamples: number = 1;
  private debugRenderOverrideVisibility: RenderOverrideVisibility =
    new RenderOverrideVisibility(true);
  private shadowRenderOverrideVisibility: RenderOverrideVisibility;
  private renderPass: RenderPass = new RenderPass();
  private shadowAndAoPass: ShadowAndAoPass;
  public screenSpaceShadow: ScreenSpaceShadowMap;
  public depthNormalRenderTarget: DepthNormalRenderTarget;
  public groundContactShadow: GroundContactShadow;
  public effectComposer: EffectComposer;
  public scenePass: ScenePass;
  public outlineRenderer: OutLineRenderer;
  public selectedObjects: Object3D[] = [];
  private grayMaterial = new MeshStandardMaterial({
    color: 0xc0c0c0,
    side: DoubleSide,
    envMapIntensity: 0.4,
  });
  private depthRenderMaterial?: LinearDepthRenderMaterial;
  private copyMaterial?: CopyTransformMaterial;

  public get shadowAndAoParameters(): ShadowAndAoParameters {
    return this.shadowAndAoPass.shadowAndAoParameters;
  }

  public get outlineParameters(): OutlineParameters {
    return this.outlineRenderer.outlineParameters;
  }

  public constructor(renderer: WebGLRenderer, width: number, height: number) {
    this.width = width;
    this.height = height;
    this.maxSamples = getMaxSamples(renderer);
    this.renderer = renderer;
    this.depthNormalRenderTarget = new DepthNormalRenderTarget({
      width: this.width,
      height: this.height,
      samples: this.maxSamples,
      renderPass: this.renderPass,
      textureFilter: NearestFilter,
    });
    this.shadowAndAoPass = new ShadowAndAoPass(
      this.width,
      this.height,
      this.maxSamples,
      {
        depthNormalRenderTarget: this.depthNormalRenderTarget,
      }
    );
    this.screenSpaceShadow = new ScreenSpaceShadowMap(
      new Vector2(this.width, this.height),
      {
        samples: this.maxSamples,
        shadowIntensity: 0.3,
        alwaysUpdate: false,
        depthNormalRenderTarget: this.depthNormalRenderTarget,
      }
    );
    this.groundContactShadow = new GroundContactShadow(this.renderer, {});
    this.groundContactShadow.group.rotateX(-Math.PI / 2);
    const renderTarget = new WebGLRenderTarget(this.width, this.height, {
      samples: this.maxSamples,
    });
    this.effectComposer = new EffectComposer(this.renderer, renderTarget);
    this.scenePass = new ScenePass();
    this.effectComposer.addPass(this.scenePass);
    this.outlineRenderer = new OutLineRenderer(
      null, //this.effectComposer,
      this.width,
      this.height,
      {}
    );
    this.shadowRenderOverrideVisibility = new RenderOverrideVisibility(false, [
      this.groundContactShadow.group,
    ]);
  }

  public dispose(): void {
    this.grayMaterial.dispose();
    this.depthRenderMaterial?.dispose();
    this.copyMaterial?.dispose();
    this.depthNormalRenderTarget.dispose();
    this.screenSpaceShadow.dispose();
    this.shadowAndAoPass.dispose();
    this.effectComposer.dispose();
    this.outlineRenderer.dispose();
    this.renderer.dispose();
  }

  public setSize(width: number, height: number): void {
    this.width = width;
    this.height = height;
    this.depthNormalRenderTarget.setSize(width, height);
    this.screenSpaceShadow.setSize(width, height);
    this.shadowAndAoPass.setSize(width, height);
    this.effectComposer.setSize(width, height);
    this.outlineRenderer.setSize(width, height);
    this.renderer.setSize(width, height);
  }

  private getDepthRenderMaterial(camera: Camera): LinearDepthRenderMaterial {
    this.depthRenderMaterial =
      this.depthRenderMaterial ??
      new LinearDepthRenderMaterial({
        depthTexture: this.depthNormalRenderTarget.depthTexture,
      });
    return this.depthRenderMaterial.update({ camera });
  }

  protected getCopyMaterial(parameters?: any): ShaderMaterial {
    this.copyMaterial = this.copyMaterial ?? new CopyTransformMaterial();
    return this.copyMaterial.update(parameters);
  }

  public addRectAreaLight(
    rectAreaLight: RectAreaLight,
    parent: Object3D
  ): void {
    this.screenSpaceShadow.addRectAreaLight(rectAreaLight, parent);
  }

  public updateRectAreaLights(
    rectAreaLights: RectAreaLight[],
    parent: Object3D
  ): void {
    this.screenSpaceShadow.updateRectAreaLights(rectAreaLights, parent);
  }

  public selectObjects(selectedObjects: Object3D[]) {
    this.selectedObjects = selectedObjects;
  }

  public setBestQuality(): void {
    this.deferredShadowAndAoFrame = 0;
    this.shadowAndAoFadeInFrames = 0;
  }

  public optimizeForInteraction(): void {
    this.deferredShadowAndAoFrame = 3;
    this.shadowAndAoFadeInFrames = 5;
  }

  public render(
    scene: Scene,
    camera: Camera,
    renderPostEffects: boolean
  ): void {
    //this.renderer.info.autoReset = false;
    //this.renderer.info.reset();
    this.screenSpaceShadow.alwaysUpdate =
      this.shadowAndAoParameters.aoAlwaysUpdate;
    this.outlineRenderer.updateOutline(scene, camera, this.selectedObjects);
    this.renderer.setRenderTarget(null);
    this.depthNormalRenderTarget.needsUpdate = true;
    this.groundContactShadow.render(scene);
    if (this.debugOutput === 'color') {
      this.renderer.render(scene, camera);
      return;
    }
    if (renderPostEffects) {
      this.scenePass.setRenderFunction((renderer: WebGLRenderer) =>
        this.renderScene(renderer, scene, camera)
      );
      this.effectComposer.render();
    } else {
      //const cameraClone = camera.clone() as PerspectiveCamera;
      //const offset_y = this.groundContactShadow.group.position.y
      //cameraClone.position.set(camera.position.x, -camera.position.y + 2*offset_y, camera.position.z);
      //cameraClone.lookAt(0, 2*offset_y, 0);
      //cameraClone.scale.set(1, -1, 1);
      //cameraClone.updateMatrixWorld();
      //cameraClone.updateProjectionMatrix();
      this.renderScene(this.renderer, scene, camera);
    }
    if (
      this.outlineRenderer.outlinePassActivated &&
      this.outlineRenderer.outlinePass
    ) {
      this.outlineRenderer.outlinePass.renderToScreen = false;
      this.outlineRenderer.outlinePass.render(
        this.renderer,
        // @ts-ignore
        null,
        null,
        0,
        false
      );
    }
    this.renderDebug(camera);
  }

  private renderScene(renderer: WebGLRenderer, scene: Scene, camera: Camera) {
    if (this.debugOutput === 'grayscale') {
      this.debugRenderOverrideVisibility.render(scene, () => {
        this.renderPass.renderWithOverrideMaterial(
          renderer,
          scene,
          camera,
          this.grayMaterial,
          null,
          0,
          1
        );
      });
    } else {
      renderer.render(scene, camera);
    }
    this.renderShadowAndAo(renderer, scene, camera);
  }

  private renderShadowAndAo(
    renderer: WebGLRenderer,
    scene: Scene,
    camera: Camera
  ): void {
    const update = this.evaluateIfShadowAndAoUpdateIsNeeded(camera);
    if (update.needsUpdate) {
      this.shadowRenderOverrideVisibility.render(scene, () => {
        this.screenSpaceShadow.renderShadowMap(this.renderer, scene, camera);
        this.shadowAndAoPass.updateShadowTexture(
          this.screenSpaceShadow.shadowTexture
        );
        this.shadowAndAoPass.render(
          renderer,
          scene,
          camera,
          update.intensityScale
        );
      });
    }
  }

  private evaluateIfShadowAndAoUpdateIsNeeded(camera: Camera) {
    const alwaysUpdate = this.shadowAndAoParameters.aoAlwaysUpdate;
    let needsUpdate =
      this.deferredShadowAndAoFrame === 0 || this.cameraUpdate.changed(camera);
    if (needsUpdate) {
      this.noUpdateNeededCount = 0;
    }
    let intensityScale = 1;
    if (!alwaysUpdate && this.deferredShadowAndAoFrame > 0) {
      this.noUpdateNeededCount++;
      needsUpdate = this.noUpdateNeededCount >= this.deferredShadowAndAoFrame;
      intensityScale = Math.max(
        0,
        Math.min(
          1,
          (this.noUpdateNeededCount - this.deferredShadowAndAoFrame) /
            this.shadowAndAoFadeInFrames
        )
      );
    }
    return { needsUpdate: alwaysUpdate || needsUpdate, intensityScale };
  }

  public renderDebug(camera: Camera): void {
    switch (this.debugOutput) {
      default:
        break;
      case 'normal':
        this.renderPass.renderScreenSpace(
          this.renderer,
          this.getCopyMaterial({
            texture: this.depthNormalRenderTarget?.normalTexture,
            blending: NoBlending,
            colorTransform: CopyTransformMaterial.defaultTransform,
          }),
          null
        );
        break;
      case 'depth':
        this.renderPass.renderScreenSpace(
          this.renderer,
          this.getDepthRenderMaterial(camera),
          null
        );
        break;
      case 'ssao':
        this.renderPass.renderScreenSpace(
          this.renderer,
          this.getCopyMaterial({
            texture:
              this.shadowAndAoPass.shadowAndAoRenderTargets.passRenderTarget
                .texture,
            blending: NoBlending,
            colorTransform: CopyTransformMaterial.grayscaleTransform,
          }),
          null
        );
        break;
      case 'ssaoblur':
        this.renderPass.renderScreenSpace(
          this.renderer,
          this.getCopyMaterial({
            texture:
              this.shadowAndAoPass.shadowAndAoRenderTargets.blurRenderTarget
                .texture,
            blending: NoBlending,
            colorTransform: CopyTransformMaterial.grayscaleTransform,
          }),
          null
        );
        break;
      case 'shadow':
        this.renderPass.renderScreenSpace(
          this.renderer,
          this.getCopyMaterial({
            texture:
              this.shadowAndAoPass.shadowAndAoRenderTargets.passRenderTarget
                .texture,
            blending: NoBlending,
            colorTransform: ShadowAndAoPass.shadowTransform,
          }),
          null
        );
        break;
      case 'shadowblur':
        this.renderPass.renderScreenSpace(
          this.renderer,
          this.getCopyMaterial({
            texture:
              this.shadowAndAoPass.shadowAndAoRenderTargets.blurRenderTarget
                .texture,
            blending: NoBlending,
            colorTransform: ShadowAndAoPass.shadowTransform,
          }),
          null
        );
        break;
    }
  }
}
