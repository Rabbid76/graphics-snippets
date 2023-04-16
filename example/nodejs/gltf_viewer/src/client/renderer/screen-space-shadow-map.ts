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
  Mesh,
  MeshBasicMaterial,
  MeshLambertMaterial,
  MeshPhongMaterial,
  MeshStandardMaterial,
  Object3D,
  PCFShadowMap,
  PCFSoftShadowMap,
  PointLight,
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
  [key: string]: any;
  alwaysUpdate: boolean;
  enableShadowMap: boolean;
  layers: Layers | null;
}

export interface ShadowLightSource {
  getShadowLight(): Light;
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
  updateShadowType(typeParameters: ShadowTypeParameters, shadowScale: number): void;
}

export class RectAreaShadowLightSource implements ShadowLightSource {
  private _rectAreaLight: RectAreaLight;
  private _shadowLightSource: DirectionalLight;
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
    this._shadowLightSource = new DirectionalLight(0xffffff, 1);
    this._shadowLightSource.position.copy(this._rectAreaLight.position);
    this._shadowLightSource.lookAt(0, 0, 0);
    this._shadowLightSource.visible = false;
    this._shadowLightSource.castShadow = true;
    this._shadowLightSource.shadow.mapSize = new Vector2(
      this.shadowMapSize,
      this.shadowMapSize
    );
  }

  public getRectAreaLight(): RectAreaLight {
    return this._rectAreaLight;
  }

  public getShadowLight(): Light {
    return this._shadowLightSource;
  }

  public getOriginalLight(): Light | null {
    return this._rectAreaLight;
  }

  public dispose(): void {
    this._shadowLightSource.dispose();
  }

  addTo(parent: Object3D): void {
    parent.add(this._shadowLightSource);
  }

  removeFrom(parent: Object3D): void {
    parent.remove(this._shadowLightSource);
  }

  public updatePositionAndTarget() {
    this.updateShadowPositionAndTarget(
      this._rectAreaLight.position,
      new Vector3(0, 0, 0)
    );
  }

  public setShadowVolume(shadowVolumeBox: Box3): void {
    // TODO setViewVolumeFromBox perspective and orthographic
    setOrthographicViewVolumeFromBox(
      this._shadowLightSource.shadow.camera,
      shadowVolumeBox
    );
  }

  private updateShadowPositionAndTarget(
    cameraPosition: Vector3,
    targetPosition: Vector3
  ): void {
    this._shadowLightSource.position.copy(cameraPosition);
    this._shadowLightSource.lookAt(targetPosition);
    this._shadowLightSource.shadow.camera.position.copy(targetPosition);
    this._shadowLightSource.shadow.camera.lookAt(targetPosition);
    this._shadowLightSource.updateMatrixWorld();
    this._shadowLightSource.shadow.camera.updateMatrixWorld();
  }

  public getShadowLightIntensity(): number {
    return this._rectAreaLight.visible ? this._rectAreaLight.intensity : 0;
  }

  public prepareRenderShadow(
    castShadow: boolean,
    sumOfShadowLightIntensity: number
  ): void {
    this.castShadowBackup = this._shadowLightSource.castShadow;
    this.isVisibleBackup = this._rectAreaLight.visible;
    this._shadowLightSource.visible = this._rectAreaLight.visible;
    this._shadowLightSource.castShadow &&= castShadow;
    this._rectAreaLight.visible = false;
    this._shadowLightSource.intensity =
      this._rectAreaLight.intensity / sumOfShadowLightIntensity;
  }

  public finishRenderShadow(): void {
    this._shadowLightSource.visible = false;
    this._shadowLightSource.castShadow = this.castShadowBackup;
    this._rectAreaLight.visible = this.isVisibleBackup;
  }

  public updateShadowType(typeParameters: ShadowTypeParameters, shadowScale: number): void {
    const shadow = this._shadowLightSource.shadow;
    shadow.bias = typeParameters.bias;
    shadow.normalBias = typeParameters.normalBias * shadowScale;
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

	float dotNL = dot(geometry.normal, directLight.direction);
  float nonLinearDotNL = saturate(min(dotNL * 1000.0 + 0.9, 1.0));
	vec3 irradiance = nonLinearDotNL * directLight.color;
	reflectedLight.directDiffuse += irradiance * BRDF_Lambert( material.diffuseColor );
}

void RE_IndirectDiffuse_Lambert( const in vec3 irradiance, const in GeometricContext geometry, const in LambertMaterial material, inout ReflectedLight reflectedLight ) {
	reflectedLight.indirectDiffuse += irradiance * BRDF_Lambert( material.diffuseColor );
}

#define RE_Direct				RE_Direct_Lambert
#define RE_IndirectDiffuse		RE_IndirectDiffuse_Lambert
`;

enum ShadowMaterialType {
  Default,
  Unlit,
  Emissive,
}

export class ScreenSpaceShadowMap {
  static useModifiedMaterial: boolean = true;
  public parameters: ScreenSpaceShadowMapParameters;
  public needsUpdate: boolean = false;
  public shadowTypeNeedsUpdate: boolean = true;
  public shadowConfiguration = new ShadowTypeConfiguration();
  private shadowLightSources: ShadowLightSource[] = [];
  private viewportSize: Vector2;
  private samples: number;
  private sceneBounds: Box3 = new Box3();
  private shadowScale: number = 1;
  private shadowMapSize: number;
  public castShadow: boolean;
  private renderPass: RenderPass;
  private shadowMaterial: Material;
  private unlitMaterial: Material;
  private emissiveMaterial: Material;
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
    this.shadowMaterial = this.createShadowMaterial(ShadowMaterialType.Default);
    this.unlitMaterial = this.createShadowMaterial(ShadowMaterialType.Unlit);
    this.emissiveMaterial = this.createShadowMaterial(
      ShadowMaterialType.Emissive
    );
    const samples = this.samples;
    this.shadowRenderTarget = new WebGLRenderTarget(
      this.viewportSize.x,
      this.viewportSize.y,
      { samples, format: RedFormat }
    );
  }

  private createShadowMaterial(type: ShadowMaterialType): Material {
    let material: Material;
    if (type === ShadowMaterialType.Emissive) {
      material = new MeshBasicMaterial({
        color: 0xffffff,
        side: DoubleSide,
      });
    } else if (type === ShadowMaterialType.Unlit) {
      material = new MeshBasicMaterial({
        color: 0xffffff,
        side: DoubleSide,
      });
    } else if (ScreenSpaceShadowMap.useModifiedMaterial) {
      material = new MeshLambertMaterial({
        side: DoubleSide,
      });
      material.onBeforeCompile = (shader) => {
        shader.fragmentShader = shader.fragmentShader.replace(
          '#include <lights_lambert_pars_fragment>',
          replaceLightsLambertParsFragment
        );
      };
    } else {
      material = new MeshPhongMaterial({
        color: 0xffffff,
        shininess: 0,
        polygonOffsetFactor: 0,
        polygonOffsetUnits: 0,
        side: DoubleSide,
      });
    }
    return material;
  }

  private getScreenSpaceShadowMapParameters(
    parameters?: any
  ): ScreenSpaceShadowMapParameters {
    return {
      alwaysUpdate: false,
      enableShadowMap: true,
      layers: null,
      ...parameters,
    };
  }

  public dispose(): void {
    this.shadowLightSources.forEach((item) => item.dispose());
    this.shadowMaterial.dispose();
    this.shadowRenderTarget.dispose();
  }

  public updateParameters(parameters: any) {
    for (let propertyName in parameters) {
      if (this.parameters.hasOwnProperty(propertyName)) {
        this.parameters[propertyName] = parameters[propertyName];
      }
    }
  }

  public updateBounds(sceneBounds: Box3, scaleShadow: number) {
    const currentScale = this.shadowScale;
    this.sceneBounds.copy(sceneBounds);
    this.shadowScale = scaleShadow;
    if (Math.abs(currentScale - this.shadowScale) > 0.00001) {
      this.shadowTypeNeedsUpdate = true;
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
    this.needsUpdate = true;
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
    this.needsUpdate = true;
    this.shadowTypeNeedsUpdate = true;
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
    const objectCache = new Map();
    scene.traverse((object: any) => {
      if (object.isMesh && object.visible) {
        objectCache.set(object, {
          visible: object.visible,
          material: object.material,
        });
        this.setMeshMaterialAndVisibility(object as Mesh);
      } else if (object.isLine) {
        objectCache.set(object, {
          visible: object.visible,
          material: object.material,
        });
        object.visible = false;
      }
    });
    this.renderPass.render(renderer, scene, camera, renderTarget, 0xffffff, 1);
    objectCache.forEach((data: any, object: any) => {
      object.visible = data.visible;
      if (data.material) {
        object.material = data.material;
      }
    });
  }

  private setMeshMaterialAndVisibility(object: Mesh) {
    if (
      object.material &&
      object.castShadow &&
      !Array.isArray(object.material) &&
      !(object.material.transparent === true && object.material.opacity < 0.9)
    ) {
      const material = object.material;
      if (
        material instanceof LineBasicMaterial ||
        material instanceof MeshBasicMaterial
      ) {
        object.material = this.unlitMaterial;
      } else if (material instanceof MeshStandardMaterial) {
        this.setMeshShadowStandardMaterial(object, material);
      } else {
        object.material = object.receiveShadow
          ? this.shadowMaterial
          : this.unlitMaterial;
      }
    } else {
      object.visible = false;
    }
  }

  private setMeshShadowStandardMaterial(
    object: Mesh,
    material: MeshStandardMaterial
  ) {
    const isEmissive =
      material.emissiveIntensity > 0 &&
      (material.emissive.r > 0 ||
        material.emissive.g > 0 ||
        material.emissive.b > 0);
    object.material = isEmissive
      ? this.emissiveMaterial
      : object.receiveShadow
      ? this.shadowMaterial
      : this.unlitMaterial;
  }

  private updateShadowType(renderer: WebGLRenderer): void {
    renderer.shadowMap.type =
      this.shadowConfiguration.currentConfiguration.type;
    const castShadow =
      this.parameters.enableShadowMap &&
      this.shadowConfiguration.currentConfiguration.castShadow;
    renderer.shadowMap.enabled = castShadow;
    renderer.shadowMap.needsUpdate = true;
    this.castShadow =
      castShadow && this.shadowConfiguration.currentConfiguration.castShadow;
    this.shadowLightSources.forEach((item) =>
      item.updateShadowType(this.shadowConfiguration.currentConfiguration, this.shadowScale)
    );
  }

  public switchType(type: string): boolean {
    if (!this.shadowConfiguration.switchType(type)) {
      return false;
    }
    this.needsUpdate = true;
    this.shadowTypeNeedsUpdate = true;
    return true;
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
  // see THREE.LightShadow - https://threejs.org/docs/#api/en/lights/shadows/LightShadow
  // bias: Shadow map bias, how much to add or subtract from the normalized depth when deciding whether a surface is in shadow.
  //       This value depends on the normalized depth and must not be scaled with the size of the scene.
  // normalBias: Defines how much the position used to query the shadow map is offset along the object normal. 
  //       This value is in world space units and must be scaled with the size of the scene.
  private static noShadow: ShadowTypeParameters = {
    castShadow: false,
    type: PCFShadowMap,
    bias: 0,
    normalBias: 0,
    radius: 0,
  };
  private static basicShadow: ShadowTypeParameters = {
    castShadow: true,
    type: BasicShadowMap,
    bias: -0.00005,
    normalBias: 0.005,
    radius: 0,
  };
  private static pcfShadow: ShadowTypeParameters = {
    castShadow: true,
    type: PCFShadowMap,
    bias: -0.00005, // -0.0002,
    normalBias: 0.01,
    radius: 1,
  };
  private static pcfSoftShadow: ShadowTypeParameters = {
    castShadow: true,
    type: PCFSoftShadowMap,
    bias: -0.00005,
    normalBias: 0.01,
    radius: 1,
  };
  private static vcmShadow: ShadowTypeParameters = {
    castShadow: true,
    type: VSMShadowMap,
    bias: -0.0005,
    normalBias: 0,
    radius: 30,
  };
  public types = new Map<string, ShadowTypeParameters>([
    ['off', ShadowTypeConfiguration.noShadow],
    ['BasicShadowMap', ShadowTypeConfiguration.basicShadow],
    ['PCFShadowMap', ShadowTypeConfiguration.pcfShadow],
    ['PCFSoftShadowMap', ShadowTypeConfiguration.pcfSoftShadow],
    ['VSMShadowMap', ShadowTypeConfiguration.vcmShadow],
  ]);
  private static defaultType: ShadowTypeParameters =
    ShadowTypeConfiguration.pcfShadow;
  public shadowType: string = 'PCFShadowMap';
  public currentConfiguration: ShadowTypeParameters =
    this.types.get(this.shadowType) ?? ShadowTypeConfiguration.defaultType;

  public switchType(type: string): boolean {
    if (!this.types.has(type)) {
      return false;
    }
    this.currentConfiguration =
      this.types.get(type) ?? ShadowTypeConfiguration.defaultType;
    return true;
  }
}
