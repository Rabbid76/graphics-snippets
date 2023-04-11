import { DimensioningArrow } from '../three/dimensioningArrow';
import {
    AxesHelper,
    Color,
    DirectionalLight,
    GridHelper,
    Mesh,
    MeshPhysicalMaterial,
    PerspectiveCamera,
    PCFSoftShadowMap,
    PlaneGeometry,
    PMREMGenerator,
    Scene,
    ShadowMaterial,
    SphereGeometry,
    WebGLRenderer,
} from 'three';
import { CSS2DRenderer } from 'three/examples/jsm/renderers/CSS2DRenderer.js';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls'
import { TransformControls } from 'three/examples/jsm/controls/TransformControls.js';
import { RoomEnvironment } from 'three/examples/jsm/environments/RoomEnvironment.js';
// @ts-ignore
import Stats from 'three/examples/jsm/libs/stats.module' 
import { GUI } from 'dat.gui'

export const dimensioning = (canvas: any) => {
    const renderer = new WebGLRenderer({canvas: canvas, antialias: true, alpha: true});
    renderer.setSize(window.innerWidth, window.innerHeight);
    document.body.appendChild(renderer.domElement);
    //renderer.shadowMap.enabled = true;
    renderer.shadowMap.type = PCFSoftShadowMap;
    renderer.setSize(window.innerWidth, window.innerHeight);
    renderer.setPixelRatio(window.devicePixelRatio);
    //renderer.outputEncoding = sRGBEncoding;
    const labelRenderer = new CSS2DRenderer();
    labelRenderer.setSize(window.innerWidth, window.innerHeight);
    labelRenderer.domElement.style.position = 'absolute';
    labelRenderer.domElement.style.top = '0px';
    document.body.appendChild(labelRenderer.domElement);

    const camera = new PerspectiveCamera(60, window.innerWidth / window.innerHeight, 0.1, 1000);
    camera.position.y = 3;
    camera.position.z = 6;
    const controls = new OrbitControls(camera, labelRenderer.domElement);

    const scene = new Scene();
    scene.background = new Color(0xffffff);
    const pmremGenerator = new PMREMGenerator(renderer);
    const environmentTexture = pmremGenerator.fromScene(new RoomEnvironment(), 0.04).texture;
    scene.environment = environmentTexture;
    //scene.background = environmentTexture;

    const gridHelper = new GridHelper(10, 10);
    //gridHelper.visible = false;
    scene.add(gridHelper);
    const axesHelper = new AxesHelper(2);
    //axesHelper.visible = false;
    scene.add(axesHelper);

    const directionalLight = new DirectionalLight(0xffffff, 0.5);
    directionalLight.position.set(1, 3, 1);
    directionalLight.castShadow = true;
    scene.add(directionalLight);
    
    const groundGeometry = new PlaneGeometry(10, 10);
    groundGeometry.rotateX(-Math.PI / 2);
    const groundMaterial = new ShadowMaterial();
    const groundMesh = new Mesh(groundGeometry, groundMaterial);
    groundMesh.receiveShadow = true;
    scene.add(groundMesh);

    const material = new MeshPhysicalMaterial({color: 0xe02020});
    const geometry = new SphereGeometry(0.1, 32, 16);
    const startMesh = new Mesh(geometry, material);
    startMesh.position.set(-1.6, 0, 0);
    //startMesh.visible = false;
    scene.add(startMesh);
    const startMeshTransformControl = new TransformControls(camera, labelRenderer.domElement);
    startMeshTransformControl.addEventListener( 'dragging-changed', (event: any) => {
        controls.enabled = !event.value;
    });
    startMeshTransformControl.attach(startMesh);
    startMeshTransformControl.visible = false;
    scene.add(startMeshTransformControl);
    const endMesh = new Mesh(geometry, material);
    endMesh.position.set(1.6, 0, 0);
    //endMesh.visible = false;
    scene.add(endMesh);
    const endMeshTransformControl = new TransformControls(camera, labelRenderer.domElement);
    endMeshTransformControl.addEventListener( 'dragging-changed', (event: any) => {
        controls.enabled = !event.value;
    });
    endMeshTransformControl.attach(endMesh);
    endMeshTransformControl.visible = false;
    scene.add(endMeshTransformControl);

    const dimensioningArrow = new DimensioningArrow(startMesh.position, endMesh.position, { 
        color: 0x00000,
        arrowPixelWidth: 30.0,
        arrowPixelHeight: 50.0,
        shaftPixelWidth: 10.0,
        shaftPixelOffset: 3.0, 
        labelClass: 'label',
        deviceRatio: window.devicePixelRatio,
    });
    scene.add(dimensioningArrow);
    const updateDimensioning = () => {
        dimensioningArrow.setPosition(startMesh.position, endMesh.position, 0.1, 0.1);
    }

    // @ts-ignore
    const stats = new Stats();
    document.body.appendChild(stats.dom);
    const gui = new GUI();
    const uiProperties = {
        'mesh transform control': startMeshTransformControl.visible,
    }
    gui.add(uiProperties, 'mesh transform control').onChange((value) => {
        startMeshTransformControl.visible = value;
    });

    window.addEventListener('resize', () => {
        const width = window.innerWidth;
        const height = window.innerHeight;
        camera.aspect = width / height;
        camera.updateProjectionMatrix();
        renderer.setSize(width, height);
        labelRenderer.setSize(width, height);
    }, false);

    let previousTimeStamp: number | undefined;
    const animate = (timestamp: number) => {
        const deltaTimeMs = timestamp - (previousTimeStamp ?? timestamp);
        previousTimeStamp = timestamp;
        requestAnimationFrame(animate);
        //mesh.rotation.y += 45 * Math.PI / 180 * deltaTimeMs / 1000;
        controls.update();
        updateDimensioning();
        render();
        stats.update()
    }

    const render = () => {
        renderer.render(scene, camera);
        labelRenderer.render(scene, camera);
    }
    requestAnimationFrame(animate);
}
