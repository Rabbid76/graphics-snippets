import { createUniformColorTexture } from '../../renderer/render-util'
import { EnvMapReader } from './environemtMapReader';
import * as THREE from 'three';
import { RoomEnvironment } from 'three/examples/jsm/environments/RoomEnvironment.js';
import { EXRLoader } from 'three/examples/jsm/loaders/EXRLoader'
import { RGBELoader } from 'three/examples/jsm/loaders/RGBELoader'

export class EnvironmentLoader {
    public defaultBackgroundColor: THREE.Color = new THREE.Color(0xffffff);
    public defaultEnvironmentColor: THREE.Color = new THREE.Color(0xc0c0c0);
    public defaultEnvironmentTexture: THREE.Texture;
    private renderer: THREE.WebGLRenderer
    private environment: THREE.Texture | null = null;
    private background: THREE.Texture | THREE.Color | null = null;
    private pmremGenerator?: THREE.PMREMGenerator;
    private envMapReader?: EnvMapReader;
    private exrLoader?: EXRLoader;
    private rgbeLoader?: RGBELoader;

    constructor(renderer: THREE.WebGLRenderer) {
        this.renderer = renderer;
        this.defaultEnvironmentTexture = createUniformColorTexture(this.defaultEnvironmentColor);
    }

    public setEnvironment(scene: THREE.Scene) {
        scene.background = this.background ?? this.defaultBackgroundColor;
        scene.environment = this.environment ?? this.defaultEnvironmentTexture;
    }

    public loadDefaultEnvironment(setBackground: boolean) {
        const environmentTexture = this.getPmremGenerator().fromScene(new RoomEnvironment(), 0.04).texture;
        this.environment = environmentTexture;
        this.background = setBackground ? this.environment : this.defaultBackgroundColor;
    }

    public loadEnvmap(resource: string, setBackground: boolean) {
        this.loadAndSetCubeTexture((cubeTexture: THREE.CubeTexture) => {
            const environmentTexture = this.getPmremGenerator().fromCubemap(cubeTexture).texture;
            this.environment = environmentTexture;
            this.background = setBackground ? environmentTexture : this.defaultBackgroundColor;
        }, resource);
    }

    public loadExr(resource: string, setBackground: boolean) {
        this.loadExrAndSetTexture((texture: THREE.Texture) => {
            const environmentTexture = this.getPmremGenerator().fromEquirectangular(texture).texture;
            this.environment = environmentTexture;
            this.background = setBackground ? environmentTexture : this.defaultBackgroundColor;
        }, resource)
    }

    public loadHdr(resource: string, setBackground: boolean) {
        this.loadHdrAndSetTexture((texture: THREE.Texture) => {
            const environmentTexture = this.getPmremGenerator().fromEquirectangular(texture).texture;
            this.environment = environmentTexture;
            this.background = setBackground ? environmentTexture : this.defaultBackgroundColor;
        }, resource)
    }

    private getPmremGenerator(): THREE.PMREMGenerator {
        if (!this.pmremGenerator) {
            this.pmremGenerator = new THREE.PMREMGenerator(this.renderer);
        }
        return this.pmremGenerator;
    }

    private loadAndSetCubeTexture(setCubeTexture: (cubeTexture: THREE.CubeTexture) => void, resource: string): void {
        if (!resource) {
            return;
        }
        if (!this.envMapReader) {
            this.envMapReader = new EnvMapReader();
        }
        this.envMapReader.load(resource).then((texture: any) => {
            const cubeTexture = texture as THREE.CubeTexture;
            if (cubeTexture) {
                setCubeTexture(cubeTexture)
            }
        });
    };
    
    private async loadExrAndSetTexture(setTexture: (texture: THREE.Texture) => void, resource: string) {
        if (!resource) {
            return;
        }
        if (!this.exrLoader) {
            this.exrLoader = new EXRLoader();
        }
        this.exrLoader.load(resource, (texture: THREE.Texture, _textureData: any) => {
            setTexture(texture);
        });
    };
    
    private loadHdrAndSetTexture(setTexture: (texture: THREE.Texture) => void, resource: string): void {
        if (!resource) {
            return;
        }
        if (!this.rgbeLoader) {
            this.rgbeLoader = new RGBELoader();
        }
        this.rgbeLoader.load(resource, (texture: THREE.Texture, _textureData: any) => {
            setTexture(texture);
        });
    };
}