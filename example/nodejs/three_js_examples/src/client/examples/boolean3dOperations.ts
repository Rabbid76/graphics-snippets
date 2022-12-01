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
    calculateIntersectionBox,
    trianglesInBox,
    intersectMesh,
} from '../three/mathUtility'
import { DataGUI, Statistic } from '../three/uiUtility' 
import * as THREE from 'three';

// fade background

export const boolean3dOperations = (canvas: any) => {
    const renderer = new THREE.WebGLRenderer({canvas: canvas, antialias: true, alpha: true});
    renderer.setSize(window.innerWidth, window.innerHeight);
    document.body.appendChild(renderer.domElement);
    
    const camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);
    camera.position.y = 2;
    camera.position.z = 4;
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

    //const geometry = createTestGeometry();
    //const geometry = new THREE.BoxGeometry(1, 1, 1);
    const radii = [];
    const heights = [];
    for (let i = 0; i <= 10; ++i) {
        const radius = Math.pow((i - 5) / 8, 2) + 1 - Math.pow(5 / 8, 2); 
        radii.push(radius);
        if (i != 0) {
            heights.push(0.1);
        }
    }
    const segments = 32;
    const specification1 = mergeBuffers(mergeBuffers(createTube(radii, heights, segments), createCircle(1, 1, segments)), createCircle(1, 0, segments, true));
    const geometry1 = createBufferGeometry(specification1);
    geometry1.rotateX(-Math.PI / 2);
    const material1 = new THREE.MeshPhysicalMaterial({color: 0xc0c0c0, polygonOffset: true, polygonOffsetFactor: 2, polygonOffsetUnits: 2});
    const object1 = addGeometry(scene, geometry1, material1);
    object1.group.position.set(0, 0, 0);
    const geometry2 = new THREE.BoxGeometry(1, 1, 1);
    const material2 = new THREE.MeshPhysicalMaterial({color: 0xc0f0c0, transparent: true, opacity: 0.5});
    const object2 = addGeometry(scene, geometry2, material2);
    //object2.group.position.set(0.75, 1, 0);
    object2.group.position.set(-0.0048903324633537346, 0.6085501449797492, 0.369337411298443);
    object2.group.position.set(-0.0300905570996814, 0.6153913812282826, 0.3281334023730805);
    
    const statistic = new Statistic();
    const dataGui = new DataGUI();
    const uiProperties = {
        intersectionBox: true,
        intersectionTriangles: false,
        intersect: true,
    };
    dataGui.gui.add(uiProperties, 'intersectionBox').onChange(() => updateIntersection());
    dataGui.gui.add(uiProperties, 'intersectionTriangles').onChange(() => updateIntersection());
    dataGui.gui.add(uiProperties, 'intersect').onChange(() => updateIntersection());
    dataGui.gui.add(material2, 'opacity', 0, 1).onChange(() => material2.needsUpdate = true);

    const updateObjectFromMeshSpecification = (geometryObject: GeometryObject | undefined, specification: MeshSpecification, material: THREE.Material): GeometryObject => {
        const geometry = createBufferGeometry(specification);
        //if (geometryObject) {
        //    geometryObject.mesh.geometry = geometry;
        //    geometryObject.group.position.set(0, 0, 0);
        //    geometryObject.group.rotation.set(0, 0, 0);
        //    geometryObject.group.scale.set(1, 1, 1);
        //    geometryObject.group.visible = true;
        //    geometryObject.group.updateMatrix();
        //} else {
        //    const intersectionMaterial = new THREE.MeshBasicMaterial({color, polygonOffset: true, polygonOffsetFactor: 1, polygonOffsetUnits: 1});
        //    geometryObject = addGeometry(scene, geometry, intersectionMaterial);
        //}
        if (geometryObject) {
            scene.remove(geometryObject.group);
        }
        geometryObject = addGeometry(scene, geometry, material);
        geometryObject.group.applyMatrix4(object1.mesh.matrixWorld);
        geometryObject.group.updateMatrixWorld();
        return geometryObject;
    }

    const errorMaterial = new THREE.MeshBasicMaterial({color: 0xff0000, side: THREE.FrontSide, polygonOffset: true, polygonOffsetFactor: 2, polygonOffsetUnits: 2});
    const hightLightIntersectionMaterial = new THREE.MeshBasicMaterial({color: 0x808000, polygonOffset: true, polygonOffsetFactor: 1, polygonOffsetUnits: 1});
    const highlightIntersection = () => {
        const intersectionTriangles = trianglesInBox(object1.mesh, calculateIntersectionBox(object1.mesh, object2.mesh, true));
        if (intersectionTriangles.length > 0) {
            const intersectionSpecification: MeshSpecification = {
                vertices: specification1.vertices,
                normals: specification1.normals,
                indices: new Uint32Array(intersectionTriangles)
            };
            highlightIntersectionObject = updateObjectFromMeshSpecification(highlightIntersectionObject, intersectionSpecification, hightLightIntersectionMaterial);
            highlightIntersectionObject.group.visible = true;
        } else if (highlightIntersectionObject) {
            highlightIntersectionObject.group.visible = false;
        }
    }

    const intersectMeshes = () => {
        const intersectionGeometry = intersectMesh(object1.mesh, object2.mesh);
        if (intersectionGeometry) {
            intersectionObject = updateObjectFromMeshSpecification(intersectionObject, intersectionGeometry, intersectionGeometry.error ? errorMaterial : material1);
            intersectionObject.group.visible = true;
            object1.group.visible = false;
        } else if (intersectionObject) {
            intersectionObject.group.visible = false;
            object1.group.visible = true;
        }
    }

    let boxHelper: THREE.Box3Helper | undefined;
    let highlightIntersectionObject: GeometryObject | undefined;
    let intersectionObject: GeometryObject | undefined;
    const updateIntersection = () => {
        console.log(object2.group.position);
        object1.group.updateMatrixWorld();
        object2.group.updateMatrixWorld();
        if (boxHelper) {
            scene.remove(boxHelper);
        }
        const intersection = calculateIntersectionBox(object1.mesh, object2.mesh);
        if (!intersection.isEmpty() && uiProperties.intersectionBox) { 
            boxHelper = new THREE.Box3Helper(intersection, new THREE.Color(1, 0, 0));
            scene.add(boxHelper);
        } else {
            boxHelper = undefined;
        }
        
        if (uiProperties.intersectionTriangles) {
            highlightIntersection();
        } else if (highlightIntersectionObject) {
            highlightIntersectionObject.group.visible = false;
        }
        if (uiProperties.intersect) {
            intersectMeshes();
        } else if (intersectionObject) {
            intersectionObject.group.visible = false;
            object1.group.visible = true;
        }
    }
    updateIntersection();
    const transformControl = controls.addTransformControl(object2.group, scene);
    transformControl.addEventListener('objectChange', (event: any) => {
        updateIntersection();
    });

    const highlightGeometry = createEquilateralTriangleGeometry();
    const highlightMaterial = new THREE.MeshBasicMaterial({color: 0xffff00});
    const highlightTriangleMesh = new THREE.Mesh(highlightGeometry, highlightMaterial);
    scene.add(highlightTriangleMesh);
    highlightTriangleMesh.visible = false;

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
        highlightTriangleOfMesh(highlightTriangleMesh, object1.mesh, camera, canvasPointer);
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
