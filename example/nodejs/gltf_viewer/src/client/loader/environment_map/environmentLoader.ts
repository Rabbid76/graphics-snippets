import { createUniformColorTexture } from '../../renderer/render-utility'
import { EnvMapReader } from './environemtMapReader';
import {
    Color,
    CubeTexture,
    PMREMGenerator,
    Scene,
    Texture,
    WebGLRenderer,
} from 'three';
import { RoomEnvironment } from 'three/examples/jsm/environments/RoomEnvironment.js';
import { EXRLoader } from 'three/examples/jsm/loaders/EXRLoader'
import { RGBELoader } from 'three/examples/jsm/loaders/RGBELoader'
import { GUI, GUIController } from 'dat.gui';

class Environment {
    public environment: Texture | null = null;
    public background: Texture | Color | null = null;
    constructor(environment: Texture, background: Texture | Color) {
        this.environment = environment;
        this.background = background;
    }
}

export class EnvironmentLoader {
    public defaultBackgroundColor: Color = new Color(0xffffff);
    public defaultEnvironmentColor: Color = new Color(0xc0c0c0);
    public defaultEnvironmentTexture: Texture;
    private renderer: WebGLRenderer
    private pmremGenerator?: PMREMGenerator;
    private envMapReader?: EnvMapReader;
    private exrLoader?: EXRLoader;
    private rgbeLoader?: RGBELoader;
    private environemtMap: Map<string, Environment> = new Map();
    private uiFolder?: GUI = undefined;
    private environmentController?: GUIController = undefined;
    private environmentName: string = '';

    constructor(renderer: WebGLRenderer) {
        this.renderer = renderer;
        this.defaultEnvironmentTexture = createUniformColorTexture(this.defaultEnvironmentColor);
    }

    public setEnvironment(scene: Scene, showBackground: boolean) {
        const environment = this.environemtMap.get(this.environmentName.length > 0 ? this.environmentName : 'room environment');
        if (environment) {
            scene.background = (showBackground && environment.background) ? environment.background : this.defaultBackgroundColor;
            scene.environment = environment.environment ?? this.defaultEnvironmentTexture;
        }
    }

    public loadDefaultEnvironment(changeEnvironment: boolean, createScene?: () => Scene) {
        const defaultEnvironmentName = 'room environment';
        const roomScene = (createScene && createScene()) ?? new RoomEnvironment();
        const environmentTexture = this.getPmremGenerator().fromScene(roomScene, 0.04).texture;
        this.environemtMap.set(defaultEnvironmentName, new Environment(environmentTexture, environmentTexture));
        if (changeEnvironment) {
            this.environmentName = defaultEnvironmentName;
        }
        this.updateUI();
    }

    public loadEnvmap(resourceName: string, resource: string, changeEnvironment: boolean) {
        this.loadAndSetCubeTexture((cubeTexture: CubeTexture) => {
            const environmentTexture = this.getPmremGenerator().fromCubemap(cubeTexture).texture;
            this.environemtMap.set(resourceName, new Environment(environmentTexture, environmentTexture));
            if (changeEnvironment) {
                this.environmentName = resourceName;
            }
            this.updateUI();
        }, resource);
    }

    public loadExr(resourceName: string, resource: string, changeEnvironment: boolean) {
        this.loadExrAndSetTexture((texture: Texture) => {
            const environmentTexture = this.getPmremGenerator().fromEquirectangular(texture).texture;
            this.environemtMap.set(resourceName, new Environment(environmentTexture, environmentTexture));
            if (changeEnvironment) {
                this.environmentName = resourceName;
            }
            this.updateUI();
        }, resource)
    }

    public loadHdr(resourceName: string, resource: string, changeEnvironment: boolean) {
        this.loadHdrAndSetTexture((texture: Texture) => {
            const environmentTexture = this.getPmremGenerator().fromEquirectangular(texture).texture;
            this.environemtMap.set(resourceName, new Environment(environmentTexture, environmentTexture));
            if (changeEnvironment) {
                this.environmentName = resourceName;
            }
            this.updateUI();
        }, resource)
    }

    private getPmremGenerator(): PMREMGenerator {
        if (!this.pmremGenerator) {
            this.pmremGenerator = new PMREMGenerator(this.renderer);
        }
        return this.pmremGenerator;
    }

    private loadAndSetCubeTexture(setCubeTexture: (cubeTexture: CubeTexture) => void, resource: string): void {
        if (!resource) {
            return;
        }
        if (!this.envMapReader) {
            this.envMapReader = new EnvMapReader();
        }
        this.envMapReader.load(resource).then((texture: any) => {
            const cubeTexture = texture as CubeTexture;
            if (cubeTexture) {
                setCubeTexture(cubeTexture)
            }
        });
    }
    
    private async loadExrAndSetTexture(setTexture: (texture: Texture) => void, resource: string) {
        if (!resource) {
            return;
        }
        if (!this.exrLoader) {
            this.exrLoader = new EXRLoader();
        }
        this.exrLoader.load(resource, (texture: Texture, _textureData: any) => {
            setTexture(texture);
        });
    }
    
    private loadHdrAndSetTexture(setTexture: (texture: Texture) => void, resource: string): void {
        if (!resource) {
            return;
        }
        if (!this.rgbeLoader) {
            this.rgbeLoader = new RGBELoader();
        }
        this.rgbeLoader.load(resource, (texture: Texture, _textureData: any) => {
            setTexture(texture);
        });
    }

    public addGUI(uiFolder: GUI): void {
        this.uiFolder = uiFolder;
        this.updateUI();
    }

    private updateUI(): void {
        if (this.uiFolder) {
            const environmentNames = Array.from(this.environemtMap.keys());
            if (this.environmentController) {
                let innerHTMLStr = "";
                environmentNames.forEach((environmentName) => {
                    innerHTMLStr += "<option value='" + environmentName + "'>" + environmentName + "</option>";
                });
                this.environmentController.domElement.children[0].innerHTML = innerHTMLStr;
                this.environmentController.setValue(this.environmentName);
                this.environmentController.updateDisplay();
            } else {
                this.environmentController = this.uiFolder.add<any>(this, 'environmentName', environmentNames);
            }
        }
    }
}