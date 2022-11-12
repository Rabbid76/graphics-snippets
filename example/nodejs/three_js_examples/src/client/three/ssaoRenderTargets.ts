import {
    SSAOShader,
    SSAOBlurShader,
    ssaoKernelSize,
    ssaoBlurShaderOptimized
} from './ssaoMaterialsAndShaders';
import * as THREE from 'three';
import { SimplexNoise } from 'three/examples/jsm/math/SimplexNoise.js';

export interface SSAOParameters {
    enabled: boolean;
    alwaysUpdate: boolean;
    kernelRadius: number;
    minDistance: number;
    maxDistance: number;
    intensity: number;
}

export class SSAORenderTargets {
    public ssaoParameters: SSAOParameters;
    private width: number;
    private height: number;
    private samples: number;
    private depthNormalScale = 2;
    private noiseTexture?: THREE.DataTexture;
    private kernel: THREE.Vector3[] = [];
    private normalRenderMaterial?: THREE.MeshNormalMaterial;
    private ssaoRenderMaterial?: THREE.ShaderMaterial;
    private blurRenderMaterial?: THREE.ShaderMaterial;
    public depthNormalRenderTarget?: THREE.WebGLRenderTarget;
    public ssaoRenderTarget?: THREE.WebGLRenderTarget;
    public blurRenderTarget?: THREE.WebGLRenderTarget;

    constructor(width: number, height: number, samples: number, parameters?: any) {
        this.ssaoParameters = {
            enabled: parameters?.enabled ?? true,
            alwaysUpdate: parameters?.alwaysUpdate ?? true,
            kernelRadius: parameters?.kernelRadius ?? 0.03,
            minDistance: parameters?.minDistance ?? 0.005,
            maxDistance: parameters?.maxDistance ?? 0.3,
            intensity: parameters?.intensity ?? 1
        };
        this.width = width;
        this.height = height;
        this.samples = samples;
    }

    public dispose() {
        this.noiseTexture?.dispose();
        this.normalRenderMaterial?.dispose();
        this.normalRenderMaterial?.dispose();
        this.ssaoRenderMaterial?.dispose();
        this.blurRenderMaterial?.dispose();
        this.depthNormalRenderTarget?.dispose();
        this.ssaoRenderTarget?.dispose();
        this.blurRenderTarget?.dispose();
    }

    public setSize(width: number, height: number) {
        this.width = width;
        this.height = height;
        this.ssaoRenderMaterial?.uniforms['resolution'].value.set(this.width, this.height);
        this.blurRenderMaterial?.uniforms['resolution'].value.set(this.width, this.height);
        this.depthNormalRenderTarget?.setSize(this.width * this.depthNormalScale, this.height * this.depthNormalScale);
        this.ssaoRenderTarget?.setSize(this.width, this.height);
        this.blurRenderTarget?.setSize(this.width, this.height);
    }

    public updateSSAOMaterial(camera: THREE.Camera): THREE.ShaderMaterial {
        const ssaoMaterial = this.getSSAORenderMaterial(camera);
        ssaoMaterial.uniforms.kernelRadius.value = this.ssaoParameters.kernelRadius;
        ssaoMaterial.uniforms.minDistance.value = this.ssaoParameters.minDistance;
        ssaoMaterial.uniforms.maxDistance.value = this.ssaoParameters.maxDistance;
        ssaoMaterial.uniforms.intensity.value = this.ssaoParameters.intensity;
        return ssaoMaterial;
    }

    public updateSSAOKernel() {
        if (this.noiseTexture) {
            this.noiseTexture = this.generateRandomKernelRotations();
        }
        if (this.kernel) {
            this.kernel = this.generateSampleKernel();
        }
        if (this.ssaoRenderMaterial) {
            this.ssaoRenderMaterial.uniforms.tNoise.value = this.noiseTexture;
            this.ssaoRenderMaterial.uniforms.kernel.value = this.kernel;
        }
    }

    public getDepthNormalRenderTarget(): THREE.WebGLRenderTarget {
        if (!this.depthNormalRenderTarget) {
            const depthTexture = new THREE.DepthTexture(this.width * this.depthNormalScale, this.height * this.depthNormalScale);
            depthTexture.format = THREE.DepthStencilFormat;
            depthTexture.type = THREE.UnsignedInt248Type;
            //depthTexture.minFilter = this.depthScale != 1.0 ? THREE.LinearFilter : THREE.NearestFilter,
            //depthTexture.magFilter = this.depthScale != 1.0 ? THREE.LinearFilter : THREE.NearestFilter,
            this.depthNormalRenderTarget = new THREE.WebGLRenderTarget(this.width * this.depthNormalScale, this.height * this.depthNormalScale, {
                minFilter: ssaoBlurShaderOptimized || this.depthNormalScale !== 1.0 ? THREE.LinearFilter : THREE.NearestFilter,
                magFilter: ssaoBlurShaderOptimized || this.depthNormalScale !== 1.0 ? THREE.LinearFilter : THREE.NearestFilter,
                depthTexture
            });
        }
        return this.depthNormalRenderTarget;
    }

    public getSSSAORenderTarget(): THREE.WebGLRenderTarget {
        if (!this.ssaoRenderTarget) {
            this.ssaoRenderTarget = new THREE.WebGLRenderTarget(this.width, this.height, { samples: this.samples });
        }
        return this.ssaoRenderTarget;
    }

    public getBlurRenderTarget(): THREE.WebGLRenderTarget {
        if (!this.blurRenderTarget) {
            this.blurRenderTarget = new THREE.WebGLRenderTarget(this.width, this.height, { samples: this.samples });
        }
        return this.blurRenderTarget;
    }

    public getNormalRenderMaterial(): THREE.MeshNormalMaterial {
        if (!this.normalRenderMaterial) {
            this.normalRenderMaterial = new THREE.MeshNormalMaterial();
            this.normalRenderMaterial.blending = THREE.NoBlending;
        }
        return this.normalRenderMaterial;
    }

    public getSSAORenderMaterial(camera: THREE.Camera): THREE.ShaderMaterial {
        if (!this.ssaoRenderMaterial) {
            this.ssaoRenderMaterial = new THREE.ShaderMaterial( {
                defines: Object.assign({}, SSAOShader.defines),
                uniforms: THREE.UniformsUtils.clone(SSAOShader.uniforms),
                vertexShader: SSAOShader.vertexShader,
                fragmentShader: SSAOShader.fragmentShader,
                blending: THREE.NoBlending
            } );
            const depthNormalTarget = this.getDepthNormalRenderTarget();
            const noiseTexture = this.getNoiseTexture();
            const kernel = this.getKernel();
            //this.ssaoRenderMaterial.uniforms['tDiffuse'].value = this.previousRenderTarget.texture;
            this.ssaoRenderMaterial.uniforms.tNormal.value = depthNormalTarget.texture;
            this.ssaoRenderMaterial.uniforms.tDepth.value = depthNormalTarget.depthTexture;
            this.ssaoRenderMaterial.uniforms.tNoise.value = noiseTexture;
            this.ssaoRenderMaterial.uniforms.kernel.value = kernel;
        }
        // @ts-ignore
        this.ssaoRenderMaterial.uniforms.cameraNear.value = camera.near;
        // @ts-ignore
        this.ssaoRenderMaterial.uniforms.cameraFar.value = camera.far;
        this.ssaoRenderMaterial.uniforms.resolution.value.set(this.width, this.height);
        this.ssaoRenderMaterial.uniforms.cameraProjectionMatrix.value.copy(camera.projectionMatrix);
        this.ssaoRenderMaterial.uniforms.cameraInverseProjectionMatrix.value.copy(camera.projectionMatrixInverse);
        return this.ssaoRenderMaterial;
    }

    public getBlurRenderMaterial(): THREE.ShaderMaterial {
        if (!this.blurRenderMaterial) {
            this.blurRenderMaterial = new THREE.ShaderMaterial( {
                defines: Object.assign({}, SSAOBlurShader.defines),
                uniforms: THREE.UniformsUtils.clone(SSAOBlurShader.uniforms),
                vertexShader: SSAOBlurShader.vertexShader,
                fragmentShader: SSAOBlurShader.fragmentShader
            });
            const ssaoRenderTarget = this.getSSSAORenderTarget();
            this.blurRenderMaterial.uniforms.tDiffuse.value = ssaoRenderTarget.texture;
        }
        this.blurRenderMaterial.uniforms.resolution.value.set(this.width, this.height);
        return this.blurRenderMaterial;
    }

    private getNoiseTexture(): THREE.DataTexture {
        if (!this.noiseTexture) {
            this.noiseTexture = this.generateRandomKernelRotations();
        }
        return this.noiseTexture;
    }

    private getKernel(): THREE.Vector3[] {
        if (!this.kernel.length) {
            this.kernel = this.generateSampleKernel();
        }
        return this.kernel;
    }

    private generateSampleKernel(): THREE.Vector3[] {
        const kernelSize = ssaoKernelSize;
        const kernel: THREE.Vector3[] = [];
        for (let i = 0; i < kernelSize; i ++) {
            const sample = new THREE.Vector3();
            sample.x = ( Math.random() * 2 ) - 1;
            sample.y = ( Math.random() * 2 ) - 1;
            sample.z = Math.random();
            sample.normalize();
            let scale = i / kernelSize;
            scale = THREE.MathUtils.lerp(0.1, 1, scale * scale);
            sample.multiplyScalar(scale);
            kernel.push(sample);
        }
        return kernel;
    }

    private generateRandomKernelRotations(): THREE.DataTexture  {
        const width = 4;
        const height = 4;
        if (SimplexNoise === undefined ) {
            console.error( 'SSSO MSAA Pass: The pass relies on SimplexNoise.' );
        }
        const simplex = new SimplexNoise();
        const size = width * height;
        const data = new Float32Array( size );
        for ( let i = 0; i < size; i ++ ) {
            const x = ( Math.random() * 2 ) - 1;
            const y = ( Math.random() * 2 ) - 1;
            const z = 0;
            data[ i ] = simplex.noise3d( x, y, z );
        }
        const noiseTexture = new THREE.DataTexture(data, width, height, THREE.RedFormat, THREE.FloatType);
        noiseTexture.wrapS = THREE.RepeatWrapping;
        noiseTexture.wrapT = THREE.RepeatWrapping;
        noiseTexture.needsUpdate = true;
        return noiseTexture;
    }
}
