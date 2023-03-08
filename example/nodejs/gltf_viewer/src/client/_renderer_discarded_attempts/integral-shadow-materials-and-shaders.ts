import {
  NoBlending,
  Matrix4,
  OrthographicCamera,
  PerspectiveCamera,
  ShaderMaterial,
  Texture,
  UniformsUtils,
  Vector2,
} from 'three';

const glslIntegralShadowVertexShader = `varying vec2 vUv;
  void main() {
    vUv = uv;
    gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
  }`;

const glslIntegralShadowFragmentShader = `uniform sampler2D tShadow;
  uniform sampler2D tNormal;
  uniform sampler2D tDepth;
  uniform sampler2D tNoise;
  uniform vec3 shKernel[ KERNEL_SIZE ];
  uniform vec2 resolution;
  uniform float cameraNear;
  uniform float cameraFar;
  uniform mat4 cameraProjectionMatrix;
  uniform mat4 cameraInverseProjectionMatrix;
  uniform float shKernelRadius;
  
  varying vec2 vUv;
  
  #include <packing>
  
  float getDepth( const in vec2 screenPosition ) {
    return texture2D( tDepth, screenPosition ).x;
  }
  
  float accuratePerspectiveDepthToViewZ(float depth, float n, float f) {
    float z_ndc = 2.0 * depth - 1.0;
    float z_eye = 2.0 * n * f / (f + n - z_ndc * (f - n));
    return z_eye;
  }
  
  float getLinearDepth( const in vec2 screenPosition ) {
    #if PERSPECTIVE_CAMERA == 1
        float fragCoordZ = texture2D( tDepth, screenPosition ).x;
        float viewZ = perspectiveDepthToViewZ( fragCoordZ, cameraNear, cameraFar );
        return viewZToOrthographicDepth( viewZ, cameraNear, cameraFar );
    #else
        return texture2D( tDepth, screenPosition ).x;
    #endif
  }
  
  float getViewZ( const in float depth ) {
    #if PERSPECTIVE_CAMERA == 1
        return perspectiveDepthToViewZ( depth, cameraNear, cameraFar );
    #else
        return orthographicDepthToViewZ( depth, cameraNear, cameraFar );
    #endif
  }
  
  vec3 getViewPosition( const in vec2 screenPosition, const in float depth, const in float viewZ ) {
    float clipW = cameraProjectionMatrix[2][3] * viewZ + cameraProjectionMatrix[3][3];
    vec4 clipPosition = vec4( ( vec3( screenPosition, depth ) - 0.5 ) * 2.0, 1.0 );
    clipPosition *= clipW; // unprojection.
    return ( cameraInverseProjectionMatrix * clipPosition ).xyz;
  }
  
  vec3 getViewNormal( const in vec2 screenPosition ) {
    return unpackRGBToNormal( texture2D( tNormal, screenPosition ).xyz );
  }
  
  void main() {
  
    float depth = getDepth(vUv);
    float viewZ = getViewZ(depth);
  
    vec3 viewPosition = getViewPosition(vUv, depth, viewZ);
    vec3 viewNormal = getViewNormal(vUv);
  
    vec2 noiseScale = resolution.xy / 4.0;
    vec3 random = texture2D(tNoise, vUv * noiseScale).rrr;
  
    // compute matrix used to reorient a kernel vector
    vec3 tangent = normalize(random - viewNormal * dot(random, viewNormal));
    vec3 bitangent = cross(viewNormal, tangent);
    mat3 kernelMatrix = mat3(tangent, bitangent, viewNormal);
  
    float shOcclusion = texture2D(tShadow, vUv).r;
    float shSamples = 0.0;
    for (int i = 0; i < KERNEL_SIZE; i ++) {
  
        vec3 shSampleVector = kernelMatrix * shKernel[i]; // reorient sample vector in view space
        vec3 shSamplePoint = viewPosition + shSampleVector * shKernelRadius; // calculate sample point
        vec4 shSamplePointNDC = cameraProjectionMatrix * vec4(shSamplePoint, 1.0); // project point and calculate NDC
        shSamplePointNDC /= shSamplePointNDC.w;
        vec2 shSamplePointUv = shSamplePointNDC.xy * 0.5 + 0.5; // compute uv coordinates
        float shRealDepth = getLinearDepth(shSamplePointUv); // get linear depth from depth texture
        float shSampleDepth = viewZToOrthographicDepth(shSamplePoint.z, cameraNear, cameraFar); // compute linear depth of the sample view Z value
  
        float shDepthDelta = shSampleDepth - shRealDepth;
        float shDeltaDistance = shDepthDelta * (cameraFar - cameraNear);
        float w = step(abs(shDeltaDistance), shKernelRadius);
        shSamples += w;
        shOcclusion += texture2D(tShadow, shSamplePointUv).r * w;
    }
  
    shOcclusion = clamp(shOcclusion / (shSamples + 1.0), 0.0, 1.0);
    gl_FragColor = vec4(vec3(shOcclusion), 1.0);
  }`;

export class IntegralShadowRenderMaterial extends ShaderMaterial {
  public static kernelSize: number = 64;
  private static shader: any = {
    uniforms: {
      tShadow: { value: null as Texture | null },
      tNormal: { value: null as Texture | null },
      tDepth: { value: null as Texture | null },
      tNoise: { value: null as Texture | null },
      shKernel: { value: null },
      cameraNear: { value: 0.1 },
      cameraFar: { value: 1 },
      resolution: { value: new Vector2() },
      cameraProjectionMatrix: { value: new Matrix4() },
      cameraInverseProjectionMatrix: { value: new Matrix4() },
      shKernelRadius: { value: 0.1 },
    },
    defines: {
      PERSPECTIVE_CAMERA: 1,
      KERNEL_SIZE: IntegralShadowRenderMaterial.kernelSize,
    },
    vertexShader: glslIntegralShadowVertexShader,
    fragmentShader: glslIntegralShadowFragmentShader,
  };

  constructor(parameters?: any) {
    super({
      defines: Object.assign({}, IntegralShadowRenderMaterial.shader.defines),
      uniforms: UniformsUtils.clone(
        IntegralShadowRenderMaterial.shader.uniforms
      ),
      vertexShader: IntegralShadowRenderMaterial.shader.vertexShader,
      fragmentShader: IntegralShadowRenderMaterial.shader.fragmentShader,
      blending: NoBlending,
    });
    this.update(parameters);
  }

  public update(parameters?: any): IntegralShadowRenderMaterial {
    if (parameters?.normalTexture !== undefined) {
      this.uniforms.tNormal.value = parameters?.normalTexture;
    }
    if (parameters?.depthTexture !== undefined) {
      this.uniforms.tDepth.value = parameters?.depthTexture;
    }
    if (parameters?.noiseTexture !== undefined) {
      this.uniforms.tNoise.value = parameters?.noiseTexture;
    }
    if (parameters?.kernel !== undefined) {
      this.uniforms.shKernel.value = parameters?.kernel;
    }
    if (parameters?.width || parameters?.height) {
      const width = parameters?.width ?? this.uniforms.resolution.value.x;
      const height = parameters?.width ?? this.uniforms.resolution.value.y;
      this.uniforms.resolution.value.set(width, height);
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
    }
    this.updateSettings(parameters);
    return this;
  }

  private updateSettings(parameters?: any) {
    if (parameters?.texture !== undefined) {
      this.uniforms.tShadow.value = parameters?.texture;
    }
    if (parameters?.kernelRadius !== undefined) {
      this.uniforms.shKernelRadius.value = parameters?.kernelRadius;
    }
  }
}

const glslIntegralShadowBlurVertexShader = `varying vec2 vUv;
  void main() {
    vUv = uv;
    gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
  }`;

const glslIntegralShadowBlurFragmentShader = `uniform sampler2D tShadow;
  uniform sampler2D tDepth;
  uniform vec2 resolution;
  uniform float cameraNear;
  uniform float cameraFar;
  uniform mat4 cameraProjectionMatrix;
  uniform mat4 cameraInverseProjectionMatrix;
  uniform float shKernelRadius;
  
  varying vec2 vUv;
  
  #include <packing>
  
  float getDepth(const in vec2 screenPosition) {
    return texture2D(tDepth, screenPosition).x;
  }
  
  float accuratePerspectiveDepthToViewZ(float depth, float n, float f) {
    float z_ndc = 2.0 * depth - 1.0;
    float z_eye = 2.0 * n * f / (f + n - z_ndc * (f - n));
    return z_eye;
  }
  
  float getLinearDepth( const in vec2 screenPosition ) {
    #if PERSPECTIVE_CAMERA == 1
        float fragCoordZ = texture2D( tDepth, screenPosition ).x;
        float viewZ = perspectiveDepthToViewZ( fragCoordZ, cameraNear, cameraFar );
        return viewZToOrthographicDepth( viewZ, cameraNear, cameraFar );
    #else
        return texture2D( tDepth, screenPosition ).x;
    #endif
  }
  
  float getViewZ( const in float depth ) {
    #if PERSPECTIVE_CAMERA == 1
        return perspectiveDepthToViewZ( depth, cameraNear, cameraFar );
    #else
        return orthographicDepthToViewZ( depth, cameraNear, cameraFar );
    #endif
  }
  
  void main() {
    vec2 texelSize = 1.0 / resolution;
    float shResult = 0.0;
  #if OPTIMIZED_BLUR == 1
    result += texture2D(tShadow, vUv + texelSize * vec2(-1.0, -1.0)).r;
    result += texture2D(tShadow, vUv + texelSize * vec2(-1.0, 1.0)).r;
    result += texture2D(tShadow, vUv + texelSize * vec2(1.0, -1.0)).r;
    result += texture2D(tShadow, vUv + texelSize * vec2(1.0, 1.0)).r;
    gl_FragColor = vec4(vec3(result / 4.0), 1.0);
  #else
    float referenceDepth = getLinearDepth(vUv);
    float shSamples = 0.0;
    for (int i = -2; i <= 2; i++) {
        for (int j = -2; j <= 2; j++) {
            vec2 offset = vec2(float(i), float(j)) * texelSize;
            float shSampleDepth = getLinearDepth(vUv + offset);
            float shDepthDelta = shSampleDepth - referenceDepth;
            float shDeltaDistance = shDepthDelta * (cameraFar - cameraNear);
            float shW = step(abs(shDeltaDistance), shKernelRadius);
            shSamples += shW;
            shResult += texture2D(tShadow, vUv + offset).r * shW;
        }
    }
    gl_FragColor = vec4(vec3(shResult / shSamples), 1.0);
  #endif
  }`;

export class IntegralShadowBlurMaterial extends ShaderMaterial {
  public static optimized: boolean = false;
  private static shader: any = {
    uniforms: {
      tShadow: { value: null as Texture | null },
      tDepth: { value: null as Texture | null },
      cameraNear: { value: 0.1 },
      cameraFar: { value: 1 },
      resolution: { value: new Vector2() },
      cameraProjectionMatrix: { value: new Matrix4() },
      cameraInverseProjectionMatrix: { value: new Matrix4() },
      shKernelRadius: { value: 0.1 },
    },
    defines: {
      PERSPECTIVE_CAMERA: 1,
      OPTIMIZED_BLUR: IntegralShadowBlurMaterial.optimized ? 1 : 0,
    },
    vertexShader: glslIntegralShadowBlurVertexShader,
    fragmentShader: glslIntegralShadowBlurFragmentShader,
  };

  constructor(parameters?: any) {
    super({
      defines: Object.assign({}, IntegralShadowBlurMaterial.shader.defines),
      uniforms: UniformsUtils.clone(IntegralShadowBlurMaterial.shader.uniforms),
      vertexShader: IntegralShadowBlurMaterial.shader.vertexShader,
      fragmentShader: IntegralShadowBlurMaterial.shader.fragmentShader,
      blending: NoBlending,
    });
    this.update(parameters);
  }

  public update(parameters?: any): IntegralShadowBlurMaterial {
    if (parameters?.depthTexture !== undefined) {
      this.uniforms.tDepth.value = parameters?.depthTexture;
    }
    if (parameters?.texture !== undefined) {
      this.uniforms.tShadow.value = parameters?.texture;
    }
    if (parameters?.width || parameters?.height) {
      const width = parameters?.width ?? this.uniforms.resolution.value.x;
      const height = parameters?.height ?? this.uniforms.resolution.value.y;
      this.uniforms.resolution.value.set(width, height);
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
    }
    this.updateSettings(parameters);
    return this;
  }

  private updateSettings(parameters?: any) {
    if (parameters?.kernelRadius !== undefined) {
      this.uniforms.shKernelRadius.value = parameters?.kernelRadius;
    }
  }
}
