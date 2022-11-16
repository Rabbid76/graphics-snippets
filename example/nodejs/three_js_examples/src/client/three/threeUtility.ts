import * as THREE from 'three';
import { FullScreenQuad } from 'three/examples/jsm/postprocessing/Pass';

const textureLoader: THREE.TextureLoader = new THREE.TextureLoader();

export const createUniformColorTexture = (color: THREE.Color): THREE.Texture => {
    const colorTextureData = new Uint8Array([Math.floor(color.r * 255), Math.floor(color.g * 255), Math.floor(color.b * 255), 255]);
    const colorTexture = new THREE.DataTexture(colorTextureData, 1, 1);
    colorTexture.needsUpdate = true;
    return colorTexture;
};

export const createUniformColorCubeTexture = (color: THREE.Color) => {
    const cubeTexture = new THREE.CubeTexture();
    cubeTexture.format = THREE.RGBAFormat;
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

export const createNoiseTexture = (size: number, min: number, max: number): THREE.Texture => {
    const noiseTextureData = new Uint8Array(size*size*4);
    for (let i = 0; i < noiseTextureData.length; ++i) {
        noiseTextureData[i] = Math.floor((min + (max - min) * Math.random()) * 255);
    }
    const noiseTexture = new THREE.DataTexture(noiseTextureData, size, size);
    noiseTexture.needsUpdate = true;
    return noiseTexture;
};

export const loadAndSetTexture = (setTexture: (texture: THREE.Texture) => void, resource: string, color?: THREE.Color): void => {
    if (color) {
        setTexture(createUniformColorTexture(color));
    }
    if (resource) {
        textureLoader.load(resource, setTexture);
    }
};

export class SceneVolume {
    public sceneBounds: THREE.Box3 = new THREE.Box3();
    public maxSceneDistanceFrom0: number = 10;

    public update(sceneObject: THREE.Object3D): void {
        this.sceneBounds.setFromObject(sceneObject);
        this.maxSceneDistanceFrom0 = new THREE.Vector3(
            Math.max(Math.abs(this.sceneBounds.min.x), Math.abs(this.sceneBounds.max.x)),
            Math.max(Math.abs(this.sceneBounds.min.y), Math.abs(this.sceneBounds.max.y)),
            Math.max(Math.abs(this.sceneBounds.min.z), Math.abs(this.sceneBounds.max.z))).length();
    }

    public getNearAndFarForCameraThatLooksAtOriginOfScene(cameraPosition: THREE.Vector3, backScale: number = 1): number[] {
        const distanceFromCenter = cameraPosition.length();
        const near = Math.max(0.01, distanceFromCenter - this.maxSceneDistanceFrom0 - 0.01);
        const far = distanceFromCenter + this.maxSceneDistanceFrom0 * backScale + 0.01;
        return [near, far];
    }
}

export const getMaxSamples = (renderTarget: THREE.WebGLRenderer): number => {
    const capabilities: THREE.WebGLCapabilities = renderTarget.capabilities
    // @ts-ignore
    const maxSamples: number = capabilities.maxSamples;
    return maxSamples;
}

export class RenderOverrideVisibility {
    private _visibilityCache = new Map();
    private background: any;
    private hideLinesAndPoints: boolean;
    private invisibleObjects?: any[]

    constructor(hideLinesAndPoints?: boolean, invisibleObjects?: any[], background?: any) {
        this.background = background;
        this.hideLinesAndPoints = hideLinesAndPoints ?? false;
        this.invisibleObjects = invisibleObjects;
    }

    public render(scene: THREE.Scene, renderMethod: () => void) {
        const sceneBackground = scene.background;
        if (this.background !== undefined) {
            scene.background = this.background;
        }
        const backup = this.invisibleObjects?.map(item => { return { object: item, visible: item.visible }; }) ?? null;
        this.invisibleObjects?.forEach(item => item.visible = false);
        if (this.hideLinesAndPoints) {
            this.overrideVisibility(scene);
        }
        renderMethod();
        if (this.hideLinesAndPoints) {
            this.restoreVisibility(scene);
        }
        backup?.forEach(item => item.object.visible = item.visible);
        if (this.background !== undefined) {
            scene.background = sceneBackground;
        }
    }

    private overrideVisibility(scene: THREE.Scene) {
        const cache = this._visibilityCache;
        scene.traverse(object => {
            cache.set(object, object.visible);
            // @ts-ignore
            if (object.isPoints || object.isLine) {
                object.visible = false;
            }
        })
	}

	private restoreVisibility(scene: THREE.Scene) {
        const cache = this._visibilityCache;
        scene.traverse(object => {
            const visible = cache.get(object);
            object.visible = visible;
        })
        cache.clear();
	}
}

export class RenderPass {
    private originalClearColor = new THREE.Color();
    private originalClearAlpha: number = 0;
    private originalAutoClear: boolean = false;
    private originalRenderTarget: THREE.WebGLRenderTarget | null = null;
    private screenSpaceQuad = new FullScreenQuad(undefined);

    public renderWithOverrideMaterial(renderer: THREE.WebGLRenderer, scene: THREE.Scene, camera: THREE.Camera, overrideMaterial: THREE.Material, renderTarget: THREE.WebGLRenderTarget | null, clearColor?: any, clearAlpha?: any): void {
        this.backup(renderer);
        // @ts-ignore    
        clearColor = overrideMaterial.clearColor || clearColor;
        // @ts-ignore  
        clearAlpha = overrideMaterial.clearAlpha || clearAlpha;
        this.prepareRenderer(renderer, renderTarget, clearColor, clearAlpha);
        const originalOverrideMaterial = scene.overrideMaterial
        scene.overrideMaterial = overrideMaterial;
        renderer.render(scene, camera);
        scene.overrideMaterial = originalOverrideMaterial;
        this.restore(renderer);
    }

    public renderScreenSpace(renderer: THREE.WebGLRenderer, passMaterial: THREE.Material, renderTarget: THREE.WebGLRenderTarget | null, clearColor?: any, clearAlpha?: any): void {
        this.backup(renderer);
        this.prepareRenderer(renderer, renderTarget, clearColor, clearAlpha);
        this.screenSpaceQuad.material = passMaterial;
        this.screenSpaceQuad.render(renderer);
        this.restore(renderer);
    }

    private prepareRenderer(renderer: THREE.WebGLRenderer, renderTarget: THREE.WebGLRenderTarget | null, clearColor?: any, clearAlpha?: any): void {
        renderer.setRenderTarget(renderTarget);
        renderer.autoClear = false;
        if ((clearColor !== undefined) && (clearColor !== null)) {
            renderer.setClearColor(clearColor);
            renderer.setClearAlpha(clearAlpha || 0.0);
            renderer.clear();
        }
    }

    private backup(renderer: THREE.WebGLRenderer): void {
        renderer.getClearColor(this.originalClearColor);
        this.originalClearAlpha = renderer.getClearAlpha();
        this.originalAutoClear = renderer.autoClear;
        this.originalRenderTarget = renderer.getRenderTarget();
        
    }

    private restore(renderer: THREE.WebGLRenderer): void {
        renderer.setClearColor(this.originalClearColor);
        renderer.setClearAlpha(this.originalClearAlpha);
        renderer.setRenderTarget(this.originalRenderTarget);
        renderer.autoClear = this.originalAutoClear;
    }
}

export class BoxHelper {
    public box: THREE.Box3;
    public color: THREE.Color;
    public opacity: number;
    private group: THREE.Group;
    private boxMesh: THREE.Mesh;
    private boxWire: THREE.BoxHelper;

    public get visible(): boolean { return this.group.visible; }
    public set visible(isVisible: boolean) { this.group.visible = isVisible; }
    public get object(): THREE.Object3D { return this.group; }

    constructor(box: THREE.Box3, parameters?: any) {
        this.group = new THREE.Group();
        this.box = box;
        this.color = parameters?.color ?? 0x808080;
        this.opacity = parameters?.opacity ?? 0.5;
        const boxSize = this.box.getSize(new THREE.Vector3);
        const boxCenter = this.box.getCenter(new THREE.Vector3);
        const sceneBoxGeometry = new THREE.BoxGeometry(boxSize.x, boxSize.y, boxSize.z);
        sceneBoxGeometry.translate(boxCenter.x, boxCenter.y, boxCenter.z);
        this.boxMesh = new THREE.Mesh(sceneBoxGeometry, new THREE.MeshBasicMaterial({
            color: this.color,
            transparent: true,
            opacity: this.opacity
        }));
        this.boxWire = new THREE.BoxHelper(this.boxMesh, this.color);
        this.boxWire.material = new THREE.LineBasicMaterial({color: this.color});
        this.group.add(this.boxMesh);
        this.group.add(this.boxWire);
    }

    public update(): void {
        const boxMaterial = this.boxMesh.material as THREE.MeshBasicMaterial;
        boxMaterial.opacity = this.opacity;
        boxMaterial.color = new THREE.Color(this.color);
        boxMaterial.needsUpdate = true;
        const boxSize = this.box.getSize(new THREE.Vector3);
        const boxCenter = this.box.getCenter(new THREE.Vector3);
        const sceneBoxGeometry = new THREE.BoxGeometry(boxSize.x, boxSize.y, boxSize.z);
        sceneBoxGeometry.translate(boxCenter.x, boxCenter.y, boxCenter.z);
        this.boxMesh.geometry = sceneBoxGeometry;
        this.group.remove(this.boxWire);
        this.boxWire = new THREE.BoxHelper(new THREE.Mesh(sceneBoxGeometry), this.color);
        this.boxWire.material = new THREE.LineBasicMaterial({color: this.color});
        this.group.add(this.boxWire);
    }

    public addTo(scene: THREE.Object3D): void {
        scene.add(this.group);
    }

    public removeFrom(scene: THREE.Object3D): void {
        scene.remove(this.group);
    }
}

export const boxFromOrthographicViewVolume = (camera: THREE.OrthographicCamera): THREE.Box3 => {
    const min = new THREE.Vector3(Math.min(camera.left, camera.right), Math.min(camera.bottom, camera.top), Math.min(camera.near, camera.far));
    const max = new THREE.Vector3(Math.max(camera.left, camera.right), Math.max(camera.bottom, camera.top), Math.max(camera.near, camera.far));
    const box = new THREE.Box3(min, max);
    return box;
}

export const setOrthographicViewVolumeFromBox = (camera: THREE.OrthographicCamera, viewBox: THREE.Box3): void => {
    camera.left = viewBox.min.x;
    camera.right = viewBox.max.x;
    camera.bottom = viewBox.min.y;
    camera.top = viewBox.max.y;
    camera.near = Math.min(-viewBox.min.z, -viewBox.max.z);
    camera.far = Math.max(-viewBox.min.z, -viewBox.max.z);
    camera.updateProjectionMatrix();
}

export const boundingBoxInViewSpace = (worldBox: THREE.Box3, camera: THREE.Camera): THREE.Box3 => {
    camera.updateMatrixWorld();
    const viewMatrix = camera.matrixWorldInverse;
    const viewBox = new THREE.Box3();
    viewBox.expandByPoint(new THREE.Vector3(worldBox.min.x, worldBox.min.y, worldBox.min.z).applyMatrix4(viewMatrix));
    viewBox.expandByPoint(new THREE.Vector3(worldBox.min.x, worldBox.min.y, worldBox.max.z).applyMatrix4(viewMatrix));
    viewBox.expandByPoint(new THREE.Vector3(worldBox.min.x, worldBox.max.y, worldBox.min.z).applyMatrix4(viewMatrix));
    viewBox.expandByPoint(new THREE.Vector3(worldBox.min.x, worldBox.max.y, worldBox.max.z).applyMatrix4(viewMatrix));
    viewBox.expandByPoint(new THREE.Vector3(worldBox.max.x, worldBox.min.y, worldBox.min.z).applyMatrix4(viewMatrix));
    viewBox.expandByPoint(new THREE.Vector3(worldBox.max.x, worldBox.min.y, worldBox.max.z).applyMatrix4(viewMatrix));
    viewBox.expandByPoint(new THREE.Vector3(worldBox.max.x, worldBox.max.y, worldBox.min.z).applyMatrix4(viewMatrix));
    viewBox.expandByPoint(new THREE.Vector3(worldBox.max.x, worldBox.max.y, worldBox.max.z).applyMatrix4(viewMatrix));
    return viewBox;
}