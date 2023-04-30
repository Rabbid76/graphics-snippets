import { ElapsedTime } from '../three/timeUtility'
import { Controls } from '../three/controls'
import { DataGUI, Statistic } from '../three/uiUtility'
import { LinearDepthRenderMaterial } from '../three/shaderUtility';
import { 
    BoxUpdateHelper, 
    boundingBoxInViewSpace,
    boxFromOrthographicViewVolume,
    getMaxSamples,
    setOrthographicViewVolumeFromBox
} from '../three/threeUtility'
import { 
    RenderOverrideVisibility,
    RenderPass,
} from '../three/renderPass'
import {
    RectAreaLightAndShadow,
    RectAreaLightAndShadowWithDirectionalLight
} from '../three/rectAreaLightAndShadow'
import * as THREE from 'three'
import { RoomEnvironment } from 'three/examples/jsm/environments/RoomEnvironment.js';

export const rectAreaLightShadow = (canvas: any) => {
    const width = window.innerWidth;
    const height = window.innerHeight;
    const renderer = new THREE.WebGLRenderer({canvas: canvas, antialias: true, alpha: true})
    renderer.setSize(width, height)
    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.shadowMap.enabled = true;
    renderer.shadowMap.type = THREE.PCFSoftShadowMap;
    document.body.appendChild(renderer.domElement);
    const statistic = new Statistic();
    const renderPass = new RenderPass();
    const maxSamples = getMaxSamples(renderer);

    const camera = new THREE.PerspectiveCamera(45, width / height, 0.1, 100);
    camera.position.y = 10;
    camera.position.z = 15;
    const controls = new Controls(renderer, camera);

    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0xc0c0c0);
    const pmremGenerator = new THREE.PMREMGenerator(renderer);
    scene.environment = pmremGenerator.fromScene(new RoomEnvironment(), 0.04).texture;

    const sceneBox = new THREE.Box3(new THREE.Vector3(-3.5, 0, -3), new THREE.Vector3(3, 3, 2.5));
    const sceneBoxHelper = new BoxUpdateHelper(sceneBox, { color: 0xff8080, opacity: 0.25 });
    sceneBoxHelper.addTo(scene);
    sceneBoxHelper.visible = false;

    const directionalLight = new THREE.DirectionalLight(0xffffff, 0.5);
    directionalLight.position.set(2, 5, 2);
    directionalLight.lookAt(0, 0, 0);
    directionalLight.castShadow = true;
    directionalLight.shadow.mapSize = new THREE.Vector2(1024, 1024);
    const directionalLightCamera = directionalLight.shadow.camera as THREE.OrthographicCamera;
    directionalLightCamera.far = 20;
    scene.add(directionalLight);  
    
    const rectAreaLight = new THREE.RectAreaLight(0xffffff, 100, 1, 1);
    const viewportSize = new THREE.Vector2(width, height);
    //const rectAreaLightAndShadow: RectAreaLightAndShadow = new RectAreaLightAndShadowWithShadowMap(rectAreaLight, viewportSize, undefined, false);
    const rectAreaLightAndShadow: RectAreaLightAndShadow = new RectAreaLightAndShadowWithDirectionalLight(rectAreaLight, viewportSize, { samples: maxSamples, shadowIntensity: 0.5, alwaysUpdate: true });
    rectAreaLightAndShadow.addToScene(scene);  

    const shadowBoxHelper = new BoxUpdateHelper(boxFromOrthographicViewVolume(directionalLightCamera), { color: 0x80ff80, opacity: 0.25 });
    shadowBoxHelper.visible = false;
    shadowBoxHelper.addTo(scene);

    const setLightSource = (source: string) => {
        const enableRectAreaLight = source === 'rectAreaLight';
        rectAreaLightAndShadow.visible = enableRectAreaLight;
        directionalLight.visible = !enableRectAreaLight;
    }

    const updateShadowBox = () => {
        const pos = directionalLight.position.clone();
        directionalLight.lookAt(0, 0, 0);
        shadowBoxHelper.object.position.set(pos.x, pos.y, pos.z);
        shadowBoxHelper.object.lookAt(0, 0, 0);
        rectAreaLightAndShadow.setLineOfSight(pos.clone(), new THREE.Vector3(0, 0, 0));
        const shadowVolumeBox = boundingBoxInViewSpace(sceneBox, rectAreaLightAndShadow.shadowCamera);
        rectAreaLightAndShadow.setShadowVolume(shadowVolumeBox);
        setOrthographicViewVolumeFromBox(directionalLightCamera, shadowVolumeBox);
        shadowBoxHelper.box = boxFromOrthographicViewVolume(directionalLightCamera)
        shadowBoxHelper.update();
    };
    updateShadowBox();
    const transformControl = controls.addTransformControl(directionalLight, scene);
    transformControl.addEventListener('objectChange', (event: any) => {
        updateShadowBox();
    });
    
    let meshes: THREE.Mesh[] = [];
    const addMesh = (position: THREE.Vector3, geometry: THREE.BufferGeometry, material: THREE.Material): THREE.Mesh => {
        const mesh = new THREE.Mesh(geometry, material);
        mesh.position.set(position.x, position.y, position.z);
        mesh.castShadow = true;
        mesh.receiveShadow = true;
        meshes.push(mesh);
        scene.add(mesh);
        return mesh;
    };
    const envMapIntensity = 0.25;
    const sqrt3 = Math.sqrt(3)
    addMesh(new THREE.Vector3(-1, 1, 1), new THREE.BoxGeometry(1/sqrt3, 1/sqrt3, 1/sqrt3), new THREE.MeshPhysicalMaterial({color: 0xffff00, envMapIntensity: envMapIntensity}));
    addMesh(new THREE.Vector3(1, 1, 1), new THREE.IcosahedronGeometry(0.5, 0), new THREE.MeshPhysicalMaterial({color: 0xff0000, envMapIntensity: envMapIntensity}));
    addMesh(new THREE.Vector3(-2, 1, -1), new THREE.TorusGeometry(0.5, 0.2, 32, 100), new THREE.MeshPhysicalMaterial({color: 0x00ff00, envMapIntensity: envMapIntensity}));
    addMesh(new THREE.Vector3(0, 1, -1), new THREE.SphereGeometry(0.5, 32, 16), new THREE.MeshPhysicalMaterial({color: 0x0000ff, envMapIntensity: envMapIntensity}));
    addMesh(new THREE.Vector3(2, 1, -1), new THREE.TorusKnotGeometry(0.3, 0.1, 100, 32), new THREE.MeshPhysicalMaterial({color: 0xff00ff, envMapIntensity: envMapIntensity}));

    const groundGroup = new THREE.Group();
    groundGroup.rotation.x = -Math.PI / 2;
    scene.add(groundGroup);
    const groundGeometry = new THREE.PlaneGeometry(10, 10, 10, 10);
    const groundMaterial = new THREE.MeshPhysicalMaterial({color: 0x808080, envMapIntensity: envMapIntensity});
    const groundMesh = new THREE.Mesh(groundGeometry, groundMaterial);
    groundMesh.receiveShadow = true;
    groundGroup.add(groundMesh);

    let shadowDebugPlane: THREE.Mesh | undefined;
    if (rectAreaLightAndShadow.shadowMapTexture) {
        shadowDebugPlane = new THREE.Mesh(
            new THREE.PlaneGeometry(5, 5),
            new THREE.MeshBasicMaterial({ map: rectAreaLightAndShadow.shadowMapTexture })
        );
        shadowDebugPlane.position.x = -8;
        shadowDebugPlane.position.y = 3;
        shadowDebugPlane.visible = false;
        scene.add(shadowDebugPlane);
    }

    const generalUiProperties = {
        'light source': 'rectAreaLight',
        'scene volume': sceneBoxHelper.visible, 
        'shadow volume': shadowBoxHelper.visible, 
        'debug shadow map': false, 
        'debug output': 'off', 
    };
    setLightSource(generalUiProperties['light source']);
    const dataGui = new DataGUI();
    dataGui.gui.add(generalUiProperties, 'light source', {
        'rectAreaLight': 'rectAreaLight',
        'directionalLight': 'directionalLight'
    }).onChange((value) => setLightSource(value));
    dataGui.gui.add(generalUiProperties, 'scene volume').onChange((enabled: boolean) => sceneBoxHelper.visible = enabled);
    dataGui.gui.add(generalUiProperties, 'shadow volume').onChange((enabled: boolean) => shadowBoxHelper.visible = enabled);
    dataGui.gui.add(generalUiProperties, 'debug shadow map');
    let debugOptions: any = {};
    debugOptions['off'] = 'off';
    if (rectAreaLightAndShadow.depthTexture) {
        debugOptions['depth'] = 'depth';
    }
    debugOptions['shadow'] = 'shadow';
    dataGui.gui.add(generalUiProperties, 'debug output', debugOptions);
    const shadowFolder = dataGui.gui.addFolder('rectAreaLight shadow');
    shadowFolder.add<any>(rectAreaLightAndShadow, 'shadowIntensity', 0.0, 1.0);

    const onWindowResize = () => {
        const width = window.innerWidth;
        const height = window.innerHeight;
        camera.aspect = width / height;
        camera.updateProjectionMatrix();
        renderer.setSize(width, height);
        rectAreaLightAndShadow.setSize(width, height);
    };
    window.addEventListener('resize', onWindowResize, false);

    const elapsedTime = new ElapsedTime();
    const animate = (timestamp: number) => {
        elapsedTime.update(timestamp);
        requestAnimationFrame(animate);
        for (let i = 0; i < meshes.length; ++i) {
            meshes[i].position.y = 1.5 + Math.sin(elapsedTime.allTimeMs / 1000 * Math.PI * 2 * 0.2 + i / 5 * Math.PI * 2) * 1;
            meshes[i].rotation.x += elapsedTime.getDegreePerSecond((1 + i / 5) * 60, true);
            meshes[i].rotation.y += elapsedTime.getDegreePerSecond((1 + i / (i-5)) * 60, true);
        }
        controls.update();
        render();
        statistic.update();
    }

    let invisibleObjects: any[]= [sceneBoxHelper, shadowBoxHelper, transformControl, rectAreaLightAndShadow.rectAreaLight];
    if (shadowDebugPlane) {
        invisibleObjects.push(shadowDebugPlane);
    }
    const renderOverrideVisibility = new RenderOverrideVisibility(
        false, 
        (object: any) => invisibleObjects.includes(object), 
        null);
    const renderShadow = () => {
        renderOverrideVisibility.render(scene, () => {
            if (generalUiProperties['debug output'] === 'off' && generalUiProperties['light source'] !== 'rectAreaLight' && !generalUiProperties['debug output']) {
                return;
            }
            rectAreaLightAndShadow.renderShadow(renderer, scene, camera);
        });
    }

    const depthRenderMaterial = rectAreaLightAndShadow.depthTexture ? new LinearDepthRenderMaterial({ depthTexture: rectAreaLightAndShadow.depthTexture }) : null;
    const render = () => {
        renderShadow();
        switch(generalUiProperties['debug output']) {
            case 'depth':
                if (depthRenderMaterial) {
                    renderPass.renderScreenSpace(renderer, depthRenderMaterial.update({camera: camera}), null);
                }
                break;
            case 'shadow':
                rectAreaLightAndShadow.blendShadow(renderer, true);
                break;
            default:
                if (shadowDebugPlane) {
                    shadowDebugPlane.visible = generalUiProperties['debug shadow map'];
                }
                renderer.render(scene, camera);
                if (shadowDebugPlane) {
                    shadowDebugPlane.visible = false;
                }
                if (generalUiProperties['light source'] === 'rectAreaLight') {
                    rectAreaLightAndShadow.blendShadow(renderer, false);
                }
                break;
        }
    }
    animate(0);
    updateShadowBox();
}
