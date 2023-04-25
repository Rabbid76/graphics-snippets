import { BlurContactShadowShader } from './shader-utility';
import {
  Box3,
  BoxGeometry,
  BoxHelper,
  Camera,
  Color,
  CubeTexture,
  DataTexture,
  Group,
  LineBasicMaterial,
  Material,
  MathUtils,
  Matrix4,
  Mesh,
  MeshBasicMaterial,
  Object3D,
  OrthographicCamera,
  PerspectiveCamera,
  RepeatWrapping,
  RGBAFormat,
  ShaderMaterial,
  ShaderMaterialParameters,
  Scene,
  Texture,
  TextureLoader,
  Vector3,
  WebGLCapabilities,
  WebGLRenderer,
  WebGLRenderTarget,
} from 'three';
import { FullScreenQuad } from 'three/examples/jsm/postprocessing/Pass';

const textureLoader: TextureLoader = new TextureLoader();

export const createUniformColorTexture = (color: Color): Texture => {
  const colorTextureData = new Uint8Array([
    Math.floor(color.r * 255),
    Math.floor(color.g * 255),
    Math.floor(color.b * 255),
    255,
  ]);
  const colorTexture = new DataTexture(colorTextureData, 1, 1);
  colorTexture.needsUpdate = true;
  return colorTexture;
};

export const createUniformColorCubeTexture = (color: Color) => {
  const cubeTexture = new CubeTexture();
  cubeTexture.format = RGBAFormat;
  cubeTexture.generateMipmaps = false;
  for (let i = 0; i < 6; ++i) {
    const dataTexture = createUniformColorTexture(color);
    dataTexture.format = cubeTexture.format;
    dataTexture.type = cubeTexture.type;
    dataTexture.minFilter = cubeTexture.minFilter;
    dataTexture.magFilter = cubeTexture.magFilter;
    dataTexture.generateMipmaps = cubeTexture.generateMipmaps;
    dataTexture.needsUpdate = true;
    cubeTexture.images[i] = dataTexture;
  }
  cubeTexture.needsUpdate = true;
  return cubeTexture;
};

export const createNoiseTexture = (
  size: number,
  min: number,
  max: number
): Texture => {
  const noiseTextureData = new Uint8Array(size * size * 4);
  for (let i = 0; i < noiseTextureData.length; ++i) {
    noiseTextureData[i] = Math.floor((min + (max - min) * Math.random()) * 255);
  }
  const noiseTexture = new DataTexture(noiseTextureData, size, size);
  noiseTexture.needsUpdate = true;
  return noiseTexture;
};

export const loadAndSetTexture = (
  setTexture: (texture: Texture) => void,
  resource: string,
  color?: Color
): void => {
  if (color) {
    setTexture(createUniformColorTexture(color));
  }
  if (resource) {
    textureLoader.load(resource, setTexture);
  }
};

export const setOrthographicViewVolumeFromBox = (
  camera: OrthographicCamera,
  viewBox: Box3
): void => {
  camera.left = viewBox.min.x;
  camera.right = viewBox.max.x;
  camera.bottom = viewBox.min.y;
  camera.top = viewBox.max.y;
  camera.near = Math.min(-viewBox.min.z, -viewBox.max.z);
  camera.far = Math.max(-viewBox.min.z, -viewBox.max.z);
  camera.updateProjectionMatrix();
};

export const setPerspectiveViewVolumeFromBox = (
  camera: PerspectiveCamera,
  viewBox: Box3
): void => {
  const near = Math.min(-viewBox.min.z, -viewBox.max.z);
  const far = Math.max(-viewBox.min.z, -viewBox.max.z);
  if (near < 0.001) {
    return;
  }
  const halfWidth = Math.max(Math.abs(viewBox.min.x), Math.abs(viewBox.max.x));
  const halfHeight = Math.max(Math.abs(viewBox.min.y), Math.abs(viewBox.max.y));
  camera.aspect = halfWidth / halfHeight;
  camera.fov = MathUtils.radToDeg(Math.atan2(halfHeight, near) * 2);
  camera.near = near;
  camera.far = far;
  camera.updateProjectionMatrix();
};

export class SceneVolume {
  public bounds: Box3 = new Box3(new Vector3(-1, -1, -1), new Vector3(1, 1, 1));
  public size: Vector3 = new Vector3(2, 2, 2);
  public center: Vector3 = new Vector3(0, 0, 0);
  public maxSceneDistanceFromCenter: number = Math.sqrt(3);
  public maxSceneDistanceFrom0: number = Math.sqrt(3);

  public copyFrom(other: SceneVolume): void {
    this.bounds.copy(other.bounds);
    this.size.copy(other.size);
    this.center.copy(other.center);
    this.maxSceneDistanceFromCenter = other.maxSceneDistanceFromCenter;
    this.maxSceneDistanceFrom0 = other.maxSceneDistanceFrom0;
  }

  public updateFromObject(sceneObject: Object3D): void {
    sceneObject.updateMatrixWorld();
    this.bounds.setFromObject(sceneObject);
    this.updateFromBox(this.bounds);
  }

  public updateFromBox(boundingBox: Box3): void {
    if (this.bounds !== boundingBox) {
      this.bounds.copy(boundingBox);
    }
    this.bounds.getSize(this.size);
    this.bounds.getCenter(this.center);
    this.maxSceneDistanceFromCenter = this.size.length() / 2;
    this.maxSceneDistanceFrom0 = new Vector3(
      Math.max(Math.abs(this.bounds.min.x), Math.abs(this.bounds.max.x)),
      Math.max(Math.abs(this.bounds.min.y), Math.abs(this.bounds.max.y)),
      Math.max(Math.abs(this.bounds.min.z), Math.abs(this.bounds.max.z))
    ).length();
  }

  public updateCameraViewVolumeFromBounds(camera: Camera): void {
    camera.updateMatrixWorld();
    const cameraViewBounds = this.bounds
      .clone()
      .applyMatrix4(camera.matrixWorldInverse);
    if (camera instanceof OrthographicCamera) {
      setOrthographicViewVolumeFromBox(camera, cameraViewBounds);
    } else if (camera instanceof PerspectiveCamera) {
      setPerspectiveViewVolumeFromBox(camera, cameraViewBounds);
    }
  }

  public getNearAndFarForPerspectiveCamera(
    cameraPosition: Vector3,
    backScale: number = 1
  ): number[] {
    const distanceFromCenter = cameraPosition.clone().sub(this.center).length();
    const near = Math.max(
      0.01,
      distanceFromCenter - this.maxSceneDistanceFromCenter - 0.01
    );
    const far =
      distanceFromCenter + this.maxSceneDistanceFromCenter * backScale + 0.01;
    return [near, far];
  }
}

export const getMaxSamples = (renderTarget: WebGLRenderer): number => {
  const capabilities: WebGLCapabilities = renderTarget.capabilities;
  const maxSamples: number = capabilities.maxSamples;
  return maxSamples;
};

export class BoxUpdateHelper {
  public box: Box3;
  public color: Color;
  public opacity: number;
  private group: Group;
  private boxMesh: Mesh;
  private boxWire: BoxHelper;

  public get visible(): boolean {
    return this.group.visible;
  }
  public set visible(isVisible: boolean) {
    this.group.visible = isVisible;
  }
  public get object(): Object3D {
    return this.group;
  }

  constructor(box: Box3, parameters?: any) {
    this.group = new Group();
    this.box = box;
    this.color = parameters?.color ?? 0x808080;
    this.opacity = parameters?.opacity ?? 0.5;
    const boxSize = this.box.getSize(new Vector3());
    const boxCenter = this.box.getCenter(new Vector3());
    const sceneBoxGeometry = new BoxGeometry(boxSize.x, boxSize.y, boxSize.z);
    sceneBoxGeometry.translate(boxCenter.x, boxCenter.y, boxCenter.z);
    this.boxMesh = new Mesh(
      sceneBoxGeometry,
      new MeshBasicMaterial({
        color: this.color,
        transparent: true,
        opacity: this.opacity,
      })
    );
    this.boxWire = new BoxHelper(this.boxMesh, this.color);
    this.boxWire.material = new LineBasicMaterial({ color: this.color });
    this.group.add(this.boxMesh);
    this.group.add(this.boxWire);
  }

  public update(): void {
    const boxMaterial = this.boxMesh.material as MeshBasicMaterial;
    boxMaterial.opacity = this.opacity;
    boxMaterial.color = new Color(this.color);
    boxMaterial.needsUpdate = true;
    const boxSize = this.box.getSize(new Vector3());
    const boxCenter = this.box.getCenter(new Vector3());
    const sceneBoxGeometry = new BoxGeometry(boxSize.x, boxSize.y, boxSize.z);
    sceneBoxGeometry.translate(boxCenter.x, boxCenter.y, boxCenter.z);
    this.boxMesh.geometry = sceneBoxGeometry;
    this.group.remove(this.boxWire);
    this.boxWire = new BoxHelper(new Mesh(sceneBoxGeometry), this.color);
    this.boxWire.material = new LineBasicMaterial({ color: this.color });
    this.group.add(this.boxWire);
  }

  public addTo(scene: Object3D): void {
    scene.add(this.group);
  }

  public removeFrom(scene: Object3D): void {
    scene.remove(this.group);
  }
}

export const boxFromOrthographicViewVolume = (
  camera: OrthographicCamera
): Box3 => {
  const min = new Vector3(
    Math.min(camera.left, camera.right),
    Math.min(camera.bottom, camera.top),
    Math.min(camera.near, camera.far)
  );
  const max = new Vector3(
    Math.max(camera.left, camera.right),
    Math.max(camera.bottom, camera.top),
    Math.max(camera.near, camera.far)
  );
  const box = new Box3(min, max);
  return box;
};

export const boundingBoxInViewSpace = (
  worldBox: Box3,
  camera: Camera
): Box3 => {
  camera.updateMatrixWorld();
  const viewMatrix = camera.matrixWorldInverse;
  const viewBox = new Box3();
  viewBox.expandByPoint(
    new Vector3(worldBox.min.x, worldBox.min.y, worldBox.min.z).applyMatrix4(
      viewMatrix
    )
  );
  viewBox.expandByPoint(
    new Vector3(worldBox.min.x, worldBox.min.y, worldBox.max.z).applyMatrix4(
      viewMatrix
    )
  );
  viewBox.expandByPoint(
    new Vector3(worldBox.min.x, worldBox.max.y, worldBox.min.z).applyMatrix4(
      viewMatrix
    )
  );
  viewBox.expandByPoint(
    new Vector3(worldBox.min.x, worldBox.max.y, worldBox.max.z).applyMatrix4(
      viewMatrix
    )
  );
  viewBox.expandByPoint(
    new Vector3(worldBox.max.x, worldBox.min.y, worldBox.min.z).applyMatrix4(
      viewMatrix
    )
  );
  viewBox.expandByPoint(
    new Vector3(worldBox.max.x, worldBox.min.y, worldBox.max.z).applyMatrix4(
      viewMatrix
    )
  );
  viewBox.expandByPoint(
    new Vector3(worldBox.max.x, worldBox.max.y, worldBox.min.z).applyMatrix4(
      viewMatrix
    )
  );
  viewBox.expandByPoint(
    new Vector3(worldBox.max.x, worldBox.max.y, worldBox.max.z).applyMatrix4(
      viewMatrix
    )
  );
  return viewBox;
};

export class CameraUpdate {
  private lastCameraProjection: Matrix4 | undefined;
  private lastCameraWorld: Matrix4 | undefined;

  public changed(camera: Camera): boolean {
    const hasChanged =
      !this.lastCameraProjection?.equals(camera.projectionMatrix) ||
      !this.lastCameraWorld?.equals(camera.matrixWorld);
    this.lastCameraProjection = camera.projectionMatrix.clone();
    this.lastCameraWorld = camera.matrixWorld.clone();
    return hasChanged;
  }
}

export const viewSpacePositionFromAltitudeAndAzimuth = (
  distance: number,
  altitudeDeg: number,
  azimuthDeg: number
): Vector3 => {
  return new Vector3().setFromSphericalCoords(
    distance,
    MathUtils.degToRad(90 - altitudeDeg),
    MathUtils.degToRad(180 - azimuthDeg)
  );
};

export const viewSpacePositionFromUV = (
  distance: number,
  u: number,
  v: number
): Vector3 => {
  return new Vector3().setFromSphericalCoords(
    distance,
    Math.PI * (1 - v),
    Math.PI * 2 * (1 - u)
  );
};

export const spiralQuadraticSampleKernel = (kernelSize: number): Vector3[] => {
  const kernel: Vector3[] = [];
  for (let kernelIndex = 0; kernelIndex < kernelSize; kernelIndex++) {
    const spiralAngle = kernelIndex * Math.PI * (3 - Math.sqrt(5));
    const z = 0.99 - (kernelIndex / (kernelSize - 1)) * 0.8;
    const radius = Math.sqrt(1 - z * z);
    const x = Math.cos(spiralAngle) * radius;
    const y = Math.sin(spiralAngle) * radius;
    const scaleStep = 8;
    const scaleRange = Math.floor(kernelSize / scaleStep);
    const scaleIndex =
      Math.floor(kernelIndex / scaleStep) +
      (kernelIndex % scaleStep) * scaleRange;
    let scale = 1 - scaleIndex / kernelSize;
    scale = MathUtils.lerp(0.1, 1, scale * scale);
    kernel.push(new Vector3(x * scale, y * scale, z * scale));
  }
  return kernel;
};

export const generateUniformKernelRotations = (): DataTexture => {
  const width = 4;
  const height = 4;
  const noiseSize = width * height;
  const data = new Uint8Array(noiseSize * 4);
  for (let inx = 0; inx < noiseSize; ++inx) {
    const iAng = Math.floor(inx / 2) + (inx % 2) * 8;
    const angle = (2 * Math.PI * iAng) / noiseSize;
    const randomVec = new Vector3(
      Math.cos(angle),
      Math.sin(angle),
      0
    ).normalize();
    data[inx * 4] = (randomVec.x * 0.5 + 0.5) * 255;
    data[inx * 4 + 1] = (randomVec.y * 0.5 + 0.5) * 255;
    data[inx * 4 + 2] = 127;
    data[inx * 4 + 3] = 0;
  }
  const noiseTexture = new DataTexture(data, width, height);
  noiseTexture.wrapS = RepeatWrapping;
  noiseTexture.wrapT = RepeatWrapping;
  noiseTexture.needsUpdate = true;
  return noiseTexture;
};

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

export class BlurPass {
  private renderPass: RenderPass;
  private blurMaterial: ShaderMaterial;
  
  constructor(blurShaderParameters?: ShaderMaterialParameters, parameters?: any) {
    this.renderPass = parameters?.renderPass ?? new RenderPass();
    this.blurMaterial = new ShaderMaterial(blurShaderParameters ?? BlurContactShadowShader);
    this.blurMaterial.depthTest = false;
  }

  public dispose() {
    this.blurMaterial.dispose();
  }

  public render(renderer: WebGLRenderer, renderTargets: WebGLRenderTarget[], uvMin: number[], uvMax: number[]) {
    this.blurMaterial.uniforms.tDiffuse.value = renderTargets[0].texture;
    this.blurMaterial.uniforms.rangeMin.value.x = uvMin[0];
    this.blurMaterial.uniforms.rangeMin.value.y = 0;
    this.blurMaterial.uniforms.rangeMax.value.x = uvMax[0];
    this.blurMaterial.uniforms.rangeMax.value.y = 0;
    this.renderPass.renderScreenSpace(
      renderer,
      this.blurMaterial,
      renderTargets[1]
    );
    this.blurMaterial.uniforms.tDiffuse.value = renderTargets[1].texture;
    this.blurMaterial.uniforms.rangeMin.value.x = 0;
    this.blurMaterial.uniforms.rangeMin.value.y = uvMin[1];
    this.blurMaterial.uniforms.rangeMax.value.x = 0;
    this.blurMaterial.uniforms.rangeMax.value.y = uvMax[1];
    this.renderPass.renderScreenSpace(
      renderer,
      this.blurMaterial,
      renderTargets[2]
    );
  }
}
