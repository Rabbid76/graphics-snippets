import { ElapsedTime } from '../three/timeUtility'
import { Controls } from '../three/controls'
import { GroundAndGroundShadow } from '../three/groundAndGroundShadow'
import { DataGUI, Statistic } from '../three/uiUtility'
import * as THREE from 'three'
import { RoomEnvironment } from 'three/examples/jsm/environments/RoomEnvironment.js';

export const contactShadow = (canvas: any) => {
    const renderer = new THREE.WebGLRenderer({canvas: canvas, antialias: true, alpha: true})
    renderer.setSize(window.innerWidth, window.innerHeight)
    renderer.setPixelRatio(window.devicePixelRatio);
    document.body.appendChild(renderer.domElement)
    const statistic = new Statistic();

    const camera = new THREE.PerspectiveCamera(45, window.innerWidth / window.innerHeight, 0.1, 100);
    camera.position.y = 5;
    camera.position.z = 10;
    const controls = new Controls(renderer, camera);

    const scene = new THREE.Scene()
    scene.background = new THREE.Color(0xc0c0c0)
    const pmremGenerator = new THREE.PMREMGenerator( renderer );
    scene.environment = pmremGenerator.fromScene(new RoomEnvironment(), 0.04).texture;

    const ambientLight = new THREE.AmbientLight(0x404040)
    scene.add(ambientLight); 
    const directionalLight = new THREE.DirectionalLight(0xffffff, 0.5);
    directionalLight.position.set(2, 2, 2)
    scene.add(directionalLight);

    const sqrt3 = Math.sqrt(3)
    const cube = new THREE.Mesh(new THREE.BoxGeometry(1/sqrt3, 1/sqrt3, 1/sqrt3), new THREE.MeshLambertMaterial({color: 0xffff00}))
    cube.position.set(-1, 1, 1)
    scene.add(cube)
    const icosahedron = new THREE.Mesh(new THREE.IcosahedronGeometry(0.5, 0), new THREE.MeshLambertMaterial({color: 0xff0000}))
    icosahedron.position.set(1, 1, 1)
    scene.add(icosahedron)
    const torus = new THREE.Mesh(new THREE.TorusGeometry(0.5, 0.2, 32, 100), new THREE.MeshLambertMaterial({color: 0x00ff00}))
    torus.position.set(-2, 1, -1)
    scene.add(torus)
    const sphere = new THREE.Mesh(new THREE.SphereGeometry(0.5, 32, 16), new THREE.MeshLambertMaterial({color: 0x0000ff}))
    sphere.position.set(0, 1, -1)
    scene.add(sphere)
    const torusKnot = new THREE.Mesh(new THREE.TorusKnotGeometry(0.3, 0.1, 100, 32), new THREE.MeshLambertMaterial({color: 0xff00ff}))
    torusKnot.position.set(2, 1, -1)
    scene.add(torusKnot)
    const meshes: THREE.Mesh[] = [cube, torusKnot, icosahedron, torus, sphere] 

    const groundGeometry = new THREE.PlaneGeometry(10, 10, 10, 10)
    const groundMaterial = new THREE.MeshLambertMaterial({color: 0x808080})
    const ground = new GroundAndGroundShadow(renderer, { planeSize: 6, alwaysUpdate: true, cameraHelper: true } )
    ground.updateGround(groundGeometry, groundMaterial)
    ground.addToScene(scene)
    ground.group.rotateX(-Math.PI/2)

    const dataGui = new DataGUI();
    const shadowFolder = dataGui.gui.addFolder('shadow');
    shadowFolder.open();

    shadowFolder.add(ground.parameters, 'cameraHelper').onChange(() => ground.updateParameters())
    shadowFolder.add(ground.parameters, 'alwaysUpdate')
    shadowFolder.add(ground.parameters, 'blur', 0, 15, 0.1).onChange(() => ground.needsUpdate = true)
    shadowFolder.add(ground.parameters, 'darkness', 1, 5, 0.1).onChange(() => ground.updateParameters())
    shadowFolder.add(ground.parameters, 'opacity', 0, 1, 0.01).onChange(() => ground.updateParameters())
    shadowFolder.add(ground.parameters, 'planeSize', 3, 10, 0.1).onChange(() => ground.updatePlaneAndShadowCamera())
    shadowFolder.add(ground.parameters, 'cameraFar', 0.1, 10, 0.1).onChange(() => ground.updatePlaneAndShadowCamera())
    shadowFolder.add(ground.group.position, 'x', -3, 3, 0.1).onChange(() => ground.needsUpdate = true)
    shadowFolder.add(ground.group.position, 'y', -3, 3, 0.1).onChange(() => ground.needsUpdate = true)
    shadowFolder.add(ground.group.position, 'z', -3, 3, 0.1).onChange(() => ground.needsUpdate = true)

    const onWindowResize = () => {
        camera.aspect = window.innerWidth / window.innerHeight
        camera.updateProjectionMatrix()
        renderer.setSize(window.innerWidth, window.innerHeight)
        render()
    }
    window.addEventListener('resize', onWindowResize, false)

    const elapsedTime = new ElapsedTime();
    const animate = (timestamp: number) => {
        elapsedTime.update(timestamp);
        requestAnimationFrame(animate)
        for (let i = 0; i < meshes.length; ++i) {
            meshes[i].position.y = 1.5 + Math.sin(elapsedTime.allTimeMs / 1000 * Math.PI * 2 * 0.2 + i / 5 * Math.PI * 2) * 1
            meshes[i].rotation.x += elapsedTime.deltaTimeMs / 1000 *  (1 + i / 5)
            meshes[i].rotation.y += elapsedTime.deltaTimeMs / 1000  * (1 + i / (i-5))
        }
        controls.update()
        ground.render(scene, camera)
        render()
        statistic.update();
    }

    const render = () => {
        renderer.render(scene, camera)
    }
    animate(0)
}