import { ElapsedTime } from '../three/timeUtility'
import { Controls } from '../three/controls'
import { 
    MeshSpecification, 
    mergeBuffers 
} from '../geometry/geometryBuffer'
import { 
    createCircle, 
    createTube 
} from '../geometry/geometryConstructors'
import { 
    findTriangleInMesh,
    intersectMesh,
} from '../three/mathUtility'
import { DataGUI, Statistic } from '../three/uiUtility' 
import * as THREE from 'three';

// fade background

export const triangleIntersectionTest = (canvas: any) => {
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

    const highlightGeometry = createEquilateralTriangleGeometry();
    const highlightMaterial = new THREE.MeshBasicMaterial({color: 0xffff00});
    const highlightTriangleMesh = new THREE.Mesh(highlightGeometry, highlightMaterial);
    scene.add(highlightTriangleMesh);
    highlightTriangleMesh.visible = false;

    const errorMaterial = new THREE.MeshBasicMaterial({color: 0xff0000, side: THREE.FrontSide, polygonOffset: true, polygonOffsetFactor: 2, polygonOffsetUnits: 2});
    const split = (o1: GeometryObject, o2: GeometryObject): GeometryObject | undefined => {
        const splittedMeshBuffer = intersectMesh(o1.mesh, o2.mesh);
        let splitted: GeometryObject | undefined = undefined;
        if (splittedMeshBuffer && splittedMeshBuffer.anyChanged) {
            splitted = addGeometry(scene, createBufferGeometry(splittedMeshBuffer), splittedMeshBuffer.error ? errorMaterial :  tMaterial1);
            o1.group.visible = false;
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
    const tMaterial1 = new THREE.MeshBasicMaterial({color: 0xff8000, side: THREE.FrontSide, polygonOffset: true, polygonOffsetFactor: 1, polygonOffsetUnits: 1});
    const tMaterial2 = new THREE.MeshBasicMaterial({color: 0x00c0c0, side: THREE.FrontSide, polygonOffset: true, polygonOffsetFactor: 1, polygonOffsetUnits: 1});
    const tMaterial3 = new THREE.MeshBasicMaterial({color: 0x0000ff, side: THREE.FrontSide, polygonOffset: true, polygonOffsetFactor: 1, polygonOffsetUnits: 1});
    intersectT(new THREE.Vector3(-2.1, 0, 2.1), [[-1, 0, 0], [0, 0, 0.5], [0, 0, -0.5]], [[0.1, 0, 0], [1, -0.5, 0], [1, 0.5, 0]]);
    intersectT(new THREE.Vector3(0.1, 0, 2.1), [[-1, 0, 0], [1, 0, 0], [1, 0, -1]], [[-1, 0, 0], [1, 0, 1], [1, 1, 1]]);
    intersectT(new THREE.Vector3(2.1, 0, 2.1), [[1, 0, -1], [-1, 0, 0], [1, 0, 1]], [[1, 0, -1], [1, 1, -1], [-1, 0, 0]]);
    intersectT(new THREE.Vector3(2.1, 0, 4.1), [[1, 0, -1], [-1, 0, 0], [1, 0, 1]], [[1, 0, 1], [1, 1, 1], [-1, 0, 0]]);
    intersectT(new THREE.Vector3(2.1, 0, 0.1), [[1, 0, -1], [-1, 0, 0], [1, 0, 1]], [[-1, 0, 0], [1, -1, 0], [1, 1, 0]]);
    intersectT(new THREE.Vector3(4.1, 0, 0.1), [[1, 0, -1], [-1, 0, 0], [1, 0, 1]], [[-1, 0, 0], [0.5, -1, 0], [0.5, 1, 0]]);
    intersectT(new THREE.Vector3(4.1, 0, 2.1), [[0.5, 0, -1], [-1, 0, 0], [0.5, 0, 1]], [[-1, 0, 0], [1, -1, 0], [1, 1, 0]]);
    intersectT(new THREE.Vector3(-4, 0, 0), [[0, 0, -1], [-1, 0, 1], [1, 0, 1]], [[0, 1.5, 0], [-1, -0.5, 0], [1, -0.5, 0]]);
    intersectT(new THREE.Vector3(-2, 0, 0), [[0, 0, -1], [-1, 0, 1], [1, 0, 1]], [[0, 0.5, 0], [-1, -1.5, 0], [1, -1.5, 0]]);
    intersectT(new THREE.Vector3(3, 0, -2), [[0, 0, -1], [-1, 0, 1], [1, 0, 1]], [[0, 1, 0], [-1, -1, 0], [1, -1, 0]]);
    intersectT(new THREE.Vector3(0, 0, -3), [[-1, 0, 1.5], [1, 0, 2], [-0.5, 0.5, -1], [1.5, 0, 0.5]], [[-0.8, -1, 1], [1.2, -0.5, 1], [0.7, 3, 1]]);
    intersectT(new THREE.Vector3(-2, 0, -1.9), [[0, 0, -1], [-1, 0, 1], [1, 0, 1]], [[-0.5, 0, 0], [1, -1, 0], [1, 1, 0]]);
    intersectT(new THREE.Vector3(-4, 0, -1.9), [[0, 0, -1], [-1, 0, 1], [1, 0, 1]], [[1, 0, 0], [-0.5, 1, 0], [-0.5, -1, 0]]);
    intersectT(new THREE.Vector3(-4.1, 0, 2.1), [[0.5, 0, 1], [0.5, 0, 0], [-0.5, 0, 0], [0.5, 0, -1]], [[-1, 0, 0], [1, -1, 0], [1, 1, 0]]);
    intersectT(new THREE.Vector3(0.1, 0, 4.1), [[-1, 0, -1], [-1, 0, 1], [1, 0, 0]], [[-1, 0, 0], [1, -1, 0], [1, 1, 0],]);
    intersectT(new THREE.Vector3(-2.1, 0, 4.1), [[-1, 0, 0], [1, 0, 1], [1, 0, -1]], [[-1, 0, 0.25], [0.5, 1, 0.25], [0.5, -1, 0.25], [0.5, 0, 1.25]]);
    
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
    const geometry1 = createBufferGeometry(createTestArc());
    geometry1.rotateX(-Math.PI / 2);
    const object1 = addGeometry(scene, geometry1, new THREE.MeshPhysicalMaterial({color: 0xc0c0c0, polygonOffset: true, polygonOffsetFactor: 2, polygonOffsetUnits: 2}));
    const object2 = addGeometry(scene, new THREE.PlaneGeometry(1, 1), new THREE.MeshPhysicalMaterial({color: 0xc0f0c0, transparent: true, opacity: 0.5}));
    object1.group.position.set(0, 0, 0);
    object2.group.position.set(-0.18329594489171122, 0.2554615574565818, 0.11484286654124837-0.5);
    object1.group.updateMatrixWorld();
    object2.group.updateMatrixWorld();
    split(object1, object2);

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
