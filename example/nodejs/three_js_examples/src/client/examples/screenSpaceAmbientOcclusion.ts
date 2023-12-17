import { ElapsedTime } from '../three/timeUtility'
import { Controls } from '../three/controls'
import { AoAlgorithms } from "../three/aoRenderPass";
import { SsaoDebugRenderPass } from '../three/ssaoDebugRenderPass';
import { 
    getMaxSamples, 
    SceneVolume,
} from '../three/threeUtility'
import { DataGUI, Statistic } from '../three/uiUtility' 
import { setupDragDrop } from '../util/drag_target'
import {
    Box3,
    BoxGeometry,
    BufferGeometry,
    CapsuleGeometry,
    Color,
    ConeGeometry,
    CylinderGeometry,
    DodecahedronGeometry,
    Group,
    IcosahedronGeometry,
    Material,
    Mesh,
    MeshStandardMaterial,
    OctahedronGeometry,
    PerspectiveCamera,
    PlaneGeometry,
    PMREMGenerator,
    Scene,
    SphereGeometry,
    TetrahedronGeometry,
    TorusGeometry,
    TorusKnotGeometry,
    Vector2,
    Vector3,
    WebGLRenderer,
    WebGLRenderTarget,
    Object3D,
} from 'three';
import { EffectComposer } from 'three/examples/jsm/postprocessing/EffectComposer'
import { GLTFLoader } from "three/examples/jsm/loaders/GLTFLoader";
import { DRACOLoader } from 'three/examples/jsm/loaders/DRACOLoader.js';
import { RoomEnvironment } from 'three/examples/jsm/environments/RoomEnvironment.js';

// TODO:
// - ao normal vector anti aliasing ?
// - sampler2DMS ?
// - render just a rectangle of the screen depending on the projected scene box
// - find scale for each algorithm (SSAO, SAO, HBAO, N8H0, GTAO)
// - presets: screenSpaceRadius, aoNoiseType
// - consider alpha map in g-buffer

export const screenSpaceAmbientOcclusion = (canvas: any) => {
    const renderer = new WebGLRenderer({canvas: canvas, antialias: true, alpha: true});
    renderer.setSize(window.innerWidth, window.innerHeight);
    document.body.appendChild(renderer.domElement);
    const statistic = new Statistic();
    const dataGui = new DataGUI();
    const gltfLoader = new GLTFLoader();
    const dracoLoader = new DRACOLoader();
	dracoLoader.setDecoderPath( '../draco/' );
	dracoLoader.setDecoderConfig( { type: 'js' } );
	gltfLoader.setDRACOLoader( dracoLoader );

    const camera = new PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 50);
    camera.position.set(6, 0, 0);
    const controls = new Controls(renderer, camera);

    const scene = new Scene();
    scene.background = new Color(0xffffff);
    const pmremGenerator = new PMREMGenerator(renderer);
    const roomEnvironment = new RoomEnvironment(renderer);
    //roomEnvironment.rotateY(Math.PI);
    const environmentTexture = pmremGenerator.fromScene(roomEnvironment, 0.04).texture;
    scene.environment = environmentTexture;
    scene.background = environmentTexture;

    const maxSamples = getMaxSamples(renderer);
    const ssaoRenderTarget = new WebGLRenderTarget(window.innerWidth, window.innerHeight, { samples: maxSamples })
    const effectComposer = new EffectComposer(renderer, ssaoRenderTarget);
    const ssaoPass = new SsaoDebugRenderPass(window.innerWidth, window.innerHeight, maxSamples, {
        camera,
        scene,
        capabilities: renderer.capabilities,
    });
    effectComposer.addPass(ssaoPass);

    const generateAoTestScene = (group: Group) => {
        const n = 128;
        //const unitPositions = sphereSamples(n);
        for (let i = 0; i < n; ++ i) {
            const position = new Vector3(Math.random() - 0.5, Math.random() - 0.5, Math.random() - 0.5).normalize();
            const p = position.multiplyScalar(2);
            const mesh = randomMesh();
            mesh.rotation.x = Math.random() * Math.PI * 2;
            mesh.rotation.z = Math.random() * Math.PI;
            mesh.position.set(p.x, p.y, p.z);
            group.add(mesh);
        };
    }

    const generateSponzaScene = (group: Group) => {
        gltfLoader.load( '../sponza_cd.glb', ( gltf: any ) => {
            const model = gltf.scene;
            const box = new Box3().setFromObject(model);
            console.log(box);
            model.position.set( 0, -1.5, 0 );
            group.add( model );
        }, undefined, ( e: any ) => console.error( e ) );
    };

    const sceneGroups = [ new Group(), new Group() ];
    sceneGroups.forEach( ( group ) => scene.add( group ) );
    generateAoTestScene( sceneGroups[ 0 ] );
    generateSponzaScene( sceneGroups[ 1 ] );
    const sceneParameter = {
        scene: 0,
    };
    sceneGroups[ 1 - sceneParameter.scene ].visible = false;

    const boundingVolume = new SceneVolume();
    boundingVolume.updateFromObject(scene);

    dataGui.gui.add( sceneParameter, 'scene', {
        'AO test scene': 0,
        'Sponza': 1,
    } ).onChange( ( value ) => {
    
        for ( let i = 0; i < sceneGroups.length; ++ i ) sceneGroups[ i ].visible = i == value;
        if (value == 0) {
            camera.position.set(0, 0, 5.);
            camera.rotation.set(0, 0, 0);
            generalProperties.rotate = true;
        } else {
            camera.position.set(6, 0, 0);
            camera.rotation.set(0, 0, 0);
            generalProperties.rotate = false;
        }
        camera.lookAt(0, 0, 0);
        rotationController.updateDisplay();
    } );

    const generalProperties = {
        'rotate': true,
        'debug pass': 'off'
    };
    ssaoPass.debugOutput = generalProperties['debug pass'];
    const rotationController = dataGui.gui.add(generalProperties, 'rotate');
    dataGui.gui.add<any>(ssaoPass, 'debugOutput', {
      'off ': 'off',
      'color buffer': 'color',
      'linear depth': 'lineardepth',
      'g-buffer normal vector': 'g-normal',
      'g-buffer depth': 'g-depth',
      'AO': 'ao',
      'AO + denoise': 'aodenoise',
    })
    const parameters = ssaoPass.parameters;
    const updateParameters = () => {
        ssaoPass.needsUpdate =true;
        ssaoPass.aoRenderTargets.parametersNeedsUpdate = true;
    };
    const aoFolder = dataGui.gui.addFolder('SSAO');
    aoFolder.add<any>(parameters, 'aoEnabled').onChange(() => updateParameters());
    aoFolder.add<any>(parameters, 'aoAlwaysUpdate').onChange(() => updateParameters());
    aoFolder.add<any>(parameters, 'aoIntensity', 0, 1).onChange(() => updateParameters());
    aoFolder.add( parameters, 'algorithm', {
        'SSAO': AoAlgorithms.SSAO,
        'SAO': AoAlgorithms.SAO,
        'N8AO': AoAlgorithms.N8AO,
        'HBAO': AoAlgorithms.HBAO,
        'GTAO': AoAlgorithms.GTAO,
    } ).onChange( () => {
        const number = parseInt(parameters.algorithm.toString());
        parameters.nvAlignedSamples = (number !== AoAlgorithms.GTAO && number !== AoAlgorithms.HBAO);
		nvAlignedSamplesController.updateDisplay();
        updateParameters() 
    });
    aoFolder.add( parameters, 'aoSamples' ).min( 1 ).max( 32 ).step( 1 ).onChange( () => updateParameters() );
    const nvAlignedSamplesController = aoFolder.add( parameters, 'nvAlignedSamples' ).onChange( () => updateParameters() );
    aoFolder.add( parameters, 'screenSpaceRadius' ).onChange( () => updateParameters() );
    aoFolder.add( parameters, 'radius' ).min( 0.01 ).max( 10 ).step( 0.01 ).onChange( () => updateParameters() );
    aoFolder.add( parameters, 'distanceExponent' ).min( 1 ).max( 4 ).step( 0.01 ).onChange( () => updateParameters() );
    aoFolder.add( parameters, 'thickness' ).min( 0.01 ).max( 10 ).step( 0.01 ).onChange( () => updateParameters() );
    aoFolder.add( parameters, 'distanceFallOff' ).min( 0.0 ).max( 1.0 ).step( 0.01 ).onChange( () => updateParameters() );
    aoFolder.add( parameters, 'bias' ).min( 0 ).max( 0.1 ).step( 0.0001 ).onChange( () => updateParameters() );
    aoFolder.add( parameters, 'scale' ).min( 0.01 ).max( 2.0 ).step( 0.01 ).onChange( () => updateParameters() );
    aoFolder.add( parameters, 'pdLumaPhi' ).min( 0 ).max( 20 ).step( 0.01 ).onChange( () => updateParameters() );
    aoFolder.add( parameters, 'pdDepthPhi' ).min( 0.01 ).max( 20 ).step( 0.01 ).onChange( () => updateParameters() );
    aoFolder.add( parameters, 'pdNormalPhi' ).min( 0.01 ).max( 20 ).step( 0.01 ).onChange( () => updateParameters() );
    aoFolder.add( parameters, 'pdRadius' ).min( 0 ).max( 32 ).step( 1 ).onChange( () => updateParameters() );
    aoFolder.add( parameters, 'pdSamples' ).min( 2 ).max( 32 ).step( 1 ).onChange( () => updateParameters() );
    
    const updateSceneBounds = () => {
        boundingVolume.updateFromObject(scene);
        const sceneSize = boundingVolume.size;
        ssaoPass.updateBounds((sceneSize.x + sceneSize.y + sceneSize.z) / 3);
    }
    updateSceneBounds();
    const onWindowResize = () => {
        camera.aspect = window.innerWidth / window.innerHeight;
        camera.updateProjectionMatrix();
        effectComposer.setSize(window.innerWidth, window.innerHeight);
        effectComposer.setSize(window.innerWidth, window.innerHeight);
        renderer.setSize(window.innerWidth, window.innerHeight);
    };
    window.addEventListener('resize', onWindowResize, false);
    setupDragDrop('drag_target', 'hover', (file: File, event: ProgressEvent<FileReader>) => {
        // @ts-ignore
        loadResource(file.name, event.target.result, objectGroup, () => {
            updateSceneBounds();
        });
    });

    const elapsedTime = new ElapsedTime();
    const animate = (timestamp: number) => {
        elapsedTime.update(timestamp);
        requestAnimationFrame(animate);
        if (generalProperties.rotate) {
            scene.rotateY(-elapsedTime.getDegreePerSecond(15, true));
        }
        controls.update();
        const nearFar = boundingVolume.getNearAndFarForPerspectiveCamera(
            camera.position,
            3
          );
          camera.near = Math.max(0.00001, nearFar[0] * 0.9);
          camera.far = Math.max(1, camera.near, nearFar[1]);
          camera.updateProjectionMatrix();
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
    { id: 'Tetrahedron', geometry: new TetrahedronGeometry(0.5) },
    { id: 'Cube', geometry: new BoxGeometry(1/sqrt3, 1/sqrt3, 1/sqrt3) },
    { id: 'Octahedron', geometry: new OctahedronGeometry(0.5) },
    { id: 'Dodecahedron', geometry: new DodecahedronGeometry(0.5) },
    { id: 'Icosahedron', geometry:  new IcosahedronGeometry(0.5) },
    { id: 'Cone', geometry:  new ConeGeometry(0.5, 1, 32) },
    { id: 'Cylinder', geometry:  new CylinderGeometry(0.5, 0.5, 1, 32) },
    { id: 'Sphere', geometry:  new SphereGeometry(0.5, 32, 16) },
    { id: 'Capsule', geometry:  new CapsuleGeometry(0.3, 0.5, 32, 32) },
    { id: 'Torus', geometry:  new TorusGeometry(0.5, 0.25, 32, 100) },
    { id: 'TorusKnot', geometry:  new TorusKnotGeometry(0.4, 0.2, 100, 32) }
];

const randomGeometry = (): BufferGeometry => {
    const index = Math.floor(Math.random() * geometries.length);
    return geometries[index].geometry;
}

const randomColor = (): Color => {
    const color = new Color();
    color.setHSL(Math.random(), 0.5, 0.75);
    return color;
}

const randomMaterial = (): Material => {
    const material = new MeshStandardMaterial({color: randomColor()});
    return material;
}

const randomMesh = (): Mesh => {
    const mesh = new Mesh(randomGeometry(), randomMaterial());
    return mesh
}

const sphereToEquirectangular = (unitDirection3D: Vector3): Vector2 => {
    const xy = { x: Math.atan2(unitDirection3D.y, unitDirection3D.x), y: Math.asin(unitDirection3D.z) };
    const uv = new Vector2(xy.x / (2 * Math.PI) + 0.5, xy.y / Math.PI + 0.5);
    return uv;
}

const equirectangularToSphere = (uv: Vector2): Vector3 => {
    const theta = (uv.x - 0.5) * Math.PI * 2;
    const phi = (uv.y - 0.5) * Math.PI;
    const sinTheta = Math.sin(theta);
    const cosTheta = Math.cos(theta);
    const sinPhi = Math.sin(phi);
    const cosPhi = Math.cos(phi);
    const unitDirection3D = new Vector3(cosTheta * cosPhi, sinTheta * cosPhi, sinPhi);
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

const uvToUnitSphere = (u: number, v: number): Vector3 => {
    const phi = v * 2 * Math.PI;
    const cosTheta = 2 * u - 1;
    const sinTheta = Math.sqrt(1.0 - cosTheta * cosTheta);
    const sinPhi = Math.sin(phi);
    const cosPhi = Math.cos(phi);
    const unitSphere = new Vector3(cosTheta * cosPhi, sinTheta * cosPhi, sinPhi);
    return unitSphere;
}

const sphereSamples = (n: number): Vector3[] => {
    const samples = [];
    for (let i=0; i < n; ++i) {
        const uv = hammersleySequence(i, n);
        const unitSphere = uvToUnitSphere(uv.u, uv.v);
        samples.push(unitSphere);
    }
    return samples;
}

const resourceCache = new Map<string, any>();
const loadResource = async (name: string, resource: string, container: Group, updateCallback: () => void) => {
    let newScene = resourceCache.get(name);
    if (!newScene) {
        const gltfLoader = new GLTFLoader();
        const gltf = await gltfLoader.loadAsync(resource);
        newScene = gltf.scene;
    }
    updateScene(container, newScene, updateCallback);
}

const updateScene = (container: Group, newScene: Object3D, updateCallback: () => void) =>{
    newScene.updateMatrixWorld();
    const sceneBox = new Box3().setFromObject(newScene);
    const sceneSize = sceneBox.getSize(new Vector3());
    const sceneCenter = sceneBox.getCenter(new Vector3());
    newScene.position.set(-sceneCenter.x, -sceneBox.min.y, -sceneCenter.z);
    container.clear();
    container.add(newScene);
    const planSize = Math.max(sceneSize.x, sceneSize.z) * 2;
    const planMesh = new Mesh(new PlaneGeometry(planSize, planSize), new MeshStandardMaterial({color: 0x808080}));
    planMesh.rotation.x = -Math.PI / 2;
    container.add(planMesh);
    container.position.set(0, -sceneSize.y/2, 0);
    container.updateMatrixWorld();
    updateCallback();
}