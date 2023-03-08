import * as THREE from 'three'

export interface MaterialData {
    materialId: string
    material: THREE.Material
}

export interface GeometryAndMaterial {
    geometry: THREE.BufferGeometry,
    material: THREE.Material,
    transform?: THREE.Matrix4,
    materialId: string,
    environment: boolean
}

export const createSceneGroup = (geometryAndMaterial: GeometryAndMaterial[]) : THREE.Group => {
    const sceneGroup = new THREE.Group();
    geometryAndMaterial.forEach(item => {
        const mesh = new THREE.Mesh(item.geometry, item.material);
        if (item.transform) {
            mesh.applyMatrix4(item.transform);
        }
        mesh.castShadow = !item.environment; 
        mesh.receiveShadow = true;
        sceneGroup.add(mesh);
    });
    return sceneGroup;
}