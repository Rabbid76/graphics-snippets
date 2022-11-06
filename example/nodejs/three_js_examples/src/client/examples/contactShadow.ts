import { GroundAndGroundShadow } from '../three/groundAndGroundShadow'
import * as THREE from 'three'
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls'
import { RoomEnvironment } from 'three/examples/jsm/environments/RoomEnvironment.js';
import Stats from 'three/examples/jsm/libs/stats.module' 
import { GUI } from 'dat.gui'

export const contactShadow = (canvas: any) => {
    const renderer = new THREE.WebGLRenderer({canvas: canvas, antialias: true, alpha: true})
    renderer.setSize(window.innerWidth, window.innerHeight)
    renderer.setPixelRatio(window.devicePixelRatio);
    document.body.appendChild(renderer.domElement)
    // @ts-ignore
    const stats = new Stats();
    document.body.appendChild(stats.dom);

    const camera = new THREE.PerspectiveCamera(45, window.innerWidth / window.innerHeight, 0.1, 1000)
    camera.position.y = 5
    camera.position.z = 10
    const controls = new OrbitControls(camera, renderer.domElement)

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
    const ground = new GroundAndGroundShadow(renderer, scene, camera, groundGeometry, groundMaterial)
    ground.group.rotateX(-Math.PI/2)

    const gui = new GUI();
    const shadowFolder = gui.addFolder('shadow');
    shadowFolder.open();

    shadowFolder.add(ground.parameters, 'cameraHelper')
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

    let start: number, previousTimeStamp: number;
    const animate = (timestamp: number) => {
        if (start === undefined) {
            start = timestamp;
        }
        if (previousTimeStamp === undefined) {
            previousTimeStamp = timestamp;
        }
        const allTimeMs = timestamp - start;
        const elapsedMs = timestamp - previousTimeStamp;
        previousTimeStamp = timestamp
        requestAnimationFrame(animate)
        for (let i = 0; i < meshes.length; ++i) {
            meshes[i].position.y = 1.5 + Math.sin(allTimeMs / 1000 * Math.PI * 2 * 0.2 + i / 5 * Math.PI * 2) * 1
            meshes[i].rotation.x += elapsedMs / 1000 *  (1 + i / 5)
            meshes[i].rotation.y += elapsedMs / 1000  * (1 + i / (i-5))
        }
        controls.update()
        ground.render()
        render()
        stats.update()
    }

    const render = () => {
        renderer.render(scene, camera)
    }
    animate(0)
}