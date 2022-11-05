import * as THREE from 'three'
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls'
import { RoomEnvironment } from 'three/examples/jsm/environments/RoomEnvironment.js';

export const helloCube = (canvas: any) => {
    const renderer = new THREE.WebGLRenderer({canvas: canvas, antialias: true, alpha: true})
    renderer.setSize(window.innerWidth, window.innerHeight)
    document.body.appendChild(renderer.domElement)

    const camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000)
    camera.position.z = 2
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

    const cubeGeometry = new THREE.BoxGeometry()
    //const cylinderGeometry = new THREE.CylinderGeometry(0.5, 0.5, 1, 32 );
    //const material = new THREE.MeshLambertMaterial({color: 0xffff00})
    const metallicMaterial = new THREE.MeshPhysicalMaterial({color: 0xc0c0c0})
    metallicMaterial.metalness = 1
    metallicMaterial.roughness = 0
    const cube = new THREE.Mesh(cubeGeometry, metallicMaterial)
    scene.add(cube)

    window.addEventListener('resize', onWindowResize, false)
    function onWindowResize() {
        camera.aspect = window.innerWidth / window.innerHeight
        camera.updateProjectionMatrix()
        renderer.setSize(window.innerWidth, window.innerHeight)
        render()
    }

    function animate() {
        requestAnimationFrame(animate)
        cube.rotation.x += 0.01
        cube.rotation.y += 0.01
        controls.update()
        render()
    }

    function render() {
        renderer.render(scene, camera)
    }
    animate()
}