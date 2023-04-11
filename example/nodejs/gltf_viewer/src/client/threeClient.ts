import { SceneProperties, RenderScene } from './scene/sceneManager';
import { QualityLevel} from './renderer/scene-renderer'
import { SceneRendererGUI } from './renderer/scene-renderer-gui'
import { MaterialGUI } from './scene/material-gui'
import { LightSourcesGUI } from './scene/lightSources'
import { SkyEnvironmentGUI } from './scene/skyEnvironment';
import { GroundMaterialType } from './scene/materials'
import { Group, Vector2, WebGLRenderer } from 'three'
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

const sceneCache: any = {}
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
    scene: Group;
}
const loadResource = (resourceName: string, resource: string) => {
    const loadedScene = sceneCache[resourceName];
    if (loadedScene) {
        setSatus(`render ${resourceName}`, '#ff0000')
        renderScene.update(undefined, loadedScene.scene);
        finishScene(resourceName);
    } else {
        renderScene.loadResource(resourceName, resource,
            (_:string) => {},
            (modelName: string, sceneGroup: Group | null) => {
                if (sceneGroup) {
                    sceneCache[resourceName] = { scene: sceneGroup };
                    finishScene(modelName);
                    addNewGlbToMenu(resourceName);
                } else {
                    setSatus(`load ${modelName}`, '#ff0000');
                }
            }
        );  
    }
}

const renderer = new WebGLRenderer({
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
renderScene.sceneRenderer.setQualityLevel(isMobile ? QualityLevel.LOW : QualityLevel.HIGHEST);
renderScene.createControls()
renderScene.setEnvironment()
renderScene.updateLightAndShadow()

interface GeneralProperties {
    sceneProperties: SceneProperties,
    glb: string,
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
    glb: '',
    groundMaterial: 'onlyshadow',
    bloom: false, 
    ssr: false, 
    sceneName: 'default',
}

const loadCachedMesh = async (configurationId: string) => {
    const statusLine = document.getElementById("status-line");
    let loadedScene = sceneCache[configurationId];
    if (!loadedScene) {
        return;
    }
    setSatus(`render ${configurationId}`, '#ff0000')
    renderScene.update(undefined, loadedScene.scene);
    finishScene(configurationId);
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
const glbMenuItems: any[] = [];
const glbMenu = gui.add<any>(generalProperties, 'glb', glbMenuItems).onChange((value) => { 
    if (value !== '') {
        loadCachedMesh(value);
    }
});
const addNewGlbToMenu = (resourceName: string) => {
    glbMenuItems.push(resourceName);
    let innerHTMLStr = "";
    glbMenuItems.forEach((name) => {
        innerHTMLStr += "<option value='" + name + "'>" + name + "</option>";
    });
    glbMenu.domElement.children[0].innerHTML = innerHTMLStr;
    glbMenu.setValue(resourceName);
    glbMenu.updateDisplay();
}
gui.add<any>(generalProperties.sceneProperties, 'rotate', 0, 0.25).onChange(() => renderScene.update(generalProperties.sceneProperties))
gui.add<any>(generalProperties, 'groundMaterial', {
    'only shadow': 'onlyshadow',
    'parquet': 'parquet', 
    'pavement': 'pavement'
}).onChange(() => setGroundMaterial())
const environmentFolder = gui.addFolder('environment');
environmentFolder.add<any>(renderScene, 'showEnvironment');
const skaEnvironmentFolder = environmentFolder.addFolder('sky');
const skyEnvironmentGUI = new SkyEnvironmentGUI(renderScene.skyEnvironment);
skyEnvironmentGUI.addGUI(skaEnvironmentFolder);
renderScene.environmentLoader.addGUI(environmentFolder);
const experimentalFolder = gui.addFolder('experimatal options');
experimentalFolder.add<any>(renderScene.sceneRenderer, 'groundReflection');
experimentalFolder.add<any>(generalProperties, 'bloom').onChange((enabled: boolean) => renderScene.getPostProcessingEffects().setBloom(enabled));
experimentalFolder.add<any>(generalProperties, 'ssr').onChange((enabled: boolean) => renderScene.getPostProcessingEffects().setSSR(enabled));
const sceneRendererGUI = new SceneRendererGUI(renderScene.sceneRenderer);
sceneRendererGUI.addGUI(gui, () => {});
const materialGUI = new MaterialGUI();
const lightFolder = gui.addFolder('Light')
const lightSourceGUI = new LightSourcesGUI(renderScene.getLightSources());
lightSourceGUI.addGUI(lightFolder, 
    () => (on: boolean) => renderScene.changeLightControls(on),
    () => renderScene.updateLightAndShadow());

const onWindowResize = () => {
    const width = window.innerWidth
    const height = window.innerHeight
    renderScene.resize(width, height)
}
window.addEventListener('resize', onWindowResize, false)
const mousePosition = new Vector2()
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
