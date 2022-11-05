import * as THREE from 'three'
import { HorizontalBlurShader } from 'three/examples/jsm/shaders/HorizontalBlurShader.js';
import { VerticalBlurShader } from 'three/examples/jsm/shaders/VerticalBlurShader.js';

export interface GroundShadowParameters {
    cameraHelper: boolean
    alwaysUpdate: boolean
    blur: number,
    darkness: number,
    opacity: number,
    planeSize: number,
    cameraFar: number
}

export class GroundAndGroundShadow {
    public shadowMapSize: number = 1024
    public parameters: GroundShadowParameters = {
        cameraHelper: true,
        alwaysUpdate: true,
        blur: 5,
	    darkness: 1,
	    opacity: 1,
        planeSize: 6,
        cameraFar: 5
    }
    public renderer: THREE.WebGLRenderer
    public scene: THREE.Scene
    public camera: THREE.Camera
    public group: THREE.Group
    public needsUpdate: boolean = true
    private groundMesh: THREE.Mesh
    private shadowGround: THREE.Mesh
    private blurPlane: THREE.Mesh
    private renderTarget: THREE.WebGLRenderTarget
    private renderTargetBlur: THREE.WebGLRenderTarget
    private depthMaterial: THREE.MeshDepthMaterial
    private horizontalBlurMaterial: THREE.ShaderMaterial
    private verticalBlurMaterial: THREE.ShaderMaterial
    private shadowCamera: THREE.OrthographicCamera
    private cameraHelper: THREE.CameraHelper
    private lastCameraProjection: THREE.Matrix4 | undefined
    private lastCameraWorld: THREE.Matrix4 | undefined
    
    constructor(renderer: THREE.WebGLRenderer, scene: THREE.Scene, camera: THREE.Camera, groundGeometry: THREE.BufferGeometry, groundMaterial: THREE.Material) {
        this.renderer = renderer
        this.scene = scene
        this.camera = camera
        this.group = new THREE.Group();
        this.scene.add(this.group);

        this.renderTarget = new THREE.WebGLRenderTarget(this.shadowMapSize, this.shadowMapSize)
        this.renderTarget.texture.generateMipmaps = false;
        this.renderTargetBlur = new THREE.WebGLRenderTarget(this.shadowMapSize, this.shadowMapSize)
        this.renderTargetBlur.texture.generateMipmaps = false;

        const size = this.parameters.planeSize
        const shadowGroundGeometry = new THREE.PlaneGeometry(1, 1, 10, 10)
        const shadowGroundMaterial = new THREE.MeshBasicMaterial( {
            map: this.renderTarget.texture,
            opacity: this.parameters.opacity,
            transparent: true,
            depthWrite: false,
            side: THREE.DoubleSide
        } );
        shadowGroundMaterial.polygonOffset = true
        shadowGroundMaterial.polygonOffsetFactor = 2
        shadowGroundMaterial.polygonOffsetUnits = 2
        this.shadowGround = new THREE.Mesh(shadowGroundGeometry, shadowGroundMaterial)
        this.shadowGround.renderOrder = 1
        this.shadowGround.receiveShadow = true
        this.group.add(this.shadowGround)

        this.blurPlane = new THREE.Mesh(shadowGroundGeometry)
        this.blurPlane.visible = false;
        this.group.add(this.blurPlane);

        this.groundMesh = new THREE.Mesh()
        this.updateGround(groundGeometry, groundMaterial)
        this.group.add(this.groundMesh)

        this.shadowCamera = new THREE.OrthographicCamera(-size/2, size/2, -size/2, size/2)
		this.shadowCamera.rotation.x = Math.PI // mke camera look upwards
		this.group.add(this.shadowCamera);

        this.cameraHelper = new THREE.CameraHelper(this.shadowCamera);
        this.scene.add(this.cameraHelper);

        this.depthMaterial = new THREE.MeshDepthMaterial();
        this.depthMaterial.userData.darkness = { value: this.parameters.darkness };
        this.depthMaterial.onBeforeCompile = (shader) => {
            shader.uniforms.darkness = this.depthMaterial.userData.darkness;
            shader.fragmentShader = /* glsl */`
                uniform float darkness;
                ${shader.fragmentShader.replace(
            'gl_FragColor = vec4( vec3( 1.0 - fragCoordZ ), opacity );',
            'gl_FragColor = vec4( vec3( 0.0 ), ( 1.0 - fragCoordZ ) * darkness );'
        )}
            `;
        };
        this.depthMaterial.depthTest = false;
        this.depthMaterial.depthWrite = false;
        this.horizontalBlurMaterial = new THREE.ShaderMaterial(HorizontalBlurShader)
		this.horizontalBlurMaterial.depthTest = false
		this.verticalBlurMaterial = new THREE.ShaderMaterial(VerticalBlurShader)
		this.verticalBlurMaterial.depthTest = false

        this.updatePlaneAndShadowCamera()
    }

    public updateParameters() {
        this.depthMaterial.userData.darkness.value = this.parameters.darkness
        const shadowGroundMaterial = this.shadowGround.material as THREE.MeshBasicMaterial
        shadowGroundMaterial.opacity = this.parameters.opacity;
        this.needsUpdate = true
    }

    public updatePlaneAndShadowCamera() {
        const size = this.parameters.planeSize
        this.shadowGround.scale.x = size
        this.shadowGround.scale.y = size
        this.blurPlane.scale.x = size
        this.blurPlane.scale.y = size
        this.shadowCamera.left = -size / 2
        this.shadowCamera.right = size / 2
        this.shadowCamera.top = -size / 2
        this.shadowCamera.bottom = size / 2
        this.shadowCamera.near = 0
        this.shadowCamera.far = this.parameters.cameraFar
        this.shadowCamera.updateProjectionMatrix()
        this.cameraHelper.update()
        this.needsUpdate = true
    }

    public updateGround(geometry?: THREE.BufferGeometry, material?: THREE.Material) {
        if (geometry) {
            this.groundMesh.geometry = geometry
        }
        if (material) {
            material.polygonOffset = true
            material.polygonOffsetFactor = 4
            material.polygonOffsetUnits = 4
            material.needsUpdate
            this.groundMesh.material = material
        }
    }

    public render(): void {
        const needsUpdate = this.parameters.alwaysUpdate || this.needsUpdate ||
            !this.lastCameraProjection?.equals(this.camera.projectionMatrix) || !this.lastCameraWorld?.equals(this.camera.matrixWorld)
        if (!needsUpdate) {
            return
        }
        this.needsUpdate = false
        this.lastCameraProjection = this.camera.projectionMatrix.clone()
        this.lastCameraWorld = this.camera.matrixWorld.clone()

        const initialBackground = this.scene.background
        this.scene.background = null
        this.scene.overrideMaterial = this.depthMaterial;
        const initialClearAlpha = this.renderer.getClearAlpha()
        this.renderer.setClearAlpha(0)
        this.groundMesh.visible = false
        this.shadowGround.visible = false
        this.cameraHelper.visible = false

        this.renderer.setRenderTarget(this.renderTarget)
        this.renderer.render(this.scene, this.shadowCamera)
        this.scene.overrideMaterial = null
        this.blurShadow(this.parameters.blur / this.parameters.planeSize)
        this.blurShadow(this.parameters.blur * 0.4 / this.parameters.planeSize)

        this.renderer.setRenderTarget(null)
        this.renderer.setClearAlpha(initialClearAlpha);
        this.scene.background = initialBackground
        this.groundMesh.visible = true
        this.shadowGround.visible = true
        this.cameraHelper.visible = this.parameters.cameraHelper
    }

    public blurShadow(amount: number): void {
        this.blurPlane.visible = true
        this.blurPlane.material = this.horizontalBlurMaterial
        // @ts-ignore
        this.blurPlane.material.uniforms.tDiffuse.value = this.renderTarget.texture
        this.horizontalBlurMaterial.uniforms.h.value = amount * 1 / 256

        this.renderer.setRenderTarget(this.renderTargetBlur)
        this.renderer.render(this.blurPlane, this.shadowCamera)

        this.blurPlane.material = this.verticalBlurMaterial
        // @ts-ignore
        this.blurPlane.material.uniforms.tDiffuse.value = this.renderTargetBlur.texture
        this.verticalBlurMaterial.uniforms.v.value = amount * 1 / 256

        this.renderer.setRenderTarget(this.renderTarget)
        this.renderer.render(this.blurPlane, this.shadowCamera)

        this.blurPlane.visible = false;
    }
}