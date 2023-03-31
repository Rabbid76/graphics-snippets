import { 
    createSceneGroup,
    GeometryAndMaterial,
    MaterialData
} from './meshConstructor'
import { LightSources } from './lightSources'
import { PostProcessingEffects } from './postProcessingEffects'
import {
    createGroundMaterial,
    createPreviewMaterial,
    GroundMaterialType
} from './materials'
import { EnvironmentLoader } from '../loader/environment_map/environmentLoader'
import { 
    createNoiseTexture,
    getMaxSamples,
    SceneVolume
} from '../renderer/render-utility'
import { DRACOLoader } from 'three/examples/jsm/loaders/DRACOLoader';
import { GLTFLoader, GLTF } from "three/examples/jsm/loaders/GLTFLoader";
import { Controls } from './controls'
import { QualityLevel, SceneRenderer, SceneRendererParameters, SceneGroupType } from '../renderer/scene-renderer'
import * as THREE from 'three'
import { TransformControls } from 'three/examples/jsm/controls/TransformControls.js';

export interface SceneProperties {
    rotate: number,
    materialNoise: boolean
}

export class RenderScene {
    public properties: SceneProperties = { 
        rotate: 0,
        materialNoise: false,        
    }
    private renderer: THREE.WebGLRenderer
    public sceneRenderer: SceneRenderer;
    private camera: THREE.PerspectiveCamera
    private controls?: Controls
    private scene: THREE.Scene
    private postProcessingEffects: PostProcessingEffects
    private lightSources: LightSources
    private sceneVolume = new SceneVolume()
    private turnTableGroup = new THREE.Group()
    public groundMaterialType =  GroundMaterialType.OnlyShadow
    private noiseTexture: THREE.Texture
    private raycaster = new THREE.Raycaster();
    private environmentLoader: EnvironmentLoader;
    private transformControls?: TransformControls

    public get sceneRenderParameters() : SceneRendererParameters {
        return this.sceneRenderer.parameters;
    }

    public constructor(renderer: THREE.WebGLRenderer) {
        const width = window.innerWidth;
        const height = window.innerHeight;
        const maxSamples = getMaxSamples(renderer)
        this.renderer = renderer;
        this.environmentLoader = new EnvironmentLoader(this.renderer);
        this.camera = new THREE.PerspectiveCamera(45, width / height, 0.1, 10);
        this.camera.position.z = 2;
        this.scene = new THREE.Scene();
        renderer.outputEncoding = THREE.sRGBEncoding;
        this.sceneRenderer = new SceneRenderer(this.renderer, width, height);
        this.postProcessingEffects = new PostProcessingEffects(this.renderer, this.sceneRenderer.effectComposer, this.scene, this.camera, width, height, {
            depthNormalRenderTarget: this.sceneRenderer.depthNormalRenderTarget
        });
        this.lightSources = new LightSources(this.scene, this.sceneRenderer, width, height, maxSamples);
        // The ground must be tessellated to avoid shadow glitches
        //const groundGeometry = new THREE.PlaneGeometry(100000, 100000, 100, 100)
        const groundGeometry = new THREE.RingGeometry(0, 50000, 36, 100)
        const groundMaterial = createGroundMaterial(this.groundMaterialType) 
        this.sceneRenderer.updateGround(groundGeometry, groundMaterial)
        this.sceneRenderer.addToScene(this.scene);
        this.turnTableGroup =  new THREE.Group()
        this.scene.add(this.turnTableGroup)
        this.sceneRenderer.sceneGroups.addToGroup(SceneGroupType.OBJECTS, this.turnTableGroup);
        this.noiseTexture = createNoiseTexture(512, 0.9, 1)
        this.noiseTexture.anisotropy = 16;
        this.noiseTexture.wrapS = THREE.RepeatWrapping;
        this.noiseTexture.wrapT = THREE.RepeatWrapping;
        this.update(undefined, this.constructLoadingGeometry(1))
        this.sceneRenderer.updateParameters({
          outlineParameters: {
            enabled: false,
            edgeStrength: 5.0,
            edgeGlow: 1.0,
            edgeThickness: 1.0,
            //visibleEdgeColor: 0xff0000,
            //hiddenEdgeColor: 0xff0000,
          },
        });
        this.sceneRenderer.setQualityLevel(QualityLevel.HIGH);
    }

    public getLightSources(): LightSources {
        return this.lightSources
    }

    public getPostProcessingEffects(): PostProcessingEffects {
        return this.postProcessingEffects
    }

    public async loadResource(resourceName: string, resource: string, newEnvironment: (name: string) => void, newModel: (name: string, loading: boolean) => void) {
        try {
            const lowerName = resourceName.toLowerCase();
            if (lowerName.endsWith('.exr') ) {
                this.environmentLoader.loadExr(resource, true);
            } else if (lowerName.endsWith('.hdr') ) {
                this.environmentLoader.loadHdr(resource, true);
            } else if (lowerName.endsWith('.envmap') ) {
                this.environmentLoader.loadEnvmap(resource, true);
            } else if (lowerName.endsWith('.glb') || lowerName.endsWith('.gltf') ) {
                newModel(resourceName, true);
                await this.loadGLTF(resource);
                newModel(resourceName, false);
            }
        } catch (e) {
            console.log(e);
        } 
    }

    public createControls(): void {
        this.controls ??= new Controls(this.renderer, this.camera);
    }

    public changeLightControls(on: boolean) {
        if (on && !this.transformControls) {
            this.transformControls = this.controls?.addTransformControl(this.lightSources.getLightSources()[0], this.scene);
        }
        if (this.transformControls) {
            this.transformControls.visible = on;
        }
    }

    public setEnvironment(): void {
        //this.environmentLoader.loadDefaultEnvironment(false);
        this.environmentLoader.loadEnvmap('roomle64.envmap', false);
    }

    public updateLightAndShadow(): void {
        this.lightSources.getShadowLightSources().forEach(light => {
            light.castShadow = true;
            light.shadow.mapSize.width = 1024;
            light.shadow.mapSize.height = 1024;
            light.shadow.blurSamples = 32
        });
        this.lightSources.updateBounds(this.sceneVolume)
    }

    public setGroundMaterial(groundMaterialType: GroundMaterialType) {
        if (this.groundMaterialType !== groundMaterialType) {
            this.groundMaterialType = groundMaterialType
            this.sceneRenderer.updateGround(undefined, createGroundMaterial(this.groundMaterialType))
        }
    }

    public update(properties?: SceneProperties, meshGroup?: THREE.Group): void {
        if (meshGroup) {
            this.setNewTurntableGroup(meshGroup);
        }
        if (properties && this.properties.rotate !== properties?.rotate) {
            this.properties.rotate = properties?.rotate
            if (this.properties.rotate === 0) { 
                this.turnTableGroup.rotation.y = 0
            }
        }
    }

    public collectMaterials() {
        const materials: MaterialData[] = []
        this.scene.traverse((node) => {
            if (node instanceof THREE.Mesh && node.name.length > 0) {
                const material = node.material as THREE.MeshPhysicalMaterial
                if (material) {
                    materials.push({materialId: node.name, material})
                }
            } else if (node instanceof THREE.Mesh && node.material.name.length > 0) {
                const material = node.material as THREE.MeshPhysicalMaterial
                if (material) { 
                    const name = node.material.name;
                    materials.findIndex(materialData => materialData.materialId === name) === -1 && materials.push({materialId: name, material});
                }
            }
        })
        return materials;
    }

    public updateMaterialProperties(properties: SceneProperties, materials: MaterialData[]): void {
        if (this.properties.materialNoise !== properties.materialNoise) {
            this.properties.materialNoise = properties.materialNoise
            materials.forEach(materialData => {
                const material = materialData.material as THREE.MeshPhysicalMaterial;
                if (!material.userData || !('ormMap' in material.userData)) {
                    material.roughnessMap = this.properties.materialNoise ? this.noiseTexture : null
                    material.metalnessMap = this.properties.materialNoise ? this.noiseTexture : null
                    //material.aoMap = this.properties.materialNoise ? this.noiseTexture : null
                    material.needsUpdate = true
                }
            })
        }
    }

    public resize(width: number, height: number) {
        this.camera.aspect = width / height;
        this.camera.updateProjectionMatrix();
        this.postProcessingEffects.resize(width, height);
        this.renderer.setSize(width, height);
        this.sceneRenderer.setSize(width, height);
    }

    public animate(elapsedMilliseconds: number): void {
        if (this.properties.rotate > 0) {
            this.turnTableGroup.rotation.y += Math.PI * 2 * elapsedMilliseconds * 0.001 * this.properties.rotate
        }
    }

    public prepareRender(mousePosition: THREE.Vector2): void {
        // bring the near and far plane as close as possible to geometry
        // this is very likely the most important part for a glitch free and nice SSAO
        const nearFar = this.sceneVolume.getNearAndFarForCameraThatLooksAtOriginOfScene(this.camera.position, 3)
        this.camera.near = 0.1; // nearFar[0]
        this.camera.far = nearFar[1]; // Math.max(nearFar[1], 100)
        this.camera.updateProjectionMatrix()

        const effects = this.getPostProcessingEffects()

        this.environmentLoader.setEnvironment(this.scene);
        if (effects.bloomPass.enabled) {
            this.scene.background = new THREE.Color(0x404040);
        }

        if (this.sceneRenderer.outlineRenderer.parameters.enabled) {
            this.raycaster.setFromCamera(mousePosition, this.camera);
            const intersects = this.raycaster.intersectObject(this.turnTableGroup, true);
            let selectedObject = intersects.length > 0 ? intersects[0].object : undefined
            this.sceneRenderer.selectObjects(selectedObject ? [selectedObject as THREE.Object3D] : []);
        }
    }

    public render(): void {
        this.controls?.update()
        this.sceneRenderer.render(this.scene, this.camera, this.postProcessingEffects.anyPostProcess());
    }

    private async loadGLTF(resourceName: string) {
        const gltfLoader = new GLTFLoader();
        const dracoLoader = new DRACOLoader();
        dracoLoader.setDecoderPath('three/examples/jsm/libs/draco/'); // TODO

        // Optional: Provide a DRACOLoader instance to decode compressed mesh data
        // const dracoLoader = new DRACOLoader();
        // dracoLoader.setDecoderPath( '/examples/jsm/libs/draco/' );
        // loader.setDRACOLoader( dracoLoader );

        const gltf = await gltfLoader.loadAsync(resourceName);
        this.updateGLTFScene(gltf, (mesh: THREE.Mesh) => {
            /*
            if (mesh.name.includes("glass") || mesh.name.includes("bottle")) {
                const glassMaterial = new THREE.MeshPhysicalMaterial({
                    color: 0xffffff,
                    metalness: 0,
                    roughness: 0.1,
                    transparent: true,
                    transmission: 0.9,
                    opacity: 0.7,
                    envMapIntensity: 1,
                    side: THREE.DoubleSide,
                });
                mesh.material = glassMaterial;
            }
            */
        });
        this.setNewTurntableGroup(gltf.scene);
    }

    private updateGLTFScene(gltf: GLTF, updateMesh: (mesh: THREE.Mesh) => void): void {
        gltf.scene.traverse((child) => {
            if (child instanceof THREE.Mesh) {
                updateMesh(child);
                if (child.material instanceof THREE.MeshStandardMaterial) {
                    child.material.envMapIntensity = 1;
                    child.material.needsUpdate = true;
                }
            }
        });
    }

    private setNewTurntableGroup(newGroup: THREE.Group): void {
        this.turnTableGroup.clear();
        this.setInitialObjectPosition(newGroup);
        this.setInitialCameraPositionAndRotation();
        this.sceneRenderer.bakedGroundContactShadow.needsUpdate = true;
        this.updateLightAndShadow();
    }

    private setInitialObjectPosition(meshGroup: THREE.Object3D): void {
        const bounds = new THREE.Box3();
        bounds.setFromObject(meshGroup);
        console.log('bounds', bounds)
        const size = bounds.getSize(new THREE.Vector3)
        const center = bounds.getCenter(new THREE.Vector3)
        meshGroup.translateX(-center.x)
        meshGroup.translateY(-bounds.min.y)
        meshGroup.translateZ(-center.z)
        const maxSize = Math.max(size.x, size.y, size.z)
        const offset_y = -size.y / 2
        this.turnTableGroup.position.y = offset_y
        this.turnTableGroup.add(meshGroup)
        this.sceneRenderer.setGroundLevel(offset_y);
        this.sceneVolume.update(meshGroup)
    }

    private setInitialCameraPositionAndRotation(): void {
        this.camera.position.z = this.sceneVolume.maxSceneDistanceFrom0 * 2.5
        this.camera.position.y = this.sceneVolume.maxSceneDistanceFrom0 * 0.8
        this.camera.position.x = this.sceneVolume.maxSceneDistanceFrom0 * -1.5
        this.camera.lookAt(new THREE.Vector3(0, 0, 0))
        this.camera.updateMatrix()
    }

    private constructLoadingGeometry(type: number): THREE.Group {
        let loadingGeometry: GeometryAndMaterial[] = []
        if (type == 1) {
            const points = []
            for (let a = 0; a <= 230; a += 15) {
                const ar = a * Math.PI / 180
                points.push({ 'x': -Math.cos(ar), 'y': Math.sin(ar) + 1 })
            }
            points.push({ 'x': 0, 'y': 0 })
            for (let a = 60; a >= -180; a -= 15) {
                const ar = a * Math.PI / 180
                points.push({ 'x': -Math.cos(ar), 'y': Math.sin(ar) - 1 })
            }
            const radius = 0.6
            const gap = 2.5 * radius
            const path = new THREE.CatmullRomCurve3(points.map(p => new THREE.Vector3(p.x, p.y + 2 + gap, 0)))
            const tube = new THREE.TubeGeometry(path, 64, radius, 16, false);
            const sphereS = new THREE.SphereGeometry(radius, 32, 16)
            sphereS.translate(-1, 3 + gap, 0)
            const sphereE = new THREE.SphereGeometry(radius, 32, 16)
            sphereE.translate(1, 1 + gap, 0)
            const sphere = new THREE.SphereGeometry(radius, 32, 16)
            const material = new THREE.MeshPhysicalMaterial({color: 0xdb0000, side: THREE.DoubleSide})
            loadingGeometry = [
                { geometry: tube, material: material, materialId: '', environment: false }, 
                { geometry: sphereS, material: material, materialId: '', environment: false }, 
                { geometry: sphereE, material: material, materialId: '', environment: false }, 
                { geometry: sphere, material: material, materialId: '', environment: false }
            ]
        } else {
            const material = createPreviewMaterial()
            loadingGeometry = [{ geometry: new THREE.BoxGeometry(), material: material, materialId: '', environment: false }]
        }
        return createSceneGroup(loadingGeometry)
    }
}