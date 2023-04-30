import {
    AddEquation,
    DstAlphaFactor,
    DstColorFactor,
    Matrix3,
    Matrix4,
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
  
  export class CopyTransformMaterial extends ShaderMaterial {
    public static defaultTransform: Matrix4 = new Matrix4();
    public static rgbTransform: Matrix4 = new Matrix4().set(
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 0
    );
    public static alphaTransform: Matrix4 = new Matrix4().set(
      0, 0, 0, 1,
      0, 0, 0, 1,
      0, 0, 0, 1,
      0, 0, 0, 0
    );
    public static redTransform: Matrix4 = new Matrix4().set(
      1, 0, 0, 0,
      1, 0, 0, 0,
      1, 0, 0, 0,
      0, 0, 0, 1
    );
    public static grayscaleTransform: Matrix4 = new Matrix4().set(
      1, 0, 0, 0,
      1, 0, 0, 0,
      1, 0, 0, 0,
      0, 0, 0, 1
    );
    public static zeroRGBA: Vector4 = new Vector4(0, 0, 0, 0);
    public static alphaRGBA: Vector4 = new Vector4(0, 0, 0, 1);
    public static defaultUvTransform: Matrix3 = new Matrix3();
    public static flipYuvTransform: Matrix3 = new Matrix3().set(
      1, 0, 0,
      0, -1, 1,
      0, 0, 1
    );
    constructor(parameters?: any, additiveBlending: boolean = true) {
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
  
    update(parameters?: any): CopyTransformMaterial {
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

export const HorizontalBlurShadowShader = {
    uniforms: {
        // @ts-ignore
        tDiffuse: { value: null },
        hRange: { value: new Vector2(1.0 / 512.0, 1.0 / 512.0) },
        shadowScale: { value: 1 }
    },
    vertexShader: /* glsl */`
    varying vec2 vUv;
    void main() {
        vUv = uv;
        gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
    }`,
    fragmentShader: /* glsl */`
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
    }`
};

export const VerticalBlurShadowShader = {
    uniforms: {
        // @ts-ignore
        tDiffuse: { value: null },
        vRange: { value: new Vector2(1.0 / 512.0, 1.0 / 512.0) },
        shadowScale: { value: 1 },
    },
    vertexShader: /* glsl */`
    varying vec2 vUv;
    void main() {
        vUv = uv;
        gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
    }`,
    fragmentShader: /* glsl */`
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
    }`
};

