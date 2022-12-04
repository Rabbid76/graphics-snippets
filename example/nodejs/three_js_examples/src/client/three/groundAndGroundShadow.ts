import {
    HorizontalBlurShadowShader,
    VerticalBlurShadowShader
} from './shaderUtility';
import * as THREE from 'three';

export interface GroundContactShadowParameters {
    enabled: boolean;
    cameraHelper: boolean;
    alwaysUpdate: boolean;
    blurMin: number;
    blurMax: number;
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
    public noNeedOfUpdateCount = 0;
    private groundMesh?: THREE.Mesh;
    private shadowGround: THREE.Mesh;
    private blurPlane: THREE.Mesh;
    private renderTarget: THREE.WebGLRenderTarget;
    private renderTargetBlur: THREE.WebGLRenderTarget;
    private depthMaterial: THREE.MeshDepthMaterial;
    private horizontalBlurMaterial: THREE.ShaderMaterial;
    private verticalBlurMaterial: THREE.ShaderMaterial;
    private shadowCamera: THREE.OrthographicCamera;

    constructor(renderer: THREE.WebGLRenderer, parameters: any) {
        this.shadowMapSize = parameters.shadowMapSize ?? 1024;
        this.parameters = {
            enabled: parameters.enabled ?? true,
            cameraHelper: parameters.cameraHelper ?? false,
            alwaysUpdate: parameters.alwaysUpdate ?? false,
            blurMin: parameters.blurMin ?? 0.2,
            blurMax: parameters.blurMax ?? 6,
            darkness: parameters.darkness ?? 1,
            opacity: parameters.opacity ?? 0.5,
            planeSize: parameters.planeSize ?? 10,
            cameraFar: parameters.cameraFar ?? 3
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
        this.shadowGround.receiveShadow = false;
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
                ${shader.fragmentShader.replace('gl_FragColor = vec4( vec3( 1.0 - fragCoordZ ), opacity );', 'gl_FragColor = vec4(vec3(0.0), pow(1.0 - fragCoordZ, 2.0) * darkness);')}
            `;
        };
        this.depthMaterial.depthTest = true;
        this.depthMaterial.depthWrite = true;
        this.horizontalBlurMaterial = new THREE.ShaderMaterial(HorizontalBlurShadowShader);
        this.horizontalBlurMaterial.depthTest = false;
        this.verticalBlurMaterial = new THREE.ShaderMaterial(VerticalBlurShadowShader);
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
        // y is always 0
        this.group.position.set(boundsCenter.x, 0, boundsCenter.z);
        this.group.updateMatrixWorld();
        const boundsSize = bounds.getSize(new THREE.Vector3());
        const maxPlanSideLength = Math.max(boundsSize.x, boundsSize.z);
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

    public render(scene: THREE.Scene): void {
        const needsUpdate = this.parameters.alwaysUpdate || this.needsUpdate;
        if (!needsUpdate) {
            this.noNeedOfUpdateCount ++;
            if (this.noNeedOfUpdateCount >= 10) {
                return;
            }
        } else {
            this.noNeedOfUpdateCount = 0;
        }
        this.needsUpdate = false;

        //const shadowGroundMaterial = this.shadowGround.material as THREE.MeshBasicMaterial;
        //shadowGroundMaterial.opacity = this.parameters.opacity * (this.noNeedOfUpdateCount + 2) / 12;

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

        if (this.noNeedOfUpdateCount === 0) {
            this.renderer.setRenderTarget(this.renderTarget);
            this.renderer.clear();
            this.renderer.render(scene, this.shadowCamera);
            scene.overrideMaterial = null;
            this.blurShadow(this.parameters.blurMin / this.parameters.planeSize, this.parameters.blurMin / this.parameters.planeSize);
        } else if (this.noNeedOfUpdateCount === 1) {
            scene.overrideMaterial = null;
            this.blurShadow(this.parameters.blurMin / this.parameters.planeSize, this.parameters.blurMax / this.parameters.planeSize);
        }
        scene.overrideMaterial = null;
        const finalBlurAmount = 0.4 / this.parameters.planeSize;
        this.blurShadow(finalBlurAmount, finalBlurAmount);

        this.renderer.setRenderTarget(null);
        this.renderer.setClearAlpha(initialClearAlpha);
        scene.background = initialBackground;
        if (this.groundMesh) {
            this.groundMesh.visible = true;
        }
        this.shadowGround.visible = true;
        this.cameraHelper.visible = this.parameters.cameraHelper;
    }

    public blurShadow(amountMin: number, amountMax: number): void {
        this.blurPlane.visible = true;
        this.blurPlane.material = this.horizontalBlurMaterial;
        // @ts-ignore
        this.blurPlane.material.uniforms.tDiffuse.value = this.renderTarget.texture;
        this.horizontalBlurMaterial.uniforms.hRange.value.x = amountMin * 1 / 256;
        this.horizontalBlurMaterial.uniforms.hRange.value.y = amountMax * 1 / 256;
        this.horizontalBlurMaterial.uniforms.shadowScale.value = 1 / this.parameters.darkness;

        this.renderer.setRenderTarget(this.renderTargetBlur);
        this.renderer.render(this.blurPlane, this.shadowCamera);

        this.blurPlane.material = this.verticalBlurMaterial;
        // @ts-ignore
        this.blurPlane.material.uniforms.tDiffuse.value = this.renderTargetBlur.texture;
        this.verticalBlurMaterial.uniforms.vRange.value.x = amountMin * 1 / 256;
        this.verticalBlurMaterial.uniforms.vRange.value.y = amountMax * 1 / 256;
        this.verticalBlurMaterial.uniforms.shadowScale.value = 1 / this.parameters.darkness;

        this.renderer.setRenderTarget(this.renderTarget);
        this.renderer.render(this.blurPlane, this.shadowCamera);

        this.blurPlane.visible = false;
    }
}
