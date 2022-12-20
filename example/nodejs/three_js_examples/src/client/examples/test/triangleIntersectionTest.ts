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

// fade background

export const triangleIntersectionTest = async (canvas: any) => {
    await intiMeshUtility();

    const renderer = new THREE.WebGLRenderer({canvas: canvas, antialias: true, alpha: true});
    renderer.setSize(window.innerWidth, window.innerHeight);
    document.body.appendChild(renderer.domElement);
    const statistic = new Statistic();
    const dataGui = new DataGUI();

    const camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);
    camera.position.y = 4;
    camera.position.z = 8;
    const controls = new Controls(renderer, camera);

    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0xffffff);

    const ambientLight = new THREE.AmbientLight(0x404040);
    scene.add(ambientLight); 
    const directionalLight = new THREE.DirectionalLight(0xffffff, 0.5);
    directionalLight.position.set(2, 2, 2);
    scene.add(directionalLight);

    const gridHelper = new THREE.GridHelper(10, 10);
    //scene.add(gridHelper);
    const axesHelper = new THREE.AxesHelper(2);
    //scene.add(axesHelper);

    const highlightGeometry = createEquilateralTriangleGeometry();
    const highlightMaterial = new THREE.MeshBasicMaterial({color: 0xffff00});
    const highlightTriangleMesh = new THREE.Mesh(highlightGeometry, highlightMaterial);
    scene.add(highlightTriangleMesh);
    highlightTriangleMesh.visible = false;

    const errorMaterial = new THREE.MeshBasicMaterial({color: 0xff0000, side: THREE.FrontSide, polygonOffset: true, polygonOffsetFactor: 2, polygonOffsetUnits: 2});
    const split = (o1: GeometryObject, o2: GeometryObject, op: number = 0): GeometryObject | undefined => {
        const splittedMeshBuffer = op === 0 ? intersectMesh(o1.mesh, o2.mesh) : intersectMeshWASM(o1.mesh, o2.mesh, op);
        let splitted: GeometryObject | undefined = undefined;
        if (splittedMeshBuffer && splittedMeshBuffer.anyChanged) {
            splitted = addGeometry(scene, createBufferGeometry(splittedMeshBuffer), splittedMeshBuffer.error ? errorMaterial :  tMaterial1);
            o1.group.visible = false;
            o2.group.visible = op === 0;
            splitted.group.applyMatrix4(o1.mesh.matrixWorld);
        }
        return splitted;
    }
    const intersectT = (o: THREE.Vector3, tv1: any[], tv2: any[]) => {
        const tg1 = tv1.length == 4 
            ? createQuadGeometry(tv1[0], tv1[1], tv1[2], tv1[3])
            : createTriangleGeometry(tv1[0], tv1[1], tv1[2]);
        const tg2 = tv2.length == 4 
            ? createQuadGeometry(tv2[0], tv2[1], tv2[2], tv2[3])
            : createTriangleGeometry(tv2[0], tv2[1], tv2[2]);
        const t1 = addGeometry(scene, tg1, tMaterial3);
        t1.group.position.set(o.x, o.y, o.z);
        t1.group.updateMatrixWorld();
        const t2 = addGeometry(scene, tg2, tMaterial2);
        t2.group.position.set(o.x, o.y, o.z);
        t2.group.updateMatrixWorld();
        split(t1, t2);
    }

    const intersectGeometry = (geometry1: any, rotateX1: number | null, p1: THREE.Vector3 | null, geometry2: any, rotateX2: number | null, p2: THREE.Vector3 | null, op: number = 0) => {
        if (rotateX1) {
            geometry1.rotateX(rotateX1);
        }
        if (geometry2) {
            geometry2.rotateX(rotateX2);
        }
        const object1 = addGeometry(scene, geometry1, tMaterial1);
        const object2 = addGeometry(scene, geometry2, tMaterial2);
        if (p1) {
            object1.group.position.set(p1.x, p1.y, p1.z);
        }
        if (p2) {
            object2.group.position.set(p2.x, p2.y, p2.z);
        }
        object1.group.updateMatrixWorld();
        object2.group.updateMatrixWorld();
        split(object1, object2, op);
    }

    const tMaterial1 = new THREE.MeshBasicMaterial({color: 0xffff00, side: THREE.FrontSide, polygonOffset: true, polygonOffsetFactor: 1, polygonOffsetUnits: 1,
        //transparent: true, opacity: 0.2
    });
    const tMaterial2 = new THREE.MeshBasicMaterial({color: 0xff0000, side: THREE.FrontSide, polygonOffset: true, polygonOffsetFactor: 1, polygonOffsetUnits: 1,
        transparent: true, opacity: 0.2
    });
    const tMaterial3 = new THREE.MeshBasicMaterial({color: 0x0000ff, side: THREE.FrontSide, polygonOffset: true, polygonOffsetFactor: 1, polygonOffsetUnits: 1});
    
    const addTriangleTests = () => {
        intersectT(new THREE.Vector3(-2, 0, 0), [[0, 0, -1], [-1, 0, 1], [1, 0, 1]], [[0, 0.5, 0], [-1, -1.5, 0], [1, -1.5, 0]]);
        intersectT(new THREE.Vector3(-4, 0, 0), [[0, 0, -1], [-1, 0, 1], [1, 0, 1]], [[0, 1.5, 0], [-1, -0.5, 0], [1, -0.5, 0]]);
        intersectT(new THREE.Vector3(4.1, 0, 0.1), [[1, 0, -1], [-1, 0, 0], [1, 0, 1]], [[-1, 0, 0], [0.5, -1, 0], [0.5, 1, 0]]);
        intersectT(new THREE.Vector3(4.1, 0, 2.1), [[0.5, 0, -1], [-1, 0, 0], [0.5, 0, 1]], [[-1, 0, 0], [1, -1, 0], [1, 1, 0]]);
        intersectT(new THREE.Vector3(2.1, 0, 0.1), [[1, 0, -1], [-1, 0, 0], [1, 0, 1]], [[-1, 0, 0], [1, -1, 0], [1, 1, 0]]);
        intersectT(new THREE.Vector3(3, 0, -2), [[0, 0, -1], [-1, 0, 1], [1, 0, 1]], [[0, 1, 0], [-1, -1, 0], [1, -1, 0]]);
        intersectT(new THREE.Vector3(-2, 0, -1.9), [[0, 0, -1], [-1, 0, 1], [1, 0, 1]], [[-0.5, 0, 0], [1, -1, 0], [1, 1, 0]]);
        intersectT(new THREE.Vector3(-4, 0, -1.9), [[0, 0, -1], [-1, 0, 1], [1, 0, 1]], [[1, 0, 0], [-0.5, 1, 0], [-0.5, -1, 0]]);
        intersectT(new THREE.Vector3(0.1, 0, 4.1), [[-1, 0, -1], [-1, 0, 1], [1, 0, 0]], [[-1, 0, 0], [1, -1, 0], [1, 1, 0],]);
        intersectT(new THREE.Vector3(2.1, 0, 2.1), [[1, 0, -1], [-1, 0, 0], [1, 0, 1]], [[1, 0, -1], [1, 1, -1], [-1, 0, 0]]);
        intersectT(new THREE.Vector3(2.1, 0, 4.1), [[1, 0, -1], [-1, 0, 0], [1, 0, 1]], [[1, 0, 1], [1, 1, 1], [-1, 0, 0]]);
        intersectT(new THREE.Vector3(0.1, 0, 2.1), [[-1, 0, 0], [1, 0, 0], [1, 0, -1]], [[-1, 0, 0], [1, 0, 1], [1, 1, 1]]);
        intersectT(new THREE.Vector3(-2.1, 0, 2.1), [[-1, 0, 0], [0, 0, 0.5], [0, 0, -0.5]], [[0.1, 0, 0], [1, -0.5, 0], [1, 0.5, 0]]);
        intersectT(new THREE.Vector3(-4.1, 0, 2.1), [[0.5, 0, 1], [0.5, 0, 0], [-0.5, 0, 0], [0.5, 0, -1]], [[-1, 0, 0], [1, -1, 0], [1, 1, 0]]);
        intersectT(new THREE.Vector3(-2.1, 0, 4.1), [[-1, 0, 0], [1, 0, 1], [1, 0, -1]], [[-1, 0, 0.25], [0.5, 1, 0.25], [0.5, -1, 0.25], [0.5, 0, 1.25]]);
        intersectT(new THREE.Vector3(-4.1, 0, 4.1), [[-1, 0, 0], [1, 0, 1], [1, 0, -1]], [[0.5, 0, 1.25], [0.5, -1, 0.25], [0.5, 1, 0.25], [-1, 0, 0.25]]);
        intersectT(new THREE.Vector3(0, 0, -3), [[-1, 0, 1.5], [1, 0, 2], [-0.5, 0.5, -1], [1.5, 0, 0.5]], [[-0.8, -1, 1], [1.2, -0.5, 1], [0.7, 3, 1]]);
        intersectT(new THREE.Vector3(4, 0, 4), [[0, 0, -1], [-1, 0, 1], [1, 0, 1]], [[-0.5, -0.8, 0.2], [1.5, -0.8, 0.2], [0.5, 0.7, 0.2]]);
    };

    const createTestArc = (): MeshSpecification => {
        const v: number[] = [0, 0, 0];
        const n: number[] = [0, 0, -1];
        let i: number[] = [];
        for (let is = 0; is <= 8; ++is) {
            const a = is / 16 * Math.PI * 2;
            v.push(Math.cos(a), Math.sin(a), 0);
            n.push(0, 0, -1);
        }
        for (let is = 0; is < 8; ++is) {
            i.push(0, is+1, is+2);
        }
        return { vertices: new Float32Array(v), normals: new Float32Array(n), indices: new Uint32Array(i) }
    }

    const addGeometryTests = () => {
        intersectGeometry(
            createBufferGeometry({
                vertices: new Float32Array([0.47744316900122036, -0.7145441980725, 0.1, 0.6076698900821891, -0.6076698900821894, 0.1, 0.4166776747647014, -0.6236022092269091, 0.2, 0.5303300858899105, -0.5303300858899107, 0.2]),
                normals: new Float32Array([0.5555702330196018, -0.8314696123025455, 0, 0.7071067811865474, -0.7071067811865477, 0, 0.5555702330196018, -0.8314696123025455, 0, 0.7071067811865474, -0.7071067811865477, 0]),
                indices: new Uint32Array([0, 1, 3, 0, 3, 2]),
            }),
            -Math.PI / 2, null,
            createBufferGeometry({
                vertices: new Float32Array([-0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5]),
                normals: new Float32Array([0, -1, 0, 0, -1, 0, 0, -1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0]),
                indices: new Uint32Array([0, 1, 2, 3, 4, 5]),
            }),
            null, new THREE.Vector3(-0.0048903324633537346, 0.6085501449797492, 0.369337411298443));
        intersectGeometry(
            createBufferGeometry(createTestArc()), -Math.PI / 2, null,
            new THREE.PlaneGeometry(1, 1), null, new THREE.Vector3(-0.18329594489171122, 0.2554615574565818, 0.11484286654124837-0.5));
        intersectGeometry(
            new THREE.PlaneGeometry(1, 1), null, new THREE.Vector3(-4, 0, -3.5),
            createBufferGeometry({
                vertices: new Float32Array([0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, -0.5, -0.5, 0.5]),
                normals: new Float32Array([1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0,0, -1, 0, 0, -1, 0, 0, -1, 0, 0. -1, 0]),
                indices: new Uint32Array([0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7]),
            }),
            null, new THREE.Vector3(-0.028130438905423515-4, 0.3530915478172213, 0.38414688935046615-4));
        intersectGeometry(
            createBufferGeometry({
                vertices: new Float32Array([-0.5586436457657727, -0.37327375031004506, 0.7, -0.47508736860971174, -0.47508736860971157, 0.7, -0.6236022092269091, -0.41667767476470147, 0.7999999999999999, -0.5303300858899107, -0.5303300858899106, 0.7999999999999999]),
                normals: new Float32Array([0.8314696123025455, -0.555570233019602, 0, -0.7071067811865477, -0.7071067811865475, 0, -0.8314696123025455, -0.555570233019602, 0, -0.7071067811865477, -0.7071067811865475, 0]),
                indices: new Uint32Array([0, 1, 3]), 
            }),
            -Math.PI / 2, new THREE.Vector3(-2, 0, -4),
            createBufferGeometry({
                vertices: new Float32Array([-0.5, -0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5]),
                normals: new Float32Array([0, -1, 0, 0, -1, 0, 0, -1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0]),
                indices: new Uint32Array([0, 1, 2, 3, 4, 5]),
            }),
            null, new THREE.Vector3(-0.0300905570996814-2, 0.6153913812282826, 0.3281334023730805-4));
        intersectGeometry(
                new THREE.BoxGeometry(1, 1, 1), null, new THREE.Vector3(6, -0.5, 3.5),
                new THREE.BoxGeometry(1, 1, 1), null, new THREE.Vector3(6.5, 0, 4));
        intersectGeometry(
                new THREE.BoxGeometry(1, 1, 1), null, new THREE.Vector3(6, -0.5, 2),
                new THREE.SphereGeometry(0.5, 16, 8), null, new THREE.Vector3(6.5, 0, 1.5));
        intersectGeometry(
            createBufferGeometry(definitionOfPlainCube), Math.PI / 2, new THREE.Vector3(6, 0.5, 0.5),
            createBufferGeometry(definitionOfSphere), Math.PI / 2, new THREE.Vector3(6, 0.5, 0.5), MESH_MINUS_AB);
        intersectGeometry(
            createBufferGeometry(definitionOfSmallPlainCube), -Math.PI / 2, new THREE.Vector3(5, 0, -2),
            createBufferGeometry(definitionOfLowResolutionSphere), -Math.PI / 2, new THREE.Vector3(5, 0, -2), MESH_MINUS_AB)
    };

    addTriangleTests();
    addGeometryTests();

    //intersectGeometry(
    //    createBufferGeometry(definitionOfLowResolutionSphere), null, new THREE.Vector3(0, 0, 0),
    //    createBufferGeometry(definitionOfSmallPlainCube), null, new THREE.Vector3(0, 0, 0))
//
    const onWindowResize = () => {
        const width = window.innerWidth;
        const height = window.innerHeight;
        camera.aspect = width / height;
        camera.updateProjectionMatrix();
        renderer.setSize(width, height);
    };
    window.addEventListener('resize', onWindowResize, false);
    const canvasPointer: THREE.Vector2 = new THREE.Vector2();
    const onPointerMove = (event: any) => {
        canvasPointer.x = event.clientX / window.innerWidth * 2 - 1;
	    canvasPointer.y = 1 - event.clientY / window.innerHeight * 2;
    };
    window.addEventListener('pointermove', onPointerMove);

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

const createBufferGeometry = (meshSpecification: MeshSpecification): THREE.BufferGeometry => {
    const geometry = new THREE.BufferGeometry();
    geometry.setAttribute('position', new THREE.BufferAttribute(meshSpecification.vertices, 3));
    if (meshSpecification.normals) {
        geometry.setAttribute('normal', new THREE.BufferAttribute(meshSpecification.normals, 3));
    }
    if (meshSpecification.uvs) {
        geometry.setAttribute('uv', new THREE.BufferAttribute(meshSpecification.uvs, 3));
    }
    if (meshSpecification.colors) {
        geometry.setAttribute('color', new THREE.BufferAttribute(meshSpecification.colors, 3));
    }
    if (meshSpecification.indices) {
        geometry.setIndex(new THREE.BufferAttribute(meshSpecification.indices, 1));
    }
    return geometry;
}

const createTriangleGeometry = (a: number[], b: number[], c: number[]): THREE.BufferGeometry => {
    const vertices = new Float32Array([...a, ...b, ...c]);
    const normals = new Float32Array([0, 0, 1, 0, 0, 1, 0, 0, 1]);
    const indices = new Uint32Array([0, 1, 2]);
    return createBufferGeometry({vertices, normals, indices});
}

const createQuadGeometry = (a: number[], b: number[], c: number[], d: number[]): THREE.BufferGeometry => {
    const vertices = new Float32Array([...a, ...b, ...c, ...d]);
    const normals = new Float32Array([0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1]);
    const indices = new Uint32Array([0, 1, 2, 1, 3, 2]);
    return createBufferGeometry({vertices, normals, indices});
}

const createEquilateralTriangleGeometry = (): THREE.BufferGeometry => {
    const vertices = new Float32Array([-0.866, -0.5, 0, 0.866, -0.5, 0, 0, 1, 0]);
    const normals = new Float32Array([0, 0, 1, 0, 0, 1, 0, 0, 1]);
    const indices = new Uint32Array([0, 1, 2]);
    return createBufferGeometry({vertices, normals, indices});
}

interface GeometryObject {
    mesh: THREE.Mesh,
    group: THREE.Group
}

const highlightTriangleOfMesh = (highlightTriangleMesh: THREE.Mesh, mesh: THREE.Mesh, camera: THREE.Camera, canvasPointer: THREE.Vector2) => {
    const intersectsMesh = findTriangleInMesh(mesh, camera, canvasPointer);
    
    const material = mesh.material as THREE.MeshPhysicalMaterial;
    material.color.set(intersectsMesh ? 0xff40ff : 0xc0c0c0);
    material.needsUpdate = true;

    highlightTriangleMesh.visible = intersectsMesh?.length === 3;    
    if (highlightTriangleMesh.visible && intersectsMesh) {
        const normal = intersectsMesh[1].clone().sub(intersectsMesh[0]).cross(intersectsMesh[2].clone().sub(intersectsMesh[0])).normalize();
        const vertices = new Float32Array([
            intersectsMesh[0].x, intersectsMesh[0].y, intersectsMesh[0].z,
            intersectsMesh[1].x, intersectsMesh[1].y, intersectsMesh[1].z,
            intersectsMesh[2].x, intersectsMesh[2].y, intersectsMesh[2].z,
        ]);
        const normals = new Float32Array([
            normal.x, normal.y, normal.z,
            normal.x, normal.y, normal.z,
            normal.x, normal.y, normal.z,
        ]);
        highlightTriangleMesh.geometry = createBufferGeometry({ vertices, normals });
        highlightTriangleMesh.position.set(0, 0, 0);
        highlightTriangleMesh.rotation.set(0, 0, 0);
        highlightTriangleMesh.scale.set(1, 1, 1);
        highlightTriangleMesh.updateMatrix();
        highlightTriangleMesh.applyMatrix4(mesh.matrixWorld);
        highlightTriangleMesh.updateMatrixWorld();
    }
}

const addGeometry = (target: THREE.Object3D, geometry: THREE.BufferGeometry, material: THREE.Material, lineOpacity: number = 1): GeometryObject => {
    const mesh = new THREE.Mesh(geometry, material);
    const lineMaterial = new THREE.LineBasicMaterial({color: 0x000000, transparent: true, opacity: lineOpacity, 
        //depthTest: false
    });
    const lineSegments = new THREE.LineSegments(new THREE.WireframeGeometry(geometry), lineMaterial);
    const group = new THREE.Group();
    group.add(mesh);
    group.add(lineSegments);
    target.add(group);
    return { mesh, group };
}
