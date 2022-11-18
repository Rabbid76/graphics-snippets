import { CameraUpdate } from './threeUtility';
import * as THREE from 'three';
import { HorizontalBlurShader } from 'three/examples/jsm/shaders/HorizontalBlurShader.js';
import { VerticalBlurShader } from 'three/examples/jsm/shaders/VerticalBlurShader.js';

export interface GroundContactShadowParameters {
    cameraHelper: boolean;
    alwaysUpdate: boolean;
    blur: number;
    darkness: number;
    opacity: number;
    planeSize: number;
    cameraFar: number;
}

export class GroundAndGroundShadow {
    public shadowMapSize: number;
    public parameters: GroundContactShadowParameters;
    public renderer: THREE.WebGLRenderer;
    public group: THREE.Group;
    public cameraHelper: THREE.CameraHelper;
    public needsUpdate: boolean = true;
    private groundMesh?: THREE.Mesh;
    private shadowGround: THREE.Mesh;
    private blurPlane: THREE.Mesh;
    private renderTarget: THREE.WebGLRenderTarget;
    private renderTargetBlur: THREE.WebGLRenderTarget;
    private depthMaterial: THREE.MeshDepthMaterial;
    private horizontalBlurMaterial: THREE.ShaderMaterial;
    private verticalBlurMaterial: THREE.ShaderMaterial;
    private shadowCamera: THREE.OrthographicCamera;
    private cameraUpdate: CameraUpdate = new CameraUpdate();

    constructor(renderer: THREE.WebGLRenderer, parameters: any) {
        this.shadowMapSize = parameters.shadowMapSize ?? 1024
        this.parameters = {
            cameraHelper: parameters.cameraHelper ?? false,
            alwaysUpdate: parameters.alwaysUpdate ?? false,
            blur: parameters.blur ?? 1.5,
            darkness: parameters.darkness ?? 1,
            opacity: parameters.opacity ?? 0.5,
            planeSize: parameters.planeSize ?? 10,
            cameraFar: parameters.cameraFar ?? 5
        };

        this.renderer = renderer;
        this.group = new THREE.Group();
        //this.group.rotateX(-Math.PI/2);

        this.renderTarget = new THREE.WebGLRenderTarget(this.shadowMapSize, this.shadowMapSize);
        this.renderTarget.texture.generateMipmaps = false;
        this.renderTargetBlur = new THREE.WebGLRenderTarget(this.shadowMapSize, this.shadowMapSize);
        this.renderTargetBlur.texture.generateMipmaps = false;

        const shadowGroundGeometry = new THREE.PlaneGeometry(1, 1, 10, 10);
        const shadowGroundMaterial = new THREE.MeshBasicMaterial( {
            map: this.renderTarget.texture,
            opacity: this.parameters.opacity,
            transparent: true,
            depthWrite: false,
            //side: THREE.DoubleSide
        } );
        shadowGroundMaterial.polygonOffset = true;
        shadowGroundMaterial.polygonOffsetFactor = 2;
        shadowGroundMaterial.polygonOffsetUnits = 2;
        this.shadowGround = new THREE.Mesh(shadowGroundGeometry, shadowGroundMaterial);
        this.shadowGround.renderOrder = 1;
        this.shadowGround.receiveShadow = true;
        this.group.add(this.shadowGround);

        this.blurPlane = new THREE.Mesh(shadowGroundGeometry);
        this.blurPlane.visible = false;
        this.group.add(this.blurPlane);

        this.shadowCamera = new THREE.OrthographicCamera();
        this.shadowCamera.rotation.x = Math.PI; // make camera look upwards
        this.group.add(this.shadowCamera);

        this.cameraHelper = new THREE.CameraHelper(this.shadowCamera);
        this.cameraHelper.visible = this.parameters.cameraHelper;

        this.depthMaterial = new THREE.MeshDepthMaterial();
        this.depthMaterial.userData.darkness = { value: this.parameters.darkness };
        this.depthMaterial.onBeforeCompile = (shader) => {
            shader.uniforms.darkness = this.depthMaterial.userData.darkness;
            shader.fragmentShader = `
                uniform float darkness;
                ${shader.fragmentShader.replace('gl_FragColor = vec4( vec3( 1.0 - fragCoordZ ), opacity );', 'gl_FragColor = vec4( vec3( 0.0 ), ( 1.0 - fragCoordZ ) * darkness );')}
            `;
        };
        this.depthMaterial.depthTest = false;
        this.depthMaterial.depthWrite = false;
        this.horizontalBlurMaterial = new THREE.ShaderMaterial(HorizontalBlurShader);
        this.horizontalBlurMaterial.depthTest = false;
        this.verticalBlurMaterial = new THREE.ShaderMaterial(VerticalBlurShader);
        this.verticalBlurMaterial.depthTest = false;

        this.updatePlaneAndShadowCamera();
    }

    public addToScene(scene: THREE.Scene) {
        scene.add(this.group);
        scene.add(this.cameraHelper);
    }

    public removeFromScene(scene: THREE.Scene) {
        scene.remove(this.group);
        scene.remove(this.cameraHelper);
    }

    public updateParameters() {
        this.depthMaterial.userData.darkness.value = this.parameters.darkness;
        const shadowGroundMaterial = this.shadowGround.material as THREE.MeshBasicMaterial;
        shadowGroundMaterial.opacity = this.parameters.opacity;
        this.cameraHelper.visible = this.parameters.cameraHelper;
        this.needsUpdate = true;
    }

    public updateBounds(bounds: THREE.Box3) {
        const boundsCenter = bounds.getCenter(new THREE.Vector3());
        this.group.position.set(boundsCenter.x, bounds.min.y, boundsCenter.z);
        const boundsSize = bounds.getSize(new THREE.Vector3());
        const maxPlanSideLength = Math.max(boundsSize.x, boundsSize.y);
        this.parameters.planeSize = maxPlanSideLength + 2;
        this.updatePlaneAndShadowCamera();
    }

    public updatePlaneAndShadowCamera() {
        const size = this.parameters.planeSize;
        this.shadowGround.scale.x = size;
        this.shadowGround.scale.y = size;
        this.blurPlane.scale.x = size;
        this.blurPlane.scale.y = size;
        this.shadowCamera.left = -size / 2;
        this.shadowCamera.right = size / 2;
        this.shadowCamera.top = -size / 2;
        this.shadowCamera.bottom = size / 2;
        this.shadowCamera.near = 0;
        this.shadowCamera.far = this.parameters.cameraFar;
        this.shadowCamera.updateProjectionMatrix();
        this.cameraHelper.update();
        this.needsUpdate = true;
    }

    public updateGround(geometry?: THREE.BufferGeometry, material?: THREE.Material) {
        if (!this.groundMesh) {
            this.groundMesh = new THREE.Mesh();
            this.group.add(this.groundMesh);
        }
        if (geometry) {
            this.groundMesh.geometry = geometry;
        }
        if (material) {
            material.polygonOffset = true;
            material.polygonOffsetFactor = 4;
            material.polygonOffsetUnits = 4;
            material.needsUpdate = true;
            this.groundMesh.material = material;
        }
    }

    public render(scene: THREE.Scene, camera: THREE.Camera): void {
        const needsUpdate = this.parameters.alwaysUpdate || this.needsUpdate || this.cameraUpdate.changed(camera);
        if (!needsUpdate) {
            return;
        }
        this.needsUpdate = false;

        const initialBackground = scene.background;
        scene.background = null;
        scene.overrideMaterial = this.depthMaterial;
        const initialClearAlpha = this.renderer.getClearAlpha();
        this.renderer.setClearAlpha(0);
        if (this.groundMesh) {
            this.groundMesh.visible = false;
        }
        this.shadowGround.visible = false;
        this.cameraHelper.visible = false;

        this.renderer.setRenderTarget(this.renderTarget);
        this.renderer.render(scene, this.shadowCamera);
        scene.overrideMaterial = null;
        this.blurShadow(this.parameters.blur / this.parameters.planeSize);
        this.blurShadow(this.parameters.blur * 0.4 / this.parameters.planeSize);

        this.renderer.setRenderTarget(null);
        this.renderer.setClearAlpha(initialClearAlpha);
        scene.background = initialBackground;
        if (this.groundMesh) {
            this.groundMesh.visible = true;
        }
        this.shadowGround.visible = true;
        this.cameraHelper.visible = this.parameters.cameraHelper;
    }

    public blurShadow(amount: number): void {
        this.blurPlane.visible = true;
        this.blurPlane.material = this.horizontalBlurMaterial;
        // @ts-ignore
        this.blurPlane.material.uniforms.tDiffuse.value = this.renderTarget.texture;
        this.horizontalBlurMaterial.uniforms.h.value = amount * 1 / 256;

        this.renderer.setRenderTarget(this.renderTargetBlur);
        this.renderer.render(this.blurPlane, this.shadowCamera);

        this.blurPlane.material = this.verticalBlurMaterial;
        // @ts-ignore
        this.blurPlane.material.uniforms.tDiffuse.value = this.renderTargetBlur.texture;
        this.verticalBlurMaterial.uniforms.v.value = amount * 1 / 256;

        this.renderer.setRenderTarget(this.renderTarget);
        this.renderer.render(this.blurPlane, this.shadowCamera);

        this.blurPlane.visible = false;
    }
}
