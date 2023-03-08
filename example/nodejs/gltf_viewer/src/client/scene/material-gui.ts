import { MaterialData } from './meshConstructor';
import { GUI } from 'dat.gui';

export class MaterialGUI {
    private materials: MaterialData[] = [];
    private materialFolder: GUI | undefined;
    private materialProperties: GUI | undefined;
    private materialId: string = '';

    public updateMaterialUI(gui: GUI, materialsData: MaterialData[]) {
        if (this.materialFolder) {
            gui.removeFolder(this.materialFolder)
            this.materialProperties = undefined;
        }
        this.materialFolder = gui.addFolder('Materials');
        this.materials = materialsData;
        this. materialId = materialsData[0]?.materialId ?? '';
        const materialNames = materialsData.map(item => { 
            let name: string = item.materialId;
            return { 'materialId': item.materialId, 'name': name };
        })
        let parameterMenuItems = Object.assign({}, ...materialNames.map((item) => ({[item.name]: item.materialId})));
        this.materialFolder.add(this, 'materialId', parameterMenuItems).onChange((value) => this.updateMaterialPropertiesUI(value));
        this.updateMaterialPropertiesUI(this.materialId);
    }

    private updateMaterialPropertiesUI(materialId: string) {
        this.materialId = materialId;
        if (!this.materialFolder) {
            return;
        }
        if (this.materialProperties) {
            this.materialFolder.removeFolder(this.materialProperties);
            this.materialProperties = undefined;
        }
        const materialData = this.materials.find(item => item.materialId === materialId);
        if (!materialData) {
            return;
        }
        const material = materialData.material as THREE.MeshPhysicalMaterial;
        if (!material) {
            return;
        }
        const uiMaterialData = {
            color: material.color.getHex(),
            specularColor: material.color.getHex(),
            emissive: material.emissive.getHex(),
            sheenColor: material.emissive.getHex(),
            attenuationColor: material.emissive.getHex()
        };
        let folderName = 'properties';
        if (materialData.material.userData) {
            if (materialData.material.userData.diffuseMap) {
                folderName += materialData.material.userData.diffuseMapHasAlpha ? ' RGBA' : ' RGB';
            }
            if (materialData.material.userData.normalMap) {
                folderName += ' XYZ';
            }
            if (materialData.material.userData.ormMap) {
                folderName += ' ORM';
            }
        }
        this.materialProperties = this.materialFolder.addFolder(folderName);
        this.materialProperties.addColor(uiMaterialData, 'color').onChange(MaterialGUI.handleColorChange(material.color, true));
        this.materialProperties.add(material, 'opacity', 0, 1).onChange((value: number) => { 
            const transparent = value < 1;
            if (transparent !== material.transparent) {
                material.transparent = transparent;
                material.needsUpdate = true;
            }
        });
        this.materialProperties.add(material, 'metalness', 0, 1);
        this.materialProperties.add(material, 'roughness', 0, 1);
        this.materialProperties.add(material, 'transmission', 0, 1);
        this.materialProperties.add(material, 'ior', 1, 2.333);
        this.materialProperties.add(material, 'specularIntensity', 0, 1);
        this.materialProperties.addColor(uiMaterialData, 'specularColor').onChange(MaterialGUI.handleColorChange(material.specularColor, true));
        this.materialProperties.add(material, 'reflectivity', 0, 1);
        this.materialProperties.add(material, 'clearcoat', 0, 1);
        this.materialProperties.add(material, 'clearcoatRoughness', 0, 1.0);
        this.materialProperties.add(material, 'sheen', 0, 1.0);
        this.materialProperties.add(material, 'sheenRoughness', 0, 1);
        this.materialProperties.addColor(uiMaterialData, 'sheenColor').onChange(MaterialGUI.handleColorChange(material.sheenColor, true));
        this.materialProperties.add(material, 'emissiveIntensity', 0, 1);
        this.materialProperties.addColor(uiMaterialData, 'emissive').onChange(MaterialGUI.handleColorChange(material.emissive, true));
        this.materialProperties.add(material, 'attenuationDistance', 0, 50);
        this.materialProperties.addColor(uiMaterialData, 'attenuationColor').onChange(MaterialGUI.handleColorChange(material.attenuationColor, true));
        this.materialProperties.add(material, 'thickness', 0, 50);
    }

    private static handleColorChange(color: THREE.Color, converSRGBToLinear: boolean = false) {
        return (value: any) => {
            if (typeof value === 'string') {
                value = value.replace('#', '0x');
            }
            color.setHex(value)
            if (converSRGBToLinear === true) {
                color.convertSRGBToLinear()
            }
        }
    }
}