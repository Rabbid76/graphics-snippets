import * as THREE from 'three';

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

    public render(scene: THREE.Scene, renderMethod: () => void ) {
        this.overrideVisibility(scene);
        renderMethod();
        this.restoreVisibility(scene);
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

const originalClearColor = new THREE.Color()

export const renderOverride = (renderer: THREE.WebGLRenderer, scene: THREE.Scene, camera: THREE.Camera, overrideMaterial: THREE.Material, renderTarget: THREE.WebGLRenderTarget | null, clearColor: any, clearAlpha: any): void => {
    renderer.getClearColor(originalClearColor);
    const originalClearAlpha = renderer.getClearAlpha();
    const originalAutoClear = renderer.autoClear;

    renderer.setRenderTarget(renderTarget);
    renderer.autoClear = false;

    // @ts-ignore    
    clearColor = overrideMaterial.clearColor || clearColor;
    // @ts-ignore  
    clearAlpha = overrideMaterial.clearAlpha || clearAlpha;

    if ((clearColor !== undefined ) && (clearColor !== null)) {
        renderer.setClearColor(clearColor);
        renderer.setClearAlpha(clearAlpha || 0.0);
        renderer.clear();
    }

    scene.overrideMaterial = overrideMaterial;
    renderer.render(scene, camera);
    scene.overrideMaterial = null;

    renderer.autoClear = originalAutoClear;
    renderer.setClearColor(originalClearColor);
    renderer.setClearAlpha(originalClearAlpha );
}