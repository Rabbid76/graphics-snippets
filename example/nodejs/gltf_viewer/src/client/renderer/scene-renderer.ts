import {
  CameraUpdate,
  getMaxSamples,
  RenderOverrideVisibility,
  RenderPass,
  SceneVolume,
} from './render-utility';
import {
  CopyTransformMaterial,
  LinearDepthRenderMaterial,
} from './shader-utility';
import {
  DepthNormalRenderTarget,
  DepthNormalBufferParameters,
} from './depth-normal-render-targets';
import {
  BakedGroundContactShadow,
  BakedGroundContactShadowParameters,
} from './baked-ground-contact-shadow';
import { OutLineRenderer, OutlineParameters } from './outline-renderer';
import {
  ScreenSpaceShadowMap,
  ScreenSpaceShadowMapParameters,
} from './screen-space-shadow-map';
import { ShadowAndAoPass, ShadowAndAoParameters } from './shadow-and-ao-pass';
import {
  GroundReflectionPass,
  GroundReflectionParameters,
} from './ground-reflection-pass';
import { EffectComposer } from 'three/examples/jsm/postprocessing/EffectComposer';
import { Pass } from 'three/examples/jsm/postprocessing/Pass';
import {
  Box3,
  Camera,
  Color,
  CustomBlending,
  DoubleSide,
  Group,
  Matrix4,
  MeshStandardMaterial,
  NearestFilter,
  NoBlending,
  Object3D,
  PerspectiveCamera,
  RectAreaLight,
  Scene,
  ShaderMaterial,
  Vector2,
  WebGLRenderer,
  WebGLRenderTarget,
} from 'three';

export { BakedGroundContactShadowParameters } from './baked-ground-contact-shadow';
export { OutlineParameters } from './outline-renderer';
export { ShadowAndAoParameters } from './shadow-and-ao-pass';

export enum SceneGroupType {
  OBJECTS,
  GROUND,
  ENVIRONMENT,
  LIGHTS,
}

export enum QualityLevel {
  HIGHEST,
  HIGH,
  MEDIUM,
  LOW,
}

export interface SceneRendererParameters {
  depthNormalRenderTargetParameters: DepthNormalBufferParameters;
  shadowAndAoParameters: ShadowAndAoParameters;
  screenSpaceShadowMapParameters: ScreenSpaceShadowMapParameters;
  groundReflectionParameters: GroundReflectionParameters;
  bakedGroundContactShadowParameters: BakedGroundContactShadowParameters;
  outlineParameters: OutlineParameters;
  deferredShadowAndAoFrame: number;
  shadowAndAoFadeInFrames: number;
  hardShadowOnCameraChange: boolean;
}

export class SceneRenderer {
  public parameters: SceneRendererParameters;
  public debugOutput = 'off';
  public shadowOnGround = false;
  public outputEncoding = '';
  public toneMapping = '';
  public groundReflection = false;
  public movingCamera: boolean = false;
  public groundLevel: number = 0;
  private noUpdateNeededCount = 0;
  private cameraUpdate: CameraUpdate = new CameraUpdate();
  public renderer: WebGLRenderer;
  private width: number = 0;
  private height: number = 0;
  private maxSamples: number = 1;
  public boundingVolume = new SceneVolume();
  private debugRenderOverrideVisibility: RenderOverrideVisibility =
    new RenderOverrideVisibility(true);
  private shadowRenderOverrideVisibility: RenderOverrideVisibility =
    new RenderOverrideVisibility(true, (object: any) => {
      return !this.shadowOnGround && object === this.groundGroup;
    });
  private renderPass: RenderPass = new RenderPass();
  public shadowAndAoPass: ShadowAndAoPass;
  public screenSpaceShadow: ScreenSpaceShadowMap;
  public groundReflectionPass: GroundReflectionPass;
  public depthNormalRenderTarget: DepthNormalRenderTarget;
  public bakedGroundContactShadow: BakedGroundContactShadow;
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
  public sceneGroups: SceneGroups = new SceneGroups();
  public readonly groundGroup: Group = new Group();

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
        alwaysUpdate: false,
        depthNormalRenderTarget: this.depthNormalRenderTarget,
      }
    );
    this.groundReflectionPass = new GroundReflectionPass(
      this.width,
      this.height,
      {
        renderPass: this.renderPass,
      }
    );
    this.bakedGroundContactShadow = new BakedGroundContactShadow(
      this.renderer,
      this.groundGroup,
      {
        renderPass: this.renderPass,
      }
    );
    this.groundGroup.rotateX(-Math.PI / 2);
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
      {
        depthNormalRenderTarget: this.depthNormalRenderTarget,
      }
    );
    this.sceneGroups.addToGroup(SceneGroupType.GROUND, this.groundGroup);
    this.parameters = {
      depthNormalRenderTargetParameters:
        this.depthNormalRenderTarget.parameters,
      bakedGroundContactShadowParameters:
        this.bakedGroundContactShadow.parameters,
      screenSpaceShadowMapParameters: this.screenSpaceShadow.parameters,
      shadowAndAoParameters: this.shadowAndAoPass.parameters,
      groundReflectionParameters: this.groundReflectionPass.parameters,
      outlineParameters: this.outlineRenderer.parameters,
      deferredShadowAndAoFrame: 0,
      shadowAndAoFadeInFrames: 0,
      hardShadowOnCameraChange: false,
    };
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

  public addToScene(scene: Scene) {
    if (this.groundGroup.parent !== scene) {
      scene.add(this.groundGroup);
    }
  }

  public removeFromScene(scene: Scene) {
    if (this.groundGroup.parent === scene) {
      scene.remove(this.groundGroup);
    }
  }

  private getDepthRenderMaterial(camera: Camera): LinearDepthRenderMaterial {
    this.depthRenderMaterial =
      this.depthRenderMaterial ??
      new LinearDepthRenderMaterial({
        depthTexture: this.depthNormalRenderTarget.depthTexture,
      });
    return this.depthRenderMaterial.update({ camera });
  }

  public setQualityLevel(qualityLevel: QualityLevel): void {
    switch (qualityLevel) {
      default:
      case QualityLevel.HIGHEST:
        this.updateParameters({
          deferredShadowAndAoFrame: 0,
          shadowAndAoFadeInFrames: 0,
          hardShadowOnCameraChange: false,
          shadowAndAoParameters: {
            aoAndSoftShadowEnabled: true,
            aoAndSoftShadowFxaa: true,
          },
          groundReflectionParameters: {
            enabled: false,
          },
          bakedGroundContactShadowParameters: {
            enabled: true,
          },
        });
        break;
      case QualityLevel.HIGH:
        this.updateParameters({
          deferredShadowAndAoFrame: 3,
          shadowAndAoFadeInFrames: 6,
          hardShadowOnCameraChange: true,
          shadowAndAoParameters: {
            aoAndSoftShadowEnabled: true,
            aoAndSoftShadowFxaa: true,
          },
          groundReflectionParameters: {
            enabled: false,
          },
          bakedGroundContactShadowParameters: {
            enabled: true,
          },
        });
        break;
      case QualityLevel.MEDIUM:
        this.updateParameters({
          deferredShadowAndAoFrame: 3,
          shadowAndAoFadeInFrames: 5,
          hardShadowOnCameraChange: false,
          shadowAndAoParameters: {
            aoAndSoftShadowEnabled: true,
            aoAndSoftShadowFxaa: false,
          },
          groundReflectionParameters: {
            enabled: false,
          },
          bakedGroundContactShadowParameters: {
            enabled: true,
          },
        });
        break;
      case QualityLevel.LOW:
        this.updateParameters({
          shadowAndAoParameters: {
            aoAndSoftShadowEnabled: false,
            aoAndSoftShadowFxaa: false,
          },
          groundReflectionParameters: {
            enabled: false,
          },
          bakedGroundContactShadowParameters: {
            enabled: true,
          },
        });
        break;
    }
  }

  public forceShadowUpdates(updateBakedGroundShadow: boolean): void {
    this.shadowAndAoPass.needsUpdate = true;
    this.screenSpaceShadow.needsUpdate = true;
    if (updateBakedGroundShadow) {
      this.bakedGroundContactShadow.needsUpdate = true;
    }
  }

  public updateParameters(parameters: any) {
    if (parameters.shadowAndAoParameters !== undefined) {
      this.shadowAndAoPass.updateParameters(parameters.shadowAndAoParameters);
    }
    if (parameters.bakedGroundContactShadowParameters !== undefined) {
      this.bakedGroundContactShadow.updateParameters(
        parameters.bakedGroundContactShadowParameters
      );
    }
    if (parameters.screenSpaceShadowMapParameters !== undefined) {
      this.screenSpaceShadow.updateParameters(
        parameters.screenSpaceShadowMapParameters
      );
    }
    if (parameters.groundReflectionParameters !== undefined) {
      this.groundReflectionPass.updateParameters(
        parameters.groundReflectionParameters
      );
    }
    if (parameters.outlineParameters !== undefined) {
      this.outlineRenderer.updateParameters(parameters.outlineParameters);
    }
    if (parameters.deferredShadowAndAoFrame !== undefined) {
      this.parameters.deferredShadowAndAoFrame =
        parameters.deferredShadowAndAoFrame;
    }
    if (parameters.shadowAndAoFadeInFrames !== undefined) {
      this.parameters.shadowAndAoFadeInFrames =
        parameters.shadowAndAoFadeInFrames;
    }
    if (parameters.hardShadowOnCameraChange !== undefined) {
      this.parameters.hardShadowOnCameraChange =
        parameters.hardShadowOnCameraChange;
    }
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
    this.shadowAndAoPass.needsUpdate = true;
  }

  public updateRectAreaLights(
    rectAreaLights: RectAreaLight[],
    parent: Object3D
  ): void {
    this.screenSpaceShadow.updateRectAreaLights(rectAreaLights, parent);
    this.shadowAndAoPass.needsUpdate = true;
  }

  public selectObjects(selectedObjects: Object3D[]) {
    this.selectedObjects = selectedObjects;
  }

  public updateBounds(bounds: Box3, scaleShadowAndAo: boolean) {
    this.boundingVolume.updateFromBox(bounds);
    const size = this.boundingVolume.size;
    const shadowAndAoScale = (size.x + size.y + size.z) / 3;
    const minBoundsSize = Math.min(size.x, size.y, size.z);
    const defaultScale =
      minBoundsSize < 0.5 ? minBoundsSize / 0.5 : size.z > 5 ? size.z / 5 : 1;
    this.bakedGroundContactShadow.needsUpdate = true;
    this.bakedGroundContactShadow.updateBounds(
      this.boundingVolume,
      scaleShadowAndAo ? shadowAndAoScale : defaultScale,
      this.groundLevel
    );
    this.groundReflectionPass.parameters.groundLevel = this.groundLevel;
    this.screenSpaceShadow.updateBounds(this.boundingVolume, shadowAndAoScale);
    this.shadowAndAoPass.updateBounds(
      this.boundingVolume,
      scaleShadowAndAo ? shadowAndAoScale : 1
    );
  }

  public updateNearAndFarPlaneOfPerspectiveCamera(camera: PerspectiveCamera) {
    // bring the near and far plane as close as possible to geometry
    // this is very likely the most important part for a glitch free and nice SSAO
    const nearFar = this.boundingVolume.getNearAndFarForPerspectiveCamera(
      camera.position,
      3
    );
    camera.near = Math.max(0.00001, nearFar[0] * 0.9);
    camera.far = nearFar[1];
    camera.updateProjectionMatrix();
  }

  public render(
    scene: Scene,
    camera: Camera,
    renderPostEffects: boolean
  ): void {
    //this.renderer.info.autoReset = false;
    //this.renderer.info.reset();
    this.screenSpaceShadow.parameters.alwaysUpdate =
      this.shadowAndAoPass.parameters.aoAlwaysUpdate;
    this.outlineRenderer.updateOutline(
      scene,
      camera,
      this.movingCamera ? [] : this.selectedObjects
    );
    this.renderer.setRenderTarget(null);
    this.depthNormalRenderTarget.needsUpdate = true;
    this.bakedGroundContactShadow.render(scene);
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
      this.renderScene(this.renderer, scene, camera);
    }
    this.renderOutline();
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
      this.bakedGroundContactShadow.setGroundVisibility(true, camera);
      renderer.render(scene, camera);
      this.bakedGroundContactShadow.setGroundVisibility(false, camera);
    }
    if (this.groundReflectionPass.parameters.enabled) {
      //const background = scene.background;
      //scene.background = null;
      this.sceneGroups.setVisibilityOfGroup(SceneGroupType.GROUND, false);
      this.groundReflectionPass.render(renderer, scene, camera);
      this.sceneGroups.setVisibilityOfGroup(SceneGroupType.GROUND, true);
      //scene.background = background;
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
        if (update.hardShadow && update.intensityScale < 0.99) {
          this.renderHardShadow(1.0 - update.intensityScale);
        }
        this.shadowAndAoPass.render(
          renderer,
          scene,
          camera,
          update.intensityScale
        );
      });
    } else if (update.hardShadow) {
      this.shadowRenderOverrideVisibility.render(scene, () => {
        this.screenSpaceShadow.renderShadowMap(this.renderer, scene, camera);
        this.renderHardShadow(1.0);
      });
    }
  }

  private renderHardShadow(shadowScale: number) {
    const shIntensity =
      this.shadowAndAoPass.parameters.shadowIntensity * shadowScale;
    this.renderPass.renderScreenSpace(
      this.renderer,
      this.getCopyMaterial({
        texture: this.screenSpaceShadow.shadowTexture,
        blending: CustomBlending,
        colorTransform: new Matrix4().set(
          shIntensity,
          0,
          0,
          1 - shIntensity,
          0,
          shIntensity,
          0,
          1,
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
        uvTransform: CopyTransformMaterial.defaultUvTransform,
      }),
      null
    );
  }

  private evaluateIfShadowAndAoUpdateIsNeeded(camera: Camera) {
    const updateNow =
      this.shadowAndAoPass.parameters.aoAlwaysUpdate ||
      this.screenSpaceShadow.needsUpdate ||
      this.screenSpaceShadow.shadowTypeNeedsUpdate;
    let needsUpdate =
      this.shadowAndAoPass.parameters.aoAndSoftShadowEnabled &&
      (this.parameters.deferredShadowAndAoFrame === 0 ||
        this.cameraUpdate.changed(camera));
    if (needsUpdate) {
      this.noUpdateNeededCount = 0;
    }
    let intensityScale = 1;
    if (!updateNow && this.parameters.deferredShadowAndAoFrame > 0) {
      this.noUpdateNeededCount++;
      needsUpdate =
        this.noUpdateNeededCount >= this.parameters.deferredShadowAndAoFrame;
      intensityScale = Math.max(
        0,
        Math.min(
          1,
          (this.noUpdateNeededCount -
            this.parameters.deferredShadowAndAoFrame) /
            this.parameters.shadowAndAoFadeInFrames
        )
      );
    }
    needsUpdate = updateNow || needsUpdate;
    const hardShadow =
      this.parameters.hardShadowOnCameraChange &&
      (!needsUpdate || intensityScale < 0.99);
    return { needsUpdate, hardShadow, intensityScale };
  }

  private renderOutline() {
    if (
      this.outlineRenderer.outlinePassActivated &&
      this.outlineRenderer.outlinePass
    ) {
      const clearColor = this.renderer.getClearColor(new Color());
      const clearAlpha = this.renderer.getClearAlpha();
      if (this.debugOutput === 'outline') {
        this.renderer.setClearColor(0x000000, 0xff);
        this.renderer.clear(true, false, false);
      }
      this.outlineRenderer.outlinePass.renderToScreen = false;
      this.outlineRenderer.outlinePass.render(
        this.renderer,
        null,
        null,
        0,
        false
      );
      if (this.debugOutput === 'outline') {
        this.renderer.setClearColor(clearColor, clearAlpha);
      }
    }
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
            multiplyChannels: 0,
            uvTransform: CopyTransformMaterial.defaultUvTransform,
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
            multiplyChannels: 0,
            uvTransform: CopyTransformMaterial.defaultUvTransform,
          }),
          null
        );
        break;
      case 'ssaoblur':
        this.renderPass.renderScreenSpace(
          this.renderer,
          this.getCopyMaterial({
            texture: this.shadowAndAoPass.finalRenderTarget.texture,
            blending: NoBlending,
            colorTransform: CopyTransformMaterial.grayscaleTransform,
            multiplyChannels: 0,
            uvTransform: CopyTransformMaterial.defaultUvTransform,
          }),
          null
        );
        break;
      case 'shadowmap':
        this.renderPass.renderScreenSpace(
          this.renderer,
          this.getCopyMaterial({
            texture: this.screenSpaceShadow.shadowTexture,
            blending: NoBlending,
            colorTransform: CopyTransformMaterial.grayscaleTransform,
            multiplyChannels: 0,
            uvTransform: CopyTransformMaterial.defaultUvTransform,
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
            multiplyChannels: 0,
            uvTransform: CopyTransformMaterial.defaultUvTransform,
          }),
          null
        );
        break;
      case 'shadowblur':
        this.renderPass.renderScreenSpace(
          this.renderer,
          this.getCopyMaterial({
            texture: this.shadowAndAoPass.finalRenderTarget.texture,
            blending: NoBlending,
            colorTransform: ShadowAndAoPass.shadowTransform,
            multiplyChannels: 0,
            uvTransform: CopyTransformMaterial.defaultUvTransform,
          }),
          null
        );
        break;
      case 'groundshadow':
        this.renderPass.renderScreenSpace(
          this.renderer,
          this.getCopyMaterial({
            texture: this.groundReflectionPass.shadowRenderTarget.texture,
            blending: NoBlending,
            colorTransform: CopyTransformMaterial.defaultTransform,
            multiplyChannels: 0,
            uvTransform: CopyTransformMaterial.defaultUvTransform,
          }),
          null
        );
        break;
      case 'groundreflection':
        this.renderPass.renderScreenSpace(
          this.renderer,
          this.getCopyMaterial({
            texture: this.groundReflectionPass.reflectionRenderTarget.texture,
            blending: NoBlending,
            colorTransform: CopyTransformMaterial.defaultTransform,
            multiplyChannels: 0,
            uvTransform: CopyTransformMaterial.flipYuvTransform,
          }),
          null
        );
        break;
      case 'bakedgroundshadow':
        this.renderPass.renderScreenSpace(
          this.renderer,
          this.getCopyMaterial({
            texture: this.bakedGroundContactShadow.renderTarget.texture,
            blending: NoBlending,
            colorTransform: CopyTransformMaterial.defaultTransform,
            multiplyChannels: 0,
            uvTransform: CopyTransformMaterial.defaultUvTransform,
          }),
          null
        );
        break;
    }
  }
}

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

export class SceneGroups {
  public sceneGroups = new Map<SceneGroupType, Object3D[]>();

  public addToGroup(group: SceneGroupType, object: Object3D) {
    if (!this.sceneGroups.has(group)) {
      this.sceneGroups.set(group, []);
    }
    this.sceneGroups.get(group)?.push(object);
  }

  public setVisibilityOfGroup(group: SceneGroupType, visible: boolean) {
    this.sceneGroups.get(group)?.forEach((object) => {
      object.visible = visible;
    });
  }
}
