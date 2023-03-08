import {
  AddEquation,
  DstAlphaFactor,
  DstColorFactor,
  Matrix4,
  ShaderMaterial,
  Texture,
  UniformsUtils,
  Vector2,
  ZeroFactor,
} from 'three';

const CopyShader = {
  uniforms: {
    tDiffuse: { value: null as Texture | null },
    colorTransform: { value: new Matrix4() },
    multiplyChannels: { value: 0 },
  },
  vertexShader: `
        varying vec2 vUv;
  
        void main() {
            vUv = uv;
            gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
        }`,
  fragmentShader: `
        uniform sampler2D tDiffuse;
        uniform mat4 colorTransform;
        uniform float multiplyChannels;
        varying vec2 vUv;
  
        void main() {
            vec4 color = colorTransform * texture2D(tDiffuse, vUv);
            color.rgb = mix(color.rgb, vec3(color.r * color.g * color.b), multiplyChannels);
            gl_FragColor = color;
        }`,
};

export class CopyTransformMaterial extends ShaderMaterial {
  public static defaultTransform: Matrix4 = new Matrix4();
  public static grayscaleTransform: Matrix4 = new Matrix4().set(
    1,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    1
  );
  constructor(parameters?: any) {
    super({
      uniforms: UniformsUtils.clone(CopyShader.uniforms),
      vertexShader: CopyShader.vertexShader,
      fragmentShader: CopyShader.fragmentShader,
      transparent: true,
      depthTest: false,
      depthWrite: false,
      blendSrc: DstColorFactor,
      blendDst: ZeroFactor,
      blendEquation: AddEquation,
      blendSrcAlpha: DstAlphaFactor,
      blendDstAlpha: ZeroFactor,
      blendEquationAlpha: AddEquation,
    });
    this.update(parameters);
  }

  update(parameters?: any): CopyTransformMaterial {
    if (parameters?.texture !== undefined) {
      this.uniforms.tDiffuse.value = parameters?.texture;
    }
    if (parameters?.colorTransform !== undefined) {
      this.uniforms.colorTransform.value = parameters?.colorTransform;
    }
    if (parameters?.multiplyChannels !== undefined) {
      this.uniforms.multiplyChannels.value = parameters?.multiplyChannels;
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

export const HorizontalBlurContactShadowShader = {
  uniforms: {
    tDiffuse: { value: null as Texture | null },
    hRange: { value: new Vector2(1.0 / 512.0, 1.0 / 512.0) },
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
    uniform vec2 hRange;
    uniform float shadowScale;
    varying vec2 vUv;
  
    void main() {
        vec4 baseColor = texture2D(tDiffuse, vUv);
        float h = mix(hRange.y, hRange.x, baseColor.a * shadowScale + 0.05);
        vec4 sum = vec4( 0.0 );
        sum += texture2D( tDiffuse, vec2( vUv.x - 4.0 * h, vUv.y ) ) * 0.051;
        sum += texture2D( tDiffuse, vec2( vUv.x - 3.0 * h, vUv.y ) ) * 0.0918;
        sum += texture2D( tDiffuse, vec2( vUv.x - 2.0 * h, vUv.y ) ) * 0.12245;
        sum += texture2D( tDiffuse, vec2( vUv.x - 1.0 * h, vUv.y ) ) * 0.1531;
        sum += baseColor * 0.1633;
        sum += texture2D( tDiffuse, vec2( vUv.x + 1.0 * h, vUv.y ) ) * 0.1531;
        sum += texture2D( tDiffuse, vec2( vUv.x + 2.0 * h, vUv.y ) ) * 0.12245;
        sum += texture2D( tDiffuse, vec2( vUv.x + 3.0 * h, vUv.y ) ) * 0.0918;
        sum += texture2D( tDiffuse, vec2( vUv.x + 4.0 * h, vUv.y ) ) * 0.051;
        gl_FragColor = sum;
    }`,
};

export const VerticalBlurContactShadowShader = {
  uniforms: {
    tDiffuse: { value: null as Texture | null },
    vRange: { value: new Vector2(1.0 / 512.0, 1.0 / 512.0) },
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
    uniform vec2 vRange;
    uniform float shadowScale;
    varying vec2 vUv;
  
    void main() {
        vec4 baseColor = texture2D(tDiffuse, vUv);
        float v = mix(vRange.y, vRange.x, baseColor.a * shadowScale + 0.05);
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
