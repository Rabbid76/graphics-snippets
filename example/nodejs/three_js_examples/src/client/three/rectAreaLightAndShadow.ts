import { ShadowMaterial } from '../three/shadowMaterialAndShader'
import { RectAreaLightHelper } from 'three/examples/jsm/helpers/RectAreaLightHelper.js';
import { CopyMaterial } from '../three/shaderUtility'
import {
    CameraUpdate,
    RenderPass,
    setOrthographicViewVolumeFromBox
} from '../three/threeUtility'
import * as THREE from 'three'

export interface RectAreaLightAndShadow {
    get visible(): boolean;
    set visible(isVisible: boolean);
    get shadowIntensity(): number;
    set shadowIntensity(intensity: number);
    get rectAreaLight(): THREE.RectAreaLight;
    get shadowCamera(): THREE.Camera;
    get rectLightHelper(): RectAreaLightHelper | undefined;
    get shadowMapTexture(): THREE.Texture | undefined;
    get depthTexture(): THREE.Texture | undefined;
    get shadowTexture(): THREE.Texture;
    dispose(): void;
    addToScene(scene: THREE.Scene): void;
    removeFromScene(scene: THREE.Scene): void;
    setSize(width: number, height: number): void;
    setLineOfSight(cameraPosition: THREE.Vector3, targetPosition: THREE.Vector3): void;
    setShadowVolume(shadowVolumeBox: THREE.Box3): void;
    renderShadow(renderer: THREE.WebGLRenderer, scene: THREE.Scene, camera: THREE.Camera): void;
    blendShadow(renderer: THREE.WebGLRenderer, overwriteTarget: boolean): void;
}

export class RectAreaLightAndShadowWithDirectionalLight implements RectAreaLightAndShadow {
    public needsUpdate: boolean = false;
    private _shadowIntensity: number;
    private _rectAreaLight: THREE.RectAreaLight;
    private _directionalLight: THREE.DirectionalLight;
    private _rectLightHelper?: RectAreaLightHelper;
    private viewportSize: THREE.Vector2;
    private samples: number;
    private shadowMapSize;
    private alwaysUpdate: boolean;
    private renderPass: RenderPass;
    private shadowMaterial: THREE.ShadowMaterial;
    private shadowRenderTarget: THREE.WebGLRenderTarget;
    private copyMaterial: CopyMaterial;
    private cameraUpdate: CameraUpdate = new CameraUpdate();
    private shadowCameraUpdate: CameraUpdate = new CameraUpdate();

    public get visible(): boolean { return this._rectAreaLight.visible; }
    public set visible(isVisible: boolean) { this._rectAreaLight.visible = isVisible; }
    public get shadowIntensity(): number { return this._shadowIntensity; } 
    public set shadowIntensity(intensity: number) { this._shadowIntensity = intensity; } 
    public get rectAreaLight(): THREE.RectAreaLight { return this._rectAreaLight; } 
    public get shadowCamera(): THREE.Camera { return this._directionalLight.shadow.camera; } 
    public get rectLightHelper(): RectAreaLightHelper | undefined { return this._rectLightHelper; } 
    public get shadowMapTexture(): THREE.Texture | undefined { return this._directionalLight.shadow.map?.texture; } 
    public get depthTexture(): THREE.Texture | undefined { return undefined; } 
    public get shadowTexture(): THREE.Texture { return this.shadowRenderTarget.texture; } 

    constructor(rectAreaLight: THREE.RectAreaLight, viewportSize: THREE.Vector2, parameter: any) {
        this.viewportSize = new THREE.Vector2(viewportSize.x, viewportSize.y);
        this._shadowIntensity = parameter?.shadowIntensity ?? 0.5
        this.samples = parameter?.samples ?? 0;
        this.shadowMapSize = parameter?.shadowMapSize ?? 1024;
        this.alwaysUpdate = parameter?.alwaysUpdate ?? false;
        this._rectAreaLight = rectAreaLight;
        if (parameter?.addHelper) {
            this._rectLightHelper = new RectAreaLightHelper(this._rectAreaLight);
            // @ts-ignore
            this.rectLightHelper.material.depthWrite = false;
            this._rectAreaLight.add(this._rectLightHelper);
        }
        this._directionalLight = new THREE.DirectionalLight(0xffffff, 5.0);
        this._directionalLight.position.set(this._rectAreaLight.position.x, this._rectAreaLight.position.y, this._rectAreaLight.position.z);
        this._directionalLight.lookAt(0, 0, 0);
        this._directionalLight.visible = false;
        this._directionalLight.castShadow = true;
        this._directionalLight.shadow.mapSize = new THREE.Vector2(this.shadowMapSize, this.shadowMapSize);
        this.renderPass = new RenderPass();
        //this.shadowMaterial = new THREE.MeshStandardMaterial({color: 0xffffff, envMapIntensity: 0});
        this.shadowMaterial = new THREE.MeshPhongMaterial({color: 0xffffff});
        this.shadowRenderTarget = new THREE.WebGLRenderTarget(this.viewportSize.x, this.viewportSize.y, { samples: this.samples });
        this.copyMaterial = new CopyMaterial({
            blendSrc: THREE.ZeroFactor,
            blendDst: THREE.SrcColorFactor,
            blendEquation: THREE.AddEquation,
            blendSrcAlpha: THREE.ZeroFactor,
            blendDstAlpha: THREE.OneFactor,
            blendEquationAlpha: THREE.AddEquation,
        });
    }

    public dispose(): void {
        this._rectAreaLight.dispose();
        this._directionalLight.dispose();
        this.shadowMaterial.dispose();
        this.shadowRenderTarget.dispose();
    }

    addToScene(scene: THREE.Scene): void {
        scene.add(this.rectAreaLight);  
        scene.add(this._directionalLight);   
    }

    removeFromScene(scene: THREE.Scene): void{
        scene.remove(this.rectAreaLight);  
        scene.remove(this._directionalLight);   
    }

    public setSize(width: number, height: number): void {
        this.viewportSize = new THREE.Vector2(width, height);
        this.shadowRenderTarget.setSize(this.viewportSize.x, this.viewportSize.y);
    }

    public setLineOfSight(cameraPosition: THREE.Vector3, targetPosition: THREE.Vector3): void {
        this._rectAreaLight.position.set(cameraPosition.x, cameraPosition.y, cameraPosition.z);
        this._directionalLight.position.set(cameraPosition.x, cameraPosition.y, cameraPosition.z);
        this._directionalLight.shadow.camera.position.set(cameraPosition.x, cameraPosition.y, cameraPosition.z);
        this._rectAreaLight.lookAt(targetPosition.x, targetPosition.y, targetPosition.z);
        this._directionalLight.lookAt(targetPosition.x, targetPosition.y, targetPosition.z);
        this._directionalLight.shadow.camera.lookAt(targetPosition.x, targetPosition.y, targetPosition.z);
        this._directionalLight.updateMatrixWorld();
        this._directionalLight.shadow.camera.updateMatrixWorld();
    }

    public setShadowVolume(shadowVolumeBox: THREE.Box3): void {
        setOrthographicViewVolumeFromBox(this._directionalLight.shadow.camera, shadowVolumeBox);
    }

    public renderShadow(renderer: THREE.WebGLRenderer, scene: THREE.Scene, camera: THREE.Camera): void {
        const needsUpdate = this.needsUpdate || this.alwaysUpdate || this.cameraUpdate.changed(camera) || this.shadowCameraUpdate.changed(this.shadowCamera);
        if (!needsUpdate) {
            return;
        }
        this.needsUpdate = false;
        const isVisible = this.visible;
        this._directionalLight.visible = true;
        this._rectAreaLight.visible = false;
        this.renderPass.renderWithOverrideMaterial(renderer, scene, camera, this.shadowMaterial, this.shadowRenderTarget, 0xffffff, 1);
        this._directionalLight.visible = false;
        this._rectAreaLight.visible = isVisible;
    }

    public blendShadow(renderer: THREE.WebGLRenderer, overwriteTarget: boolean): void {
        this.renderPass.renderScreenSpace(renderer, this.copyMaterial.update({
            texture: this.shadowTexture, 
            blending: overwriteTarget ? THREE.NoBlending : THREE.CustomBlending,
            opacity: overwriteTarget ? 1 : this._shadowIntensity,
            brightness: overwriteTarget ? 0 : (1 -this._shadowIntensity),
        }), null);
    }
}


export class RectAreaLightAndShadowWithShadowMap implements RectAreaLightAndShadow {
    private _shadowIntensity: number = 0.5;
    private _rectAreaLight: THREE.RectAreaLight;
    private _shadowCamera: THREE.OrthographicCamera;
    private _rectLightHelper?: RectAreaLightHelper;
    private viewportSize: THREE.Vector2;
    private shadowMapSize = 1024;
    private renderPass: RenderPass;
    private shadowDepthRenderTarget: THREE.WebGLRenderTarget;
    private depthMaterial: THREE.MeshDepthMaterial;
    private normalRenderMaterial: THREE.MeshNormalMaterial;
    private depthNormalRenderTarget: THREE.WebGLRenderTarget;
    private shadowRenderMaterial: ShadowMaterial;
    private shadowRenderTarget: THREE.WebGLRenderTarget;
    private copyMaterial: CopyMaterial;
    
    public get visible(): boolean { return this._rectAreaLight.visible; }
    public set visible(isVisible: boolean) { this._rectAreaLight.visible = isVisible; }
    public get shadowIntensity(): number { return this._shadowIntensity; } 
    public set shadowIntensity(intensity: number) { this._shadowIntensity = intensity; } 
    public get rectAreaLight(): THREE.RectAreaLight { return this._rectAreaLight; } 
    public get shadowCamera(): THREE.Camera { return this._shadowCamera; } 
    public get rectLightHelper(): RectAreaLightHelper | undefined { return this._rectLightHelper; } 
    public get shadowMapTexture(): THREE.Texture | undefined { return this.shadowDepthRenderTarget.texture; } 
    public get depthTexture(): THREE.Texture | undefined { return this.depthNormalRenderTarget.depthTexture; } 
    public get shadowTexture(): THREE.Texture { return this.shadowRenderTarget.texture; } 
    
    constructor(light: THREE.RectAreaLight, viewportSize: THREE.Vector2, shadowCamera?: THREE.OrthographicCamera, addHelper?: boolean) {
        this.viewportSize = new THREE.Vector2(viewportSize.x, viewportSize.y);
        this._rectAreaLight = light;
        this._shadowCamera = shadowCamera ?? new THREE.OrthographicCamera;
        if (addHelper) {
            this._rectLightHelper = new RectAreaLightHelper(this._rectAreaLight);
            // @ts-ignore
            this.rectLightHelper.material.depthWrite = false;
            this._rectAreaLight.add(this._rectLightHelper);
        }
        this.renderPass = new RenderPass();

        const shadowDepthTexture = new THREE.DepthTexture(this.shadowMapSize, this.shadowMapSize);
        shadowDepthTexture.format = THREE.DepthStencilFormat;
        shadowDepthTexture.type = THREE.UnsignedInt248Type;
        this.shadowDepthRenderTarget = new THREE.WebGLRenderTarget(this.shadowMapSize, this.shadowMapSize, {
            minFilter: THREE.NearestFilter,
            magFilter: THREE.NearestFilter,
            depthTexture: shadowDepthTexture
        });
        this.depthMaterial = new THREE.MeshDepthMaterial();

        const depthTexture = new THREE.DepthTexture(this.viewportSize.x, this.viewportSize.y);
        depthTexture.format = THREE.DepthStencilFormat;
        depthTexture.type = THREE.UnsignedInt248Type;
        this.depthNormalRenderTarget = new THREE.WebGLRenderTarget(this.viewportSize.x, this.viewportSize.y, {
            minFilter: THREE.NearestFilter,
            magFilter: THREE.NearestFilter,
            depthTexture
        });
        this.normalRenderMaterial = new THREE.MeshNormalMaterial();
        this.normalRenderMaterial.blending = THREE.NoBlending;

        this.shadowRenderMaterial = new ShadowMaterial();
        this.shadowRenderTarget = new THREE.WebGLRenderTarget(this.viewportSize.x, this.viewportSize.y);
        this.copyMaterial = new CopyMaterial();
    }

    public dispose(): void {
        this.shadowDepthRenderTarget.dispose();
        this.depthMaterial.dispose();
        this.normalRenderMaterial.dispose();
        this.depthNormalRenderTarget.dispose();
        this.shadowRenderMaterial.dispose();
    }

    addToScene(scene: THREE.Scene): void {
        scene.add(this.rectAreaLight);  
        scene.add(this.shadowCamera);   
    }

    removeFromScene(scene: THREE.Scene): void{
        scene.remove(this.rectAreaLight);  
        scene.remove(this.shadowCamera);   
    }

    public setSize(width: number, height: number): void {
        this.viewportSize = new THREE.Vector2(width, height);
        this.depthNormalRenderTarget.setSize(this.viewportSize.x, this.viewportSize.y)
        this.shadowRenderTarget.setSize(this.viewportSize.x, this.viewportSize.y);
    }

    public setLineOfSight(cameraPosition: THREE.Vector3, targetPosition: THREE.Vector3): void {
        this._rectAreaLight.position.set(cameraPosition.x, cameraPosition.y, cameraPosition.z);
        this._shadowCamera.position.set(cameraPosition.x, cameraPosition.y, cameraPosition.z);
        this._rectAreaLight.lookAt(targetPosition.x, targetPosition.y, targetPosition.z);
        this._shadowCamera.lookAt(targetPosition.x, targetPosition.y, targetPosition.z);
        this._shadowCamera.updateMatrixWorld();
    }

    public setShadowVolume(shadowVolumeBox: THREE.Box3): void {
        setOrthographicViewVolumeFromBox(this._shadowCamera, shadowVolumeBox);
    }

    public renderShadow(renderer: THREE.WebGLRenderer, scene: THREE.Scene, camera: THREE.Camera): void {
        this.renderShadowMapTexture(renderer, scene);
        this.renderShadowTexture(renderer, scene, camera);
    }

    public renderShadowMapTexture(renderer: THREE.WebGLRenderer, scene: THREE.Scene): void {
        this._shadowCamera.updateMatrixWorld();
        this._shadowCamera.updateProjectionMatrix();
        this.renderPass.renderWithOverrideMaterial(renderer, scene, this._shadowCamera, this.depthMaterial, this.shadowDepthRenderTarget, 0x000000, undefined);
    }

    public blendShadow(renderer: THREE.WebGLRenderer, overwriteTarget: boolean): void {
        const blendMode = overwriteTarget ? THREE.NoBlending : THREE.CustomBlending;
        this.renderPass.renderScreenSpace(renderer, this.copyMaterial.update({texture: this.shadowTexture, blending: blendMode}), null);
    }

    public renderShadowTexture(renderer: THREE.WebGLRenderer, scene: THREE.Scene, camera: THREE.Camera): void {
        this.renderPass.renderWithOverrideMaterial(renderer, scene, camera, this.normalRenderMaterial, this.depthNormalRenderTarget, 0x7777ff, 1.0);
        this.shadowRenderMaterial.update({
            depthTexture: this.depthNormalRenderTarget.depthTexture,
            shadowTexture: this.shadowDepthRenderTarget.depthTexture ?? this.shadowDepthRenderTarget.texture,
            width: this.viewportSize.x,
            height: this.viewportSize.y, 
            camera, 
            shadowMapResolution: new THREE.Vector2(this.shadowMapSize, this.shadowMapSize),
            shadowCamera: this.shadowCamera,
            intensity: this._shadowIntensity
        });
        this.renderPass.renderScreenSpace(renderer, this.shadowRenderMaterial, this.shadowRenderTarget);
    }
}