import * as THREE from 'three';
import Stats from 'three/examples/jsm/libs/stats.module' 
import { GUI } from 'dat.gui'

export class Statistic {
    public stats: Stats;

    constructor() {
        // @ts-ignore
        this.stats = new Stats();
        document.body.appendChild(this.stats.dom);
    }

    public update() {
        this.stats.update();
    }
}

export class DataGUI {
    public gui: GUI

    constructor(parameter?: any) {
        this.gui = new GUI(parameter);
    }

    public static addPhysicalMaterialPropertiesUI(gui: GUI, material: THREE.MeshPhysicalMaterial): void {
        const uiMaterialData = {
            color: material.color.getHex(),
            specularColor: material.color.getHex(),
            emissive: material.emissive.getHex(),
            sheenColor: material.emissive.getHex(),
            attenuationColor: material.emissive.getHex()
        };
        gui.addColor(uiMaterialData, 'color').onChange(handleColorChange(material.color, true));
        gui.add(material, 'opacity', 0, 1).onChange((value: number) => { 
            const transparent = value < 1; 
            if (transparent !== material.transparent) {
                material.transparent = transparent;  
                material.needsUpdate = true;
            }
        });
        gui.add(material, 'metalness', 0, 1);
        gui.add(material, 'roughness', 0, 1);
        gui.add(material, 'transmission', 0, 1);
        gui.add(material, 'ior', 1, 2.333);
        gui.add(material, 'specularIntensity', 0, 1);
        gui.addColor(uiMaterialData, 'specularColor').onChange(handleColorChange(material.specularColor, true));
        gui.add(material, 'reflectivity', 0, 1);
        gui.add(material, 'clearcoat', 0, 1);
        gui.add(material, 'clearcoatRoughness', 0, 1.0);
        gui.add(material, 'sheen', 0, 1.0);
        gui.add(material, 'sheenRoughness', 0, 1);
        gui.addColor(uiMaterialData, 'sheenColor').onChange(handleColorChange(material.sheenColor, true));
        gui.add(material, 'emissiveIntensity', 0, 1);
        gui.addColor(uiMaterialData, 'emissive').onChange(handleColorChange(material.emissive, true));
        gui.add(material, 'attenuationDistance', 0, 50);
        gui.addColor(uiMaterialData, 'attenuationColor').onChange(handleColorChange(material.attenuationColor, true));
        gui.add(material, 'thickness', 0, 50);
    } 
}

export const handleColorChange = (color: THREE.Color, converSRGBToLinear: boolean = false) => {
    return (value: any) => {
        if (typeof value === 'string') {
            value = value.replace('#', '0x');
        }
        color.setHex(value);
        if (converSRGBToLinear === true) {
            color.convertSRGBToLinear();
        }
    }
}