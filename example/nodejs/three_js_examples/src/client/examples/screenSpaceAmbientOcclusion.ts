import { ElapsedTime } from '../three/timeUtility'
import { Controls } from '../three/controls'
import { SmoothSSAOPass } from '../three/smoothSsaoPass';
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

    const camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);
    camera.position.z = 4;
    const controls = new Controls(renderer, camera);

    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0xc0c0c0);

    const maxSamples = getMaxSamples(renderer);
    const ssaoRenderTarget = new THREE.WebGLRenderTarget(window.innerWidth, window.innerHeight, { samples: maxSamples })
    const effectComposer = new EffectComposer(renderer, ssaoRenderTarget);
    const ssaoPass = new SmoothSSAOPass(renderer, scene, camera, window.innerWidth, window.innerHeight, maxSamples)
    effectComposer.addPass(ssaoPass)

    const ambientLight = new THREE.AmbientLight(0x404040);
    scene.add(ambientLight); 
    const directionalLight = new THREE.DirectionalLight(0xffffff, 0.5);
    directionalLight.position.set(1, 1, 10);
    scene.add(directionalLight);

    const objectGroup = new THREE.Group();
    scene.add(objectGroup);

    //const torusKnot = new THREE.Mesh(new THREE.TorusKnotGeometry(1, 0.3, 100, 32), new THREE.MeshLambertMaterial({color: 0xff80ff}))
    //torusKnot.position.set(100, 150, 0)
    //scene.add(torusKnot)
    //const geometry = new THREE.BoxGeometry();
    //const geometry = new THREE.CylinderGeometry(1, 1, 1, 32);
    const geometry = new THREE.TorusKnotGeometry(1, 0.3, 100, 32);
    //const material = new THREE.MeshLambertMaterial({color: 0xffff00});
    const material = new THREE.MeshLambertMaterial({color: 0xff80ff});
    const cube = new THREE.Mesh(geometry, material);
    objectGroup.add(cube);

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
        objectGroup.rotation.y -= elapsedTime.getDegreePerSecond(45, true);
        controls.update();
        render();
        statistic.update();
    }

    const render = () => {
        effectComposer.render();
        renderer.render(scene, camera);
    }
    animate(0);
}
