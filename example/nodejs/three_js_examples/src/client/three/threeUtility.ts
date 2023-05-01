import {
    Box3,
    BoxHelper,
    BoxGeometry,
    Camera,
    Color,
    CubeTexture,
    DataTexture,
    Group,
    LineBasicMaterial,
    MathUtils,
    Matrix4,
    Mesh,
    MeshBasicMaterial,
    Object3D,
    PerspectiveCamera,
    OrthographicCamera,
    RGBAFormat,
    Texture,
    TextureLoader,
    Vector3,
    WebGLCapabilities,
    WebGLRenderer,
} from 'three';

const textureLoader: TextureLoader = new TextureLoader();

export const createUniformColorTexture = (color: Color): Texture => {
    const colorTextureData = new Uint8Array([Math.floor(color.r * 255), Math.floor(color.g * 255), Math.floor(color.b * 255), 255]);
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

export const createNoiseTexture = (size: number, min: number, max: number): Texture => {
    const noiseTextureData = new Uint8Array(size*size*4);
    for (let i = 0; i < noiseTextureData.length; ++i) {
        noiseTextureData[i] = Math.floor((min + (max - min) * Math.random()) * 255);
    }
    const noiseTexture = new DataTexture(noiseTextureData, size, size);
    noiseTexture.needsUpdate = true;
    return noiseTexture;
};

export const loadAndSetTexture = (setTexture: (texture: Texture) => void, resource: string, color?: Color): void => {
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
    const capabilities: WebGLCapabilities = renderTarget.capabilities
    // @ts-ignore
    const maxSamples: number = capabilities.maxSamples;
    return maxSamples;
}

export class BoxUpdateHelper {
    public box: Box3;
    public color: Color;
    public opacity: number;
    private group: Group;
    private boxMesh: Mesh;
    private boxWire: BoxHelper;

    public get visible(): boolean { return this.group.visible; }
    public set visible(isVisible: boolean) { this.group.visible = isVisible; }
    public get object(): Object3D { return this.group; }

    constructor(box: Box3, parameters?: any) {
        this.group = new Group();
        this.box = box;
        this.color = parameters?.color ?? 0x808080;
        this.opacity = parameters?.opacity ?? 0.5;
        const boxSize = this.box.getSize(new Vector3);
        const boxCenter = this.box.getCenter(new Vector3);
        const sceneBoxGeometry = new BoxGeometry(boxSize.x, boxSize.y, boxSize.z);
        sceneBoxGeometry.translate(boxCenter.x, boxCenter.y, boxCenter.z);
        this.boxMesh = new Mesh(sceneBoxGeometry, new MeshBasicMaterial({
            color: this.color,
            transparent: true,
            opacity: this.opacity
        }));
        this.boxWire = new BoxHelper(this.boxMesh, this.color);
        this.boxWire.material = new LineBasicMaterial({color: this.color});
        this.group.add(this.boxMesh);
        this.group.add(this.boxWire);
    }

    public update(): void {
        const boxMaterial = this.boxMesh.material as MeshBasicMaterial;
        boxMaterial.opacity = this.opacity;
        boxMaterial.color = new Color(this.color);
        boxMaterial.needsUpdate = true;
        const boxSize = this.box.getSize(new Vector3);
        const boxCenter = this.box.getCenter(new Vector3);
        const sceneBoxGeometry = new BoxGeometry(boxSize.x, boxSize.y, boxSize.z);
        sceneBoxGeometry.translate(boxCenter.x, boxCenter.y, boxCenter.z);
        this.boxMesh.geometry = sceneBoxGeometry;
        this.group.remove(this.boxWire);
        this.boxWire = new BoxHelper(new Mesh(sceneBoxGeometry), this.color);
        this.boxWire.material = new LineBasicMaterial({color: this.color});
        this.group.add(this.boxWire);
    }

    public addTo(scene: Object3D): void {
        scene.add(this.group);
    }

    public removeFrom(scene: Object3D): void {
        scene.remove(this.group);
    }
}

export const boxFromOrthographicViewVolume = (camera: OrthographicCamera): Box3 => {
    const min = new Vector3(Math.min(camera.left, camera.right), Math.min(camera.bottom, camera.top), Math.min(camera.near, camera.far));
    const max = new Vector3(Math.max(camera.left, camera.right), Math.max(camera.bottom, camera.top), Math.max(camera.near, camera.far));
    const box = new Box3(min, max);
    return box;
}

export const boundingBoxInViewSpace = (worldBox: Box3, camera: Camera): Box3 => {
    camera.updateMatrixWorld();
    const viewMatrix = camera.matrixWorldInverse;
    const viewBox = new Box3();
    viewBox.expandByPoint(new Vector3(worldBox.min.x, worldBox.min.y, worldBox.min.z).applyMatrix4(viewMatrix));
    viewBox.expandByPoint(new Vector3(worldBox.min.x, worldBox.min.y, worldBox.max.z).applyMatrix4(viewMatrix));
    viewBox.expandByPoint(new Vector3(worldBox.min.x, worldBox.max.y, worldBox.min.z).applyMatrix4(viewMatrix));
    viewBox.expandByPoint(new Vector3(worldBox.min.x, worldBox.max.y, worldBox.max.z).applyMatrix4(viewMatrix));
    viewBox.expandByPoint(new Vector3(worldBox.max.x, worldBox.min.y, worldBox.min.z).applyMatrix4(viewMatrix));
    viewBox.expandByPoint(new Vector3(worldBox.max.x, worldBox.min.y, worldBox.max.z).applyMatrix4(viewMatrix));
    viewBox.expandByPoint(new Vector3(worldBox.max.x, worldBox.max.y, worldBox.min.z).applyMatrix4(viewMatrix));
    viewBox.expandByPoint(new Vector3(worldBox.max.x, worldBox.max.y, worldBox.max.z).applyMatrix4(viewMatrix));
    return viewBox;
}

export class CameraUpdate {
    private lastCameraProjection: Matrix4 | undefined;
    private lastCameraWorld: Matrix4 | undefined;

    public changed(camera: Camera): boolean {
        const hasChanged = !this.lastCameraProjection?.equals(camera.projectionMatrix) || !this.lastCameraWorld?.equals(camera.matrixWorld);
        this.lastCameraProjection = camera.projectionMatrix.clone();
        this.lastCameraWorld = camera.matrixWorld.clone();
        return hasChanged;
    }
} 
