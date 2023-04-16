import { RenderOverrideVisibility } from './render-utility';
import { BlurContactShadowShader } from './shader-utility';
import {
  Box3,
  CameraHelper,
  DoubleSide,
  Group,
  Layers,
  Mesh,
  MeshBasicMaterial,
  MeshDepthMaterial,
  MeshPhysicalMaterial,
  OrthographicCamera,
  PlaneGeometry,
  Scene,
  SphereGeometry,
  ShaderMaterial,
  Vector3,
  WebGLRenderer,
  WebGLRenderTarget,
} from 'three';

export interface BakedGroundContactShadowParameters {
  [key: string]: any;
  enabled: boolean;
  cameraHelper: boolean;
  alwaysUpdate: boolean;
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
  public static alphaMap: boolean = false;
  public static addTestMesh: boolean = false;
  public shadowMapSize: number;
  public parameters: BakedGroundContactShadowParameters;
  public renderer: WebGLRenderer;
  public cameraHelper: CameraHelper;
  public needsUpdate: boolean = true;
  public noNeedOfUpdateCount = 0;
  private sceneBounds: Box3 = new Box3(
    new Vector3(-1, -1, -1),
    new Vector3(1, 1, 1)
  );
  private boundsSize: Vector3 = new Vector3(2, 2);
  private boundsCenter: Vector3 = new Vector3(0, 0);
  private blurScale: number = 1;
  private groundShadowFar: number;
  private shadowGround: Mesh;
  private blurPlane: Mesh;
  public renderTarget: WebGLRenderTarget;
  private renderTargetBlur: WebGLRenderTarget;
  private depthMaterial: MeshDepthMaterial;
  private blurMaterial: ShaderMaterial;
  private shadowCamera: OrthographicCamera;
  private groundGroup: Group;
  private testMesh: Mesh;
  private renderOverrideVisibility: RenderOverrideVisibility =
    new RenderOverrideVisibility(true, []);

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

    const shadowGroundGeometry = new PlaneGeometry(1, 1, 10, 10);
    const shadowGroundMaterial = new MeshBasicMaterial({
      map: this.renderTarget.texture,
      //color: 0x000000,
      //alphaMap: this.renderTarget.texture,
      opacity: this.parameters.opacity,
      transparent: true,
      depthWrite: false,
      //side: DoubleSide
    });
    if (BakedGroundContactShadow.alphaMap) {
      shadowGroundMaterial.color.set(0x000000);
      shadowGroundMaterial.alphaMap = this.renderTarget.texture;
    }
    shadowGroundMaterial.polygonOffset = true;
    shadowGroundMaterial.polygonOffsetFactor = this.parameters.polygonOffset;
    shadowGroundMaterial.polygonOffsetUnits = this.parameters.polygonOffset;
    this.shadowGround = new Mesh(shadowGroundGeometry, shadowGroundMaterial);
    this.shadowGround.renderOrder = 1;
    this.shadowGround.receiveShadow = false;
    this.groundGroup.add(this.shadowGround);

    this.blurPlane = new Mesh(shadowGroundGeometry);
    this.blurPlane.visible = false;
    this.groundGroup.add(this.blurPlane);

    this.shadowCamera = new OrthographicCamera(-1, 1, -1, 1, -1, 1);
    this.shadowCamera.rotation.x = Math.PI; // make camera look upwards
    this.groundGroup.add(this.shadowCamera);

    this.cameraHelper = new CameraHelper(this.shadowCamera);
    this.cameraHelper.visible = this.parameters.cameraHelper;

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
                BakedGroundContactShadow.alphaMap
                  ? 'gl_FragColor = vec4(clamp(pow(1.0 + fadeoutBias - fragCoordZ, 1.0/(1.0-fadeoutFalloff)), 0.0, 1.0));'
                  : 'gl_FragColor = vec4(vec3(0.0), clamp(pow(1.0 + fadeoutBias - fragCoordZ, 1.0/(1.0-fadeoutFalloff)), 0.0, 1.0));'
              )}
          `;
    };
    this.depthMaterial.side = DoubleSide;
    this.depthMaterial.depthTest = true;
    this.depthMaterial.depthWrite = true;
    this.blurMaterial = new ShaderMaterial(BlurContactShadowShader);
    this.blurMaterial.depthTest = false;
    this.updatePlaneAndShadowCamera();
    this.testMesh = new Mesh(
      new SphereGeometry(1, 32, 16),
      new MeshPhysicalMaterial({
        color: 0xff0000,
        side: DoubleSide,
        transparent: true,
      })
    );
    this.renderOverrideVisibility.onIsObjectInvisible = (object: any) => {
      return (
        (object.isMesh && !object.castShadow) ||
        (object.name !== undefined && ['Ground', 'Floor'].includes(object.name))
      );
    };
  }

  // eslint-disable-next-line complexity
  private getDefaultParameters(parameters: any) {
    return {
      enabled: true,
      cameraHelper: false,
      alwaysUpdate: false,
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
    const shadowGroundMaterial = this.shadowGround
      .material as MeshBasicMaterial;
    if (shadowGroundMaterial.opacity !== this.parameters.opacity) {
      shadowGroundMaterial.opacity = this.parameters.opacity;
    }
    if (
      shadowGroundMaterial.polygonOffsetFactor !== this.parameters.polygonOffset
    ) {
      shadowGroundMaterial.polygonOffset = true;
      shadowGroundMaterial.polygonOffsetFactor = this.parameters.polygonOffset;
      shadowGroundMaterial.polygonOffsetUnits = this.parameters.polygonOffset;
    }
    if (this.cameraHelper.visible !== this.parameters.cameraHelper) {
      this.cameraHelper.visible = this.parameters.cameraHelper;
      if (!this.cameraHelper.visible && this.cameraHelper.parent) {
        this.cameraHelper.parent.remove(this.cameraHelper);
      }
    }
    if (this.shadowCamera.far !== this.parameters.cameraFar) {
      this.updatePlaneAndShadowCamera();
    }
    const fadeoutFalloff = this.parameters.fadeoutFalloff;
    if (this.depthMaterial.userData.fadeoutFalloff.value !== fadeoutFalloff) {
      this.depthMaterial.userData.fadeoutFalloff.value =
        this.parameters.fadeoutFalloff;
    }
    const fadeoutBias = this.parameters.fadeoutBias / this.shadowCamera.far;
    if (this.depthMaterial.userData.fadeoutBias.value !== fadeoutBias) {
      this.depthMaterial.userData.fadeoutBias.value = fadeoutBias;
    }
    this.needsUpdate = true;
  }

  public updateBounds(bounds: Box3, keepGroundLevel: boolean) {
    this.sceneBounds.copy(bounds);
    bounds.getSize(this.boundsSize);
    const minBoundsSize = Math.min(
      this.boundsSize.x,
      this.boundsSize.y,
      this.boundsSize.z
    );
    this.blurScale =
      minBoundsSize < 0.5
        ? minBoundsSize / 0.5
        : this.boundsSize.z > 5
        ? this.boundsSize.z / 5
        : 1;
    bounds.getCenter(this.boundsCenter);
    this.groundShadowFar = this.parameters.cameraFar;
    if (this.groundShadowFar < this.boundsSize.z) {
      this.groundShadowFar = this.boundsSize.z * 1.01;
    }
    // y is always 0
    // TODO - this.shadowGround.position?
    const groundLevel = keepGroundLevel ? this.groundGroup.position.y : 0;
    this.groundGroup.position.set(
      this.boundsCenter.x,
      groundLevel,
      this.boundsCenter.z
    );
    this.groundGroup.updateMatrixWorld();
    const maxPlanSideLength = Math.max(this.boundsSize.x, this.boundsSize.z);
    this.parameters.planeSize = maxPlanSideLength + 2 * this.blurScale;
    this.updatePlaneAndShadowCamera();
  }

  public updatePlaneAndShadowCamera() {
    const size = this.parameters.planeSize;
    this.shadowGround.scale.x = size;
    this.shadowGround.scale.y = size;
    this.blurPlane.scale.x = size;
    this.blurPlane.scale.y = size;
    this.shadowCamera.left = -size / 2;
    this.shadowCamera.right = size / 2;
    this.shadowCamera.top = -size / 2;
    this.shadowCamera.bottom = size / 2;
    this.shadowCamera.near = 0;
    this.shadowCamera.far = this.groundShadowFar;
    this.shadowCamera.updateProjectionMatrix();
    this.cameraHelper.update();
    this.needsUpdate = true;
  }

  public render(scene: Scene, groundObjectGroup: Mesh | undefined): void {
    if (this.cameraHelper.visible && !this.cameraHelper.parent) {
      scene.add(this.cameraHelper);
    }
    const maxIterations = 10;
    this.shadowGround.visible = this.parameters.enabled;
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

    const shadowGroundMaterial = this.shadowGround
      .material as MeshBasicMaterial;
    shadowGroundMaterial.opacity = this.parameters.alwaysUpdate
      ? this.parameters.opacity
      : (this.parameters.opacity * (this.noNeedOfUpdateCount + 2)) /
        (maxIterations + 2);

    const initialBackground = scene.background;
    scene.background = null;
    scene.overrideMaterial = this.depthMaterial;
    const initialClearAlpha = this.renderer.getClearAlpha();
    this.renderer.setClearAlpha(0);
    if (groundObjectGroup) {
      groundObjectGroup.visible = false;
    }
    this.shadowGround.visible = false;
    this.cameraHelper.visible = false;

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
    if (groundObjectGroup) {
      groundObjectGroup.visible = true;
    }
    this.shadowGround.visible = this.parameters.enabled;
    this.cameraHelper.visible = this.parameters.cameraHelper;
  }

  private renderGroundContact(scene: Scene) {
    this.renderer.setRenderTarget(this.renderTarget);
    this.renderer.clear();
    const autoClearBackup = this.renderer.autoClear;
    this.renderer.autoClear = false;
    if (this.parameters.hardLayers) {
      this.shadowCamera.layers.mask = this.parameters.hardLayers.mask;
      this.shadowCamera.far = 10;
      this.shadowCamera.updateProjectionMatrix();
      this.depthMaterial.userData.fadeoutBias.value = 0.99;
      this.renderer.render(scene, this.shadowCamera);
      this.shadowCamera.far = this.groundShadowFar;
      this.shadowCamera.updateProjectionMatrix();
      this.depthMaterial.userData.fadeoutBias.value =
        this.parameters.fadeoutBias / this.groundShadowFar;
    }
    this.shadowCamera.layers.enableAll();
    if (this.parameters.softLayers) {
      this.shadowCamera.layers.mask = this.parameters.softLayers.mask;
    }
    if (BakedGroundContactShadow.addTestMesh) {
      this.testMesh.position.set(0, 1, 0);
      this.testMesh.updateMatrixWorld();
      scene.add(this.testMesh);
      this.renderer.render(scene, this.shadowCamera);
      scene.remove(this.testMesh);
    } else if (this.parameters.excludeGroundObjects) {
      this.renderOverrideVisibility.render(scene, () => {
        this.renderer.render(scene, this.shadowCamera);
      });
    } else {
      this.renderer.render(scene, this.shadowCamera);
    }
    this.renderer.autoClear = autoClearBackup;
  }

  private renderBlur(scene: Scene) {
    scene.overrideMaterial = null;
    this.blurShadow(
      (this.blurScale * this.parameters.blurMin) / this.parameters.planeSize,
      (this.blurScale * this.parameters.blurMax) / this.parameters.planeSize
    );
  }

  private renderReduceBandingBlur(scene: Scene) {
    scene.overrideMaterial = null;
    const finalBlurAmount = (this.blurScale * 0.01) / this.parameters.planeSize;
    this.blurShadow(finalBlurAmount, finalBlurAmount);
  }

  public blurShadow(uvMin: number, uvMax: number): void {
    this.blurPlane.visible = true;
    this.blurPlane.material = this.blurMaterial;
    (this.blurPlane.material as ShaderMaterial).uniforms.tDiffuse.value =
      this.renderTarget.texture;
    this.blurMaterial.uniforms.rangeMin.value.x = uvMin;
    this.blurMaterial.uniforms.rangeMin.value.y = 0;
    this.blurMaterial.uniforms.rangeMax.value.x = uvMax;
    this.blurMaterial.uniforms.rangeMax.value.y = 0;

    this.renderer.setRenderTarget(this.renderTargetBlur);
    this.renderer.render(this.blurPlane, this.shadowCamera);

    this.blurPlane.material = this.blurMaterial;
    (this.blurPlane.material as ShaderMaterial).uniforms.tDiffuse.value =
      this.renderTargetBlur.texture;
    this.blurMaterial.uniforms.rangeMin.value.x = 0;
    this.blurMaterial.uniforms.rangeMin.value.y = uvMin;
    this.blurMaterial.uniforms.rangeMax.value.x = 0;
    this.blurMaterial.uniforms.rangeMax.value.y = uvMax;

    this.renderer.setRenderTarget(this.renderTarget);
    this.renderer.render(this.blurPlane, this.shadowCamera);

    this.blurPlane.visible = false;
  }
}
