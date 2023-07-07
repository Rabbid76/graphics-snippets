import { loadAndSetTexture } from '../renderer/render-utility'
import {
    Color,
    DoubleSide,
    Material,
    MeshPhysicalMaterial,
    MeshStandardMaterial,
    RepeatWrapping,
    ShadowMaterial,
    Texture,
} from 'three'
// @ts-ignore
import LogoSquareImage from './../../../resources/rabbid.png'

export const createPreviewMaterial = (): Material => {
    const material = new MeshStandardMaterial({side: DoubleSide})
    loadAndSetTexture(texture => material.map = texture, LogoSquareImage, new Color(0.859, 0, 0))
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
export const createGroundMaterial = (materialType: GroundMaterialType): Material => {
    let material: Material = groundMaterialCache[materialType]
    if (!material) {
        switch(materialType) {
            default: {
                material = new MeshStandardMaterial()
                break
            }
            case GroundMaterialType.OnlyShadow: { 
                material = new ShadowMaterial()
                material.opacity = 0.5
                break
            }
            case GroundMaterialType.Transparent: {
                material = new MeshStandardMaterial()
                material.transparent = true;
                material.opacity = 0
                break
            }
            case GroundMaterialType.White: {
                material = new MeshStandardMaterial()
                break
            }
            case GroundMaterialType.Parquet: {
                const groundMaterial = new MeshPhysicalMaterial()
                loadAndSetTexture(texture => { setTextureProperties(texture); groundMaterial.map = texture }, 'TexturesCom_Wood_ParquetChevron7_1K_albedo.jpg', new Color(1.0, 0.6, 0.2))
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
                const groundMaterial = new MeshPhysicalMaterial()
                loadAndSetTexture(texture => { setTextureProperties(texture); groundMaterial.map = texture }, 'TexturesCom_Pavement_HerringboneNew_1K_albedo.jpg', new Color(0.6, 0.3, 0))
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

const setTextureProperties = (texture: Texture): void => {
    texture.anisotropy = 16;
    texture.wrapS = RepeatWrapping;
    texture.wrapT = RepeatWrapping;
    texture.repeat.set(100000, 100000);
}