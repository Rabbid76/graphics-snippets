import { DenoisePass } from './render-utility';
import {
  Camera,
  Color,
  CustomBlending,
  MinEquation,
  OneFactor,
  Scene,
  ShaderMaterial,
  Texture,
  WebGLRenderer,
  WebGLRenderTarget,
} from 'three';
// @ts-ignore:7016
import { getPointsOnSphere } from 'realism-effects/src/ssao/utils/ssaoUtils.js';
// @ts-ignore:7016
import { SSAOPass } from 'realism-effects/src/ssao/SSAOPass.js';
// @ts-ignore:7016
import { HBAOPass } from 'realism-effects/src/hbao/HBAOPass.js';
// @ts-ignore:7016
import { PoissionDenoisePass } from 'realism-effects/src/poissionDenoise/PoissionDenoisePass.js';

export interface PoisonDenoiseParameters {
  [key: string]: any;
  iterations: number;
  radius: number;
  rings: number;
  lumaPhi: number;
  depthPhi: number;
  normalPhi: number;
  samples: number;
}

export interface SSAOEffectParameters {
  [key: string]: any;
  resolutionScale: number;
  spp: number;
  distance: number;
  distancePower: number;
  power: number;
  bias: number;
  thickness: number;
}

export interface HBAOEffectParameters {
  [key: string]: any;
  resolutionScale: number;
  spp: number;
  distance: number;
  distancePower: number;
  power: number;
  bias: number;
  thickness: number;
}

export const defaultSSAOEffectParameters: SSAOEffectParameters = {
  resolutionScale: 1,
  spp: 16,
  distance: 0.25,
  distancePower: 0.25,
  power: 2,
  bias: 40,
  thickness: 0.075,
};

export const defaultHBAOEffectParameters: HBAOEffectParameters = {
  resolutionScale: 1,
  spp: 16,
  distance: 0.25,
  distancePower: 1,
  power: 2,
  bias: 20,
  thickness: 0.05,
};

export const defaultPoisonDenoiseParameters: PoisonDenoiseParameters = {
  iterations: 1,
  radius: 10,
  rings: 5.625,
  lumaPhi: 10,
  depthPhi: 2,
  normalPhi: 3.25,
  samples: 16,
};

export class SSAOEffect {
  public needsUpdate: boolean = true;
  public parameters: SSAOEffectParameters = {
    ...defaultSSAOEffectParameters,
  };
  private width: number = 0;
  private height: number = 0;
  private loaded: boolean = false;
  private modulateRedChannel: boolean = false;
  public depthTexture: Texture | null = null;
  public normalTexture: Texture | null = null;
  public ssaoPass?: SSAOPass;

  public get texture(): Texture {
    return this.ssaoPass ? this.ssaoPass.texture : null;
  }

  constructor(width: number, height: number, parameters?: any) {
    this.width = width;
    this.height = height;
    this.modulateRedChannel = parameters?.modulateRedChannel || false;
    if (parameters?.ssaoParameters) {
      this.parameters = parameters.ssaoParameters as SSAOEffectParameters;
    }
    if (parameters) {
      this.updateParameters(parameters);
    }
  }

  private getPass(camera: Camera, scene: Scene): HBAOPass {
    if (!this.ssaoPass) {
      this.ssaoPass = new SSAOPass(camera, scene);
      const material = this.ssaoPass.fullscreenMaterial;
      material.defines.useNormalTexture = '';
      if (this.modulateRedChannel) {
        material.blending = CustomBlending;
        material.blendEquation = MinEquation;
        material.blendEquationAlpha = null;
        material.blendSrc = OneFactor;
        material.blendSrcAlpha = null;
        material.blendDst = OneFactor;
        material.blendDstAlpha = null;
      }
      material.onBeforeCompile = (shader: ShaderMaterial) => {
        if (this.modulateRedChannel) {
          shader.fragmentShader = shader.fragmentShader.replace(
            'gl_FragColor = vec4(normal, occ);',
            'gl_FragColor = vec4(occ, 1.0, 1.0, 1.0);'
          );
          shader.fragmentShader = shader.fragmentShader.replace(
            'gl_FragColor=vec4(normal,occ);',
            'gl_FragColor=vec4(occ,1.0,1.0,1.0);'
          );
        }
      };
      this.needsUpdate = true;
    }
    return this.ssaoPass;
  }

  public dispose(): void {
    this.ssaoPass?.dispose();
  }

  public setSize(width: number, height: number): void {
    this.width = width;
    this.height = height;
    this.ssaoPass?.setSize(width, height);
    this.needsUpdate = true;
  }

  public updateParameters(parameters: any) {
    for (let propertyName in parameters) {
      if (this.parameters.hasOwnProperty(propertyName)) {
        this.parameters[propertyName] = parameters[propertyName];
        this.needsUpdate = true;
      }
    }
    if (parameters.depthTexture) {
      this.depthTexture = parameters.depthTexture;
      this.needsUpdate = true;
    }
    if (parameters.normalTexture) {
      this.depthTexture = parameters.normalTexture;
      this.needsUpdate = true;
    }
  }

  public render(
    renderer: WebGLRenderer,
    camera: Camera,
    scene: Scene,
    renderTarget?: WebGLRenderTarget
  ) {
    if (
      !this.loaded &&
      this.ssaoPass?.fullscreenMaterial.uniforms.blueNoiseTexture.value !== null
    ) {
      this.loaded = true;
      this.needsUpdate = true;
    }
    const updateSamples = !this.ssaoPass;
    const ssaoPass = this.getPass(camera, scene);
    if (this.needsUpdate || updateSamples) {
      this.needsUpdate = false;
      ssaoPass.fullscreenMaterial.uniforms.aoDistance.value =
        this.parameters.distance;
      ssaoPass.fullscreenMaterial.uniforms.distancePower.value =
        this.parameters.distancePower;
      ssaoPass.fullscreenMaterial.uniforms.bias.value = this.parameters.bias;
      ssaoPass.fullscreenMaterial.uniforms.thickness.value =
        this.parameters.thickness;
      ssaoPass.fullscreenMaterial.uniforms.power.value = this.parameters.power;
      ssaoPass.fullscreenMaterial.uniforms.depthTexture.value =
        this.depthTexture;
      ssaoPass.fullscreenMaterial.uniforms.normalTexture.value =
        this.normalTexture;
      ssaoPass.fullscreenMaterial.needsUpdate = true;
      if (updateSamples) {
        const spp = Math.round(this.parameters.spp);
        ssaoPass.fullscreenMaterial.defines.spp = spp;
        const samples = getPointsOnSphere(spp);
        const samplesR = [];
        for (let i = 0; i < spp; i++) {
          samplesR.push((i + 1) / spp);
        }
        ssaoPass.fullscreenMaterial.uniforms.samples = { value: samples };
        ssaoPass.fullscreenMaterial.uniforms.samplesR = { value: samplesR };
      }
      ssaoPass.setSize(this.width, this.height);
      ssaoPass.renderToScreen = false;
    }
    ssaoPass._camera = camera;
    ssaoPass.fullscreenMaterial.uniforms.viewMatrix.value =
      camera.matrixWorldInverse;
    ssaoPass.fullscreenMaterial.uniforms.projectionMatrixInverse.value =
      camera.projectionMatrixInverse;
    ssaoPass.fullscreenMaterial.uniforms.cameraMatrixWorld.value =
      camera.matrixWorld;
    const originalRenderTarget = renderer.getRenderTarget();
    if (renderTarget) {
      ssaoPass.renderTarget = renderTarget;
    }
    ssaoPass.render(renderer);
    renderer.setRenderTarget(originalRenderTarget);
  }
}

export class HBAOEffect {
  public needsUpdate: boolean = true;
  public parameters: HBAOEffectParameters = {
    ...defaultHBAOEffectParameters,
  };
  private width: number = 0;
  private height: number = 0;
  private loaded: boolean = false;
  private floatBufferNormalTexture: boolean = true;
  private modulateRedChannel: boolean = false;
  public depthTexture: Texture | null = null;
  public normalTexture: Texture | null = null;
  public hbaoPass?: HBAOPass;

  public get texture(): Texture {
    return (this.hbaoPass as HBAOPass).texture;
  }

  constructor(width: number, height: number, parameters?: any) {
    this.width = width;
    this.height = height;
    this.floatBufferNormalTexture =
      parameters?.floatBufferNormalTexture || true;
    this.modulateRedChannel = parameters?.modulateRedChannel || false;
    if (parameters?.hbaoParameters) {
      this.parameters = parameters.hbaoParameters as HBAOEffectParameters;
    }
    if (parameters) {
      this.updateParameters(parameters);
    }
  }

  private getPass(camera: Camera, scene: Scene): HBAOPass {
    if (!this.hbaoPass) {
      this.hbaoPass = new HBAOPass(camera, scene);
      const material = this.hbaoPass.fullscreenMaterial;
      material.defines.useNormalTexture = '';
      if (this.modulateRedChannel) {
        material.blending = CustomBlending;
        material.blendEquation = MinEquation;
        material.blendEquationAlpha = null;
        material.blendSrc = OneFactor;
        material.blendSrcAlpha = null;
        material.blendDst = OneFactor;
        material.blendDstAlpha = null;
      }
      material.onBeforeCompile = (shader: ShaderMaterial) => {
        if (this.floatBufferNormalTexture) {
          shader.fragmentShader = shader.fragmentShader.replace(
            'vec3 worldNormal = unpackRGBToNormal(textureLod(normalTexture, uv, 0.).rgb);',
            'vec3 worldNormal = normalize(textureLod(normalTexture, uv, 0.).rgb);'
          );
          shader.fragmentShader = shader.fragmentShader.replace(
            'vec3 worldNormal=unpackRGBToNormal(textureLod(normalTexture,uv,0.).rgb);',
            'vec3 worldNormal=normalize(textureLod(normalTexture,uv,0.).rgb);'
          );
        }
        if (this.modulateRedChannel) {
          shader.fragmentShader = shader.fragmentShader.replace(
            'gl_FragColor = vec4(worldNormal, ao);',
            'gl_FragColor = vec4(ao, 1.0, 1.0, 1.0);'
          );
          shader.fragmentShader = shader.fragmentShader.replace(
            'gl_FragColor=vec4(worldNormal,ao);',
            'gl_FragColor=vec4(ao,1.0,1.0,1.0);'
          );
        }
      };
      this.needsUpdate = true;
    }
    return this.hbaoPass;
  }

  public dispose(): void {
    this.hbaoPass?.dispose();
  }

  public setSize(width: number, height: number): void {
    this.width = width;
    this.height = height;
    this.hbaoPass?.setSize(width, height);
    this.needsUpdate = true;
  }

  public updateParameters(parameters: any) {
    for (let propertyName in parameters) {
      if (this.parameters.hasOwnProperty(propertyName)) {
        this.parameters[propertyName] = parameters[propertyName];
        this.needsUpdate = true;
      }
    }
    if (parameters.depthTexture) {
      this.depthTexture = parameters.depthTexture;
      this.needsUpdate = true;
    }
    if (parameters.normalTexture) {
      this.depthTexture = parameters.normalTexture;
      this.needsUpdate = true;
    }
  }

  public render(
    renderer: WebGLRenderer,
    camera: Camera,
    scene: Scene,
    renderTarget?: WebGLRenderTarget
  ) {
    if (
      !this.loaded &&
      this.hbaoPass?.fullscreenMaterial.uniforms.blueNoiseTexture.value !== null
    ) {
      this.loaded = true;
      this.needsUpdate = true;
    }
    const hbaoPass = this.getPass(camera, scene);
    if (this.needsUpdate) {
      this.needsUpdate = false;
      const spp = Math.round(this.parameters.spp);
      hbaoPass.fullscreenMaterial.defines.spp = spp;
      hbaoPass.fullscreenMaterial.uniforms.aoDistance.value =
        this.parameters.distance;
      hbaoPass.fullscreenMaterial.uniforms.distancePower.value =
        this.parameters.distancePower;
      hbaoPass.fullscreenMaterial.uniforms.bias.value = this.parameters.bias;
      hbaoPass.fullscreenMaterial.uniforms.thickness.value =
        this.parameters.thickness;
      hbaoPass.fullscreenMaterial.uniforms.power.value = this.parameters.power;
      hbaoPass.fullscreenMaterial.uniforms.depthTexture.value =
        this.depthTexture;
      // hbao_utils.glsl: vec3 worldNormal =  textureLod(normalTexture, uv, 0.).rgb; !!!
      hbaoPass.fullscreenMaterial.uniforms.normalTexture.value =
        this.normalTexture;
      hbaoPass.fullscreenMaterial.needsUpdate = true;
      hbaoPass.setSize(this.width, this.height);
      hbaoPass.renderToScreen = false;
    }
    hbaoPass._camera = camera;
    hbaoPass.fullscreenMaterial.uniforms.viewMatrix.value =
      camera.matrixWorldInverse;
    hbaoPass.fullscreenMaterial.uniforms.projectionMatrixInverse.value =
      camera.projectionMatrixInverse;
    hbaoPass.fullscreenMaterial.uniforms.cameraMatrixWorld.value =
      camera.matrixWorld;
    const originalRenderTarget = renderer.getRenderTarget();
    if (renderTarget) {
      hbaoPass.renderTarget = renderTarget;
    }
    hbaoPass.render(renderer);
    renderer.setRenderTarget(originalRenderTarget);
  }
}

export class PoissionDenoiseEffect implements DenoisePass {
  public needsUpdate: boolean = true;
  public parameters: PoisonDenoiseParameters = {
    ...defaultPoisonDenoiseParameters,
  };
  private width: number = 0;
  private height: number = 0;
  private floatBufferNormalTexture: boolean = true;
  private rgInputTexture: boolean = true;
  public _inputTexture: Texture | null = null;
  public depthTexture: Texture | null = null;
  public normalTexture: Texture | null = null;
  public denoisePass?: PoissionDenoisePass;

  public get texture(): Texture | null {
    return this.denoisePass ? this.denoisePass.texture : null;
  }

  public set inputTexture(texture: Texture | null) {
    this._inputTexture = texture;
  }

  constructor(width: number, height: number, parameters?: any) {
    this.width = width;
    this.height = height;
    this.floatBufferNormalTexture =
      parameters?.floatBufferNormalTexture || true;
    this.rgInputTexture = parameters?.rgInputTexture || true;
    this._inputTexture = parameters?.inputTexture || null;
    this.depthTexture = parameters?.depthTexture || null;
    this.normalTexture = parameters?.normalTexture || null;
    if (parameters.poisonDenoiseParameters) {
      this.parameters =
        parameters.poisonDenoiseParameters as PoisonDenoiseParameters;
    } else if (parameters) {
      this.updateParameters(parameters);
    }
  }

  private getPass(camera: Camera): PoissionDenoisePass {
    if (!this.denoisePass) {
      if (this.normalTexture) {
        this.denoisePass = new PoissionDenoisePass(
          camera,
          this._inputTexture as Texture,
          this.depthTexture as Texture
        );
        this.denoisePass.fullscreenMaterial.uniforms.normalTexture = {
          value: this.normalTexture,
        };
        delete this.denoisePass.fullscreenMaterial.defines.NORMAL_IN_RGB;
        this.denoisePass.fullscreenMaterial.onBeforeCompile = (
          shader: ShaderMaterial
        ) => {
          if (this.floatBufferNormalTexture) {
            shader.fragmentShader = shader.fragmentShader.replace(
              'return normalize(textureLod(normalTexture, uv, 0.).xyz * 2.0 - 1.0);',
              'return normalize(textureLod(normalTexture, uv, 0.).xyz);'
            );
            shader.fragmentShader = shader.fragmentShader.replace(
              'float center = texel.rgb;',
              'vec3 center = texel.rgb;'
            );
            shader.fragmentShader = shader.fragmentShader.replace(
              'float neighborColor = neighborTexel.a;',
              'vec3 neighborColor = neighborTexel.rgb;'
            );
            shader.fragmentShader = shader.fragmentShader.replace(
              'return normalize(textureLod(normalTexture,uv,0.).xyz*2.0-1.0);',
              'return normalize(textureLod(normalTexture,uv,0.).xyz);'
            );
            shader.fragmentShader = shader.fragmentShader.replace(
              'float center=texel.rgb;',
              'vec3 center=texel.rgb;'
            );
            shader.fragmentShader = shader.fragmentShader.replace(
              'float neighborColor=neighborTexel.a;',
              'vec3 neighborColor=neighborTexel.rgb;'
            );
          }
          if (this.rgInputTexture) {
            shader.fragmentShader = shader.fragmentShader.replace(
              '#define luminance(a) dot(vec3(0.2125,0.7154,0.0721),a)',
              '#define luminance(a) (a.r*a.g)'
            );
          }
        };
      } else {
        this.denoisePass = new PoissionDenoisePass(
          camera,
          this._inputTexture as Texture,
          this.depthTexture as Texture
        );
      }
      this.needsUpdate = true;
    }
    return this.denoisePass;
  }

  public dispose(): void {
    this.denoisePass?.dispose();
  }

  public setSize(width: number, height: number): void {
    this.width = width;
    this.height = height;
    this.denoisePass?.setSize(width, height);
    this.needsUpdate = true;
  }

  public updateParameters(parameters: any) {
    for (let propertyName in parameters) {
      if (this.parameters.hasOwnProperty(propertyName)) {
        this.parameters[propertyName] = parameters[propertyName];
        this.needsUpdate = true;
      }
    }
    if (parameters.inputTexture) {
      this._inputTexture = parameters.inputTexture;
      this.needsUpdate = true;
    }
    if (parameters.depthTexture) {
      this.depthTexture = parameters.depthTexture;
      this.needsUpdate = true;
    }
    if (parameters.normalTexture) {
      this.normalTexture = parameters.normalTexture;
      this.needsUpdate = true;
    }
  }

  public render(renderer: WebGLRenderer, camera: Camera) {
    const denoisePass = this.getPass(camera);
    if (this.needsUpdate) {
      this.needsUpdate = false;
      denoisePass.iterations = this.parameters.iterations;
      denoisePass.samples = this.parameters.samples;
      denoisePass.rings = this.parameters.rings;
      denoisePass.radius = this.parameters.radius;
      denoisePass.fullscreenMaterial.uniforms.lumaPhi.value =
        this.parameters.lumaPhi;
      denoisePass.fullscreenMaterial.uniforms.depthPhi.value =
        this.parameters.depthPhi;
      denoisePass.fullscreenMaterial.uniforms.normalPhi.value =
        this.parameters.normalPhi;
      denoisePass.fullscreenMaterial.needsUpdate = true;
      denoisePass.setSize(this.width, this.height);
      denoisePass.renderToScreen = false;
      denoisePass.fullscreenMaterial.uniforms.depthTexture.value =
        this.depthTexture;
      if (this.normalTexture) {
        denoisePass.fullscreenMaterial.uniforms.normalTexture.value =
          this.normalTexture;
      }
    }

    denoisePass.inputTexture = this._inputTexture;
    denoisePass.fullscreenMaterial.uniforms.projectionMatrixInverse.value =
      camera.projectionMatrixInverse;
    denoisePass.fullscreenMaterial.uniforms.cameraMatrixWorld.value =
      camera.matrixWorld;

    const originalClearColor = new Color();
    renderer.getClearColor(originalClearColor);
    const originalClearAlpha = renderer.getClearAlpha();
    const originalAutoClear = renderer.autoClear;
    const originalRenderTarget = renderer.getRenderTarget();
    renderer.autoClear = true;
    renderer.setClearColor(0xffffff, 1);
    renderer.setRenderTarget(this.denoisePass.renderTargetA);
    renderer.setRenderTarget(this.denoisePass.renderTargetB);
    renderer.clear(true, false, false);
    denoisePass.render(renderer);
    renderer.setClearColor(originalClearColor);
    renderer.setClearAlpha(originalClearAlpha);
    renderer.setRenderTarget(originalRenderTarget);
    renderer.autoClear = originalAutoClear;
  }
}
