import {
  CameraUpdate,
  getMaxSamples,
  RenderOverrideVisibility,
  RenderPass,
} from './render-utility';
import {
  CopyTransformMaterial,
  LinearDepthRenderMaterial,
} from './shader-utility';
import { DepthNormalRenderTarget } from './depth-normal-render-targets';
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
  Camera,
  BufferGeometry,
  CustomBlending,
  DoubleSide,
  Group,
  Material,
  Matrix4,
  Mesh,
  MeshStandardMaterial,
  NearestFilter,
  NoBlending,
  Object3D,
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
  public outputEncoding = '';
  public toneMapping = '';
  public groundReflection = false;
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
  public groundGroup: Group = new Group();
  private groundMesh?: Mesh;

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
      {}
    );
    this.bakedGroundContactShadow = new BakedGroundContactShadow(
      this.renderer,
      this.groundGroup,
      {}
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
      {}
    );
    this.shadowRenderOverrideVisibility = new RenderOverrideVisibility(false, [
      this.groundGroup,
    ]);
    this.sceneGroups.addToGroup(SceneGroupType.GROUND, this.groundGroup);
    this.parameters = {
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
          shadowAndAoFadeInFrames: 3,
          hardShadowOnCameraChange: true,
          shadowAndAoParameters: {
            aoAndSoftShadowEnabled: true,
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

  public forceShadowUpdates(): void {
    this.shadowAndAoPass.needsUpdate = true;
    this.screenSpaceShadow.needsUpdate = true;
    this.bakedGroundContactShadow.needsUpdate = true;
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

  public setGroundLevel(groundLevel: number) {
    this.groundGroup.position.y = groundLevel;
    this.groundReflectionPass.parameters.groundLevel = groundLevel;
  }

  public updateGround(geometry?: BufferGeometry, material?: Material) {
    if (!this.groundMesh) {
      this.groundMesh = new Mesh();
      this.groundGroup.add(this.groundMesh);
    }
    if (geometry) {
      this.groundMesh.geometry = geometry;
    }
    if (material) {
      material.depthWrite = false;
      material.polygonOffset = true;
      material.polygonOffsetFactor = 4;
      material.polygonOffsetUnits = 4;
      material.needsUpdate = true;
      this.groundMesh.material = material;
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

  public render(
    scene: Scene,
    camera: Camera,
    renderPostEffects: boolean
  ): void {
    //this.renderer.info.autoReset = false;
    //this.renderer.info.reset();
    this.screenSpaceShadow.parameters.alwaysUpdate =
      this.shadowAndAoPass.parameters.aoAlwaysUpdate;
    this.outlineRenderer.updateOutline(scene, camera, this.selectedObjects);
    this.renderer.setRenderTarget(null);
    this.depthNormalRenderTarget.needsUpdate = true;
    this.bakedGroundContactShadow.render(scene, this.groundMesh);
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
        const shIntensity = this.shadowAndAoPass.parameters.shadowIntensity;
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
      });
    }
  }

  private evaluateIfShadowAndAoUpdateIsNeeded(camera: Camera) {
    const alwaysUpdate = this.shadowAndAoPass.parameters.aoAlwaysUpdate;
    let needsUpdate =
      this.shadowAndAoPass.parameters.aoAndSoftShadowEnabled &&
      (this.parameters.deferredShadowAndAoFrame === 0 ||
        this.cameraUpdate.changed(camera));
    if (needsUpdate) {
      this.noUpdateNeededCount = 0;
    }
    let intensityScale = 1;
    if (!alwaysUpdate && this.parameters.deferredShadowAndAoFrame > 0) {
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
    needsUpdate = alwaysUpdate || needsUpdate;
    const hardShadow = !needsUpdate && this.parameters.hardShadowOnCameraChange;
    return { needsUpdate, hardShadow, intensityScale };
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
            texture:
              this.shadowAndAoPass.shadowAndAoRenderTargets.blurRenderTarget
                .texture,
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
            texture:
              this.shadowAndAoPass.shadowAndAoRenderTargets.blurRenderTarget
                .texture,
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
