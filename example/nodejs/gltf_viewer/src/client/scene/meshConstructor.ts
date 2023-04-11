import {
    BufferGeometry,
    Group,
    Material,
    Matrix4,
    Mesh,
} from 'three';

export interface MaterialData {
    materialId: string
    material: Material
}

export interface GeometryAndMaterial {
    geometry: BufferGeometry,
    material: Material,
    transform?: Matrix4,
    materialId: string,
    environment: boolean
}

export const createSceneGroup = (geometryAndMaterial: GeometryAndMaterial[]) : Group => {
    const sceneGroup = new Group();
    geometryAndMaterial.forEach(item => {
        const mesh = new Mesh(item.geometry, item.material);
        if (item.transform) {
            mesh.applyMatrix4(item.transform);
        }
        mesh.castShadow = !item.environment; 
        mesh.receiveShadow = true;
        sceneGroup.add(mesh);
    });
    return sceneGroup;
}