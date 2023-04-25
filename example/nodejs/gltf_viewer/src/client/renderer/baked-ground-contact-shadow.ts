import { BlurPass, RenderOverrideVisibility, SceneVolume } from './render-utility';
import { BlurContactShadowShader } from './shader-utility';
import {
  Camera,
  CameraHelper,
  DoubleSide,
  Group,
  Layers,
  Mesh,
  MeshBasicMaterial,
  MeshDepthMaterial,
  MeshPhysicalMaterial,
  Object3D,
  OrthographicCamera,
  PlaneGeometry,
  Scene,
  SphereGeometry,
  Texture,
  WebGLRenderer,
  WebGLRenderTarget,
} from 'three';

export interface BakedGroundContactShadowParameters {
  [key: string]: any;
  enabled: boolean;
  cameraHelper: boolean;
  alwaysUpdate: boolean;
  fadeIn: boolean;
  blurMin: number;
  blurMax: number;
  fadeoutFalloff: number;
  fadeoutBias: number;
  opacity: number;
  planeSize: number;
  cameraFar: number;
  hardLayers: Layers | null;
  softLayers: Layers | null;
  polygonOffset: number;
  excludeGroundObjects: boolean;
}

export class BakedGroundContactShadow {
  public static addTestMesh: boolean = false;
  private testMesh?: BakedGroundContactTestMesh;
  public shadowMapSize: number;
  public parameters: BakedGroundContactShadowParameters;
  private renderer: WebGLRenderer;
  public needsUpdate: boolean = true;
  public noNeedOfUpdateCount = 0;
  private blurScale: number = 1;
  private groundGroup: Group;
  private groundShadowFar: number;
  private shadowGroundPlane: ShadowGroundPlane;
  private groundContactCamera: GroundContactCamera;
  private renderTargetBlur: WebGLRenderTarget;
  private blurPass: BlurPass;
  public readonly renderTarget: WebGLRenderTarget;
  private depthMaterial: MeshDepthMaterial;
  private renderOverrideVisibility: RenderOverrideVisibility =
    new RenderOverrideVisibility(true, (object: any) => {
      return (
        (object.isMesh && !(object.castShadow || object.userData.meshId)) ||
        (object.name !== undefined && ['Ground', 'Floor'].includes(object.name))
      );
    });

  constructor(renderer: WebGLRenderer, groundGroup: Group, parameters: any) {
    this.groundGroup = groundGroup;
    this.shadowMapSize = parameters.shadowMapSize ?? 1024;
    this.parameters = this.getDefaultParameters(parameters);
    this.groundShadowFar = this.parameters.cameraFar;

    this.renderer = renderer;
    this.renderTarget = new WebGLRenderTarget(
      this.shadowMapSize,
      this.shadowMapSize
    );
    this.renderTarget.texture.generateMipmaps = false;
    //this.renderTarget.depthTexture = new DepthTexture(this.shadowMapSize, this.shadowMapSize);
    //this.renderTarget.depthTexture.generateMipmaps = false;
    this.renderTargetBlur = new WebGLRenderTarget(
      this.shadowMapSize,
      this.shadowMapSize
    );
    this.renderTargetBlur.texture.generateMipmaps = false;

    this.shadowGroundPlane = new ShadowGroundPlane(this.renderTarget.texture, this.parameters);
    this.groundGroup.add(this.shadowGroundPlane);

    this.groundContactCamera = new GroundContactCamera();
    this.groundGroup.add(this.groundContactCamera);

    this.depthMaterial = new MeshDepthMaterial();
    this.depthMaterial.userData.fadeoutBias = {
      value: this.parameters.fadeoutBias,
    };
    this.depthMaterial.userData.fadeoutFalloff = {
      value: this.parameters.fadeoutFalloff,
    };
    this.depthMaterial.onBeforeCompile = (shader) => {
      shader.uniforms.fadeoutBias = this.depthMaterial.userData.fadeoutBias;
      shader.uniforms.fadeoutFalloff =
        this.depthMaterial.userData.fadeoutFalloff;
      shader.fragmentShader = `
              uniform float fadeoutBias;
              uniform float fadeoutFalloff;
              ${shader.fragmentShader.replace(
                'gl_FragColor = vec4( vec3( 1.0 - fragCoordZ ), opacity );',
                ShadowGroundPlane.alphaMap
                  ? 'gl_FragColor = vec4(clamp(pow(1.0 + fadeoutBias - fragCoordZ, 1.0/(1.0-fadeoutFalloff)), 0.0, 1.0));'
                  : 'gl_FragColor = vec4(vec3(0.0), clamp(pow(1.0 + fadeoutBias - fragCoordZ, 1.0/(1.0-fadeoutFalloff)), 0.0, 1.0));'
              )}
          `;
    };
    this.depthMaterial.side = DoubleSide;
    this.depthMaterial.depthTest = true;
    this.depthMaterial.depthWrite = true;
    this.blurPass = new BlurPass(BlurContactShadowShader, parameters);
    this.updatePlaneAndShadowCamera();
    if (BakedGroundContactShadow.addTestMesh) {
      this.testMesh = new BakedGroundContactTestMesh();
    }
  }

  // eslint-disable-next-line complexity
  private getDefaultParameters(parameters: any) {
    return {
      enabled: true,
      cameraHelper: false,
      alwaysUpdate: false,
      fadeIn: false,
      blurMin: 0.001,
      blurMax: 0.1,
      fadeoutFalloff: 0.9,
      fadeoutBias: 0.03,
      opacity: 0.5,
      planeSize: 10,
      cameraFar: 3,
      hardLayers: null,
      softLayers: null,
      polygonOffset: 2,
      excludeGroundObjects: true,
      ...parameters,
    };
  }

  dispose() {
    this.renderTarget.dispose();
    this.renderTargetBlur.dispose();
    this.blurPass.dispose();
    this.depthMaterial.dispose();
  }

  public updateParameters(parameters: any) {
    for (let propertyName in parameters) {
      if (this.parameters.hasOwnProperty(propertyName)) {
        this.parameters[propertyName] = parameters[propertyName];
      }
    }
    if (parameters.cameraFar !== undefined) {
      this.groundShadowFar = this.parameters.cameraFar;
    }
  }

  public applyParameters() {
    this.shadowGroundPlane.updateMaterial(this.parameters);
    this.groundContactCamera.updateCameraHelper(this.parameters.cameraHelper);
    if (this.groundContactCamera.far !== this.parameters.cameraFar) {
      this.updatePlaneAndShadowCamera();
    }
    const fadeoutFalloff = this.parameters.fadeoutFalloff;
    if (this.depthMaterial.userData.fadeoutFalloff.value !== fadeoutFalloff) {
      this.depthMaterial.userData.fadeoutFalloff.value =
        this.parameters.fadeoutFalloff;
    }
    const fadeoutBias = this.parameters.fadeoutBias / this.groundContactCamera.far;
    if (this.depthMaterial.userData.fadeoutBias.value !== fadeoutBias) {
      this.depthMaterial.userData.fadeoutBias.value = fadeoutBias;
    }
    this.needsUpdate = true;
  }

  public updateBounds(
    sceneBounds: SceneVolume,
    groundContactShadowScale: number,
    groundLevel?: number
  ) {
    this.blurScale = groundContactShadowScale;
    this.groundShadowFar = this.parameters.cameraFar;
    if (this.groundShadowFar < sceneBounds.size.z) {
      this.groundShadowFar = sceneBounds.size.z * 1.01;
    }
    this.groundGroup.position.set(
      sceneBounds.center.x,
      groundLevel ?? 0,
      sceneBounds.center.z
    );
    this.groundGroup.updateMatrixWorld();
    const maxPlanSideLength = Math.max(sceneBounds.size.x, sceneBounds.size.z);
    this.parameters.planeSize = maxPlanSideLength + 2 * this.blurScale;
    this.updatePlaneAndShadowCamera();
  }

  public updatePlaneAndShadowCamera() {
    const size = this.parameters.planeSize;
    this.shadowGroundPlane.scale.x = size;
    this.shadowGroundPlane.scale.y = size;
    this.groundContactCamera.updateCameraFormPlaneSize(size, this.groundShadowFar);
    this.needsUpdate = true;
  }

  public setGroundVisibility(visible: boolean, _camera?: Camera) {
    this.shadowGroundPlane.updateVisibility(visible);
  }

  public render(scene: Scene): void {
    this.groundContactCamera.updateCameraHelper(this.parameters.cameraHelper, scene);
    const maxIterations = 10;
    this.shadowGroundPlane.visible = this.parameters.enabled;
    const needsUpdate = this.parameters.alwaysUpdate || this.needsUpdate;
    if (!needsUpdate) {
      this.noNeedOfUpdateCount++;
      if (this.noNeedOfUpdateCount >= maxIterations) {
        return;
      }
    } else {
      this.noNeedOfUpdateCount = 0;
    }
    this.needsUpdate = false;

    const shadowGroundMaterial = this.shadowGroundPlane
      .material as MeshBasicMaterial;
    shadowGroundMaterial.opacity =
      this.parameters.alwaysUpdate || !this.parameters.fadeIn
        ? this.parameters.opacity
        : (this.parameters.opacity * (this.noNeedOfUpdateCount + 2)) /
          (maxIterations + 2);

    const initialBackground = scene.background;
    scene.background = null;
    scene.overrideMaterial = this.depthMaterial;
    const initialClearAlpha = this.renderer.getClearAlpha();
    this.renderer.setClearAlpha(0);
    this.groundGroup.visible = false;
    this.shadowGroundPlane.visible = false;
    this.groundContactCamera.setCameraHelperVisibility(false);

    if (this.noNeedOfUpdateCount === 0) {
      this.renderGroundContact(scene);
      this.renderBlur(scene);
    } else if (this.noNeedOfUpdateCount === 1) {
      this.renderBlur(scene);
    }
    this.renderReduceBandingBlur(scene);

    this.renderer.setRenderTarget(null);
    this.renderer.setClearAlpha(initialClearAlpha);
    scene.background = initialBackground;
    this.groundGroup.visible = true;
    this.shadowGroundPlane.visible = this.parameters.enabled;
    this.groundContactCamera.setCameraHelperVisibility(this.parameters.cameraHelper);
  }

  private renderGroundContact(scene: Scene) {
    this.renderer.setRenderTarget(this.renderTarget);
    this.renderer.clear();
    const autoClearBackup = this.renderer.autoClear;
    this.renderer.autoClear = false;
    if (this.parameters.hardLayers) {
      this.groundContactCamera.layers.mask = this.parameters.hardLayers.mask;
      this.groundContactCamera.updateCameraFarPlane(10);
      this.depthMaterial.userData.fadeoutBias.value = 0.99;
      this.renderer.render(scene, this.groundContactCamera);
      this.groundContactCamera.updateCameraFarPlane(this.groundShadowFar);
      this.depthMaterial.userData.fadeoutBias.value =
        this.parameters.fadeoutBias / this.groundShadowFar;
    }
    this.groundContactCamera.layers.enableAll();
    if (this.parameters.softLayers) {
      this.groundContactCamera.layers.mask = this.parameters.softLayers.mask;
    }
    this.testMesh?.addToObject(scene);
    if (this.parameters.excludeGroundObjects) {
      this.renderOverrideVisibility.render(scene, () => {
        this.renderer.render(scene, this.groundContactCamera);
      });
    } else {
      this.renderer.render(scene, this.groundContactCamera);
    }
    this.testMesh?.removeFromParent();
    this.renderer.autoClear = autoClearBackup;
  }

  private renderBlur(scene: Scene) {
    scene.overrideMaterial = null;
    this.renderBlurPass(
      (this.blurScale * this.parameters.blurMin) / this.parameters.planeSize,
      (this.blurScale * this.parameters.blurMax) / this.parameters.planeSize
    );
  }

  private renderReduceBandingBlur(scene: Scene) {
    scene.overrideMaterial = null;
    const finalBlurAmount = (this.blurScale * 0.01) / this.parameters.planeSize;
    this.renderBlurPass(finalBlurAmount, finalBlurAmount);
  }

  private renderBlurPass(uvMin: number, uvMax: number): void {
    this.blurPass.render(this.renderer,
      [this.renderTarget, this.renderTargetBlur, this.renderTarget],
      [uvMin, uvMin], [uvMax, uvMax]);
  }
}

export class ShadowGroundPlane extends Mesh {
  public static alphaMap: boolean = false;

  constructor(shadowMap: Texture | null, parameters?: any) {
    const planeMaterial = new MeshBasicMaterial({
      transparent: true,
      depthWrite: false,
      //side: DoubleSide
    });
    if (ShadowGroundPlane.alphaMap) {
      planeMaterial.color.set(0x000000);
    }
    planeMaterial.polygonOffset = true;
    super(new PlaneGeometry(1, 1, 10, 10), planeMaterial);
    this.renderOrder = 1;
    this.receiveShadow = false;
    this.layers.disableAll();
    if (parameters) {
      this.updateMaterial(parameters);
    }
    this.setShadowMap(shadowMap);
  }

  public setShadowMap(shadowMap: Texture | null) {
    const shadowGroundMaterial = this.material as MeshBasicMaterial;
    shadowGroundMaterial.map = shadowMap;
    if (ShadowGroundPlane.alphaMap) {
      shadowGroundMaterial.alphaMap = shadowMap;
    }
    shadowGroundMaterial.needsUpdate = true;
  }

  public updateMaterial(parameters: any) {
    const shadowGroundMaterial = this.material as MeshBasicMaterial;
    if (shadowGroundMaterial.opacity !== parameters.opacity) {
      shadowGroundMaterial.opacity = parameters.opacity;
    }
    if (
      shadowGroundMaterial.polygonOffsetFactor !== parameters.polygonOffset
    ) {
      shadowGroundMaterial.polygonOffset = true;
      shadowGroundMaterial.polygonOffsetFactor = parameters.polygonOffset;
      shadowGroundMaterial.polygonOffsetUnits = parameters.polygonOffset;
    }
  }

  public updateVisibility(visible: boolean) {
    if (visible) {
      this.layers.enableAll();
    } else {
      this.layers.disableAll();
    }
  }
}

class GroundContactCamera extends OrthographicCamera {
  private cameraHelper?: CameraHelper;

  constructor() {
    super(-1, 1, -1, 1, -1, 1);
    this.rotation.x = Math.PI; // make camera look upwards
  }

  public updateCameraFormPlaneSize(planeSize: number, farPlane: number) {
    this.left = -planeSize / 2;
    this.right = planeSize / 2;
    this.top = -planeSize / 2;
    this.bottom = planeSize / 2;
    this.near = 0;
    this.far = farPlane;
    this.updateProjectionMatrix();
    this.cameraHelper?.update();
  }

  public updateCameraFarPlane(farPlane: number) {
    this.far = farPlane;
    this.updateProjectionMatrix();
    this.cameraHelper?.update();
  }

  public updateCameraHelper(enabled: boolean, scene?: Scene) {
    if (enabled) {
      this.cameraHelper = this.cameraHelper ?? new CameraHelper(this);
      this.cameraHelper.visible = true;
      scene?.add(this.cameraHelper);

    } else if (this.cameraHelper?.parent) {
      this.cameraHelper?.removeFromParent();
    }
  }

  public setCameraHelperVisibility(visible: boolean) {
    if (this.cameraHelper) {
      this.cameraHelper.visible = visible;
    }
  }
}

class BakedGroundContactTestMesh extends Mesh {
  constructor() {
    super(new SphereGeometry(1, 32, 16),
      new MeshPhysicalMaterial({
        color: 0xff0000,
        side: DoubleSide,
        transparent: true,
      }));
    this.castShadow = true;
  }

  public addToObject(object: Object3D) {
    this.position.set(0, 1, 0);
    this.updateMatrixWorld();
    object.add(this); 
  }
}