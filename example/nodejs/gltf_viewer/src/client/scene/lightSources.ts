import { SceneRenderer } from '../renderer/scene-renderer'
import { SceneVolume } from '../renderer/render-utility'
import * as THREE from 'three'
import { GUI } from 'dat.gui'

export class LightSources {
    private static defaultLightSources = [
        {
            type: 'ambient',
            color: '#ffffff'
        },
        {
            type: 'rectArea',
            position: { x: 0, y: 5, z: 3 },
            color: '#ffffff',
            intensity: 60,
            castShadow: true,
        },
        {
            type: 'rectArea',
            position: { x: -5, y: 3, z: 2 },
            color: '#ffffff',
            intensity: 60,
            castShadow: false,
        }
    ];

    public lightControls: boolean = false;
    private _scene: THREE.Scene
    private _lightSources: THREE.Light[] = [];
    private _useRectAreaLight: boolean = true;
    public sceneRenderer: SceneRenderer | undefined;

    get useRectAreaLight() { 
        return this._useRectAreaLight;
    }

    constructor(scene: THREE.Scene, sceneRenderer: SceneRenderer, width: number, height: number, samples: number, parameters?: any) {
        this._scene = scene
        this.sceneRenderer = sceneRenderer;
        this.readLightSources(scene, LightSources.defaultLightSources);
        this.addToScene();
    }

    private readLightSources(scene: THREE.Scene, lightSourceDefinitions: any[]) {
        let count = 0;
        lightSourceDefinitions.forEach(definition => {
            switch (definition.type) {
                default: break;
                case 'ambient':
                    const ambientLight = new THREE.AmbientLight(new THREE.Color(definition.color), 0.0);
                    this._lightSources.push(ambientLight);
                    break;
                case 'rectArea':
                    if (this._useRectAreaLight) {
                        const rectAreaLightWidth = 0.8;
                        const rectAreaLightHeight = 0.8;
                        const intensity = (definition.intensity ?? 100) / (rectAreaLightWidth * rectAreaLightHeight);
                        const rectAreaLight = new THREE.RectAreaLight(new THREE.Color(definition.color), intensity,  rectAreaLightWidth, rectAreaLightHeight);
                        rectAreaLight.position.set(definition.position.x, definition.position.y, definition.position.z);
                        rectAreaLight.matrixAutoUpdate = true;
                        rectAreaLight.visible = count === 0;
                        rectAreaLight.lookAt(new THREE.Vector3(0, 0, 0));
                        this._lightSources.push(rectAreaLight);
                        this.sceneRenderer?.addRectAreaLight(rectAreaLight, scene);
                        count ++;
                        const shadowLight = this.sceneRenderer?.screenSpaceShadow.findShadowLightSource(rectAreaLight);
                        if (shadowLight) {
                            shadowLight.castShadow = definition.castShadow;
                        }
                    } else {
                        const directionalLight = new THREE.DirectionalLight(new THREE.Color(definition.color), definition.intensity);
                        directionalLight.position.set(directionalLight.position.x, directionalLight.position.y, directionalLight.position.z);
                        directionalLight.visible = true;
                        directionalLight.intensity = definition.intensity;
                        directionalLight.castShadow = definition.castShadow;
                        directionalLight.lookAt(new THREE.Vector3(0, 0, 0));
                        this._lightSources.push(directionalLight);
                    }
                    break;
            }
        });
    }

    public getLightSources(): THREE.Light[] {
        return this._lightSources;
    }

    public getShadowLightSources(): THREE.Light[] {
        const shadowLightSources: THREE.Light[] = [];
        for (let i = 0; i < this._lightSources.length; i++) {
            const light = this._lightSources[i];
            if (light instanceof THREE.AmbientLight) {
                continue;
            }
            shadowLightSources.push(this.sceneRenderer?.screenSpaceShadow.findShadowLightSource(light) || light);
        }
        return shadowLightSources;
    }

    public updateBounds(sceneVolume: SceneVolume) {
        this.getShadowLightSources().forEach(light => {
            LightSources.updateDirectionalLightBounds(light, sceneVolume)
        });
    }

    private static updateDirectionalLightBounds(light: THREE.Light, sceneVolume: SceneVolume): void {
        const shadowMap = light.shadow
        const distanceToTarget = light.position.length()
        const shadowCamera = shadowMap.camera as THREE.PerspectiveCamera || shadowMap.camera as THREE.OrthographicCamera
        shadowCamera.near = Math.max(0.1, distanceToTarget - sceneVolume.maxSceneDistanceFrom0);
        shadowCamera.far = distanceToTarget * 2;
        const orthoGraphicShadowCamera = shadowMap.camera as THREE.OrthographicCamera
        const shadowScale = 2.5; 
        if (orthoGraphicShadowCamera) {
            orthoGraphicShadowCamera.left = -sceneVolume.maxSceneDistanceFrom0 * shadowScale
            orthoGraphicShadowCamera.right = sceneVolume.maxSceneDistanceFrom0 * shadowScale
            orthoGraphicShadowCamera.top = sceneVolume.maxSceneDistanceFrom0 * shadowScale
            orthoGraphicShadowCamera.bottom = -sceneVolume.maxSceneDistanceFrom0 * shadowScale
        }
        shadowCamera.updateProjectionMatrix()
    }

    public addToScene(): void {
        this._lightSources.forEach(light => this._scene.add(light));
    }

    public removeFromScene(): void {
        this._lightSources.forEach(light => this._scene.remove(light));
    }

    public reload(): void {
        this.removeFromScene();
        this.addToScene();
    }
}

export class LightSourcesGUI {
    private lightSources: LightSources;
    private lightColors: any = [];

    constructor(lightSources: LightSources) {
      this.lightSources = lightSources;
    }

    public addGUI(gui: GUI, lightControlsUpdate: () => void): void {
        gui.add(this.lightSources, 'lightControls').onChange(lightControlsUpdate);
        this.lightSources.getLightSources().forEach(light => {
            if (light instanceof THREE.AmbientLight) {
                this.lightColors.push({
                    color: light.color.getHex()
                });
                let ambiLight = gui.addFolder('Ambient Light');
                ambiLight.add(light, 'visible');
                ambiLight.add(light, 'intensity').min(0).max(5).step(0.1);
                ambiLight.addColor(this.lightColors[this.lightColors.length-1], 'color').onChange((color: string) => light.color = new THREE.Color(color));
            }
        });
        this.lightSources.getLightSources().forEach(light => {
            if (light instanceof THREE.DirectionalLight) {
                this.lightColors.push({
                    color: light.color.getHex()
                });
                let lightFolder = gui.addFolder(`Directional Light ${this.lightColors.length-1}`);
                lightFolder.add(light, 'visible');
                lightFolder.add(light, 'intensity').min(0).max(10).step(0.1);
                lightFolder.add(light, 'castShadow');
                lightFolder.addColor(this.lightColors[this.lightColors.length-1], 'color').onChange((color: string) => light.color = new THREE.Color(color));
            } else if (light instanceof THREE.RectAreaLight) {
                this.lightColors.push({
                    color: light.color.getHex()
                });
                let lightFolder = gui.addFolder(`Rect area Light ${this.lightColors.length-1}`);
                const shadowLight = this.lightSources.sceneRenderer?.screenSpaceShadow.findShadowLightSource(light);
                lightFolder.add(light, 'visible');
                lightFolder.add(light, 'intensity').min(0).max(200).step(1);
                if (shadowLight) {
                    lightFolder.add(shadowLight, 'castShadow');
                }
                lightFolder.addColor(this.lightColors[this.lightColors.length-1], 'color').onChange((color: string) => light.color = new THREE.Color(color));
            }
        });
    }
}