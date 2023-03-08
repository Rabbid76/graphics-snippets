import {
  HorizontalBlurContactShadowShader,
  VerticalBlurContactShadowShader,
} from './shader-utility';
import {
  Box3,
  BufferGeometry,
  CameraHelper,
  DoubleSide,
  Group,
  Material,
  Mesh,
  MeshBasicMaterial,
  MeshDepthMaterial,
  OrthographicCamera,
  PlaneGeometry,
  Scene,
  ShaderMaterial,
  Vector3,
  WebGLRenderer,
  WebGLRenderTarget,
} from 'three';

export interface GroundContactShadowParameters {
  enabled: boolean;
  cameraHelper: boolean;
  alwaysUpdate: boolean;
  blurMin: number;
  blurMax: number;
  darkness: number;
  opacity: number;
  planeSize: number;
  cameraFar: number;
}

export class GroundContactShadow {
  public shadowMapSize: number;
  public parameters: GroundContactShadowParameters;
  public renderer: WebGLRenderer;
  public group: Group;
  public cameraHelper: CameraHelper;
  public needsUpdate: boolean = true;
  public noNeedOfUpdateCount = 0;
  private groundMesh?: Mesh;
  private shadowGround: Mesh;
  private blurPlane: Mesh;
  private renderTarget: WebGLRenderTarget;
  private renderTargetBlur: WebGLRenderTarget;
  private depthMaterial: MeshDepthMaterial;
  private horizontalBlurMaterial: ShaderMaterial;
  private verticalBlurMaterial: ShaderMaterial;
  private shadowCamera: OrthographicCamera;

  constructor(renderer: WebGLRenderer, parameters: any) {
    this.shadowMapSize = parameters.shadowMapSize ?? 1024;
    this.parameters = {
      enabled: parameters.enabled ?? true,
      cameraHelper: parameters.cameraHelper ?? false,
      alwaysUpdate: parameters.alwaysUpdate ?? false,
      blurMin: parameters.blurMin ?? 0.5,
      blurMax: parameters.blurMax ?? 6,
      darkness: parameters.darkness ?? 1,
      opacity: parameters.opacity ?? 0.5,
      planeSize: parameters.planeSize ?? 10,
      cameraFar: parameters.cameraFar ?? 3,
    };

    this.renderer = renderer;
    this.group = new Group();
    //this.group.rotateX(-Math.PI/2);

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
    shadowGroundMaterial.polygonOffset = true;
    shadowGroundMaterial.polygonOffsetFactor = 2;
    shadowGroundMaterial.polygonOffsetUnits = 2;
    this.shadowGround = new Mesh(shadowGroundGeometry, shadowGroundMaterial);
    this.shadowGround.renderOrder = 1;
    this.shadowGround.receiveShadow = false;
    this.group.add(this.shadowGround);

    this.blurPlane = new Mesh(shadowGroundGeometry);
    this.blurPlane.visible = false;
    this.group.add(this.blurPlane);

    this.shadowCamera = new OrthographicCamera(-1, 1, -1, 1, -1, 1);
    this.shadowCamera.rotation.x = Math.PI; // make camera look upwards
    this.group.add(this.shadowCamera);

    this.cameraHelper = new CameraHelper(this.shadowCamera);
    this.cameraHelper.visible = this.parameters.cameraHelper;

    this.depthMaterial = new MeshDepthMaterial();
    this.depthMaterial.userData.darkness = { value: this.parameters.darkness };
    this.depthMaterial.onBeforeCompile = (shader) => {
      shader.uniforms.darkness = this.depthMaterial.userData.darkness;
      shader.fragmentShader = `
              uniform float darkness;
              ${shader.fragmentShader.replace(
                'gl_FragColor = vec4( vec3( 1.0 - fragCoordZ ), opacity );',
                'gl_FragColor = vec4(vec3(0.0), pow(1.0 - fragCoordZ, 8.0) * darkness);'
              )}
          `;
    };
    this.depthMaterial.side = DoubleSide;
    this.depthMaterial.depthTest = true;
    this.depthMaterial.depthWrite = true;
    this.horizontalBlurMaterial = new ShaderMaterial(
      HorizontalBlurContactShadowShader
    );
    this.horizontalBlurMaterial.depthTest = false;
    this.verticalBlurMaterial = new ShaderMaterial(
      VerticalBlurContactShadowShader
    );
    this.verticalBlurMaterial.depthTest = false;

    this.updatePlaneAndShadowCamera();
  }

  public addToScene(scene: Scene) {
    if (this.group.parent !== scene) {
      scene.add(this.group);
      scene.add(this.cameraHelper);
    }
  }

  public removeFromScene(scene: Scene) {
    scene.remove(this.group);
    scene.remove(this.cameraHelper);
  }

  public updateParameters() {
    if (
      this.depthMaterial.userData.darkness.value !== this.parameters.darkness
    ) {
      this.depthMaterial.userData.darkness.value = this.parameters.darkness;
      this.needsUpdate = true;
    }
    const shadowGroundMaterial = this.shadowGround
      .material as MeshBasicMaterial;
    if (shadowGroundMaterial.opacity !== this.parameters.opacity) {
      shadowGroundMaterial.opacity = this.parameters.opacity;
    }
    if (this.cameraHelper.visible !== this.parameters.cameraHelper) {
      this.cameraHelper.visible = this.parameters.cameraHelper;
      this.needsUpdate = true;
    }
  }

  public updateBounds(bounds: Box3) {
    const boundsCenter = bounds.getCenter(new Vector3());
    // y is always 0
    this.group.position.set(boundsCenter.x, 0, boundsCenter.z);
    this.group.updateMatrixWorld();
    const boundsSize = bounds.getSize(new Vector3());
    const maxPlanSideLength = Math.max(boundsSize.x, boundsSize.z);
    this.parameters.planeSize = maxPlanSideLength + 2;
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
    this.shadowCamera.far = this.parameters.cameraFar;
    this.shadowCamera.updateProjectionMatrix();
    this.cameraHelper.update();
    this.needsUpdate = true;
  }

  public updateGround(geometry?: BufferGeometry, material?: Material) {
    if (!this.groundMesh) {
      this.groundMesh = new Mesh();
      this.group.add(this.groundMesh);
    }
    if (geometry) {
      this.groundMesh.geometry = geometry;
    }
    if (material) {
      material.polygonOffset = true;
      material.polygonOffsetFactor = 4;
      material.polygonOffsetUnits = 4;
      material.needsUpdate = true;
      this.groundMesh.material = material;
    }
  }

  public render(scene: Scene): void {
    const needsUpdate = this.parameters.alwaysUpdate || this.needsUpdate;
    if (!needsUpdate) {
      this.noNeedOfUpdateCount++;
      if (this.noNeedOfUpdateCount >= 10) {
        return;
      }
    } else {
      this.noNeedOfUpdateCount = 0;
    }
    this.needsUpdate = false;

    const shadowGroundMaterial = this.shadowGround
      .material as MeshBasicMaterial;
    shadowGroundMaterial.opacity =
      (this.parameters.opacity * (this.noNeedOfUpdateCount + 2)) / 12;

    const initialBackground = scene.background;
    scene.background = null;
    scene.overrideMaterial = this.depthMaterial;
    const initialClearAlpha = this.renderer.getClearAlpha();
    this.renderer.setClearAlpha(0);
    if (this.groundMesh) {
      this.groundMesh.visible = false;
    }
    this.shadowGround.visible = false;
    this.cameraHelper.visible = false;

    if (this.noNeedOfUpdateCount === 0) {
      this.renderer.setRenderTarget(this.renderTarget);
      this.renderer.clear();
      this.renderer.render(scene, this.shadowCamera);
      scene.overrideMaterial = null;
      this.blurShadow(
        this.parameters.blurMin / this.parameters.planeSize,
        this.parameters.blurMin / this.parameters.planeSize
      );
    } else if (this.noNeedOfUpdateCount === 1) {
      scene.overrideMaterial = null;
      this.blurShadow(
        this.parameters.blurMin / this.parameters.planeSize,
        this.parameters.blurMax / this.parameters.planeSize
      );
    }
    scene.overrideMaterial = null;
    const finalBlurAmount = 0.4 / this.parameters.planeSize;
    this.blurShadow(finalBlurAmount, finalBlurAmount);

    this.renderer.setRenderTarget(null);
    this.renderer.setClearAlpha(initialClearAlpha);
    scene.background = initialBackground;
    if (this.groundMesh) {
      this.groundMesh.visible = true;
    }
    this.shadowGround.visible = true;
    this.cameraHelper.visible = this.parameters.cameraHelper;
  }

  public blurShadow(amountMin: number, amountMax: number): void {
    this.blurPlane.visible = true;
    this.blurPlane.material = this.horizontalBlurMaterial;
    (this.blurPlane.material as ShaderMaterial).uniforms.tDiffuse.value =
      this.renderTarget.texture;
    this.horizontalBlurMaterial.uniforms.hRange.value.x = (amountMin * 1) / 256;
    this.horizontalBlurMaterial.uniforms.hRange.value.y = (amountMax * 1) / 256;
    this.horizontalBlurMaterial.uniforms.shadowScale.value =
      1 / this.parameters.darkness;

    this.renderer.setRenderTarget(this.renderTargetBlur);
    this.renderer.render(this.blurPlane, this.shadowCamera);

    this.blurPlane.material = this.verticalBlurMaterial;
    (this.blurPlane.material as ShaderMaterial).uniforms.tDiffuse.value =
      this.renderTargetBlur.texture;
    this.verticalBlurMaterial.uniforms.vRange.value.x = (amountMin * 1) / 256;
    this.verticalBlurMaterial.uniforms.vRange.value.y = (amountMax * 1) / 256;
    this.verticalBlurMaterial.uniforms.shadowScale.value =
      1 / this.parameters.darkness;

    this.renderer.setRenderTarget(this.renderTarget);
    this.renderer.render(this.blurPlane, this.shadowCamera);

    this.blurPlane.visible = false;
  }
}
