import { viewSpacePositionFromUV } from '../renderer/render-utility'
import {
    Object3D,
} from 'three';
import { Sky } from 'three/examples/jsm/objects/Sky.js';
import { GUI, GUIController } from 'dat.gui'

export interface SkyParameter {
    [key: string]: any;
    visible: boolean;
    distance: number;
    turbidity: number;
    rayleigh: number;
    mieCoefficient: number;
    mieDirectionalG: number;
    inclination: number;
    azimuth: number;
}

export class SkyEnvironment {
    public parameters: SkyParameter;
    public sky: Sky;

    constructor(parameters?: any) {
        this.sky = new Sky();
        this.sky.name = 'Sky';
        this.parameters = {
            visible: true,
            distance: 400000,
            turbidity: 10,
            rayleigh: 2,
            mieCoefficient: 0.005,
            mieDirectionalG: 0.8,
            inclination: 0.6,
            azimuth: 0,
            ...parameters,
        };
        this.updateSky();
    }

    private updateParameters(parameters: any) {
        for (let propertyName in parameters) {
            if (this.parameters.hasOwnProperty(propertyName)) {
                this.parameters[propertyName] = parameters[propertyName];
            }
        }
    }

    public updateSky() {
        this.sky.scale.setScalar(450000);
        this.sky.frustumCulled = false;
  
        this.sky.material.uniforms.turbidity.value = this.parameters.turbidity;
        this.sky.material.uniforms.rayleigh.value = this.parameters.rayleigh;
        this.sky.material.uniforms.mieCoefficient.value = this.parameters.mieCoefficient;
        this.sky.material.uniforms.mieDirectionalG.value = this.parameters.mieDirectionalG;
        let sunPosition = viewSpacePositionFromUV(
          this.parameters.distance,
          this.parameters.azimuth,
          this.parameters.inclination,
        );
        this.sky.material.uniforms.sunPosition.value.copy(sunPosition);
        this.sky.visible = this.parameters.visible;
        /*
        const sunSphere = new Mesh(
          new SphereGeometry(20000, 16, 8),
          new MeshBasicMaterial({ color: 0xffffff })
        );
        sunSphere.name = 'Sunsphere';
        sunSphere.position.y = -700000;
        sunSphere.visible = false;
        sunSphere.position.set(sunPosition.x, -sunPosition.z, sunPosition.y);
        //sunSphere.visible = true;
        */
    }

    public addToScene(scene: Object3D) {
        scene.add(this.sky);
    }

    public changeVisibility(visible: boolean) {
        if (this.parameters.visible !== visible) {
            this.updateParameters({ visible });
            this.updateSky();
        }
    }
}

export class SkyEnvironmentGUI {
    private skyEnvironment: SkyEnvironment;
    private showSkyController?: GUIController; 

    constructor(skyEnvironment: SkyEnvironment) {
        this.skyEnvironment = skyEnvironment;
    }

    public hideSky(): void {
        this.showSkyController?.setValue(false);
        this.showSkyController?.updateDisplay();
    }

    public addGUI(gui: GUI, updateCallback?: (skyEnvironment: SkyEnvironment) => void): void {
        this.showSkyController = gui.add<any>(this.skyEnvironment.parameters, 'visible').onChange(() => {
            this.skyEnvironment.updateSky();
            if (updateCallback) {
                updateCallback(this.skyEnvironment);
            }
        });
    }
}
