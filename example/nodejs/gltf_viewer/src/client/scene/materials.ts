import { loadAndSetTexture } from '../renderer/render-utility'
import * as THREE from 'three'

export const createPreviewMaterial = (): THREE.Material => {
    const material = new THREE.MeshStandardMaterial({side: THREE.DoubleSide})
    loadAndSetTexture(texture => material.map = texture, 'roomle_square.jpeg', new THREE.Color(0.859, 0, 0))
    return material
}

export const enum GroundMaterialType {
    OnlyShadow,
    Transparent,
    White,
    Parquet,
    Pavement
}

const groundMaterialCache: any = {}
export const createGroundMaterial = (materialType: GroundMaterialType): THREE.Material => {
    let material: THREE.Material = groundMaterialCache[materialType]
    if (!material) {
        switch(materialType) {
            default: {
                material = new THREE.MeshStandardMaterial()
                break
            }
            case GroundMaterialType.OnlyShadow: { 
                material = new THREE.ShadowMaterial()
                material.opacity = 0.5
                break
            }
            case GroundMaterialType.Transparent: {
                material = new THREE.MeshStandardMaterial()
                material.transparent = true;
                material.opacity = 0
                break
            }
            case GroundMaterialType.White: {
                material = new THREE.MeshStandardMaterial()
                break
            }
            case GroundMaterialType.Parquet: {
                const groundMaterial = new THREE.MeshPhysicalMaterial()
                loadAndSetTexture(texture => { setTextureProperties(texture); groundMaterial.map = texture }, 'TexturesCom_Wood_ParquetChevron7_1K_albedo.jpg', new THREE.Color(1.0, 0.6, 0.2))
                loadAndSetTexture(texture => { setTextureProperties(texture); groundMaterial.normalMap = texture }, 'TexturesCom_Wood_ParquetChevron7_1K_normal.jpg')
                loadAndSetTexture(texture => { setTextureProperties(texture); groundMaterial.roughnessMap = texture }, 'TexturesCom_Wood_ParquetChevron7_1K_roughness.jpg')
                groundMaterial.aoMapIntensity = 0
                groundMaterial.roughness = 1
                groundMaterial.metalness = 0
                groundMaterial.envMapIntensity = 0.5
                material = groundMaterial
                break;
            }
            case GroundMaterialType.Pavement: {
                const groundMaterial = new THREE.MeshPhysicalMaterial()
                loadAndSetTexture(texture => { setTextureProperties(texture); groundMaterial.map = texture }, 'TexturesCom_Pavement_HerringboneNew_1K_albedo.jpg', new THREE.Color(0.6, 0.3, 0))
                loadAndSetTexture(texture => { setTextureProperties(texture); groundMaterial.normalMap = texture }, 'TexturesCom_Pavement_HerringboneNew_1K_normal.jpg')
                loadAndSetTexture(texture => { setTextureProperties(texture); groundMaterial.roughnessMap = texture }, 'TexturesCom_Pavement_HerringboneNew_1K_roughness.jpg')
                loadAndSetTexture(texture => { setTextureProperties(texture); groundMaterial.aoMap = texture }, 'TexturesCom_Pavement_HerringboneNew_1K_ao.jpg')
                //loadAndSetTexture(texture => { setTextureProperties(texture); groundMaterial.displacementMap = texture }, 'TexturesCom_Pavement_HerringboneNew_1K_height.jpg')
                groundMaterial.aoMapIntensity = 1
                groundMaterial.roughness = 1
                groundMaterial.metalness = 0
                //groundMaterial.displacementScale = 1
                groundMaterial.envMapIntensity = 0.5
                material = groundMaterial
                break;
            }
        }
        groundMaterialCache[materialType] = material
    }
    return material
}

const setTextureProperties = (texture: THREE.Texture): void => {
    texture.anisotropy = 16;
    texture.wrapS = THREE.RepeatWrapping;
    texture.wrapT = THREE.RepeatWrapping;
    texture.repeat.set(100000, 100000);
}