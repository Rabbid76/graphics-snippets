import {
  CameraUpdate,
  getMaxSamples,
  RenderOverrideVisibility,
  RenderPass,
  SceneVolume,
} from './render-utility';
import {
  ALPHA_RGBA,
  ALPHA_TRANSFORM,
  CopyTransformMaterial,
  DEFAULT_TRANSFORM,
  DEFAULT_UV_TRANSFORM,
  FLIP_Y_UV_TRANSFORM,
  GRAYSCALE_TRANSFORM,
  LinearDepthRenderMaterial,
  RED_TRANSFORM,
  RGB_TRANSFORM,
  ZERO_RGBA,
} from './shader-utility';
import {
  GBufferParameters,
  GBufferRenderTargets,
} from './gbuffer-render-target';
import {
  BakedGroundContactShadow,
  BakedGroundContactShadowParameters,
} from './baked-ground-contact-shadow';
import { OutlineParameters, OutLineRenderer } from './outline-renderer';
import {
  ScreenSpaceShadowMap,
  ScreenSpaceShadowMapParameters,
} from './screen-space-shadow-map';
import {
  AmbientOcclusionType,
  ShadowAndAoPass,
  ShadowAndAoPassParameters,
} from './shadow-and-ao-pass';
import {
  GroundReflectionParameters,
  GroundReflectionPass,
} from './ground-reflection-pass';
import { LightSource, LightSourceDetector } from './light-source-detection';
import { EffectComposer } from 'three/examples/jsm/postprocessing/EffectComposer';
import { Pass } from 'three/examples/jsm/postprocessing/Pass';
export { BakedGroundContactShadowParameters } from './baked-ground-contact-shadow';
export { OutlineParameters } from './outline-renderer';
export {
  AmbientOcclusionType,
  ShadowAndAoParameters,
  ShadowAndAoPassParameters,
} from './shadow-and-ao-pass';
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
  Texture,
  Vector2,
  Vector4,
  WebGLRenderer,
  WebGLRenderTarget,
} from 'three';

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
  gBufferRenderTargetParameters: GBufferParameters;
  shAndAoPassParameters: ShadowAndAoPassParameters;
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
  private prevDebugOutput = 'off';
  public shadowOnGround = false;
  public outputColorSpace = '';
  public toneMapping = '';
  public environmentLights = false;
  public groundReflection: boolean = false;
  public movingCamera: boolean = false;
  public groundLevel: number = 0;
  private noUpdateNeededCount = 0;
  private cameraUpdate: CameraUpdate = new CameraUpdate();
  public renderer: WebGLRenderer;
  public width: number = 0;
  public height: number = 0;
  private maxSamples: number = 1;
  private cameraChanged: boolean = true;
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
  public gBufferRenderTarget: GBufferRenderTargets;
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
  private copyMaterial?: CopyTransformMaterial;
  public sceneGroups: SceneGroups = new SceneGroups();
  public readonly groundGroup: Group = new Group();
  private debugPass?: DebugPass;
  private qualityLevel: QualityLevel = QualityLevel.HIGHEST;

  public constructor(renderer: WebGLRenderer, width: number, height: number) {
    this.width = width;
    this.height = height;
    this.maxSamples = getMaxSamples(renderer);
    this.renderer = renderer;
    this.gBufferRenderTarget = new GBufferRenderTargets({
      shared: true,
      capabilities: renderer.capabilities,
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
        gBufferRenderTarget: this.gBufferRenderTarget,
      }
    );
    this.screenSpaceShadow = new ScreenSpaceShadowMap(
      new Vector2(this.width, this.height),
      {
        samples: this.maxSamples,
        alwaysUpdate: false,
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
        gBufferRenderTarget: this.gBufferRenderTarget,
      }
    );
    this.sceneGroups.addToGroup(SceneGroupType.GROUND, this.groundGroup);
    this.parameters = {
      gBufferRenderTargetParameters: this.gBufferRenderTarget.parameters,
      bakedGroundContactShadowParameters:
        this.bakedGroundContactShadow.parameters,
      screenSpaceShadowMapParameters: this.screenSpaceShadow.parameters,
      shAndAoPassParameters: this.shadowAndAoPass.parameters,
      groundReflectionParameters: this.groundReflectionPass.parameters,
      outlineParameters: this.outlineRenderer.parameters,
      deferredShadowAndAoFrame: 0,
      shadowAndAoFadeInFrames: 0,
      hardShadowOnCameraChange: false,
    };
  }

  public dispose(): void {
    this.debugPass?.dispose();
    this.grayMaterial.dispose();
    this.copyMaterial?.dispose();
    this.gBufferRenderTarget.dispose();
    this.screenSpaceShadow.dispose();
    this.shadowAndAoPass.dispose();
    this.effectComposer.dispose();
    this.outlineRenderer.dispose();
    this.renderer.dispose();
  }

  public setSize(width: number, height: number): void {
    this.width = width;
    this.height = height;
    this.gBufferRenderTarget.setSize(width, height);
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

  public setQualityLevel(qualityLevel: QualityLevel): void {
    this.qualityLevel = qualityLevel;
    switch (qualityLevel) {
      default:
      case QualityLevel.HIGHEST:
        this.updateParameters({
          deferredShadowAndAoFrame: 0,
          shadowAndAoFadeInFrames: 0,
          hardShadowOnCameraChange: false,
          shAndAoPassParameters: {
            enabled: true,
            shAndAo: {
              aoAndSoftShadowFxaa: true,
            },
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
          shAndAoPassParameters: {
            enabled: true,
            shAndAo: {
              aoAndSoftShadowFxaa: true,
            },
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
          shAndAoPassParameters: {
            enabled: true,
            shAndAo: {
              aoAndSoftShadowFxaa: false,
            },
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
          shAndAoPassParameters: {
            enabled: false,
            shAndAo: {
              aoAndSoftShadowFxaa: false,
            },
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

  public getQualityLevel() {
    return this.qualityLevel;
  }

  public forceShadowUpdates(updateBakedGroundShadow: boolean): void {
    this.shadowAndAoPass.needsUpdate = true;
    this.screenSpaceShadow.needsUpdate = true;
    if (updateBakedGroundShadow) {
      this.bakedGroundContactShadow.needsUpdate = true;
    }
  }

  public updateParameters(parameters: any) {
    if (parameters.shAndAoPassParameters !== undefined) {
      this.shadowAndAoPass.updateParameters(parameters.shAndAoPassParameters);
    }
    if (parameters.effectParameters !== undefined) {
      this.shadowAndAoPass.updateParameters({
        effectParameters: parameters.effectParameters,
      });
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
    this.environmentLights = false;
    this.screenSpaceShadow.addRectAreaLight(rectAreaLight, parent);
    this.shadowAndAoPass.needsUpdate = true;
  }

  public updateRectAreaLights(
    rectAreaLights: RectAreaLight[],
    parent: Object3D
  ): void {
    if (rectAreaLights.length > 0) {
      this.environmentLights = false;
    }
    this.screenSpaceShadow.updateRectAreaLights(rectAreaLights, parent);
    this.shadowAndAoPass.needsUpdate = true;
  }

  public createShadowFromEnvironmentMap(
    parent: Object3D,
    equirectangularTexture: Texture,
    textureData?: any
  ) {
    const lightSourceDetector = new LightSourceDetector();
    lightSourceDetector.detectLightSources(
      this.renderer,
      equirectangularTexture,
      textureData
    );
    this.createShadowFromLightSources(parent, lightSourceDetector.lightSources);
  }

  public createShadowFromLightSources(
    parent: Object3D,
    lightSources: LightSource[]
  ): void {
    this.environmentLights = true;
    this.screenSpaceShadow.createShadowFromLightSources(parent, lightSources);
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

  public updateNearAndFarPlaneOfPerspectiveCamera(
    camera: PerspectiveCamera,
    minimumFar?: number
  ) {
    // bring the near and far plane as close as possible to geometry
    // this is very likely the most important part for a glitch free and nice SSAO
    const nearFar = this.boundingVolume.getNearAndFarForPerspectiveCamera(
      camera.position,
      3
    );
    camera.near = Math.max(0.00001, nearFar[0] * 0.9);
    camera.far = Math.max(minimumFar ?? camera.near, nearFar[1]);
    camera.updateProjectionMatrix();
  }

  private setRenderState(scene: Scene, camera: Camera) {
    const needRealismEffects =
      this.shadowAndAoPass.parameters.aoType ===
        AmbientOcclusionType.EffectsSSAO ||
      this.shadowAndAoPass.parameters.aoType ===
        AmbientOcclusionType.EffectsHBAO;
    const debugModeChanged: boolean = this.debugOutput !== this.prevDebugOutput;
    this.prevDebugOutput = this.debugOutput;
    const copyToSeparateDepthBuffer =
      this.gBufferRenderTarget.copyToSeparateDepthBuffer;
    this.gBufferRenderTarget.copyToSeparateDepthBuffer =
      needRealismEffects ||
      this.shadowAndAoPass.parameters.enablePoissionDenoise;
    if (
      this.gBufferRenderTarget.copyToSeparateDepthBuffer &&
      !copyToSeparateDepthBuffer
    ) {
      this.gBufferRenderTarget.needsUpdate = true;
      this.shadowAndAoPass.needsUpdate = true;
    }
    this.screenSpaceShadow.parameters.alwaysUpdate =
      this.shadowAndAoPass.parameters.alwaysUpdate;
    this.cameraChanged = this.cameraUpdate.changed(camera);
    this.gBufferRenderTarget.needsUpdate ||=
      this.cameraChanged || debugModeChanged;
    if (this.gBufferRenderTarget.needsUpdate) {
      if (this.shadowAndAoPass.parameters.aoOnGround) {
        this.gBufferRenderTarget.render(this.renderer, scene, camera);
      } else {
        this.shadowRenderOverrideVisibility.render(scene, () => {
          this.gBufferRenderTarget.render(this.renderer, scene, camera);
        });
      }
    }
  }

  public render(
    scene: Scene,
    camera: Camera,
    renderPostEffects: boolean
  ): void {
    this.setRenderState(scene, camera);
    this.outlineRenderer.updateOutline(
      scene,
      camera,
      this.movingCamera ? [] : this.selectedObjects
    );
    this.renderer.setRenderTarget(null);
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

  private renderDebug(camera: Camera) {
    if (
      this.debugOutput &&
      this.debugOutput !== '' &&
      this.debugOutput !== 'off'
    ) {
      this.debugPass = this.debugPass ?? new DebugPass(this);
      this.debugPass.render(this.renderer, camera, this.debugOutput);
    }
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
      this.shadowAndAoPass.parameters.shAndAo.shadowIntensity * shadowScale;
    this.renderPass.renderScreenSpace(
      this.renderer,
      this.getCopyMaterial({
        texture: this.screenSpaceShadow.shadowTexture,
        blending: CustomBlending,
        colorTransform: new Matrix4().set(
          // eslint-disable-next-line prettier/prettier
          shIntensity, 0, 0, 1 - shIntensity,
          // eslint-disable-next-line prettier/prettier
          0, shIntensity, 0, 1,
          // eslint-disable-next-line prettier/prettier
          0, 0, 0, 1,
          // eslint-disable-next-line prettier/prettier
          0, 0, 0, 1
        ),
        colorBase: ZERO_RGBA,
        multiplyChannels: 1,
        uvTransform: DEFAULT_UV_TRANSFORM,
      }),
      null
    );
  }

  private evaluateIfShadowAndAoUpdateIsNeeded(_camera: Camera) {
    const updateNow =
      this.shadowAndAoPass.parameters.alwaysUpdate ||
      this.screenSpaceShadow.needsUpdate ||
      this.screenSpaceShadow.shadowTypeNeedsUpdate;
    let needsUpdate =
      this.shadowAndAoPass.parameters.enabled &&
      (this.parameters.deferredShadowAndAoFrame === 0 || this.cameraChanged);
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

class DebugPass {
  private _sceneRenderer: SceneRenderer;
  private copyMaterial?: CopyTransformMaterial;
  private depthRenderMaterial?: LinearDepthRenderMaterial;
  private renderPass: RenderPass = new RenderPass();

  constructor(sceneRenderer: SceneRenderer) {
    this._sceneRenderer = sceneRenderer;
  }

  private get gBufferRenderTarget(): GBufferRenderTargets {
    return this._sceneRenderer.gBufferRenderTarget;
  }

  private get screenSpaceShadow(): ScreenSpaceShadowMap {
    return this._sceneRenderer.screenSpaceShadow;
  }

  private get shadowAndAoPass(): ShadowAndAoPass {
    return this._sceneRenderer.shadowAndAoPass;
  }

  private get groundReflectionPass(): GroundReflectionPass {
    return this._sceneRenderer.groundReflectionPass;
  }

  private get bakedGroundContactShadow(): BakedGroundContactShadow {
    return this._sceneRenderer.bakedGroundContactShadow;
  }

  public dispose(): void {
    this.depthRenderMaterial?.dispose();
    this.copyMaterial?.dispose();
  }

  protected getCopyMaterial(parameters?: any): ShaderMaterial {
    this.copyMaterial = this.copyMaterial ?? new CopyTransformMaterial();
    return this.copyMaterial.update(parameters);
  }

  private getDepthRenderMaterial(camera: Camera): LinearDepthRenderMaterial {
    this.depthRenderMaterial =
      this.depthRenderMaterial ??
      new LinearDepthRenderMaterial({
        depthTexture: this.gBufferRenderTarget.textureWithDepthValue,
        depthFilter: this.gBufferRenderTarget
          .isFloatGBufferWithRgbNormalAlphaDepth
          ? new Vector4(0, 0, 0, 1)
          : new Vector4(1, 0, 0, 0),
      });
    return this.depthRenderMaterial.update({ camera });
  }

  // eslint-disable-next-line complexity
  public render(
    renderer: WebGLRenderer,
    camera: Camera,
    debugOutput: string
  ): void {
    switch (debugOutput) {
      default:
        break;
      case 'lineardepth':
        this.renderPass.renderScreenSpace(
          renderer,
          this.getDepthRenderMaterial(camera),
          null
        );
        break;
      case 'g-normal':
        if (this.gBufferRenderTarget.isFloatGBufferWithRgbNormalAlphaDepth) {
          this.renderPass.renderScreenSpace(
            renderer,
            this.getCopyMaterial({
              texture: this.gBufferRenderTarget?.gBufferTexture,
              blending: NoBlending,
              colorTransform: new Matrix4().set(
                // eslint-disable-next-line prettier/prettier
                0.5, 0, 0, 0,
                // eslint-disable-next-line prettier/prettier
                0, 0.5, 0, 0,
                // eslint-disable-next-line prettier/prettier
                0, 0, 0.5, 0,
                // eslint-disable-next-line prettier/prettier
                0, 0, 0, 0
              ),
              colorBase: new Vector4(0.5, 0.5, 0.5, 1),
              multiplyChannels: 0,
              uvTransform: DEFAULT_UV_TRANSFORM,
            }),
            null
          );
        } else {
          this.renderPass.renderScreenSpace(
            renderer,
            this.getCopyMaterial({
              texture: this.gBufferRenderTarget?.gBufferTexture,
              blending: NoBlending,
              colorTransform: RGB_TRANSFORM,
              colorBase: ALPHA_RGBA,
              multiplyChannels: 0,
              uvTransform: DEFAULT_UV_TRANSFORM,
            }),
            null
          );
        }
        break;
      case 'g-depth':
        if (this.gBufferRenderTarget.isFloatGBufferWithRgbNormalAlphaDepth) {
          this.renderPass.renderScreenSpace(
            renderer,
            this.getCopyMaterial({
              texture: this.gBufferRenderTarget?.gBufferTexture,
              blending: NoBlending,
              colorTransform: ALPHA_TRANSFORM,
              colorBase: ALPHA_RGBA,
              multiplyChannels: 0,
              uvTransform: DEFAULT_UV_TRANSFORM,
            }),
            null
          );
        } else {
          this.renderPass.renderScreenSpace(
            renderer,
            this.getCopyMaterial({
              texture: this.gBufferRenderTarget?.depthBufferTexture,
              blending: NoBlending,
              colorTransform: RED_TRANSFORM,
              colorBase: ALPHA_RGBA,
              multiplyChannels: 0,
              uvTransform: DEFAULT_UV_TRANSFORM,
            }),
            null
          );
        }
        break;
      case 'ssao':
        this.renderPass.renderScreenSpace(
          renderer,
          this.getCopyMaterial({
            texture:
              this.shadowAndAoPass.shadowAndAoRenderTargets.passRenderTarget
                .texture,
            blending: NoBlending,
            colorTransform: GRAYSCALE_TRANSFORM,
            colorBase: ZERO_RGBA,
            multiplyChannels: 0,
            uvTransform: DEFAULT_UV_TRANSFORM,
          }),
          null
        );
        break;
      case 'ssaodenoise':
        this.renderPass.renderScreenSpace(
          renderer,
          this.getCopyMaterial({
            texture: this.shadowAndAoPass.denoiseRenderTargetTexture,
            blending: NoBlending,
            colorTransform: GRAYSCALE_TRANSFORM,
            colorBase: ZERO_RGBA,
            multiplyChannels: 0,
            uvTransform: DEFAULT_UV_TRANSFORM,
          }),
          null
        );
        break;
      case 'shadowmap':
        this.renderPass.renderScreenSpace(
          renderer,
          this.getCopyMaterial({
            texture: this.screenSpaceShadow.shadowTexture,
            blending: NoBlending,
            colorTransform: GRAYSCALE_TRANSFORM,
            colorBase: ZERO_RGBA,
            multiplyChannels: 0,
            uvTransform: DEFAULT_UV_TRANSFORM,
          }),
          null
        );
        break;
      case 'shadow':
        this.renderPass.renderScreenSpace(
          renderer,
          this.getCopyMaterial({
            texture:
              this.shadowAndAoPass.shadowAndAoRenderTargets.passRenderTarget
                .texture,
            blending: NoBlending,
            colorTransform: ShadowAndAoPass.shadowTransform,
            colorBase: ZERO_RGBA,
            multiplyChannels: 0,
            uvTransform: DEFAULT_UV_TRANSFORM,
          }),
          null
        );
        break;
      case 'shadowblur':
        this.renderPass.renderScreenSpace(
          renderer,
          this.getCopyMaterial({
            texture: this.shadowAndAoPass.denoiseRenderTargetTexture,
            blending: NoBlending,
            colorTransform: ShadowAndAoPass.shadowTransform,
            colorBase: ZERO_RGBA,
            multiplyChannels: 0,
            uvTransform: DEFAULT_UV_TRANSFORM,
          }),
          null
        );
        break;
      case 'groundshadow':
        this.renderPass.renderScreenSpace(
          renderer,
          this.getCopyMaterial({
            texture: this.groundReflectionPass.shadowRenderTarget.texture,
            blending: NoBlending,
            colorTransform: DEFAULT_TRANSFORM,
            colorBase: ZERO_RGBA,
            multiplyChannels: 0,
            uvTransform: DEFAULT_UV_TRANSFORM,
          }),
          null
        );
        break;
      case 'groundreflection':
        this.renderPass.renderScreenSpace(
          renderer,
          this.getCopyMaterial({
            texture: this.groundReflectionPass.reflectionRenderTarget.texture,
            blending: NoBlending,
            colorTransform: DEFAULT_TRANSFORM,
            colorBase: ZERO_RGBA,
            multiplyChannels: 0,
            uvTransform: FLIP_Y_UV_TRANSFORM,
          }),
          null
        );
        break;
      case 'bakedgroundshadow':
        this.renderPass.renderScreenSpace(
          renderer,
          this.getCopyMaterial({
            texture: this.bakedGroundContactShadow.renderTarget.texture,
            blending: NoBlending,
            colorTransform: DEFAULT_TRANSFORM,
            colorBase: ZERO_RGBA,
            multiplyChannels: 0,
            uvTransform: DEFAULT_UV_TRANSFORM,
          }),
          null
        );
        break;
    }
  }
}
