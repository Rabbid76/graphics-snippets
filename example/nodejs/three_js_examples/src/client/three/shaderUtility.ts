import {
    AddEquation,
    DstAlphaFactor,
    DstColorFactor,
    ShaderMaterial,
    UniformsUtils,
    Vector2,
    ZeroFactor,
} from 'three';

const CopyShader = {
    uniforms: {
        tDiffuse: { value: null },
        opacity: { value: 1.0 },
        brightness: { value: 0.0 }
    },
    vertexShader: /* glsl */`
        varying vec2 vUv;

        void main() {
            vUv = uv;
            gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
        }`,
    fragmentShader: /* glsl */`
        uniform float opacity;
        uniform float brightness;
        uniform sampler2D tDiffuse;
        varying vec2 vUv;

        void main() {
            gl_FragColor = texture2D(tDiffuse, vUv);
            gl_FragColor.rgb = mix(gl_FragColor.rgb, vec3(1.0), brightness);
            gl_FragColor.a *= opacity;
        }`
};

export class CopyMaterial extends ShaderMaterial {
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
            blendEquationAlpha: AddEquation
        });
        this.update(parameters);
    }

    update(parameters?: any): CopyMaterial {
        if (parameters?.texture !== undefined) {
            this.uniforms.tDiffuse.value = parameters?.texture;
        }
        if (parameters?.opacity !== undefined) {
            this.uniforms.opacity.value = parameters?.opacity;
        }
        if (parameters?.brightness !== undefined) {
            this.uniforms.brightness.value = parameters?.brightness;
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

