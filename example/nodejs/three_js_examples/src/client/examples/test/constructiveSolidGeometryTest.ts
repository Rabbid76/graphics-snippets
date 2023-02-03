import { ElapsedTime } from '../../three/timeUtility'
import { Controls } from '../../three/controls'
import { 
    MeshSpecification, 
    mergeBuffers 
} from '../../geometry/geometryBuffer'
import { 
    createCircle, 
    createTube 
} from '../../geometry/geometryConstructors'
import { 
    findTriangleInMesh,
    intersectMesh,
    intersectMeshWASM,
    MESH_INTERSECT,
    MESH_MINUS_AB,
    MESH_OR_AB, 
    MESH_AND_AB,
} from '../../three/mathUtility'
import { intiMeshUtility } from '../../three/meshIntersection';
import { DataGUI, Statistic } from '../../three/uiUtility' 
import { 
    definitionOfSmallPlainCube,
    definitionOfPlainCube,
    definitionOfLowResolutionSphere, 
    definitionOfSphere,
} from './testGeometryDefinition'
import * as THREE from 'three';
import { Scene } from 'three'
// @ts-ignore
import CSG from '../../csg/three-csg';
import { markAsUntransferable } from 'worker_threads'
import { calculateMeshCSGMinusAB, calculateMeshCSGOrAB, calculateMeshCSGAndAB} from '../../three/meshIntersection'


// https://github.com/manthrax/THREE-CSGMesh
// https://github.com/chandlerprall/ThreeCSG/blob/master/ThreeCSG.js

export const constructiveSolidGeometryTest = async (canvas: any) => {
    await intiMeshUtility();

    const renderer = new THREE.WebGLRenderer({canvas: canvas, antialias: true, alpha: true});
    renderer.setSize(window.innerWidth, window.innerHeight);
    document.body.appendChild(renderer.domElement);
    
    const camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);
    camera.position.y = 4;
    camera.position.z = 8;
    const controls = new Controls(renderer, camera);

    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0xc0c0c0);

    const ambientLight = new THREE.AmbientLight(0x404040);
    scene.add(ambientLight); 
    const directionalLight = new THREE.DirectionalLight(0xffffff, 0.5);
    directionalLight.position.set(2, 2, 2);
    scene.add(directionalLight);

    const gridHelper = new THREE.GridHelper(10, 10);
    scene.add(gridHelper);
    const axesHelper = new THREE.AxesHelper(2);
    scene.add(axesHelper);

    const textureLoader = new THREE.TextureLoader();
    const material1 = new THREE.MeshBasicMaterial({color: 0xffff00, side: THREE.FrontSide, polygonOffset: true, polygonOffsetFactor: 2, polygonOffsetUnits: 2,
        transparent: true, opacity: 0.2
    });
    const material2 = new THREE.MeshBasicMaterial({color: 0xff0000, side: THREE.FrontSide, polygonOffset: true, polygonOffsetFactor: 2, polygonOffsetUnits: 2,
        transparent: true, opacity: 0.2
    });
    const material3 = new THREE.MeshBasicMaterial({color: 0x0000ff, side: THREE.FrontSide, polygonOffset: true, polygonOffsetFactor: 1, polygonOffsetUnits: 1,
        //transparent: true, opacity: 0.2
        // TODO test texture
    });
    textureLoader.load('512x512 Texel Density Texture 1.png', (texture: THREE.Texture) => {
        material3.color.set(0xffffff);
        material3.map = texture;
        material3.needsUpdate = true;
    });

    scgGeometry(scene, material3, new THREE.Vector3(-4, 0, -2), new THREE.SphereGeometry(0.5, 32, 16), material1, null, new THREE.CylinderGeometry(0.25, 0.25, 2, 32), material2, new THREE.Vector3(0, 0, 0));
    scgGeometry(scene, material3, new THREE.Vector3(-2, 0, -2), new THREE.CylinderGeometry(0.25, 0.25, 2, 32), material1, null, new THREE.SphereGeometry(0.5, 32, 16), material2, new THREE.Vector3(0, 0, 0));
    
    scgGeometry(scene, material3, new THREE.Vector3(-4, 0, 0), new THREE.BoxGeometry(1, 1, 1), material1, null, new THREE.SphereGeometry(0.5, 32, 16), material2, new THREE.Vector3(0.5, 0.5, 0.5));
    scgGeometry(scene, material3, new THREE.Vector3(-2, 0, 0), new THREE.BoxGeometry(1, 1, 1), material1, null, new THREE.SphereGeometry(1, 32, 16), material2, new THREE.Vector3(0.5, 0.5, 0.5));
    scgGeometry(scene, material3, new THREE.Vector3(0, 0, 0), new THREE.SphereGeometry(0.5, 32, 16), material1, null, new THREE.SphereGeometry(0.5, 32, 16), material2, new THREE.Vector3(0.5, 0.5, 0.5));
    scgGeometry(scene, material3, new THREE.Vector3(2, 0, 0), new THREE.SphereGeometry(0.5, 32, 16), material1, null, new THREE.BoxGeometry(1, 1, 1), material2, new THREE.Vector3(0.5, 0.5, 0.5));
    scgGeometry(scene, material3, new THREE.Vector3(4, 0, 0), new THREE.SphereGeometry(1, 32, 16), material1, null, new THREE.BoxGeometry(1, 1, 1), material2, new THREE.Vector3(0.5, 0.5, 0.5));
    
    scgGeometry(scene, material3, new THREE.Vector3(-4, 0, 2), new THREE.BoxGeometry(1, 1, 1), material1, null, new THREE.BoxGeometry(1, 1, 1), material2, new THREE.Vector3(0.5, 0.5, 0.5));
    scgGeometry(scene, material3, new THREE.Vector3(-2, 0, 2), new THREE.BoxGeometry(1, 1, 1), material1, null, new THREE.BoxGeometry(1, 1, 1), material2, new THREE.Vector3(0.5, 0, 0.5));

    //const transformControl = controls.addTransformControl(mesh2, scene);
    //transformControl.addEventListener('objectChange', (event: any) => {
        
    //});
    
    const statistic = new Statistic();
    const dataGui = new DataGUI();

    const onWindowResize = () => {
        const width = window.innerWidth;
        const height = window.innerHeight;
        camera.aspect = width / height;
        camera.updateProjectionMatrix();
        renderer.setSize(width, height);
    };
    window.addEventListener('resize', onWindowResize, false);

    const elapsedTime = new ElapsedTime();
    const animate = (timestamp: number) => {
        elapsedTime.update(timestamp);
        requestAnimationFrame(animate);
        //mesh.rotation.x += elapsedTime.getDegreePerSecond(45, true);
        //mesh.rotation.z += elapsedTime.getDegreePerSecond(45, true);
        controls.update();
        render();
        statistic.update();
    }

    const render = () => {
        renderer.render(scene, camera);
    }
    animate(0);
}

interface GeometryObject {
    mesh: THREE.Mesh,
    group: THREE.Group
}

const addGeometry = (target: THREE.Object3D, geometry: THREE.BufferGeometry, material: THREE.Material): GeometryObject => {
    const mesh = new THREE.Mesh(geometry, material);
    const lineMaterial = new THREE.LineBasicMaterial({color: 0xffffff, transparent: true, opacity: 0.5});
    const lineSegments = new THREE.LineSegments(new THREE.WireframeGeometry(geometry), lineMaterial);
    const group = new THREE.Group();
    group.add(mesh);
    group.add(lineSegments);
    target.add(group);
    return { mesh, group };
}

const scgGeometry = (s: THREE.Scene, csgMaterial: THREE.Material, csgP: THREE.Vector3 | null, g1: THREE.BufferGeometry, m1: THREE.Material, p1: THREE.Vector3 | null, g2: THREE.BufferGeometry, m2: THREE.Material, p2: THREE.Vector3 | null) => {
    const meshA = new THREE.Mesh(g1, m1);
    const meshB = new THREE.Mesh(g2, m2);
    //s.add(meshA);
    //s.add(meshB);
    if (p1) {
        meshA.position.add(p1);
    }
    if (p2) {
        meshB.position.add(p2);
    }
    if (csgP) {
        meshA.position.add(csgP);
        meshB.position.add(csgP);
    }
    meshA.updateMatrixWorld();
    meshB.updateMatrixWorld();

    //const resultGeometry = csgWasmOperation(meshA, meshB, MESH_OR_AB);
    //const resultGeometry = csgWasmOperation(meshA, meshB, MESH_AND_AB);
    const resultGeometry = csgWasmOperation(meshA, meshB, MESH_MINUS_AB);
    //const resultGeometry = csgLibOperation(meshA, meshB, MESH_MINUS_AB);

    resultGeometry.computeBoundingSphere();
    resultGeometry.computeBoundingBox();
    const object = addGeometry(s, resultGeometry, csgMaterial);
    
    object.group.applyMatrix4(meshA.matrix);
    object.group.updateMatrixWorld();
}

export const csgMeshWASM = (mesh0: THREE.Mesh, mesh1: THREE.Mesh, operator: number, epsilon = 0.0001): MeshSpecification | null => {

    const meshData0 = {
        vertices: (mesh0.geometry.attributes.position as THREE.BufferAttribute).array,
        normals: (mesh0.geometry.attributes.normal as THREE.BufferAttribute)?.array,
        uvs: (mesh0.geometry.attributes.uv as THREE.BufferAttribute)?.array,
        indices: new Uint32Array(mesh0.geometry.index?.array ?? [])
    };
    const meshData1 = {
        vertices: (mesh1.geometry.attributes.position as THREE.BufferAttribute).array,
        normals: (mesh1.geometry.attributes.normal as THREE.BufferAttribute)?.array,
        uvs: (mesh1.geometry.attributes.uv as THREE.BufferAttribute)?.array,
        indices: new Uint32Array(mesh1.geometry.index?.array ?? [])
    };

    const relativeTransform = mesh0.matrixWorld.clone().invert().multiply(mesh1.matrixWorld);
    if (relativeTransform) {
        const transformedVertices: number[] = [];
        for (let i = 0; i < meshData1.vertices.length; i += 3) {
            let v = new THREE.Vector3(meshData1.vertices[i], meshData1.vertices[i+1], meshData1.vertices[i+2]);
            v = v.applyMatrix4(relativeTransform);
            transformedVertices.push(v.x, v.y, v.z);
        }
        const normalMatrix = new THREE.Matrix3().getNormalMatrix(relativeTransform);
        const transformedNormals: number[] = [];
        for (let i = 0; i < meshData1.normals.length; i += 3) {
            let n = new THREE.Vector3(meshData1.normals[i], meshData1.normals[i+1], meshData1.normals[i+2]);
            n = n.applyNormalMatrix(normalMatrix);
            transformedNormals.push(n.x, n.y, n.z);
        }
        meshData1.vertices = new Float32Array(transformedVertices);
        meshData1.normals = new Float32Array(transformedNormals);
    }
    
    let resultMesh: any;
    switch (operator) {
        default:
        case MESH_MINUS_AB: resultMesh = calculateMeshCSGMinusAB(meshData0, meshData1); break;
        case MESH_OR_AB: resultMesh = calculateMeshCSGOrAB(meshData0, meshData1); break;
        case MESH_AND_AB: resultMesh = calculateMeshCSGAndAB(meshData0, meshData1); break;
    }
    
    if (!resultMesh || resultMesh.error) {
        return null;
    }

    const vertices: number[] = [];
    for (let i = 0; i < resultMesh.vertices.size(); ++i) {
        vertices.push(resultMesh.vertices.get(i));
    }
    const normals: number[] = [];
    for (let i = 0; i < resultMesh.normals.size(); ++i) {
        normals.push(resultMesh.normals.get(i));
    }
    const uvs: number[] = [];
    for (let i = 0; i < resultMesh.uvs.size(); ++i) {
        uvs.push(resultMesh.uvs.get(i));
    }
    const indices: number[] = [];
    for (let i = 0; i < resultMesh.indicesOut.size(); ++i) {
        indices.push(resultMesh.indicesOut.get(i));
    }
    for (let i = 0; i < resultMesh.indicesIn.size(); ++i) {
        //indices.push(resultMesh.indicesIn.get(i));
    }
    const newMesh = { 
        vertices: new Float32Array(vertices), 
        normals: normals && normals.length > 0 ? new Float32Array(normals) : undefined,
        uvs: uvs && uvs.length > 0 ? new Float32Array(uvs) : undefined,
        indices: new Uint32Array(indices),
        error: resultMesh.error,
        anyChanged: resultMesh.indicesIn.size() > 0 || vertices.length > meshData0.vertices.length
    };

    const indexOut = [];
    const indexIn = [];
    for (let i = 0; i < resultMesh.indicesOut.size(); ++i) {
        indexOut.push(resultMesh.indicesOut.get(i));
    }
    for (let i = 0; i < resultMesh.indicesIn.size(); ++i) {
        indexIn.push(resultMesh.indicesIn.get(i));
    }
    //console.log(Array.from(meshData0.vertices));
    //console.log(Array.from(meshData0.indices));
    //console.log(Array.from(meshData1.vertices));
    //console.log(Array.from(meshData1.indices));
    //console.log(vertices);
    //console.log(indexOut);
    //console.log(indexIn);

    return newMesh;
}

const createBufferGeometry = (meshSpecification: MeshSpecification): THREE.BufferGeometry => {
    const geometry = new THREE.BufferGeometry();
    geometry.setAttribute('position', new THREE.BufferAttribute(meshSpecification.vertices, 3));
    if (meshSpecification.normals) {
        geometry.setAttribute('normal', new THREE.BufferAttribute(meshSpecification.normals, 3));
    }
    if (meshSpecification.uvs) {
        geometry.setAttribute('uv', new THREE.BufferAttribute(meshSpecification.uvs, 2));
    }
    if (meshSpecification.colors) {
        geometry.setAttribute('color', new THREE.BufferAttribute(meshSpecification.colors, 3));
    }
    if (meshSpecification.indices) {
        geometry.setIndex(new THREE.BufferAttribute(meshSpecification.indices, 1));
    }
    return geometry;
}

const csgWasmOperation = (m1: THREE.Mesh, m2: THREE.Mesh, op: number = 0): THREE.BufferGeometry  => {
    const splittedMeshBuffer = csgMeshWASM(m1, m2, op);
    return splittedMeshBuffer
        ? createBufferGeometry(splittedMeshBuffer)
        : m1.geometry;
}

const csgLibOperation = (m1: THREE.Mesh, m2: THREE.Mesh, op: number = 0): THREE.BufferGeometry  => {
    const bspA = CSG.fromMesh(m1);                        
    const bspB = CSG.fromMesh(m2);

    let cspResult: any;
    switch (op) {
        case MESH_OR_AB: 
            cspResult = bspA.union(bspB);
            break;
        case MESH_AND_AB: 
            cspResult = bspA.intersect(bspB);
            break;
        default:
        case MESH_MINUS_AB: 
            cspResult = bspA.subtract(bspB);
            break;
    } 
    
    const resultGeometry = CSG.toGeometry(cspResult);
    let inv = new THREE.Matrix4().copy(m1.matrix).invert();
    resultGeometry.applyMatrix4(inv);

    return resultGeometry;
}
