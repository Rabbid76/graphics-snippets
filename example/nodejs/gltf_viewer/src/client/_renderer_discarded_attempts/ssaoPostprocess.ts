import { getMaxSamples } from '../renderer/render-utility'
//import { SSAOMSAAPass} from './ssaoMsaaPass'
// @ts-ignore
import { SSSO_MSAA_Pass } from './ssaoMsaaPassJs'
import * as THREE from 'three'
import { EffectComposer } from 'three/examples/jsm/postprocessing/EffectComposer'
import { RenderPass } from 'three/examples/jsm/postprocessing/RenderPass'
import { SSAOPass } from 'three/examples/jsm/postprocessing/SSAOPass'
import { SAOPass } from 'three/examples/jsm/postprocessing/SAOPass'
import { GUI } from 'dat.gui'

export abstract class AoPostprocess {
    public composer: EffectComposer

    constructor(renderer: THREE.WebGLRenderer, width: number, height: number) {
        const samples = getMaxSamples(renderer)
        const renderTarget = new THREE.WebGLRenderTarget(width, height, { samples: samples })
        this.composer = new EffectComposer(renderer, renderTarget);
    }

    public abstract addUiElements(parent: GUI): void
    public abstract setSize(width: number, height: number): void
    public abstract render(): void
}

export class SSAOPassPostprocess extends AoPostprocess {
    public ssaoPass: SSAOPass

    constructor(renderer: THREE.WebGLRenderer, scene: THREE.Scene, camera: THREE.Camera, width: number, height: number) {
        super(renderer, width, height)
        this.ssaoPass = new SSAOPass(scene, camera, width, height);
        this.ssaoPass.output = 0
        this.ssaoPass.kernelSize = 32
        this.ssaoPass.kernelRadius = 0.03
        this.ssaoPass.minDistance = 0.002
        this.ssaoPass.maxDistance = 0.1
        this.composer.addPass(this.ssaoPass);
    }

    public addUiElements(parent: GUI): void {
        const SSAOPassOUTPUT = {Default: 0, SSAO: 1, Blur: 2, Beauty: 3, Depth: 4, Normal: 5 };
        parent.add<any>(this.ssaoPass, 'output', {
            'Default': SSAOPassOUTPUT.Default,
            'SSAO Only': SSAOPassOUTPUT.SSAO,
            'SSAO Only + Blur': SSAOPassOUTPUT.Blur,
            'Beauty': SSAOPassOUTPUT.Beauty,
            'Depth': SSAOPassOUTPUT.Depth,
            'Normal': SSAOPassOUTPUT.Normal
        } ).onChange((value) => { this.ssaoPass.output = parseInt(value); } )
        parent.add<any>(this.ssaoPass, 'kernelSize').min(8).max(64);
        parent.add<any>(this.ssaoPass, 'kernelRadius').min(0.001).max(0.2);
        parent.add<any>(this.ssaoPass, 'minDistance').min(0.0001).max(0.1);
        parent.add<any>(this.ssaoPass, 'maxDistance').min(0.01).max(1);
    }

    public setSize(width: number, height: number): void {
        this.ssaoPass.setSize(width, height)
        this.composer.setSize(width, height)
    }

    public render(): void {
        const camera = this.ssaoPass.camera as THREE.PerspectiveCamera
        this.ssaoPass.ssaoMaterial.uniforms['cameraNear'].value = camera.near;
        this.ssaoPass.ssaoMaterial.uniforms['cameraFar'].value = camera.far;
        this.ssaoPass.depthRenderMaterial.uniforms['cameraNear'].value = camera.near;
        this.ssaoPass.depthRenderMaterial.uniforms['cameraFar'].value = camera.far;
        this.composer.render()
    }
}

export class SSAOMSAAPostprocess extends AoPostprocess {
    public ssaoPass: SSSO_MSAA_Pass

    constructor(renderer: THREE.WebGLRenderer, scene: THREE.Scene, camera: THREE.Camera, width: number, height: number) {
        super(renderer, width, height)
        this.ssaoPass = new SSSO_MSAA_Pass(scene, camera as THREE.PerspectiveCamera, width, height, getMaxSamples(renderer));
        this.ssaoPass.output = 0
        this.ssaoPass.kernelSize = 32
        this.ssaoPass.kernelRadius = 0.03
        this.ssaoPass.minDistance = 0.002
        this.ssaoPass.maxDistance = 0.1
        this.composer.addPass(this.ssaoPass);
    }

    public addUiElements(parent: GUI): void {
        const SSAOPassOUTPUT = {Default: 0, SSAO: 1, Blur: 2, Beauty: 3, Depth: 4, Normal: 5 };
        parent.add<any>(this.ssaoPass, 'output', {
            'Default': SSAOPassOUTPUT.Default,
            'SSAO Only': SSAOPassOUTPUT.SSAO,
            'SSAO Only + Blur': SSAOPassOUTPUT.Blur,
            'Beauty': SSAOPassOUTPUT.Beauty,
            'Depth': SSAOPassOUTPUT.Depth,
            'Normal': SSAOPassOUTPUT.Normal
        } ).onChange((value) => { this.ssaoPass.output = parseInt(value); } )
        parent.add<any>(this.ssaoPass, 'kernelSize').min(8).max(64);
        parent.add<any>(this.ssaoPass, 'kernelRadius').min(0.001).max(0.2);
        parent.add<any>(this.ssaoPass, 'minDistance').min(0.0001).max(0.1);
        parent.add<any>(this.ssaoPass, 'maxDistance').min(0.01).max(1);
    }

    public setSize(width: number, height: number): void {
        this.ssaoPass.setSize(width, height)
        this.composer.setSize(width, height)
    }

    public render(): void {
        const camera = this.ssaoPass.camera as THREE.PerspectiveCamera
        this.ssaoPass.ssaoMaterial.uniforms['cameraNear'].value = camera.near;
        this.ssaoPass.ssaoMaterial.uniforms['cameraFar'].value = camera.far;
        this.ssaoPass.depthRenderMaterial.uniforms['cameraNear'].value = camera.near;
        this.ssaoPass.depthRenderMaterial.uniforms['cameraFar'].value = camera.far;
        this.composer.render()
    }
}

export class SAOPassPostprocess extends AoPostprocess {
    public renderPass: RenderPass
    public saoPass: SAOPass

    constructor(renderer: THREE.WebGLRenderer, scene: THREE.Scene, camera: THREE.Camera, width: number, height: number) {
        super(renderer, width, height)
        
        this.renderPass = new RenderPass(scene, camera);
		this.composer.addPass(this.renderPass);
		this.saoPass = new SAOPass(scene, camera, false, true);
		this.composer.addPass(this.saoPass);

        this.saoPass.params.saoBias = 0.5
        this.saoPass.params.saoIntensity = 0.01
        this.saoPass.params.saoScale = 0.05
        this.saoPass.params.saoKernelRadius = 45
        this.saoPass.params.saoMinResolution = 0.01
        this.saoPass.params.saoBlur = true
        this.saoPass.params.saoBlurRadius = 3
        this.saoPass.params.saoBlurStdDev = 10
        this.saoPass.params.saoBlurDepthCutoff = 20
    }

    public addUiElements(parent: GUI): void {
        parent.add<any>(this.saoPass.params, 'output', {
            'Beauty': SAOPass.OUTPUT.Beauty,
            'Beauty+SAO': SAOPass.OUTPUT.Default,
            'SAO': SAOPass.OUTPUT.SAO,
            'Depth': SAOPass.OUTPUT.Depth,
            'Normal': SAOPass.OUTPUT.Normal
        } ).onChange((value) => {
            this.saoPass.params.output = parseInt(value);
        } );
        parent.add<any>(this.saoPass.params, 'saoBias', - 1, 1);
        parent.add<any>(this.saoPass.params, 'saoIntensity', 0, 1);
        parent.add<any>(this.saoPass.params, 'saoScale', 0, 10);
        parent.add<any>(this.saoPass.params, 'saoKernelRadius', 1, 100);
        parent.add<any>(this.saoPass.params, 'saoMinResolution', 0, 1);
        parent.add<any>(this.saoPass.params, 'saoBlur');
        parent.add<any>(this.saoPass.params, 'saoBlurRadius', 0, 200);
        parent.add<any>(this.saoPass.params, 'saoBlurStdDev', 0.5, 150);
        parent.add<any>(this.saoPass.params, 'saoBlurDepthCutoff', 0.0, 0.1);
    }

    public setSize(width: number, height: number): void {
        this.composer.setSize(width, height)
    }

    public render(): void {
        this.composer.render()
    }
}