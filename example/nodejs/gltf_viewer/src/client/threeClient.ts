import { SceneProperties, RenderScene } from './scene/sceneManager';
import { QualityLevel} from './renderer/scene-renderer'
import { SceneRendererGUI } from './renderer/scene-renderer-gui'
import { MaterialGUI } from './scene/material-gui'
import { LightSourcesGUI } from './scene/lightSources'
import { GroundMaterialType } from './scene/materials'
import * as THREE from 'three'
// @ts-ignore
import Stats from 'three/examples/jsm/libs/stats.module' 
import { GUI } from 'dat.gui'
// @ts-ignore
import { download_file } from './download_file.js'

const getDeviceType = () => {
    const ua = navigator.userAgent;
    if (/(tablet|ipad|playbook|silk)|(android(?!.*mobi))/i.test(ua)) {
        return "tablet";
    }
    else if (/Mobile|Android|iP(hone|od)|IEMobile|BlackBerry|Kindle|Silk-Accelerated|(hpw|web)OS|Opera M(obi|ini)/.test(ua)) {
        return "mobile";
    }
    return "desktop";
}
const deviceType = getDeviceType()
console.log(deviceType)
const isMobile = deviceType === 'mobile'

const queryString = window.location.search;
const urlParams = new URLSearchParams(queryString);
const parameterId = urlParams.get('id') 

const configurationCache: any = {}
const setSatus = (message: string, color: string = '#000000') => {
    console.log(`Status information: ${message}`);
    const statusLine = document.getElementById("status-line");
    if (!statusLine) {
        return
    }
    statusLine.innerText = message
    statusLine.style.setProperty('color', color)
}
const finishScene = (sceneName: string) => {
    let materialData = renderScene.collectMaterials();
    materialGUI.updateMaterialUI(gui, materialData);
    setSatus(`${sceneName}`);
    generalProperties.sceneName = sceneName.replace(/:/g, '_');
}
interface LoadedConfiguration {
    scene: THREE.Group;
}
const loadResource = (resourceName: string, resource: string) => {
    const loadedConfiguration: LoadedConfiguration = configurationCache[resourceName];
    if (loadedConfiguration) {
        setSatus(`render ${resourceName}`, '#ff0000')
        renderScene.update(undefined, loadedConfiguration.scene);
        finishScene(resourceName);
    } else {
        renderScene.loadResource(resourceName, resource,
            (_:string) => {},
            (modelName: string, loading: boolean) => {
                if (loading) {
                    setSatus(`load ${modelName}`, '#ff0000');
                } else {
                    finishScene(modelName);
                }
            }
        );  
    }
}

const renderer = new THREE.WebGLRenderer({
    // @ts-ignore
    canvas: three_canvas, 
    antialias: true, 
    alpha: true,
    preserveDrawingBuffer: true 
});
renderer.setPixelRatio(window.devicePixelRatio);
//renderer.physicallyCorrectLights = true
renderer.setSize(window.innerWidth, window.innerHeight)
document.body.appendChild(renderer.domElement)

// @ts-ignore
const stats = new Stats();
document.body.appendChild(stats.dom);

const renderScene = new RenderScene(renderer)
renderScene.sceneRenderer.setQualityLevel(isMobile ? QualityLevel.LOW : QualityLevel.HIGH);
renderScene.createControls()
renderScene.setEnvironment()
renderScene.updateLightAndShadow()

interface GeneralProperties {
    sceneProperties: SceneProperties,
    configurationId: string,
    groundMaterial: string,
    bloom: boolean,
    ssr: boolean,
    sceneName: string,
}

let generalProperties: GeneralProperties = {
    sceneProperties: {
        rotate: renderScene.properties.rotate,
        materialNoise: renderScene.properties.materialNoise,
    },
    configurationId: '',
    groundMaterial: 'onlyshadow',
    bloom: false, 
    ssr: false, 
    sceneName: 'default',
}

const setGroundMaterial = () => {
    let groundMaterial: GroundMaterialType = GroundMaterialType.Transparent
    switch(generalProperties.groundMaterial.toLocaleLowerCase()) {
        default: groundMaterial = GroundMaterialType.Transparent; break
        case 'parquet': groundMaterial = GroundMaterialType.Parquet; break
        case 'pavement': groundMaterial = GroundMaterialType.Pavement; break
    }
    renderScene.setGroundMaterial(groundMaterial)
}
setGroundMaterial()

const gui = new GUI()
gui.add<any>(generalProperties.sceneProperties, 'rotate', 0, 0.25).onChange(() => renderScene.update(generalProperties.sceneProperties))
gui.add<any>(generalProperties, 'groundMaterial', {
    'only shadow': 'onlyshadow',
    'parquet': 'parquet', 
    'pavement': 'pavement'
}).onChange(() => setGroundMaterial())
const experimentalFolder = gui.addFolder('experimatal options');
experimentalFolder.add<any>(renderScene.sceneRenderer, 'groundReflection');
experimentalFolder.add<any>(generalProperties, 'bloom').onChange((enabled: boolean) => renderScene.getPostProcessingEffects().setBloom(enabled));
experimentalFolder.add<any>(generalProperties, 'ssr').onChange((enabled: boolean) => renderScene.getPostProcessingEffects().setSSR(enabled));
const sceneRendererGUI = new SceneRendererGUI(renderScene.sceneRenderer);
sceneRendererGUI.addGUI(gui, () => {});
const materialGUI = new MaterialGUI();
const lightFolder = gui.addFolder('Light')
const lightSourceGUI = new LightSourcesGUI(renderScene.getLightSources());
lightSourceGUI.addGUI(lightFolder, () => (on: boolean) => renderScene.changeLightControls(on));

const onWindowResize = () => {
    const width = window.innerWidth
    const height = window.innerHeight
    renderScene.resize(width, height)
}
window.addEventListener('resize', onWindowResize, false)
const mousePosition = new THREE.Vector2()
const onPointerMove = (event: any) => {
    if (event.isPrimary === false ) {
        return
    }
    mousePosition.x = (event.clientX / window.innerWidth) * 2 - 1;
    mousePosition.y = - (event.clientY / window.innerHeight) * 2 + 1;
}
renderer.domElement.addEventListener('pointermove', onPointerMove);

const setupDragDrop = () => {
    const holder = document.getElementById('holder');
    if (!holder) {
        return;
    } 
 
    holder.ondragover = function() {
        // @ts-ignore
        this.className = 'hover';
        return false;
    };

    holder.ondragend = function() {
        // @ts-ignore
        this.className = '';
        return false;
    };

    holder.ondrop = function(e) {
        // @ts-ignore
        this.className = '';
        e.preventDefault();

        // @ts-ignore
        const file = e.dataTransfer.files[0];
        const reader = new FileReader();
        reader.onload = function(event) {
            // @ts-ignore
            loadResource(file.name, event.target.result);
        };
        reader.readAsDataURL(file);
    }
};
setupDragDrop();

const saveButton = document.getElementById('save-button');
if (saveButton) {
    saveButton.onclick = () => {
        try {
            const imgData = renderer.domElement.toDataURL();      
            download_file(imgData, generalProperties.sceneName + '.png', undefined)
        } 
        catch(e) {
            console.log(e);
        }
    };
    console
};

let start: number, previousTimeStamp: number;
const animate = (timestamp: number) => {
    if (start === undefined) {
        start = timestamp;
    }
    if (previousTimeStamp === undefined) {
        previousTimeStamp = timestamp;
    }
    const elapsedMilliseconds = timestamp - previousTimeStamp;
    previousTimeStamp = timestamp
    renderScene.animate(elapsedMilliseconds)
    //stats.begin();
    render()
    //stats.end();
    requestAnimationFrame(animate)
}

const render = () => {
    renderScene.prepareRender(mousePosition)
    renderScene.render()
    stats.update()
}

requestAnimationFrame(animate)
