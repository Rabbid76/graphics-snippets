import { CopyTransformMaterial } from './shader-utility';
import { RenderOverrideVisibility, RenderPass } from './render-utility';
import {
  Camera,
  CustomBlending,
  DepthTexture,
  DepthStencilFormat,
  Matrix4,
  //MaxEquation,
  MinEquation,
  NearestFilter,
  NoBlending,
  OneFactor,
  OrthographicCamera,
  PerspectiveCamera,
  RGBAFormat,
  Scene,
  ShaderMaterial,
  Texture,
  UniformsUtils,
  UnsignedInt248Type,
  Vector2,
  Vector3,
  WebGLRenderer,
  WebGLRenderTarget,
} from 'three';

export interface GroundReflectionParameters {
  enabled: boolean;
  intensity: number;
  fadeOut: number;
  brightness: number;
  blurHorizontal: number;
  blurVertical: number;
  blurAscent: number;
  groundLevel: number;
  renderTargetDownScale: number;
}

export class GroundReflectionPass {
  private width: number;
  private height: number;
  public parameters: GroundReflectionParameters;
  private _shadowRenderTarget?: WebGLRenderTarget;
  private _reflectionRenderTarget?: WebGLRenderTarget;
  private _intensityRenderTarget?: WebGLRenderTarget;
  private _blurRenderTarget?: WebGLRenderTarget;
  private renderPass: RenderPass;
  private renderOverrideVisibility: RenderOverrideVisibility;
  private groundShadowMaterial: GroundShadowMaterial;
  private reflectionIntensityMaterial: GroundReflectionIntensityMaterial;
  private blurMaterial: ShaderMaterial;
  private _copyMaterial: CopyTransformMaterial;

  public get shadowRenderTarget(): WebGLRenderTarget {
    this._shadowRenderTarget =
      this._shadowRenderTarget ?? this.newRenderTarget(false);
    return this._shadowRenderTarget;
  }

  public get reflectionRenderTarget(): WebGLRenderTarget {
    this._reflectionRenderTarget =
      this._reflectionRenderTarget ?? this.newRenderTarget(true);
    return this._reflectionRenderTarget;
  }

  public get intensityRenderTarget(): WebGLRenderTarget {
    this._intensityRenderTarget =
      this._intensityRenderTarget ?? this.newRenderTarget(false);
    return this._intensityRenderTarget;
  }

  public get blurRenderTarget(): WebGLRenderTarget {
    this._blurRenderTarget =
      this._blurRenderTarget ?? this.newRenderTarget(false);
    return this._blurRenderTarget;
  }

  constructor(width: number, height: number, parameters: any) {
    this.width = width;
    this.height = height;
    this.parameters = {
      enabled: parameters.enabled ?? false,
      intensity: parameters.intensity ?? 0.25,
      fadeOut: parameters.fadeOut ?? 1,
      brightness: parameters.brightness ?? 1.0,
      blurHorizontal: parameters.blurHorizontal ?? 3.0,
      blurVertical: parameters.blurHorizontal ?? 6.0,
      blurAscent: parameters.blurAscent ?? 0,
      groundLevel: parameters.groundLevel ?? 0,
      renderTargetDownScale: parameters.renderTargetDownScale ?? 4,
    };
    this._copyMaterial = new CopyTransformMaterial({}, false);
    this.updateCopyMaterial(null);
    this.groundShadowMaterial = new GroundShadowMaterial();
    this.reflectionIntensityMaterial = new GroundReflectionIntensityMaterial();
    this.blurMaterial = new ShaderMaterial(BlurGroundReflectionShadowShader);
    this.blurMaterial.depthTest = false;
    this.renderPass = parameters?.renderPass ?? new RenderPass();
    this.renderOverrideVisibility =
      parameters?.renderOverrideVisibility ??
      new RenderOverrideVisibility(true);
  }

  private newRenderTarget(createDepthTexture: boolean): WebGLRenderTarget {
    const width = this.width / this.parameters.renderTargetDownScale;
    const height = this.height / this.parameters.renderTargetDownScale;
    let additionalParameters: any = {};
    if (createDepthTexture) {
      const depthTexture = new DepthTexture(width, height);
      depthTexture.format = DepthStencilFormat;
      depthTexture.type = UnsignedInt248Type;
      additionalParameters.minFilter = NearestFilter;
      additionalParameters.magFilter = NearestFilter;
      additionalParameters.depthTexture = depthTexture;
    } else {
      additionalParameters.samples = 1;
    }
    return new WebGLRenderTarget(width, height, {
      format: RGBAFormat,
      ...additionalParameters,
    });
  }

  dispose() {
    this._reflectionRenderTarget?.dispose();
    this._intensityRenderTarget?.dispose();
    this._blurRenderTarget?.dispose();
    this._copyMaterial.dispose();
  }

  public setSize(width: number, height: number) {
    this.width = width;
    this.height = height;
    this._reflectionRenderTarget?.setSize(
      this.width / this.parameters.renderTargetDownScale,
      this.height / this.parameters.renderTargetDownScale
    );
    this._intensityRenderTarget?.setSize(
      this.width / this.parameters.renderTargetDownScale,
      this.height / this.parameters.renderTargetDownScale
    );
    this._blurRenderTarget?.setSize(
      this.width / this.parameters.renderTargetDownScale,
      this.height / this.parameters.renderTargetDownScale
    );
  }

  public updateParameters(parameters: any) {
    if (parameters.enabled !== undefined) {
      this.parameters.enabled = parameters.enabled;
    }
    if (parameters.intensity !== undefined) {
      this.parameters.intensity = parameters.intensity;
    }
    if (parameters.fadeOut !== undefined) {
      this.parameters.fadeOut = parameters.fadeOut;
    }
    if (parameters.brightness !== undefined) {
      this.parameters.brightness = parameters.brightness;
    }
    if (parameters.blurHorizontal !== undefined) {
      this.parameters.blurHorizontal = parameters.blurHorizontal;
    }
    if (parameters.blurVertical !== undefined) {
      this.parameters.blurVertical = parameters.blurVertical;
    }
    if (parameters.blurAscent !== undefined) {
      this.parameters.blurAscent = parameters.blurAscent;
    }
    if (parameters.groundLevel !== undefined) {
      this.parameters.groundLevel = parameters.groundLevel;
    }
    if (parameters.renderTargetDownScale !== undefined) {
      this.parameters.renderTargetDownScale = parameters.renderTargetDownScale;
    }
  }
  private updateCopyMaterial(renderTarget: WebGLRenderTarget | null) {
    const intensity = this.parameters.intensity;
    const brightness = this.parameters.brightness;
    this._copyMaterial.update({
      texture: renderTarget?.texture ?? undefined,
      colorTransform: new Matrix4().set(
        brightness,
        0,
        0,
        0,
        0,
        brightness,
        0,
        0,
        0,
        0,
        brightness,
        0,
        0,
        0,
        0,
        intensity
      ),
      multiplyChannels: 0,
      uvTransform: CopyTransformMaterial.flipYuvTransform,
    });
    this._copyMaterial.depthTest = true;
    this._copyMaterial.depthWrite = false;
  }

  public render(renderer: WebGLRenderer, scene: Scene, camera: Camera): void {
    if (!this.parameters.enabled || !(camera instanceof PerspectiveCamera)) {
      return;
    }
    this.renderGroundShadow(renderer, scene, camera);
    const groundReflectionCamera = this.createGroundReflectionCamera(camera);
    this.renderGroundReflection(
      renderer,
      scene,
      groundReflectionCamera,
      this.reflectionRenderTarget
    );
    this.renderGroundReflectionIntensity(
      renderer,
      groundReflectionCamera,
      this.intensityRenderTarget
    );
    if (
      this.parameters.blurHorizontal > 0 ||
      this.parameters.blurVertical > 0
    ) {
      this.blurReflection(renderer, camera, [
        this.intensityRenderTarget,
        this.blurRenderTarget,
        this.intensityRenderTarget,
      ]);
    }
    this.updateCopyMaterial(this.intensityRenderTarget);
    this.renderPass.renderScreenSpace(
      renderer,
      this._copyMaterial,
      renderer.getRenderTarget()
    );
  }

  private renderGroundShadow(
    renderer: WebGLRenderer,
    scene: Scene,
    camera: Camera
  ) {
    this.renderOverrideVisibility.render(scene, () => {
      this.renderPass.renderWithOverrideMaterial(
        renderer,
        scene,
        camera,
        this.groundShadowMaterial.update({
          groundLevel: this.parameters.groundLevel,
        }),
        this.shadowRenderTarget
      );
    });
  }

  private renderGroundReflection(
    renderer: WebGLRenderer,
    scene: Scene,
    groundReflectionCamera: Camera,
    renderTarget: WebGLRenderTarget | undefined
  ) {
    const renderTargetBackup = renderer.getRenderTarget();
    if (renderTarget) {
      renderer.setRenderTarget(renderTarget);
    }
    renderer.render(scene, groundReflectionCamera);
    if (renderTarget) {
      renderer.setRenderTarget(renderTargetBackup);
    }
  }

  private renderGroundReflectionIntensity(
    renderer: WebGLRenderer,
    groundReflectionCamera: Camera,
    renderTarget: WebGLRenderTarget
  ) {
    const renderTargetBackup = renderer.getRenderTarget();
    renderer.setRenderTarget(renderTarget);
    this.renderPass.renderScreenSpace(
      renderer,
      this.reflectionIntensityMaterial.update({
        texture: this.reflectionRenderTarget.texture,
        depthTexture: this.reflectionRenderTarget.depthTexture,
        shadowTexture: this.shadowRenderTarget.texture,
        camera: groundReflectionCamera,
        groundLevel: this.parameters.groundLevel,
        fadeOut: this.parameters.fadeOut,
      }),
      renderer.getRenderTarget()
    );
    renderer.setRenderTarget(renderTargetBackup);
  }

  public blurReflection(
    renderer: WebGLRenderer,
    camera: Camera,
    renderTargets: WebGLRenderTarget[]
  ): void {
    const cameraUpVector = new Vector3(
      camera.matrixWorld.elements[4],
      camera.matrixWorld.elements[5],
      camera.matrixWorld.elements[6]
    );
    const blurHorMin = this.parameters.blurHorizontal / this.width;
    const blurVerMin =
      (this.parameters.blurVertical / this.height) *
      Math.abs(cameraUpVector.dot(new Vector3(0, 0, 1)));
    const renderTargetBackup = renderer.getRenderTarget();
    this.blurMaterial.uniforms.rangeMin.value.x = blurHorMin;
    this.blurMaterial.uniforms.rangeMin.value.y = 0;
    this.blurMaterial.uniforms.rangeMax.value.x =
      blurHorMin * (1 + this.parameters.blurAscent);
    this.blurMaterial.uniforms.rangeMax.value.y = 0;
    this.blurMaterial.uniforms.tDiffuse.value = renderTargets[0].texture;
    renderer.setRenderTarget(renderTargets[1]);
    this.renderPass.renderScreenSpace(
      renderer,
      this.blurMaterial,
      renderer.getRenderTarget()
    );
    this.blurMaterial.uniforms.rangeMin.value.x = 0;
    this.blurMaterial.uniforms.rangeMin.value.y = blurVerMin;
    this.blurMaterial.uniforms.rangeMax.value.x = 0;
    this.blurMaterial.uniforms.rangeMax.value.y =
      blurVerMin * (1 + this.parameters.blurAscent);
    this.blurMaterial.uniforms.tDiffuse.value = renderTargets[1].texture;
    renderer.setRenderTarget(renderTargets[2]);
    this.renderPass.renderScreenSpace(
      renderer,
      this.blurMaterial,
      renderer.getRenderTarget()
    );
    renderer.setRenderTarget(renderTargetBackup);
  }

  private createGroundReflectionCamera(camera: Camera): Camera {
    const groundReflectionCamera = camera.clone() as PerspectiveCamera;
    groundReflectionCamera.position.set(
      camera.position.x,
      -camera.position.y + 2 * this.parameters.groundLevel,
      camera.position.z
    );
    //groundReflectionCamera.lookAt(0, 2 * groundLevel, 0);
    groundReflectionCamera.rotation.set(
      -camera.rotation.x,
      camera.rotation.y,
      -camera.rotation.z
    );
    //groundReflectionCamera.scale.set(1, -1, 1);
    groundReflectionCamera.updateMatrixWorld();
    groundReflectionCamera.updateProjectionMatrix();
    return groundReflectionCamera;
  }
}

const glslGroundReflectionIntensityVertexShader = `
  varying vec2 vUv;
  void main() {
      vUv = uv;
      gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
  }`;

const glslGroundReflectionIntensityFragmentShader = `
  uniform sampler2D tDiffuse;
  uniform sampler2D tDepth;
  uniform sampler2D tShadow;
  uniform vec2 resolution;
  uniform float cameraNear;
  uniform float cameraFar;
  uniform mat4 cameraProjectionMatrix;
  uniform mat4 cameraInverseProjectionMatrix;
  uniform mat4 inverseViewMatrix;
  uniform float groundLevel;
  uniform float fadeOut;
  varying vec2 vUv;

  #include <packing>

  float getDepth(const in vec2 screenPosition) {
    return texture2D(tDepth, screenPosition).x;
  }

  float getLinearDepth(const in vec2 screenPosition) {
    #if PERSPECTIVE_CAMERA == 1
        float fragCoordZ = texture2D(tDepth, screenPosition).x;
        float viewZ = perspectiveDepthToViewZ(fragCoordZ, cameraNear, cameraFar);
        return viewZToOrthographicDepth(viewZ, cameraNear, cameraFar);
    #else
        return texture2D(tDepth, screenPosition).x;
    #endif
  }

  float getViewZ(const in float depth) {
    #if PERSPECTIVE_CAMERA == 1
        return perspectiveDepthToViewZ(depth, cameraNear, cameraFar);
    #else
        return 0.0;//orthographicDepthToViewZ(depth, cameraNear, cameraFar);
    #endif
  }

  vec3 getViewPosition(const in vec2 screenPosition, const in float depth, const in float viewZ ) {
    float clipW = cameraProjectionMatrix[2][3] * viewZ + cameraProjectionMatrix[3][3];
    vec4 clipPosition = vec4((vec3(screenPosition, depth) - 0.5) * 2.0, 1.0);
    clipPosition *= clipW;
    return (cameraInverseProjectionMatrix * clipPosition).xyz;
  }

  void main() {
    float shadow = texture2D(tShadow, vec2(vUv.x, 1.0 - vUv.y)).r;
    shadow = pow(shadow, 8.0);
    float depth = getDepth(vUv);
    float viewZ = getViewZ(depth);
    vec4 worldPosition = inverseViewMatrix * vec4(getViewPosition(vUv, depth, viewZ), 1.0);
    float distance = worldPosition.y - groundLevel;
    vec4 fragColor = texture2D(tDiffuse, vUv).rgba;
    //fragColor.a *= clamp(1.0 - distance * fadeOut, 0.0, 1.0);
    float alpha = max(fragColor.a * clamp(1.0 - distance * fadeOut, 0.0, 1.0), 1.0 - shadow);
    fragColor.a = alpha;
    //fragColor.rgb *= clamp(distance * 3.0, 0.0, 1.0) * shadow;
    fragColor.rgb *= shadow;
    gl_FragColor = (depth < 0.9999 || shadow < 0.99) ? fragColor : vec4(0.0);
  }`;

export class GroundReflectionIntensityMaterial extends ShaderMaterial {
  private static shader: any = {
    uniforms: {
      tDiffuse: { value: null as Texture | null },
      tDepth: { value: null as Texture | null },
      tShadow: { value: null as Texture | null },
      resolution: { value: new Vector2() },
      cameraNear: { value: 0.1 },
      cameraFar: { value: 1 },
      cameraProjectionMatrix: { value: new Matrix4() },
      cameraInverseProjectionMatrix: { value: new Matrix4() },
      inverseViewMatrix: { value: new Matrix4() },
      groundLevel: { value: 0 },
      fadeOut: { value: 1 },
    },
    defines: {
      PERSPECTIVE_CAMERA: 1,
    },
    vertexShader: glslGroundReflectionIntensityVertexShader,
    fragmentShader: glslGroundReflectionIntensityFragmentShader,
  };

  constructor(parameters?: any) {
    super({
      defines: Object.assign(
        {},
        GroundReflectionIntensityMaterial.shader.defines
      ),
      uniforms: UniformsUtils.clone(
        GroundReflectionIntensityMaterial.shader.uniforms
      ),
      vertexShader: GroundReflectionIntensityMaterial.shader.vertexShader,
      fragmentShader: GroundReflectionIntensityMaterial.shader.fragmentShader,
      blending: NoBlending,
    });
    this.update(parameters);
  }

  public update(parameters?: any): GroundReflectionIntensityMaterial {
    if (parameters?.texture !== undefined) {
      this.uniforms.tDiffuse.value = parameters?.texture;
    }
    if (parameters?.depthTexture !== undefined) {
      this.uniforms.tDepth.value = parameters?.depthTexture;
    }
    if (parameters?.shadowTexture !== undefined) {
      this.uniforms.tShadow.value = parameters?.shadowTexture;
    }
    if (parameters?.camera !== undefined) {
      const camera =
        (parameters?.camera as OrthographicCamera) ||
        (parameters?.camera as PerspectiveCamera);
      this.uniforms.cameraNear.value = camera.near;
      this.uniforms.cameraFar.value = camera.far;
      this.uniforms.cameraProjectionMatrix.value.copy(camera.projectionMatrix);
      this.uniforms.cameraInverseProjectionMatrix.value.copy(
        camera.projectionMatrixInverse
      );
      this.uniforms.inverseViewMatrix.value.copy(camera.matrixWorld);
    }
    if (parameters?.groundLevel !== undefined) {
      this.uniforms.groundLevel.value = parameters?.groundLevel;
    }
    if (parameters?.fadeOut !== undefined) {
      this.uniforms.fadeOut.value = parameters?.fadeOut;
    }
    return this;
  }
}

const glslGroundShadowVertexShader = `uniform float groundLevel;
  varying float vHeight;

  void main() {
      vec4 modelPosition = modelMatrix * vec4(position, 1.0);
      vHeight = modelPosition.y - groundLevel;
      modelPosition.y = groundLevel;
      gl_Position = projectionMatrix * viewMatrix * modelPosition;
      //gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
  }`;

const glslGroundShadowFragmentShader = `varying float vHeight;

  void main() {
    float intensity = clamp(vHeight / 3.0, 0.0, 1.0);
    vec4 fragColor = vec4(vec3(intensity), 1.0);
    //fragColor = vec4(1.0, intensity, 0.0, 1.0);
    gl_FragColor = gl_FragCoord.z < 0.9999 ? fragColor : vec4(0.0);
  }`;

export class GroundShadowMaterial extends ShaderMaterial {
  private static shader: any = {
    uniforms: {
      groundLevel: { value: 0 },
    },
    vertexShader: glslGroundShadowVertexShader,
    fragmentShader: glslGroundShadowFragmentShader,
  };

  constructor(parameters?: any) {
    super({
      defines: Object.assign({}, GroundShadowMaterial.shader.defines),
      uniforms: UniformsUtils.clone(GroundShadowMaterial.shader.uniforms),
      vertexShader: GroundShadowMaterial.shader.vertexShader,
      fragmentShader: GroundShadowMaterial.shader.fragmentShader,
      transparent: true,
      blending: CustomBlending,
      blendEquation: MinEquation,
      blendEquationAlpha: MinEquation,
      blendSrc: OneFactor,
      blendSrcAlpha: OneFactor,
      blendDst: OneFactor,
      blendDstAlpha: OneFactor,
    });
    this.update(parameters);
  }

  public update(parameters?: any): GroundReflectionIntensityMaterial {
    if (parameters?.groundLevel !== undefined) {
      this.uniforms.groundLevel.value = parameters?.groundLevel;
    }
    return this;
  }
}

export const BlurGroundReflectionShadowShader = {
  uniforms: {
    tDiffuse: { value: null as Texture | null },
    rangeMin: { value: new Vector2(1.0 / 512.0, 1.0 / 512.0) },
    rangeMax: { value: new Vector2(1.0 / 512.0, 1.0 / 512.0) },
    shadowScale: { value: 1 },
  },
  vertexShader: `
    varying vec2 vUv;
    void main() {
        vUv = uv;
        gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
    }`,
  fragmentShader: `
    uniform sampler2D tDiffuse;
    uniform vec2 rangeMin;
    uniform vec2 rangeMax;
    varying vec2 vUv;
  
    void main() {
        vec4 baseColor = texture2D(tDiffuse, vUv);
        vec2 blur = mix(rangeMax, rangeMin, baseColor.a);
        vec4 sum = vec4(0.0);
        sum += texture2D(tDiffuse, vUv - 4.0 * blur) * 0.051;
        sum += texture2D(tDiffuse, vUv - 3.0 * blur) * 0.0918;
        sum += texture2D(tDiffuse, vUv - 2.0 * blur) * 0.12245;
        sum += texture2D(tDiffuse, vUv - 1.0 * blur) * 0.1531;
        sum += baseColor * 0.1633;
        sum += texture2D(tDiffuse, vUv + 1.0 * blur) * 0.1531;
        sum += texture2D(tDiffuse, vUv + 2.0 * blur) * 0.12245;
        sum += texture2D(tDiffuse, vUv + 3.0 * blur) * 0.0918;
        sum += texture2D(tDiffuse, vUv + 4.0 * blur) * 0.051;
        gl_FragColor = sum;
    }`,
};
