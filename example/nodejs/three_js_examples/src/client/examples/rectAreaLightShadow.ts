import { ElapsedTime } from '../three/timeUtility'
import { Controls } from '../three/controls'
import { DataGUI, Statistic } from '../three/uiUtility'
import { LinearDepthRenderMaterial } from '../three/depthAndNormalMaterialsAndShaders';
import { ShadowMaterial } from '../three/shadowMaterialAndShader'
import { CopyMaterial } from '../three/shaderUtility'
import { 
    BoxHelper, 
    boundingBoxInViewSpace,
    boxFromOrthographicViewVolume,
    RenderOverrideVisibility,
    RenderPass,
    setOrthographicViewVolumeFromBox
} from '../three/threeUtility'
import * as THREE from 'three'
import { RoomEnvironment } from 'three/examples/jsm/environments/RoomEnvironment.js';
import { RectAreaLightHelper } from 'three/examples/jsm/helpers/RectAreaLightHelper.js';

export const rectAreaLightShadow = (canvas: any) => {
    const width = window.innerWidth;
    const height = window.innerHeight;
    const renderer = new THREE.WebGLRenderer({canvas: canvas, antialias: true, alpha: true})
    renderer.setSize(width, height)
    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.shadowMap.enabled = true;
    renderer.shadowMap.type = THREE.PCFShadowMap;
    document.body.appendChild(renderer.domElement);
    const statistic = new Statistic();
    const renderPass = new RenderPass();

    const camera = new THREE.PerspectiveCamera(45, width / height, 0.1, 100);
    camera.position.y = 10;
    camera.position.z = 15;
    const controls = new Controls(renderer, camera);

    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0xc0c0c0);
    const pmremGenerator = new THREE.PMREMGenerator(renderer);
    scene.environment = pmremGenerator.fromScene(new RoomEnvironment(), 0.04).texture;

    const sceneBox = new THREE.Box3(new THREE.Vector3(-3, 0, -2.5), new THREE.Vector3(3, 3, 2.5));
    const sceneBoxHelper = new BoxHelper(sceneBox, { color: 0xff8080, opacity: 0.25 });
    sceneBoxHelper.addTo(scene);
    sceneBoxHelper.visible = false;

    const directionalLight = new THREE.DirectionalLight(0xffffff, 0.5);
    directionalLight.position.set(2, 5, 2);
    directionalLight.lookAt(0, 0, 0);
    directionalLight.visible;
    directionalLight.castShadow = true;
    directionalLight.shadow.mapSize = new THREE.Vector2(1024, 1024);
    const directionalLightCamera = directionalLight.shadow.camera as THREE.OrthographicCamera;
    directionalLightCamera.far = 20;
    scene.add(directionalLight);  
    
    const rectAreaLight = new THREE.RectAreaLight(0xffffff, 100, 1, 1);
    scene.add(rectAreaLight);  
    const rectAreaLightAndShadow = new RectAreaLightAndShadow(rectAreaLight, directionalLight.shadow.camera, width, height, false);

    const shadowBoxHelper = new BoxHelper(boxFromOrthographicViewVolume(directionalLightCamera), { color: 0x80ff80, opacity: 0.25 });
    shadowBoxHelper.visible = false;
    shadowBoxHelper.addTo(scene);

    const setLightSource = (source: string) => {
        const enableRectAreaLight = source === 'rectAreaLight';
        rectAreaLight.visible = enableRectAreaLight;
        directionalLight.visible = !enableRectAreaLight;
    }

    const updateShadowBox = () => {
        const pos = directionalLight.position.clone();
        directionalLight.lookAt(0, 0, 0);
        rectAreaLightAndShadow.setPosition(pos.x, pos.y, pos.z);
        rectAreaLightAndShadow.lookAt(0, 0, 0);
        shadowBoxHelper.object.position.set(pos.x, pos.y, pos.z);
        shadowBoxHelper.object.lookAt(0, 0, 0);
        setOrthographicViewVolumeFromBox(directionalLightCamera, boundingBoxInViewSpace(sceneBox, directionalLightCamera));
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

    const shadowDebugPlan = new THREE.Mesh(
        new THREE.PlaneGeometry(5, 5),
        new THREE.MeshBasicMaterial({ map: rectAreaLightAndShadow.shadowMapTexture })
    );
    shadowDebugPlan.position.x = -8;
    shadowDebugPlan.position.y = 3;
    shadowDebugPlan.visible = false;
    scene.add(shadowDebugPlan);

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
    dataGui.gui.add(generalUiProperties, 'debug output', {
        'off': 'off',
        'depth': 'depth',
        'shadow': 'shadow'
    });
    const shadowFolder = dataGui.gui.addFolder('rectAreaLight shadow');
    shadowFolder.add(rectAreaLightAndShadow, 'intensity', 0.0, 1.0);

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

    const renderOverrideVisibility = new RenderOverrideVisibility(false, [sceneBoxHelper, shadowBoxHelper, transformControl, shadowDebugPlan, rectAreaLightAndShadow.light], null);
    const renderShadow = () => {
        renderOverrideVisibility.render(scene, () => {
            rectAreaLightAndShadow.renderShadowMap(renderer, scene);
            if (generalUiProperties['debug output'] === 'off' && generalUiProperties['light source'] !== 'rectAreaLight') {
                return;
            }
            rectAreaLightAndShadow.renderShadow(renderer, scene, camera);
        });
    }

    const depthRenderMaterial = new LinearDepthRenderMaterial({ depthTexture: rectAreaLightAndShadow.depthTexture });
    const copyMaterial = new CopyMaterial();
    const render = () => {
        renderShadow();
        switch(generalUiProperties['debug output']) {
            case 'depth':
                renderPass.renderScreenSpace(renderer, depthRenderMaterial.update({camera: camera}), null);
                break;
            case 'shadow':
                renderPass.renderScreenSpace(renderer, copyMaterial.update({texture: rectAreaLightAndShadow.shadowTexture, blending: THREE.NoBlending}), null);
                break;
            default:
                shadowDebugPlan.visible = generalUiProperties['debug shadow map'];
                renderer.render(scene, camera);
                shadowDebugPlan.visible = false;
                if (generalUiProperties['light source'] === 'rectAreaLight') {
                    renderPass.renderScreenSpace(renderer, copyMaterial.update({texture: rectAreaLightAndShadow.shadowTexture, blending: THREE.CustomBlending}), null);
                }
                break;
        }
    }
    animate(0);
    updateShadowBox();
}

class RectAreaLightAndShadow {
    public intensity: number = 0.5;
    public light: THREE.RectAreaLight;
    public shadowCamera: THREE.OrthographicCamera;
    public rectLightHelper?: RectAreaLightHelper;
    private width;
    private height;
    private shadowMapSize = 1024;
    private renderPass: RenderPass;
    private shadowDepthRenderTarget: THREE.WebGLRenderTarget;
    private depthMaterial: THREE.MeshDepthMaterial;
    private normalRenderMaterial: THREE.MeshNormalMaterial;
    private depthNormalRenderTarget: THREE.WebGLRenderTarget;
    private shadowRenderMaterial: ShadowMaterial;
    private shadowRenderTarget: THREE.WebGLRenderTarget;
    
    public get shadowMapTexture(): THREE.Texture { return this.shadowDepthRenderTarget.texture; } 
    public get depthTexture(): THREE.Texture { return this.depthNormalRenderTarget.depthTexture; } 
    public get shadowTexture(): THREE.Texture { return this.shadowRenderTarget.texture; } 
    
    constructor(light: THREE.RectAreaLight, shadowCamera: THREE.OrthographicCamera, width: number, height: number, addHelper?: boolean) {
        this.width = width;
        this.height = height;
        this.light = light;
        this.shadowCamera = shadowCamera;
        if (addHelper) {
            this.rectLightHelper = new RectAreaLightHelper(this.light);
            // @ts-ignore
            this.rectLightHelper.material.depthWrite = false;
            this.light.add(this.rectLightHelper);
        }
        this.renderPass = new RenderPass();

        this.shadowDepthRenderTarget = new THREE.WebGLRenderTarget(this.shadowMapSize, this.shadowMapSize);
        this.depthMaterial = new THREE.MeshDepthMaterial();

        const depthTexture = new THREE.DepthTexture(width, height);
        depthTexture.format = THREE.DepthStencilFormat;
        depthTexture.type = THREE.UnsignedInt248Type;
        this.depthNormalRenderTarget = new THREE.WebGLRenderTarget(this.width, this.height, {
            minFilter: THREE.NearestFilter,
            magFilter: THREE.NearestFilter,
            depthTexture
        });
        this.normalRenderMaterial = new THREE.MeshNormalMaterial();
        this.normalRenderMaterial.blending = THREE.NoBlending;

        this.shadowRenderMaterial = new ShadowMaterial();
        this.shadowRenderTarget = new THREE.WebGLRenderTarget(this.width, this.height);
    }

    public dispose(): void {
        this.shadowDepthRenderTarget.dispose();
        this.depthMaterial.dispose();
        this.normalRenderMaterial.dispose();
        this.depthNormalRenderTarget.dispose();
        this.shadowRenderMaterial.dispose();
    }

    public setPosition(x: number, y: number, z: number): void {
        this.light.position.set(x, y, z);
        this.shadowCamera.position.set(x, y, z);
    }

    public lookAt(x: number, y: number, z: number): void {
        this.light.lookAt(x, y, z);
        this.shadowCamera.lookAt(x, y, z);
    }

    public setSize(width: number, height: number): void {
        this.width = width;
        this.height = height;
        this.depthNormalRenderTarget.setSize(this.width, this.height)
        this.shadowRenderTarget.setSize(this.width, this.height);
    }

    public renderShadowMap(renderer: THREE.WebGLRenderer, scene: THREE.Scene): void {
        this.shadowCamera.updateMatrixWorld();
        this.shadowCamera.updateProjectionMatrix();
        this.renderPass.renderWithOverrideMaterial(renderer, scene, this.shadowCamera, this.depthMaterial, this.shadowDepthRenderTarget, 0x000000, undefined);
    }

    public renderShadow(renderer: THREE.WebGLRenderer, scene: THREE.Scene, camera: THREE.Camera): void {
        this.renderPass.renderWithOverrideMaterial(renderer, scene, camera, this.normalRenderMaterial, this.depthNormalRenderTarget, 0x7777ff, 1.0);
        this.shadowRenderMaterial.update({
            depthTexture: this.depthNormalRenderTarget.depthTexture,
            shadowTexture: this.shadowMapTexture,
            width: this.width,
            height: this.height, 
            camera, 
            shadowMapResolution: new THREE.Vector2(this.shadowMapSize, this.shadowMapSize),
            shadowCamera: this.shadowCamera,
            intensity: this.intensity
        });
        this.renderPass.renderScreenSpace(renderer, this.shadowRenderMaterial, this.shadowRenderTarget);
    }
}