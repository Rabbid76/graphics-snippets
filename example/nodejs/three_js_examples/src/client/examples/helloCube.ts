import { ElapsedTime } from '../three/timeUtility'
import { Controls } from '../three/controls'
import { DataGUI, Statistic } from '../three/uiUtility' 
import * as THREE from 'three';
import { RoomEnvironment } from 'three/examples/jsm/environments/RoomEnvironment.js';

// fade background

export const helloCube = (canvas: any) => {
    const renderer = new THREE.WebGLRenderer({canvas: canvas, antialias: true, alpha: true});
    renderer.setSize(window.innerWidth, window.innerHeight);
    document.body.appendChild(renderer.domElement);
    const statistic = new Statistic();
    const dataGui = new DataGUI();

    const camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);
    camera.position.z = 2;
    const controls = new Controls(renderer, camera);

    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0xc0c0c0);
    const pmremGenerator = new THREE.PMREMGenerator(renderer);
    const environmentTexture = pmremGenerator.fromScene(new RoomEnvironment(renderer), 0.04).texture;
    scene.environment = environmentTexture;
    scene.background = environmentTexture;

    const ambientLight = new THREE.AmbientLight(0x404040);
    //scene.add(ambientLight); 
    const directionalLight = new THREE.DirectionalLight(0xffffff, 0.5);
    directionalLight.position.set(2, 2, 2);
    scene.add(directionalLight);

    const sqrt3 = Math.sqrt(3);
    const geometries = [
        { id: 'Tetrahedron', geometry: new THREE.TetrahedronGeometry(0.5) },
        { id: 'Cube', geometry: new THREE.BoxGeometry(1/sqrt3, 1/sqrt3, 1/sqrt3) },
        { id: 'Octahedron', geometry: new THREE.OctahedronGeometry(0.5) },
        { id: 'Dodecahedron', geometry: new THREE.DodecahedronGeometry(0.5) },
        { id: 'Icosahedron', geometry:  new THREE.IcosahedronGeometry(0.5) },
        { id: 'Cone', geometry:  new THREE.ConeGeometry(0.5, 1, 32) },
        { id: 'Cylinder', geometry:  new THREE.CylinderGeometry(0.5, 0.5, 1, 32) },
        { id: 'Sphere', geometry:  new THREE.SphereGeometry(0.5, 32, 16) },
        { id: 'Capsule', geometry:  new THREE.CapsuleGeometry(0.3, 0.5, 32, 32) },
        { id: 'Torus', geometry:  new THREE.TorusGeometry(0.5, 0.2, 32, 100) },
        { id: 'TorusKnot', geometry:  new THREE.TorusKnotGeometry(0.3, 0.1, 100, 32) }
    ]
    const geometryIdMap = {};
    // @ts-ignore
    geometries.forEach(item => geometryIdMap[item.id] = item.id)
    const geometryMap = {};
    // @ts-ignore
    geometries.forEach(item => geometryMap[item.id] = item.geometry)  
    const geometryObject = {
        geometry: 'Cube'
    };

    const material = new THREE.MeshPhysicalMaterial({color: 0xc0c0c0});
    material.metalness = 1;
    material.roughness = 0;
    // @ts-ignore
    const mesh = new THREE.Mesh(geometryMap[geometryObject.geometry], material);
    scene.add(mesh);

    dataGui.gui.add(geometryObject, 'geometry', geometryIdMap).onChange(() => { 
        // @ts-ignore
        mesh.geometry = geometryMap[geometryObject.geometry]; 
    });
    const materialGUI = dataGui.gui.addFolder('material properties');
    DataGUI.addPhysicalMaterialPropertiesUI(materialGUI, material);

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
        mesh.rotation.x += elapsedTime.getDegreePerSecond(45, true);
        mesh.rotation.z += elapsedTime.getDegreePerSecond(45, true);
        controls.update();
        render();
        statistic.update();
    }

    const render = () => {
        renderer.render(scene, camera);
    }
    animate(0);
}