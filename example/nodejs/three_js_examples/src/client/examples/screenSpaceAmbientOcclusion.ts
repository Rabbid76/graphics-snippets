import { ElapsedTime } from '../three/timeUtility'
import { Controls } from '../three/controls'
import { SmoothSSAODebugPass, SmoothSSAODebugOutput } from '../three/smoothSsaoDebugPass';
import { getMaxSamples } from '../three/threeUtility'
import { DataGUI, Statistic } from '../three/uiUtility' 
import * as THREE from 'three';
import { EffectComposer } from 'three/examples/jsm/postprocessing/EffectComposer'

// PR three.js?
// stats
// GUI
// objects (distribution)

export const screenSpaceAmbientOcclusion = (canvas: any) => {
    const renderer = new THREE.WebGLRenderer({canvas: canvas, antialias: true, alpha: true});
    renderer.setSize(window.innerWidth, window.innerHeight);
    document.body.appendChild(renderer.domElement);
    const statistic = new Statistic();
    const dataGui = new DataGUI();

    const camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 50);
    camera.position.z = 4.5;
    const controls = new Controls(renderer, camera);

    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0xc0c0c0);

    const maxSamples = getMaxSamples(renderer);
    const ssaoRenderTarget = new THREE.WebGLRenderTarget(window.innerWidth, window.innerHeight, { samples: maxSamples })
    const effectComposer = new EffectComposer(renderer, ssaoRenderTarget);
    const ssaoPass = new SmoothSSAODebugPass(renderer, scene, camera, window.innerWidth, window.innerHeight, maxSamples, {
        depthBias: 0.0001,
        kernelRadius: 0.25,
        maxDistance: 0.5
    });
    effectComposer.addPass(ssaoPass)

    const ambientLight = new THREE.AmbientLight(0x404040);
    scene.add(ambientLight); 
    const directionalLight = new THREE.DirectionalLight(0xffffff, 0.5);
    directionalLight.position.set(1, 1, 10);
    scene.add(directionalLight);

    const objectGroup = new THREE.Group();
    scene.add(objectGroup);

    const n = 128;
    //const unitPositions = sphereSamples(n);
    for (let i = 0; i < n; ++ i) {
        const position = new THREE.Vector3(Math.random() - 0.5, Math.random() - 0.5, Math.random() - 0.5).normalize();
        const p = position.multiplyScalar(2);
        const mesh = randomMesh();
        mesh.rotation.x = Math.random() * Math.PI * 2;
        mesh.rotation.z = Math.random() * Math.PI;
        mesh.position.set(p.x, p.y, p.z);
        objectGroup.add(mesh);
    };

    const generalProperties = {
        'rotate': true,
        'debug pass': 'none'
    };
    dataGui.gui.add(generalProperties, 'rotate');
    dataGui.gui.add(generalProperties, 'debug pass', {
        'off': 'none', 
        'color': 'color', 
        'depth': 'depth', 
        'normal': 'normal', 
        'SSAO': 'ssao', 
        'blur SSAO': 'ssaoblur', 
    }).onChange((value) => { 
        switch(value.toLocaleLowerCase()) {
            default: ssaoPass.debugEffect = SmoothSSAODebugOutput.None; break;
            case 'color': ssaoPass.debugEffect = SmoothSSAODebugOutput.Color; break;
            case 'depth': ssaoPass.debugEffect = SmoothSSAODebugOutput.Depth; break;
            case 'normal': ssaoPass.debugEffect = SmoothSSAODebugOutput.Normal; break;
            case 'ssao': ssaoPass.debugEffect = SmoothSSAODebugOutput.SSAO; break;
            case 'ssaoblur': ssaoPass.debugEffect = SmoothSSAODebugOutput.SSAOBlur; break;
        }
    });
    const aoFolder = dataGui.gui.addFolder('SSAO');
    aoFolder.add<any>(ssaoPass.ssaoParameters, 'enabled');
    aoFolder.add<any>(ssaoPass.ssaoParameters, 'intensity', 0.01, 1);
    aoFolder.add<any>(ssaoPass.ssaoParameters, 'fadeout', 0.0, 1.0);
    aoFolder.add<any>(ssaoPass.ssaoParameters, 'kernelRadius', 0.001, 1);
    aoFolder.add<any>(ssaoPass.ssaoParameters, 'depthBias', 0.00001, 0.1);
    aoFolder.add<any>(ssaoPass.ssaoParameters, 'maxDistance', 0.01, 1);
    aoFolder.add<any>(ssaoPass.ssaoParameters, 'maxDepth', 0.0, 1);

    const onWindowResize = () => {
        camera.aspect = window.innerWidth / window.innerHeight;
        camera.updateProjectionMatrix();
        effectComposer.setSize(window.innerWidth, window.innerHeight);
        effectComposer.setSize(window.innerWidth, window.innerHeight);
        renderer.setSize(window.innerWidth, window.innerHeight);
    };
    window.addEventListener('resize', onWindowResize, false);

    const elapsedTime = new ElapsedTime();
    const animate = (timestamp: number) => {
        elapsedTime.update(timestamp);
        requestAnimationFrame(animate);
        if (generalProperties.rotate) {
            objectGroup.rotateY(-elapsedTime.getDegreePerSecond(15, true));
        }
        controls.update();
        render();
        statistic.update();
    }

    const render = () => {
        renderer.render(scene, camera);
        effectComposer.render();
    }
    animate(0);
}

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
    { id: 'Torus', geometry:  new THREE.TorusGeometry(0.5, 0.25, 32, 100) },
    { id: 'TorusKnot', geometry:  new THREE.TorusKnotGeometry(0.4, 0.2, 100, 32) }
];

const randomGeometry = (): THREE.BufferGeometry => {
    const index = Math.floor(Math.random() * geometries.length);
    return geometries[index].geometry;
}

const randomColor = (): THREE.Color => {
    const color = new THREE.Color();
    color.setHSL(Math.random(), 0.5, 0.75);
    return color;
}

const randomMaterial = (): THREE.Material => {
    const material = new THREE.MeshLambertMaterial({color: randomColor()});
    return material;
}

const randomMesh = (): THREE.Mesh => {
    const mesh = new THREE.Mesh(randomGeometry(), randomMaterial());
    return mesh
}

const sphereToEquirectangular = (unitDirection3D: THREE.Vector3): THREE.Vector2 => {
    const xy = { x: Math.atan2(unitDirection3D.y, unitDirection3D.x), y: Math.asin(unitDirection3D.z) };
    const uv = new THREE.Vector2(xy.x / (2 * Math.PI) + 0.5, xy.y / Math.PI + 0.5);
    return uv;
}

const equirectangularToSphere = (uv: THREE.Vector2): THREE.Vector3 => {
    const theta = (uv.x - 0.5) * Math.PI * 2;
    const phi = (uv.y - 0.5) * Math.PI;
    const sinTheta = Math.sin(theta);
    const cosTheta = Math.cos(theta);
    const sinPhi = Math.sin(phi);
    const cosPhi = Math.cos(phi);
    const unitDirection3D = new THREE.Vector3(cosTheta * cosPhi, sinTheta * cosPhi, sinPhi);
    return unitDirection3D;
}

const vdcSequence = (bits: number): number => {
    bits = (bits << 16) | (bits >> 16);
    bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
    bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
    bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
    bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);
    return bits * 2.3283064365386963e-10 // 0x100000000
}

const hammersleySequence = (i: number, n: number) => {
    return { u: i / n, v: vdcSequence(i) };
}

const uvToUnitSphere = (u: number, v: number): THREE.Vector3 => {
    const phi = v * 2 * Math.PI;
    const cosTheta = 2 * u - 1;
    const sinTheta = Math.sqrt(1.0 - cosTheta * cosTheta);
    const sinPhi = Math.sin(phi);
    const cosPhi = Math.cos(phi);
    const unitSphere = new THREE.Vector3(cosTheta * cosPhi, sinTheta * cosPhi, sinPhi);
    return unitSphere;
}

const sphereSamples = (n: number): THREE.Vector3[] => {
    const samples = [];
    for (let i=0; i < n; ++i) {
        const uv = hammersleySequence(i, n);
        const unitSphere = uvToUnitSphere(uv.u, uv.v);
        samples.push(unitSphere);
    }
    return samples;
}

