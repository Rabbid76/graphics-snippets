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
  Matrix4,
  Mesh,
  MeshBasicMaterial,
  Object3D,
  OrthographicCamera,
  RGBAFormat,
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

export class SceneVolume {
  public sceneBounds: Box3 = new Box3();
  public maxSceneDistanceFrom0: number = 10;

  public update(sceneObject: Object3D): void {
    this.sceneBounds.setFromObject(sceneObject);
    this.maxSceneDistanceFrom0 = new Vector3(
      Math.max(
        Math.abs(this.sceneBounds.min.x),
        Math.abs(this.sceneBounds.max.x)
      ),
      Math.max(
        Math.abs(this.sceneBounds.min.y),
        Math.abs(this.sceneBounds.max.y)
      ),
      Math.max(
        Math.abs(this.sceneBounds.min.z),
        Math.abs(this.sceneBounds.max.z)
      )
    ).length();
  }

  public getNearAndFarForCameraThatLooksAtOriginOfScene(
    cameraPosition: Vector3,
    backScale: number = 1
  ): number[] {
    const distanceFromCenter = cameraPosition.length();
    const near = Math.max(
      0.01,
      distanceFromCenter - this.maxSceneDistanceFrom0 - 0.01
    );
    const far =
      distanceFromCenter + this.maxSceneDistanceFrom0 * backScale + 0.01;
    return [near, far];
  }
}

export const getMaxSamples = (renderTarget: WebGLRenderer): number => {
  const capabilities: WebGLCapabilities = renderTarget.capabilities;
  const maxSamples: number = capabilities.maxSamples;
  return maxSamples;
};

export class RenderOverrideVisibility {
  private _visibilityCache = new Map();
  private background: any;
  private hideLinesAndPoints: boolean;
  private invisibleObjects?: any[];

  constructor(
    hideLinesAndPoints?: boolean,
    invisibleObjects?: any[],
    background?: any
  ) {
    this.background = background;
    this.hideLinesAndPoints = hideLinesAndPoints ?? false;
    this.invisibleObjects = invisibleObjects;
  }

  public updateVisibilityCache(scene?: Scene) {
    this._visibilityCache.clear();
    this.invisibleObjects?.forEach((item: any) =>
      this._visibilityCache.set(item, item.visible)
    );
    if (scene && this.hideLinesAndPoints) {
      scene.traverse((object: any) => {
        if (object.isPoints || object.isLine) {
          this._visibilityCache.set(object, object.visible);
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
