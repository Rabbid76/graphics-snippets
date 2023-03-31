import { RectAreaLightHelper } from 'three/examples/jsm/helpers/RectAreaLightHelper.js';
import {
  CameraUpdate,
  RenderPass,
  setOrthographicViewVolumeFromBox,
} from './render-utility';
import {
  BasicShadowMap,
  Box3,
  Camera,
  DirectionalLight,
  DoubleSide,
  Layers,
  Light,
  LineBasicMaterial,
  Material,
  MeshLambertMaterial,
  MeshPhongMaterial,
  Object3D,
  PCFShadowMap,
  PCFSoftShadowMap,
  RectAreaLight,
  RedFormat,
  Scene,
  ShadowMapType,
  Texture,
  VSMShadowMap,
  Vector2,
  Vector3,
  WebGLRenderer,
  WebGLRenderTarget,
} from 'three';

export interface ScreenSpaceShadowMapParameters {
  alwaysUpdate: boolean;
  enableShadowMap: boolean;
  layers: Layers | null;
}

export interface ShadowLightSource {
  getShadowLight(): DirectionalLight;
  getOriginalLight(): Light | null;
  dispose(): void;
  addTo(object: Object3D): void;
  removeFrom(object: Object3D): void;
  updatePositionAndTarget(): void;
  setShadowVolume(shadowVolumeBox: Box3): void;
  getShadowLightIntensity(): number;
  prepareRenderShadow(
    castShadow: boolean,
    sumOfShadowLightIntensity: number
  ): void;
  finishRenderShadow(): void;
  updateShadowType(typeParameters: ShadowTypeParameters): void;
}

export class RectAreaShadowLightSource implements ShadowLightSource {
  private _rectAreaLight: RectAreaLight;
  private _directionalLight: DirectionalLight;
  private _rectLightHelper?: RectAreaLightHelper;
  private shadowMapSize: number;
  private isVisibleBackup: boolean = true;
  private castShadowBackup: boolean = true;

  constructor(rectAreaLight: RectAreaLight, parameters: any) {
    this.shadowMapSize = parameters?.shadowMapSize ?? 1024;
    this._rectAreaLight = rectAreaLight;
    if (parameters?.addHelper) {
      this._rectLightHelper = new RectAreaLightHelper(this._rectAreaLight);
      (this._rectLightHelper.material as LineBasicMaterial).depthWrite = false;
      this._rectAreaLight.add(this._rectLightHelper);
    }
    this._directionalLight = new DirectionalLight(0xffffff, 1);
    this._directionalLight.position.set(
      this._rectAreaLight.position.x,
      this._rectAreaLight.position.y,
      this._rectAreaLight.position.z
    );
    this._directionalLight.lookAt(0, 0, 0);
    this._directionalLight.visible = false;
    this._directionalLight.castShadow = true;
    this._directionalLight.shadow.mapSize = new Vector2(
      this.shadowMapSize,
      this.shadowMapSize
    );
  }

  public getRectAreaLight(): RectAreaLight {
    return this._rectAreaLight;
  }

  public getShadowLight(): DirectionalLight {
    return this._directionalLight;
  }

  public getOriginalLight(): Light | null {
    return this._rectAreaLight;
  }

  public dispose(): void {
    this._directionalLight.dispose();
  }

  addTo(parent: Object3D): void {
    parent.add(this._directionalLight);
  }

  removeFrom(parent: Object3D): void {
    parent.remove(this._directionalLight);
  }

  public updatePositionAndTarget() {
    this.updateShadowPositionAndTarget(
      this._rectAreaLight.position,
      new Vector3(0, 0, 0)
    );
  }

  public setShadowVolume(shadowVolumeBox: Box3): void {
    setOrthographicViewVolumeFromBox(
      this._directionalLight.shadow.camera,
      shadowVolumeBox
    );
  }

  private updateShadowPositionAndTarget(
    cameraPosition: Vector3,
    targetPosition: Vector3
  ): void {
    this._directionalLight.position.set(
      cameraPosition.x,
      cameraPosition.y,
      cameraPosition.z
    );
    this._directionalLight.lookAt(
      targetPosition.x,
      targetPosition.y,
      targetPosition.z
    );
    this._directionalLight.shadow.camera.position.set(
      cameraPosition.x,
      cameraPosition.y,
      cameraPosition.z
    );
    this._directionalLight.shadow.camera.lookAt(
      targetPosition.x,
      targetPosition.y,
      targetPosition.z
    );
    this._directionalLight.updateMatrixWorld();
    this._directionalLight.shadow.camera.updateMatrixWorld();
  }

  public getShadowLightIntensity(): number {
    return this._rectAreaLight.visible ? this._rectAreaLight.intensity : 0;
  }

  public prepareRenderShadow(
    castShadow: boolean,
    sumOfShadowLightIntensity: number
  ): void {
    this.castShadowBackup = this._directionalLight.castShadow;
    this.isVisibleBackup = this._rectAreaLight.visible;
    this._directionalLight.visible = this._rectAreaLight.visible;
    this._directionalLight.castShadow &&= castShadow;
    this._rectAreaLight.visible = false;
    this._directionalLight.intensity =
      this._rectAreaLight.intensity / sumOfShadowLightIntensity;
  }

  public finishRenderShadow(): void {
    this._directionalLight.visible = false;
    this._directionalLight.castShadow = this.castShadowBackup;
    this._rectAreaLight.visible = this.isVisibleBackup;
  }

  public updateShadowType(typeParameters: ShadowTypeParameters): void {
    const shadow = this._directionalLight.shadow;
    shadow.bias = typeParameters.bias;
    shadow.normalBias = typeParameters.normalBias;
    shadow.radius = typeParameters.radius;
    shadow.needsUpdate = true;
  }
}

const replaceLightsLambertParsFragment = `
varying vec3 vViewPosition;

struct LambertMaterial {

	vec3 diffuseColor;
	float specularStrength;

};

void RE_Direct_Lambert( const in IncidentLight directLight, const in GeometricContext geometry, const in LambertMaterial material, inout ReflectedLight reflectedLight ) {

	float dotNL = saturate( dot( geometry.normal, directLight.direction ) );
  float nonLinearDotNL = min(dotNL * 1000.0, 1.0);
	vec3 irradiance = nonLinearDotNL * directLight.color;

	reflectedLight.directDiffuse += irradiance * BRDF_Lambert( material.diffuseColor );

}

void RE_IndirectDiffuse_Lambert( const in vec3 irradiance, const in GeometricContext geometry, const in LambertMaterial material, inout ReflectedLight reflectedLight ) {

	reflectedLight.indirectDiffuse += irradiance * BRDF_Lambert( material.diffuseColor );

}

#define RE_Direct				RE_Direct_Lambert
#define RE_IndirectDiffuse		RE_IndirectDiffuse_Lambert
`;

export class ScreenSpaceShadowMap {
  static useModifiedMaterial: boolean = true;
  public parameters: ScreenSpaceShadowMapParameters;
  public needsUpdate: boolean = false;
  public shadowTypeNeedsUpdate: boolean = true;
  public shadowConfiguration = new ShadowTypeConfiguration();
  private shadowLightSources: ShadowLightSource[] = [];
  private viewportSize: Vector2;
  private samples: number;
  private shadowMapSize: number;
  public castShadow: boolean;
  private renderPass: RenderPass;
  private shadowMaterial: Material;
  private shadowRenderTarget: WebGLRenderTarget;
  private cameraUpdate: CameraUpdate = new CameraUpdate();

  public get shadowTexture(): Texture {
    return this.shadowRenderTarget.texture;
  }

  constructor(viewportSize: Vector2, parameters: any) {
    this.viewportSize = new Vector2(viewportSize.x, viewportSize.y);
    this.samples = parameters?.samples ?? 0;
    this.shadowMapSize = parameters?.shadowMapSize ?? 1024;
    this.parameters = this.getScreenSpaceShadowMapParameters(parameters);
    this.castShadow = this.parameters.enableShadowMap;
    this.renderPass = new RenderPass();
    if (ScreenSpaceShadowMap.useModifiedMaterial) {
      this.shadowMaterial = new MeshLambertMaterial({
        side: DoubleSide,
      });
      this.shadowMaterial.onBeforeCompile = (shader) => {
        shader.fragmentShader = shader.fragmentShader.replace(
          '#include <lights_lambert_pars_fragment>',
          replaceLightsLambertParsFragment
        );
      };
    } else {
      this.shadowMaterial = new MeshPhongMaterial({
        color: 0xffffff,
        shininess: 0,
        polygonOffsetFactor: 0,
        polygonOffsetUnits: 0,
        side: DoubleSide,
      });
    }
    const samples = this.samples;
    this.shadowRenderTarget = new WebGLRenderTarget(
      this.viewportSize.x,
      this.viewportSize.y,
      { samples, format: RedFormat }
    );
  }

  private getScreenSpaceShadowMapParameters(
    parameters?: any
  ): ScreenSpaceShadowMapParameters {
    return {
      alwaysUpdate: parameters?.alwaysUpdate ?? false,
      enableShadowMap: parameters?.enableShadowMap ?? true,
      layers: parameters.layers ?? null,
    };
  }

  public dispose(): void {
    this.shadowLightSources.forEach((item) => item.dispose());
    this.shadowMaterial.dispose();
    this.shadowRenderTarget.dispose();
  }

  public updateParameters(parameters: any) {
    if (parameters.alwaysUpdate !== undefined) {
      this.parameters.alwaysUpdate = parameters.alwaysUpdate;
    }
    if (parameters.enableShadowMap !== undefined) {
      this.parameters.enableShadowMap = parameters.enableShadowMap;
    }
    if (parameters.layers !== undefined) {
      this.parameters.layers = parameters.layers;
    }
  }

  public getShadowLightSources(): Light[] {
    const lightSources: Light[] = this.shadowLightSources.map((item) =>
      item.getShadowLight()
    );
    return lightSources;
  }

  public findShadowLightSource(lightSource: Light): Light | undefined {
    return this.shadowLightSources
      .find((item) => item.getOriginalLight() === lightSource)
      ?.getShadowLight();
  }

  public addRectAreaLight(
    rectAreaLight: RectAreaLight,
    parent: Object3D
  ): void {
    const rectAreaLightShadow = new RectAreaShadowLightSource(rectAreaLight, {
      shadowMapSize: this.shadowMapSize,
    });
    this.shadowLightSources.push(rectAreaLightShadow);
    rectAreaLightShadow.updatePositionAndTarget();
    rectAreaLightShadow.addTo(parent);
  }

  public updateRectAreaLights(
    rectAreaLights: RectAreaLight[],
    parent: Object3D
  ): void {
    this.shadowLightSources = this.shadowLightSources.filter((item) => {
      if (item instanceof RectAreaShadowLightSource) {
        const light = item.getRectAreaLight();
        if (rectAreaLights.includes(light)) {
          item.updatePositionAndTarget();
          return true;
        } else {
          item.removeFrom(parent);
          item.dispose();
          return false;
        }
      }
      return true;
    });
    rectAreaLights.forEach((light) => {
      if (
        !this.shadowLightSources.find(
          (item) =>
            item instanceof RectAreaShadowLightSource &&
            item.getRectAreaLight() === light
        )
      ) {
        this.addRectAreaLight(light, parent);
      }
    });
  }

  public setSize(width: number, height: number): void {
    this.viewportSize = new Vector2(width, height);
    this.shadowRenderTarget.setSize(this.viewportSize.x, this.viewportSize.y);
  }

  public updatePositionAndTarget(): void {
    this.shadowLightSources.forEach((item) => item.updatePositionAndTarget());
  }

  public setShadowVolume(shadowVolumeBox: Box3): void {
    this.shadowLightSources.forEach((item) =>
      item.setShadowVolume(shadowVolumeBox)
    );
  }

  public renderShadowMap(
    renderer: WebGLRenderer,
    scene: Scene,
    camera: Camera
  ): void {
    const needsUpdate =
      this.needsUpdate ||
      this.parameters.alwaysUpdate ||
      this.cameraUpdate.changed(camera);
    if (!needsUpdate) {
      return;
    }
    this.needsUpdate = false;
    if (this.shadowTypeNeedsUpdate) {
      this.shadowTypeNeedsUpdate = false;
      this.needsUpdate = true;
      this.updateShadowType(renderer);
    }
    let sumOfShadowLightIntensity = 0;
    this.shadowLightSources.forEach(
      (item) => (sumOfShadowLightIntensity += item.getShadowLightIntensity())
    );
    this.shadowLightSources.forEach((item) =>
      item.prepareRenderShadow(this.castShadow, sumOfShadowLightIntensity)
    );
    const layersMaskBackup = camera.layers.mask;
    if (this.parameters.layers) {
      camera.layers.mask = this.parameters.layers.mask;
    }
    this.renderShadowMapToTarget(
      renderer,
      scene,
      camera,
      this.shadowRenderTarget
    );
    camera.layers.mask = layersMaskBackup;
    this.shadowLightSources.forEach((item) => item.finishRenderShadow());
  }

  private renderShadowMapToTarget(
    renderer: WebGLRenderer,
    scene: Scene,
    camera: Camera,
    renderTarget: WebGLRenderTarget
  ): void {
    const materialCache = new Map();
    scene.traverse((object: any) => {
      if (
        object.isMesh &&
        object?.material &&
        !Array.isArray(object.material) &&
        !object.material.transparent
      ) {
        materialCache.set(object, object.material);
        object.material = this.shadowMaterial;
      }
    });
    this.renderPass.render(renderer, scene, camera, renderTarget, 0xffffff, 1);
    materialCache.forEach((material: any, object: any) => {
      object.material = material;
    });
  }

  private updateShadowType(renderer: WebGLRenderer): void {
    renderer.shadowMap.type = this.shadowConfiguration.currentType.type;
    const castShadow =
      this.parameters.enableShadowMap &&
      this.shadowConfiguration.currentType.castShadow;
    renderer.shadowMap.enabled = castShadow;
    renderer.shadowMap.needsUpdate = true;
    this.castShadow =
      castShadow && this.shadowConfiguration.currentType.castShadow;
    this.shadowLightSources.forEach((item) =>
      item.updateShadowType(this.shadowConfiguration.currentType)
    );
  }
}

export interface ShadowTypeParameters {
  castShadow: boolean;
  type: ShadowMapType;
  bias: number;
  normalBias: number;
  radius: number;
}

export class ShadowTypeConfiguration {
  private static defaultType: ShadowTypeParameters = {
    castShadow: true,
    type: PCFShadowMap,
    bias: -0.00005,
    normalBias: 0.01,
    radius: 0,
  };
  public types = new Map<string, ShadowTypeParameters>([
    [
      'off',
      {
        castShadow: false,
        type: PCFShadowMap,
        bias: 0,
        normalBias: 0,
        radius: 0,
      },
    ],
    [
      'BasicShadowMap',
      {
        castShadow: true,
        type: BasicShadowMap,
        bias: -0.005,
        normalBias: 0,
        radius: 0,
      },
    ],
    ['PCFShadowMap', ShadowTypeConfiguration.defaultType],
    [
      'PCFSoftShadowMap',
      {
        castShadow: true,
        type: PCFSoftShadowMap,
        bias: -0.0005,
        normalBias: 0.01,
        radius: 1,
      },
    ],
    [
      'VSMShadowMap',
      {
        castShadow: true,
        type: VSMShadowMap,
        bias: -0.0005,
        normalBias: 0,
        radius: 30,
      },
    ],
  ]);
  public shadowType: string = 'PCFShadowMap';
  public currentType: ShadowTypeParameters =
    this.types.get('PCFShadowMap') ?? ShadowTypeConfiguration.defaultType;
}
