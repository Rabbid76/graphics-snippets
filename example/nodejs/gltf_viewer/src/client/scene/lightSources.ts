import { SceneRenderer } from '../renderer/scene-renderer'
import { SceneVolume } from '../renderer/render-utility'
import {
    AmbientLight,
    Color,
    ColorRepresentation,
    DirectionalLight,
    Light,
    OrthographicCamera,
    PerspectiveCamera,
    RectAreaLight,
    Scene,
    Vector3,
} from 'three';
import { GUI } from 'dat.gui';

export class LightSources {
    public static defaultLightSources = [
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
    public static fifeLightSources = [
        {
            type: 'ambient',
            color: '#ffffff'
        },
        {
            type: 'rectArea',
            position: { x: 0, y: 5, z: 0 },
            color: '#ffffff',
            intensity: 40,
            castShadow: true,
        },
        {
            type: 'rectArea',
            position: { x: 5, y: 5, z: 5 },
            color: '#ffffff',
            intensity: 60,
            castShadow: true,
        },
        {
            type: 'rectArea',
            position: { x: -5, y: 5, z: 5 },
            color: '#ffffff',
            intensity: 60,
            castShadow: true,
        },
        {
            type: 'rectArea',
            position: { x: 5, y: 5, z: -5 },
            color: '#ffffff',
            intensity: 60,
            castShadow: true,
        },
        {
            type: 'rectArea',
            position: { x: -5, y: 5, z: -5 },
            color: '#ffffff',
            intensity: 60,
            castShadow: true,
        },
    ];

    public lightControls: boolean = false;
    private _scene: Scene
    private _lightSources: Light[] = [];
    private _useRectAreaLight: boolean = true;
    public sceneRenderer: SceneRenderer | undefined;
    private lightSourceScale?: number;
    public currentLightSourceDefinition: any[] = LightSources.defaultLightSources;

    get useRectAreaLight() { 
        return this._useRectAreaLight;
    }

    constructor(scene: Scene, sceneRenderer: SceneRenderer, width: number, height: number, samples: number, parameters?: any) {
        this._scene = scene
        this.sceneRenderer = sceneRenderer;
        this.updateLightSources();
    }

    public updateLightSources() {
        this.removeFromScene();
        this._lightSources = [];
        this.readLightSources(this._scene, this.currentLightSourceDefinition);
        this.addToScene();
    }

    private readLightSources(scene: Scene, lightSourceDefinitions: any[]) {
        const rectAreaLights: RectAreaLight[] = [];
        lightSourceDefinitions.forEach(definition => {
            switch (definition.type) {
                default: break;
                case 'ambient':
                    const ambientLight = new AmbientLight(new Color(definition.color), 0.0);
                    this._lightSources.push(ambientLight);
                    break;
                case 'rectArea':
                    const position = new Vector3(definition.position.x, definition.position.y, definition.position.z);
                    if (this.lightSourceScale && position.length() < this.lightSourceScale) {
                        position.normalize().multiplyScalar(this.lightSourceScale);
                    }
                    if (this._useRectAreaLight) {
                        const rectAreaLightWidth = 0.8;
                        const rectAreaLightHeight = 0.8;
                        const intensity = (definition.intensity ?? 100) / (rectAreaLightWidth * rectAreaLightHeight);
                        const rectAreaLight = new RectAreaLight(new Color(definition.color), intensity,  rectAreaLightWidth, rectAreaLightHeight);
                        rectAreaLight.position.copy(position);
                        rectAreaLight.matrixAutoUpdate = true;
                        rectAreaLight.visible = definition.castShadow;
                        rectAreaLight.lookAt(new Vector3(0, 0, 0));
                        this._lightSources.push(rectAreaLight);
                        rectAreaLights.push(rectAreaLight);
                    } else {
                        const directionalLight = new DirectionalLight(new Color(definition.color), definition.intensity);
                        directionalLight.position.copy(position);
                        directionalLight.visible = true;
                        directionalLight.intensity = definition.intensity;
                        directionalLight.castShadow = definition.castShadow;
                        directionalLight.lookAt(new Vector3(0, 0, 0));
                        this._lightSources.push(directionalLight);
                    }
                    break;
            }
        });
        this.sceneRenderer?.updateRectAreaLights(rectAreaLights, scene);
    }

    public getLightSources(): Light[] {
        return this._lightSources;
    }

    public getShadowLightSources(): Light[] {
        const shadowLightSources: Light[] = [];
        for (let i = 0; i < this._lightSources.length; i++) {
            const light = this._lightSources[i];
            if (light instanceof AmbientLight) {
                continue;
            }
            shadowLightSources.push(this.sceneRenderer?.screenSpaceShadow.findShadowLightSource(light) || light);
        }
        return shadowLightSources;
    }

    public setLightSourcesDistances(sceneVolume: SceneVolume, scaleDistance: boolean) {
        this.lightSourceScale = scaleDistance ? sceneVolume.maxSceneDistanceFrom0 * 1.5 : undefined;
        this.updateLightSources();
        this.updateBounds(sceneVolume);
    }

    public updateBounds(sceneVolume: SceneVolume) {
        this.getShadowLightSources().forEach(light => {
            LightSources.updateDirectionalLightBounds(light, sceneVolume)
        });
    }

    private static updateDirectionalLightBounds(light: Light, sceneVolume: SceneVolume): void {
        const shadowMap = light.shadow
        const distanceToTarget = light.position.length()
        const shadowCamera = shadowMap.camera as PerspectiveCamera || shadowMap.camera as OrthographicCamera
        shadowCamera.near = Math.max(0.1, distanceToTarget - sceneVolume.maxSceneDistanceFrom0);
        shadowCamera.far = distanceToTarget * 2;
        const orthoGraphicShadowCamera = shadowMap.camera as OrthographicCamera
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
    private lightGUI: GUI | undefined;
    private lightSourceFolders: GUI[] = [];
    private lights: string = 'default';

    constructor(lightSources: LightSources) {
      this.lightSources = lightSources;
    }

    public addGUI(gui: GUI, lightControlsUpdate: () => void, lightSourcesUpdate: () => void): void {
        gui.add<any>(this.lightSources, 'lightControls').onChange(lightControlsUpdate);
        gui.add<any>(this, 'lights', ['default', 'fife']).onChange((value: string) => {
            switch (value) {
                default:
                case 'default': this.lightSources.currentLightSourceDefinition = LightSources.defaultLightSources; break;
                case 'fife': this.lightSources.currentLightSourceDefinition = LightSources.fifeLightSources; break;
            };
            this.lightSources.updateLightSources();
            this.updateGUI();
            lightSourcesUpdate();
        });
        this.lightGUI = gui;
        this.updateGUI();
    }

    public updateGUI(): void {
        if (!this.lightGUI) {
            return;
        }
        const gui: GUI = this.lightGUI;
        this.lightSourceFolders.forEach(folder => gui.removeFolder(folder));
        this.lightSourceFolders = [];
        this.lightColors = [];
        this.lightSources.getLightSources().forEach(light => {
            if (light instanceof AmbientLight) {
                this.lightColors.push({
                    color: light.color.getHex()
                });
                const ambiLight = gui.addFolder('Ambient Light');
                this.lightSourceFolders.push(ambiLight);
                ambiLight.add<any>(light, 'visible');
                ambiLight.add<any>(light, 'intensity').min(0).max(5).step(0.1);
                ambiLight.addColor(this.lightColors[this.lightColors.length-1], 'color').onChange((color: ColorRepresentation) => light.color = new Color(color));
            }
        });
        this.lightSources.getLightSources().forEach(light => {
            if (light instanceof DirectionalLight) {
                this.lightColors.push({
                    color: light.color.getHex()
                });
                const lightFolder = gui.addFolder(`Directional Light ${this.lightColors.length-1}`);
                this.lightSourceFolders.push(lightFolder);
                lightFolder.add<any>(light, 'visible');
                lightFolder.add<any>(light, 'intensity').min(0).max(10).step(0.1);
                lightFolder.add<any>(light, 'castShadow');
                lightFolder.addColor(this.lightColors[this.lightColors.length-1], 'color').onChange((color: ColorRepresentation) => light.color = new Color(color));
            } else if (light instanceof RectAreaLight) {
                this.lightColors.push({
                    color: light.color.getHex()
                });
                const lightFolder = gui.addFolder(`Rect area Light ${this.lightColors.length-1}`);
                this.lightSourceFolders.push(lightFolder);
                const shadowLight = this.lightSources.sceneRenderer?.screenSpaceShadow.findShadowLightSource(light);
                lightFolder.add<any>(light, 'visible');
                lightFolder.add<any>(light, 'intensity').min(0).max(200).step(1);
                if (shadowLight) {
                    lightFolder.add<any>(shadowLight, 'castShadow');
                }
                lightFolder.addColor(this.lightColors[this.lightColors.length-1], 'color').onChange((color: ColorRepresentation) => light.color = new Color(color));
            }
        });
    }
}