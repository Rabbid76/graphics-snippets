import {
  AddEquation,
  DstAlphaFactor,
  DstColorFactor,
  Matrix3,
  Matrix4,
  NoBlending,
  OrthographicCamera,
  PerspectiveCamera,
  ShaderMaterial,
  Texture,
  UniformsUtils,
  Vector2,
  Vector4,
  ZeroFactor,
} from 'three';

const CopyTransformShader = {
  uniforms: {
    tDiffuse: { value: null as Texture | null },
    colorTransform: { value: new Matrix4() },
    colorBase: { value: new Vector4(0, 0, 0, 0) },
    multiplyChannels: { value: 0 },
    uvTransform: { value: new Matrix3() },
  },
  vertexShader: `
        varying vec2 vUv;
        uniform mat3 uvTransform;
  
        void main() {
            vUv = (uvTransform * vec3(uv, 1.0)).xy;
            gl_Position = (projectionMatrix * modelViewMatrix * vec4(position, 1.0)).xyww;
        }`,
  fragmentShader: `
        uniform sampler2D tDiffuse;
        uniform mat4 colorTransform;
        uniform vec4 colorBase;
        uniform float multiplyChannels;
        varying vec2 vUv;
  
        void main() {
            vec4 color = colorTransform * texture2D(tDiffuse, vUv) + colorBase;
            color.rgb = mix(color.rgb, vec3(color.r * color.g * color.b), multiplyChannels);
            gl_FragColor = color;
        }`,
};

export const DEFAULT_TRANSFORM: Matrix4 = new Matrix4();
export const RGB_TRANSFORM: Matrix4 = new Matrix4().set(
  // eslint-disable-next-line prettier/prettier
  1, 0, 0, 0,
  // eslint-disable-next-line prettier/prettier
  0, 1, 0, 0,
  // eslint-disable-next-line prettier/prettier
  0, 0, 1, 0,
  // eslint-disable-next-line prettier/prettier
  0, 0, 0, 0
);
export const ALPHA_TRANSFORM: Matrix4 = new Matrix4().set(
  // eslint-disable-next-line prettier/prettier
  0, 0, 0, 1,
  // eslint-disable-next-line prettier/prettier
  0, 0, 0, 1,
  // eslint-disable-next-line prettier/prettier
  0, 0, 0, 1,
  // eslint-disable-next-line prettier/prettier
  0, 0, 0, 0
);
export const RED_TRANSFORM: Matrix4 = new Matrix4().set(
  // eslint-disable-next-line prettier/prettier
  1, 0, 0, 0,
  // eslint-disable-next-line prettier/prettier
  1, 0, 0, 0,
  // eslint-disable-next-line prettier/prettier
  1, 0, 0, 0,
  // eslint-disable-next-line prettier/prettier
  0, 0, 0, 1
);
export const GRAYSCALE_TRANSFORM: Matrix4 = new Matrix4().set(
  // eslint-disable-next-line prettier/prettier
  1, 0, 0, 0,
  // eslint-disable-next-line prettier/prettier
  1, 0, 0, 0,
  // eslint-disable-next-line prettier/prettier
  1, 0, 0, 0,
  // eslint-disable-next-line prettier/prettier
  0, 0, 0, 1
);
export const ZERO_RGBA: Vector4 = new Vector4(0, 0, 0, 0);
export const ALPHA_RGBA: Vector4 = new Vector4(0, 0, 0, 1);
export const DEFAULT_UV_TRANSFORM: Matrix3 = new Matrix3();
export const FLIP_Y_UV_TRANSFORM: Matrix3 = new Matrix3().set(
  // eslint-disable-next-line prettier/prettier
  1, 0, 0,
  // eslint-disable-next-line prettier/prettier
  0, -1, 1,
  // eslint-disable-next-line prettier/prettier
  0, 0, 1
);

export class CopyTransformMaterial extends ShaderMaterial {
  constructor(
    parameters?: Record<string, any>,
    additiveBlending: boolean = true
  ) {
    const blendingParameters = additiveBlending
      ? {
          blendSrc: DstColorFactor,
          blendDst: ZeroFactor,
          blendEquation: AddEquation,
          blendSrcAlpha: DstAlphaFactor,
          blendDstAlpha: ZeroFactor,
          blendEquationAlpha: AddEquation,
        }
      : {};
    super({
      uniforms: UniformsUtils.clone(CopyTransformShader.uniforms),
      vertexShader: CopyTransformShader.vertexShader,
      fragmentShader: CopyTransformShader.fragmentShader,
      transparent: true,
      depthTest: false,
      depthWrite: false,
      ...blendingParameters,
    });
    this.update(parameters);
  }

  update(parameters?: Record<string, any>): CopyTransformMaterial {
    if (parameters?.texture !== undefined) {
      this.uniforms.tDiffuse.value = parameters?.texture;
    }
    if (parameters?.colorTransform !== undefined) {
      this.uniforms.colorTransform.value = parameters?.colorTransform;
    }
    if (parameters?.colorBase !== undefined) {
      this.uniforms.colorBase.value = parameters?.colorBase;
    }
    if (parameters?.multiplyChannels !== undefined) {
      this.uniforms.multiplyChannels.value = parameters?.multiplyChannels;
    }
    if (parameters?.uvTransform !== undefined) {
      this.uniforms.uvTransform.value = parameters?.uvTransform;
    }
    if (parameters?.blending !== undefined) {
      this.blending = parameters?.blending;
    }
    if (parameters?.blendSrc !== undefined) {
      this.blendSrc = parameters?.blendSrc;
    }
    if (parameters?.blendDst !== undefined) {
      this.blendDst = parameters?.blendDst;
    }
    if (parameters?.blendEquation !== undefined) {
      this.blendEquation = parameters?.blendEquation;
    }
    if (parameters?.blendSrcAlpha !== undefined) {
      this.blendSrcAlpha = parameters?.blendSrcAlpha;
    }
    if (parameters?.blendDstAlpha !== undefined) {
      this.blendDstAlpha = parameters?.blendDstAlpha;
    }
    if (parameters?.blendEquationAlpha !== undefined) {
      this.blendEquationAlpha = parameters?.blendEquationAlpha;
    }
    return this;
  }
}

export const BlurContactShadowShader = {
  uniforms: {
    tDiffuse: { value: null as Texture | null },
    rangeMin: { value: new Vector2(1.0 / 512.0, 1.0 / 512.0) },
    rangeMax: { value: new Vector2(1.0 / 512.0, 1.0 / 512.0) },
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
        vec4 sum = vec4( 0.0 );
        sum += texture2D(tDiffuse, vUv - 1.0 * blur) * 0.051;
        sum += texture2D(tDiffuse, vUv - 0.75 * blur) * 0.0918;
        sum += texture2D(tDiffuse, vUv - 0.5 * blur) * 0.12245;
        sum += texture2D(tDiffuse, vUv - 0.25 * blur) * 0.1531;
        sum += baseColor * 0.1633;
        sum += texture2D(tDiffuse, vUv + 0.25 * blur) * 0.1531;
        sum += texture2D(tDiffuse, vUv + 0.5 * blur) * 0.12245;
        sum += texture2D(tDiffuse, vUv + 0.75 * blur) * 0.0918;
        sum += texture2D(tDiffuse, vUv + 1.0 * blur) * 0.051;
        gl_FragColor = sum;
    }`,
};

export const MixShadowShader = {
  uniforms: {
    tShadow1: { value: null as Texture | null },
    tShadow2: { value: null as Texture | null },
    shadowScale1: { value: 0.5 },
    shadowScale2: { value: 0.5 },
  },
  vertexShader: `
    varying vec2 vUv;
    void main() {
        vUv = uv;
        gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
    }`,
  fragmentShader: `
    uniform sampler2D tShadow1;
    uniform sampler2D tShadow2;
    uniform float shadowScale1;
    uniform float shadowScale2;
    varying vec2 vUv;
  
    void main() {
        vec4 color1 = texture2D(tShadow1, vUv);
        vec4 color2 = texture2D(tShadow2, vUv);
        gl_FragColor = color1 * shadowScale1 + color2 * shadowScale2;
    }`,
};

export const HorizontalBlurShadowShader = {
  uniforms: {
    tDiffuse: { value: null as Texture | null },
    blur: { value: 1.0 / 512.0 },
    shadowRange: { value: new Vector2(0.1, 0.9) },
  },
  defines: {
    DEBUG_BLUR_AREA: 0,
  },
  vertexShader: `
    varying vec2 vUv;
    void main() {
        vUv = uv;
        gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
    }`,
  fragmentShader: `
    uniform sampler2D tDiffuse;
    uniform float blur;
    uniform vec2 shadowRange;
    varying vec2 vUv;
  
    void main() {
        vec4 baseColor = texture2D(tDiffuse, vUv);
        float h = blur * step(shadowRange.x, baseColor.r) * step(baseColor.r, shadowRange.y);
        vec4 sum = vec4(0.0);
        sum += texture2D( tDiffuse, vec2( vUv.x - 4.0 * h, vUv.y ) ) * 0.051;
        sum += texture2D( tDiffuse, vec2( vUv.x - 3.0 * h, vUv.y ) ) * 0.0918;
        sum += texture2D( tDiffuse, vec2( vUv.x - 2.0 * h, vUv.y ) ) * 0.12245;
        sum += texture2D( tDiffuse, vec2( vUv.x - 1.0 * h, vUv.y ) ) * 0.1531;
        sum += baseColor * 0.1633;
        sum += texture2D( tDiffuse, vec2( vUv.x + 1.0 * h, vUv.y ) ) * 0.1531;
        sum += texture2D( tDiffuse, vec2( vUv.x + 2.0 * h, vUv.y ) ) * 0.12245;
        sum += texture2D( tDiffuse, vec2( vUv.x + 3.0 * h, vUv.y ) ) * 0.0918;
        sum += texture2D( tDiffuse, vec2( vUv.x + 4.0 * h, vUv.y ) ) * 0.051;
  #if DEBUG_BLUR_AREA == 1        
        gl_FragColor = h > 0.001 ? vec4(sum.r, 0.0, 0.0, 1.0) : sum;
  #else
        gl_FragColor = min(sum, baseColor);
  #endif
    }`,
};

export const VerticalBlurShadowShader = {
  uniforms: {
    tDiffuse: { value: null as Texture | null },
    blur: { value: 1.0 / 512.0 },
    shadowRange: { value: new Vector2(0.1, 0.9) },
  },
  defines: {
    DEBUG_BLUR_AREA: 0,
  },
  vertexShader: `
    varying vec2 vUv;
    void main() {
        vUv = uv;
        gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
    }`,
  fragmentShader: `
    uniform sampler2D tDiffuse;
    uniform float blur;
    uniform vec2 shadowRange;
    varying vec2 vUv;
  
    void main() {
        vec4 baseColor = texture2D(tDiffuse, vUv);
        float v = blur * step(shadowRange.x, baseColor.r) * step(baseColor.r, shadowRange.y);
        vec4 sum = vec4(0.0);
        sum += texture2D( tDiffuse, vec2( vUv.x, vUv.y - 4.0 * v ) ) * 0.051;
        sum += texture2D( tDiffuse, vec2( vUv.x, vUv.y - 3.0 * v ) ) * 0.0918;
        sum += texture2D( tDiffuse, vec2( vUv.x, vUv.y - 2.0 * v ) ) * 0.12245;
        sum += texture2D( tDiffuse, vec2( vUv.x, vUv.y - 1.0 * v ) ) * 0.1531;
        sum += baseColor * 0.1633;
        sum += texture2D( tDiffuse, vec2( vUv.x, vUv.y + 1.0 * v ) ) * 0.1531;
        sum += texture2D( tDiffuse, vec2( vUv.x, vUv.y + 2.0 * v ) ) * 0.12245;
        sum += texture2D( tDiffuse, vec2( vUv.x, vUv.y + 3.0 * v ) ) * 0.0918;
        sum += texture2D( tDiffuse, vec2( vUv.x, vUv.y + 4.0 * v ) ) * 0.051;
  #if DEBUG_BLUR_AREA == 1        
        gl_FragColor = v > 0.001 ? vec4(sum.r, 0.0, 0.0, 1.0) : sum;
  #else
        gl_FragColor = min(sum, baseColor);
  #endif
    }`,
};

const glslLinearDepthVertexShader = `varying vec2 vUv;
  void main() {
      vUv = uv;
      gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
  }`;

const glslLinearDepthFragmentShader = `uniform sampler2D tDepth;
  uniform vec4 depthFilter;
  uniform float cameraNear;
  uniform float cameraFar;
  varying vec2 vUv;
  
  #include <packing>
  
  float getLinearDepth(const in vec2 screenPosition) {
      float fragCoordZ = dot(texture2D(tDepth, screenPosition), depthFilter);
      #if PERSPECTIVE_CAMERA == 1
          float viewZ = perspectiveDepthToViewZ(fragCoordZ, cameraNear, cameraFar);
          return viewZToOrthographicDepth(viewZ, cameraNear, cameraFar);
      #else
          return fragCoordZ;
      #endif
  }
  
  void main() {
      float depth = getLinearDepth(vUv);
      gl_FragColor = vec4(vec3(1.0 - depth), 1.0);
  }`;

export class LinearDepthRenderMaterial extends ShaderMaterial {
  private static linearDepthShader: any = {
    uniforms: {
      tDepth: { value: null as Texture | null },
      depthFilter: { value: new Vector4(1, 0, 0, 0) },
      cameraNear: { value: 0.1 },
      cameraFar: { value: 1 },
    },
    defines: {
      PERSPECTIVE_CAMERA: 1,
      ALPHA_DEPTH: 0,
    },
    vertexShader: glslLinearDepthVertexShader,
    fragmentShader: glslLinearDepthFragmentShader,
  };

  constructor(parameters: Record<string, any>) {
    super({
      defines: Object.assign(
        {},
        LinearDepthRenderMaterial.linearDepthShader.defines
      ),
      uniforms: UniformsUtils.clone(
        LinearDepthRenderMaterial.linearDepthShader.uniforms
      ),
      vertexShader: LinearDepthRenderMaterial.linearDepthShader.vertexShader,
      fragmentShader:
        LinearDepthRenderMaterial.linearDepthShader.fragmentShader,
      blending: NoBlending,
    });
    this.update(parameters);
  }

  public update(parameters?: Record<string, any>): LinearDepthRenderMaterial {
    if (parameters?.depthTexture !== undefined) {
      this.uniforms.tDepth.value = parameters?.depthTexture;
    }
    if (parameters?.camera !== undefined) {
      const camera =
        (parameters?.camera as OrthographicCamera) ||
        (parameters?.camera as PerspectiveCamera);
      this.uniforms.cameraNear.value = camera.near;
      this.uniforms.cameraFar.value = camera.far;
    }
    if (parameters?.depthFilter !== undefined) {
      this.uniforms.depthFilter.value = parameters?.depthFilter;
    }
    return this;
  }
}

const glslNormalAndDepthVertexShader = `varying vec3 vNormal;
#if LINEAR_DEPTH == 1
    varying float vZ;  
#endif

  void main() {
      vNormal = normalMatrix * normal;
      vec4 viewPosition = modelViewMatrix * vec4(position, 1.0);
      #if LINEAR_DEPTH == 1
          vZ = viewPosition.z;  
      #endif
      gl_Position = projectionMatrix * viewPosition;
  }`;

const glslNormalAndDepthFragmentShader = `varying vec3 vNormal;
#if LINEAR_DEPTH == 1
  varying float vZ;  
  uniform float cameraNear;
  uniform float cameraFar;
#endif

  void main() {
      #if FLOAT_BUFFER == 1
          vec3 normal = normalize(vNormal);
      #else
          vec3 normal = normalize(vNormal) * 0.5 + 0.5;
      #endif
      #if LINEAR_DEPTH == 1
          float depth = (-vZ - cameraNear) / (cameraFar - cameraNear);
      #else
          float depth = gl_FragCoord.z;
      #endif
      gl_FragColor = vec4(normal, depth);
  }`;

export class NormalAndDepthRenderMaterial extends ShaderMaterial {
  private static normalAndDepthShader: any = {
    uniforms: {
      cameraNear: { value: 0.1 },
      cameraFar: { value: 1 },
    },
    defines: {
      FLOAT_BUFFER: 0,
      LINEAR_DEPTH: 0,
    },
    vertexShader: glslNormalAndDepthVertexShader,
    fragmentShader: glslNormalAndDepthFragmentShader,
  };

  constructor(parameters: Record<string, any>) {
    super({
      defines: Object.assign({
        ...NormalAndDepthRenderMaterial.normalAndDepthShader.defines,
        FLOAT_BUFFER: parameters?.floatBufferType ? 1 : 0,
        LINEAR_DEPTH: parameters?.linearDepth ? 1 : 0,
      }),
      uniforms: UniformsUtils.clone(
        NormalAndDepthRenderMaterial.normalAndDepthShader.uniforms
      ),
      vertexShader:
        NormalAndDepthRenderMaterial.normalAndDepthShader.vertexShader,
      fragmentShader:
        NormalAndDepthRenderMaterial.normalAndDepthShader.fragmentShader,
      blending: parameters?.blending ?? NoBlending,
    });
    this.update(parameters);
  }

  public update(
    parameters?: Record<string, any>
  ): NormalAndDepthRenderMaterial {
    if (parameters?.camera !== undefined) {
      const camera =
        (parameters?.camera as OrthographicCamera) ||
        (parameters?.camera as PerspectiveCamera);
      this.uniforms.cameraNear.value = camera.near;
      this.uniforms.cameraFar.value = camera.far;
    }
    return this;
  }
}
