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

const glslShadowAndAoVertexShader = `varying vec2 vUv;
  void main() {
      vUv = uv;
      gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
  }`;

const glslShadowAndAoFragmentShader = `uniform sampler2D tShadow;
  uniform sampler2D tNormal;
  uniform sampler2D tDepth;
  uniform sampler2D tNoise;
  uniform vec2 resolution;
  uniform float cameraNear;
  uniform float cameraFar;
  uniform mat4 cameraProjectionMatrix;
  uniform mat4 cameraInverseProjectionMatrix;
  uniform vec3 aoKernel[KERNEL_SIZE];
  uniform float aoKernelRadius;
  uniform float aoDepthBias; // avoid artifacts caused by neighbour fragments with minimal depth difference
  uniform float aoMaxDistance; // avoid the influence of fragments which are too far away
  uniform float aoMaxDepth;
  uniform float aoIntensity;
  uniform float aoFadeout;
  uniform vec3 shKernel[KERNEL_SIZE];
  uniform float shKernelRadius;
  
  varying vec2 vUv;
  
  #include <packing>
  
  float getDepth(const in vec2 screenPosition) {
      return texture2D(tDepth, screenPosition).x;
  }
  
  float getViewZ(const in float depth) {
      #if PERSPECTIVE_CAMERA == 1
          return perspectiveDepthToViewZ(depth, cameraNear, cameraFar);
      #else
          return orthographicDepthToViewZ(depth, cameraNear, cameraFar);
      #endif
  }
  
  vec3 getViewPosition(const in vec2 screenPosition, const in float depth, const in float viewZ) {
      float clipW = cameraProjectionMatrix[2][3] * viewZ + cameraProjectionMatrix[3][3];
      vec4 clipPosition = vec4( ( vec3( screenPosition, depth ) - 0.5 ) * 2.0, 1.0 );
      clipPosition *= clipW; // unprojection.
      return ( cameraInverseProjectionMatrix * clipPosition ).xyz;
  }
  
  vec3 getViewNormal(const in vec2 screenPosition) {
      return unpackRGBToNormal(texture2D(tNormal, screenPosition).xyz);
  }
  
  void main() {
  
      float depth = getDepth(vUv);
      float viewZ = getViewZ(depth);
  
      vec3 viewPosition = getViewPosition(vUv, depth, viewZ);
      vec3 viewNormal = getViewNormal(vUv);
  
      vec2 noiseScale = resolution.xy / 4.0;
      vec3 random = texture2D(tNoise, vUv * noiseScale).xyz * 2.0 - 1.0;
  
      // compute matrix used to reorient a kernel vector
      vec3 tangent = normalize(random - viewNormal * dot(random, viewNormal));
      vec3 bitangent = cross(viewNormal, tangent);
      mat3 kernelMatrix = mat3(tangent, bitangent, viewNormal);
  
      float aoOcclusion = 0.0;
      float shOcclusion = texture2D(tShadow, vUv).r;
      float shSamples = 0.0;
      for (int i = 0; i < KERNEL_SIZE; i ++) {
          if (aoIntensity > 0.01) {
              vec3 aoSampleVector = kernelMatrix * aoKernel[i]; 
              vec3 aoSamplePoint = viewPosition + aoSampleVector * aoKernelRadius; 
              vec4 aoSamplePointNDC = cameraProjectionMatrix * vec4(aoSamplePoint, 1.0); 
              aoSamplePointNDC /= aoSamplePointNDC.w;
              vec2 aoSamplePointUv = aoSamplePointNDC.xy * 0.5 + 0.5;
              float aoRealSampleDepth = getDepth(aoSamplePointUv);
              float aoSampleDeltaZ = getViewZ(aoRealSampleDepth) - aoSamplePoint.z;
              float aoRealDeltaZ =  viewZ - aoSamplePoint.z;
              float w_long = clamp((aoMaxDistance - max(aoRealDeltaZ, aoSampleDeltaZ)) / aoMaxDistance, 0.0, 1.0);
              float w_lat = clamp(1.0 - length(aoSampleVector.xy), 0.0, 1.0);
              aoOcclusion += 
                  step(aoRealSampleDepth, aoMaxDepth) * 
                  step(aoDepthBias, aoSampleDeltaZ / (cameraFar - cameraNear)) * 
                  step(aoSampleDeltaZ, aoMaxDistance) * mix(1.0, w_long * w_lat, aoFadeout);
          }
          
          vec3 shSampleVector = kernelMatrix * shKernel[i]; // reorient sample vector in view space
          vec3 shSamplePoint = viewPosition + shSampleVector * shKernelRadius; // calculate sample point
          vec4 shSamplePointNDC = cameraProjectionMatrix * vec4(shSamplePoint, 1.0); // project point and calculate NDC
          shSamplePointNDC /= shSamplePointNDC.w;
          vec2 shSamplePointUv = shSamplePointNDC.xy * 0.5 + 0.5; // compute uv coordinates
          vec3 shSampleNormal = getViewNormal(shSamplePointUv);
          float shDeltaZ = getViewZ(getDepth(shSamplePointUv)) - shSamplePoint.z;
          float w = step(abs(shDeltaZ), shKernelRadius) * max(0.0, dot(shSampleNormal, viewNormal));
          shSamples += w;
          shOcclusion += texture2D(tShadow, shSamplePointUv).r * w;
      }
  
      aoOcclusion = clamp(aoOcclusion / float(KERNEL_SIZE) * (1.0 + aoFadeout), 0.0, 1.0);
      shOcclusion = clamp(shOcclusion / (shSamples + 1.0), 0.0, 1.0);
      gl_FragColor = vec4(clamp(1.0 - aoOcclusion, 0.0, 1.0), shOcclusion, 0.0, 1.0);
  }`;

export class ShadowAndAoRenderMaterial extends ShaderMaterial {
  public static kernelSize: number = 64;
  private static shader: any = {
    uniforms: {
      tShadow: { value: null as Texture | null },
      tNormal: { value: null as Texture | null },
      tDepth: { value: null as Texture | null },
      tNoise: { value: null as Texture | null },
      cameraNear: { value: 0.1 },
      cameraFar: { value: 1 },
      resolution: { value: new Vector2() },
      cameraProjectionMatrix: { value: new Matrix4() },
      cameraInverseProjectionMatrix: { value: new Matrix4() },
      aoKernel: { value: null },
      aoKernelRadius: { value: 0.1 },
      aoDepthBias: { value: 0.002 },
      aoMaxDistance: { value: 0.05 },
      aoMaxDepth: { value: 0.99 },
      aoIntensity: { value: 1.0 },
      aoFadeout: { value: 0.0 },
      shKernel: { value: null },
      shKernelRadius: { value: 0.15 },
    },
    defines: {
      PERSPECTIVE_CAMERA: 1,
      KERNEL_SIZE: ShadowAndAoRenderMaterial.kernelSize,
    },
    vertexShader: glslShadowAndAoVertexShader,
    fragmentShader: glslShadowAndAoFragmentShader,
  };

  constructor(parameters?: any) {
    super({
      defines: Object.assign({}, ShadowAndAoRenderMaterial.shader.defines),
      uniforms: UniformsUtils.clone(ShadowAndAoRenderMaterial.shader.uniforms),
      vertexShader: ShadowAndAoRenderMaterial.shader.vertexShader,
      fragmentShader: ShadowAndAoRenderMaterial.shader.fragmentShader,
      blending: NoBlending,
    });
    this.update(parameters);
  }

  public update(parameters?: any): ShadowAndAoRenderMaterial {
    if (parameters?.shadowTexture !== undefined) {
      this.uniforms.tShadow.value = parameters?.shadowTexture;
    }
    if (parameters?.normalTexture !== undefined) {
      this.uniforms.tNormal.value = parameters?.normalTexture;
    }
    if (parameters?.depthTexture !== undefined) {
      this.uniforms.tDepth.value = parameters?.depthTexture;
    }
    if (parameters?.noiseTexture !== undefined) {
      this.uniforms.tNoise.value = parameters?.noiseTexture;
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
    if (parameters?.aoKernel !== undefined) {
      this.uniforms.aoKernel.value = parameters?.aoKernel;
    }
    if (parameters?.shKernel !== undefined) {
      this.uniforms.shKernel.value = parameters?.shKernel;
    }
    this.updateSettings(parameters);
    return this;
  }

  private updateSettings(parameters?: any) {
    if (parameters?.aoKernelRadius !== undefined) {
      this.uniforms.aoKernelRadius.value = parameters?.aoKernelRadius;
    }
    if (parameters?.aoDepthBias !== undefined) {
      this.uniforms.aoDepthBias.value = parameters?.aoDepthBias;
    }
    if (parameters?.aoMaxDistance !== undefined) {
      this.uniforms.aoMaxDistance.value = parameters?.aoMaxDistance;
    }
    if (parameters?.aoMaxDepth !== undefined) {
      this.uniforms.aoMaxDepth.value = parameters?.aoMaxDepth;
    }
    if (parameters?.aoIntensity !== undefined) {
      this.uniforms.aoIntensity.value = parameters?.aoIntensity;
    }
    if (parameters?.aoFadeout !== undefined) {
      this.uniforms.aoFadeout.value = parameters?.aoFadeout;
    }
    if (parameters?.shadowRadius !== undefined) {
      this.uniforms.shKernelRadius.value = parameters?.shadowRadius;
    }
  }
}

const glslShadowAndAoBlurVertexShader = `varying vec2 vUv;
  void main() {
      vUv = uv;
      gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
  }`;

const glslShadowAndAoBlurFragmentShader = `uniform sampler2D tDiffuse;
  uniform sampler2D tDepth;
  uniform vec2 resolution;
  uniform float cameraNear;
  uniform float cameraFar;
  uniform mat4 cameraProjectionMatrix;
  uniform mat4 cameraInverseProjectionMatrix;
  uniform float aoKernelRadius;
  uniform float shKernelRadius;
  
  varying vec2 vUv;
  
  float perspectiveDepthToViewZ(const in float invClipZ, const in float near, const in float far) {
      #if PERSPECTIVE_CAMERA == 1    
          float z_ndc = 2.0 * invClipZ - 1.0;
          float z_eye = 2.0 * near * far / ((far - near) * z_ndc - far - near);
          return z_eye;
      #else
          return (near * far) / ((far - near) * invClipZ - far);
      #endif
  }
  
  float viewZToOrthographicDepth(const in float viewZ, const in float near, const in float far) {
      // viewZ is negative!
      return (viewZ + near) / (near - far);
  }
  
  float getLinearDepth(const in vec2 screenPosition) {
      float fragCoordZ = texture2D(tDepth, screenPosition).x;
      #if PERSPECTIVE_CAMERA == 1
          float viewZ = perspectiveDepthToViewZ(fragCoordZ, cameraNear, cameraFar);
          return viewZToOrthographicDepth(viewZ, cameraNear, cameraFar);
      #else
          return fragCoordZ;
      #endif
  }
  
  void main() {
      vec2 texelSize = 1.0 / resolution ;
      float referenceDepth = getLinearDepth(vUv);
      vec2 result = vec2(0.0);
      vec2 samples = vec2(0.0);
      for (int i = - 2; i <= 2; i++) {
          for (int j = - 2; j <= 2; j++) {
              vec2 offset = vec2(float(i), float(j)) * texelSize;
              float sampleDepth = getLinearDepth(vUv + offset);
              float depthDelta = sampleDepth - referenceDepth;
              float deltaDistance = depthDelta * (cameraFar - cameraNear);
              vec2 w = step(abs(deltaDistance), vec2(aoKernelRadius, shKernelRadius));
              samples += w;
              result += texture2D(tDiffuse, vUv + offset).rg * w;
          }
      }
      gl_FragColor = vec4(result.rg / samples.rg, 0.0, 1.0);
  }`;

export class ShadowAndAoBlurMaterial extends ShaderMaterial {
  private static shader: any = {
    uniforms: {
      tDiffuse: { value: null as Texture | null },
      tDepth: { value: null as Texture | null },
      cameraNear: { value: 0.1 },
      cameraFar: { value: 1 },
      resolution: { value: new Vector2() },
      cameraProjectionMatrix: { value: new Matrix4() },
      cameraInverseProjectionMatrix: { value: new Matrix4() },
      aoKernelRadius: { value: 0.1 },
      shKernelRadius: { value: 0.1 },
    },
    defines: {
      PERSPECTIVE_CAMERA: 1,
      ACCURATE_VIEW_Z: 0,
    },
    vertexShader: glslShadowAndAoBlurVertexShader,
    fragmentShader: glslShadowAndAoBlurFragmentShader,
  };

  constructor(parameters?: any) {
    super({
      defines: Object.assign({}, ShadowAndAoBlurMaterial.shader.defines),
      uniforms: UniformsUtils.clone(ShadowAndAoBlurMaterial.shader.uniforms),
      vertexShader: ShadowAndAoBlurMaterial.shader.vertexShader,
      fragmentShader: ShadowAndAoBlurMaterial.shader.fragmentShader,
      blending: NoBlending,
    });
    this.update(parameters);
  }

  public update(parameters?: any): ShadowAndAoBlurMaterial {
    if (parameters?.depthTexture !== undefined) {
      this.uniforms.tDepth.value = parameters?.depthTexture;
    }
    if (parameters?.texture !== undefined) {
      this.uniforms.tDiffuse.value = parameters?.texture;
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
    if (parameters?.aoKernelRadius !== undefined) {
      this.uniforms.aoKernelRadius.value = parameters?.aoKernelRadius;
    }
    if (parameters?.shadowRadius !== undefined) {
      this.uniforms.shKernelRadius.value = parameters?.shadowRadius;
    }
  }
}
