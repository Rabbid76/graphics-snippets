import { LightSources } from '../scene/lightSources'
import * as THREE from 'three'
import { GUI } from 'dat.gui'

export abstract class ShadowMap {
    public lightSources: LightSources

    constructor(renderer: THREE.WebGLRenderer, lightSources: LightSources) {
        this.lightSources = lightSources
        renderer.shadowMap.enabled = true;
        renderer.shadowMap.needsUpdate = true
    }

    public abstract dispose(): void

    public getMainShadowLight() {
        return this.lightSources.getShadowLightSources()[0]
    }

    public updateLightSource() {
        const shadowLightSource = this.getMainShadowLight()
        shadowLightSource.castShadow = true
        shadowLightSource.shadow.needsUpdate = true
        shadowLightSource.shadow.normalBias = 0
        shadowLightSource.shadow.bias = 0
    }

    public abstract addUiElements(parent: GUI): void
}

export class BasicShadow extends ShadowMap {
    constructor(renderer: THREE.WebGLRenderer, lightSources: LightSources) {
        renderer.shadowMap.type = THREE.BasicShadowMap;
        super(renderer, lightSources)
        this.updateLightSource()
    }

    public dispose(): void {
    }

    public updateLightSource() {
        super.updateLightSource()
        const shadowLightSource = this.getMainShadowLight()
        shadowLightSource.shadow.needsUpdate = true
        shadowLightSource.shadow.bias = -0.005
        shadowLightSource.shadow.normalBias = 0
    }

    public addUiElements(parent: GUI): void {
    }
}

export class PCFShadow extends ShadowMap {
    public radius: number = 1

    constructor(renderer: THREE.WebGLRenderer, lightSources: LightSources) {
        renderer.shadowMap.type = THREE.PCFShadowMap;
        super(renderer, lightSources)
        this.updateLightSource()
    }

    public dispose(): void {
    }

    public updateLightSource() {
        super.updateLightSource()
        const shadowLightSource = this.getMainShadowLight()
        shadowLightSource.shadow.bias = -0.0005;
        shadowLightSource.shadow.normalBias = 0.01
        shadowLightSource.shadow.radius = this.radius
    }


    public addUiElements(parent: GUI): void {
        parent.add<any>(this, 'radius', 0, 20).onChange((value) => {
            const shadowLightSource = this.getMainShadowLight()
            shadowLightSource.shadow.radius = parseInt(value)
            shadowLightSource.shadow.needsUpdate = true
        })
    }
}

export class PCFSoftShadow extends ShadowMap {
    constructor(renderer: THREE.WebGLRenderer, lightSources: LightSources) {
        renderer.shadowMap.type = THREE.PCFSoftShadowMap;
        super(renderer, lightSources)
        this.updateLightSource()
    }

    public dispose(): void {
    }

    public updateLightSource() {
        super.updateLightSource()
        const shadowLightSource = this.getMainShadowLight()
        shadowLightSource.shadow.bias = -0.0005;
        shadowLightSource.shadow.normalBias = 0.01
    }

    public addUiElements(parent: GUI): void {
    }
}

export class VSMShadow extends ShadowMap {
    public radius: number = 30
    public blurSamples: number = 32

    constructor(renderer: THREE.WebGLRenderer, lightSources: LightSources) {
        renderer.shadowMap.type = THREE.VSMShadowMap;
        super(renderer, lightSources)
        this.updateLightSource()
    }

    public dispose(): void {
    }

    public updateLightSource() {
        super.updateLightSource()
        const shadowLightSource = this.getMainShadowLight()
        shadowLightSource.shadow.radius = this.radius
        shadowLightSource.shadow.blurSamples = this.blurSamples
        shadowLightSource.shadow.bias = -0.0005;
        shadowLightSource.shadow.normalBias = 0
    }

    public addUiElements(parent: GUI): void {
        parent.add<any>(this, 'radius').min(2).max(50).onChange((value) => {
            const shadowLightSource = this.getMainShadowLight()
            shadowLightSource.shadow.radius = parseInt(value)
            shadowLightSource.shadow.needsUpdate = true
        })
        parent.add<any>(this, 'blurSamples').min(2).max(64).onChange((value) => {
            const shadowLightSource = this.getMainShadowLight()
            shadowLightSource.shadow.blurSamples = parseInt(value)
            shadowLightSource.shadow.needsUpdate = true
        })
    }
}

export class PercentageCloserSoftShadow extends ShadowMap {
    private originalShadowRadius: number = 1

    constructor(renderer: THREE.WebGLRenderer, lightSources: LightSources) {
        renderer.shadowMap.type = THREE.PCFShadowMap;
        super(renderer, lightSources)
        this.updateLightSource()
    }

    public dispose(): void {
        const shadowLightSource = this.getMainShadowLight()
        shadowLightSource.shadow.radius = this.originalShadowRadius
    }

    public updateLightSource() {
        super.updateLightSource()
        const shadowLightSource = this.getMainShadowLight()
        shadowLightSource.shadow.bias = 0;
        shadowLightSource.shadow.normalBias = 0
        this.originalShadowRadius = shadowLightSource.shadow.radius
        shadowLightSource.shadow.radius = 1001.0
    }

    public addUiElements(parent: GUI): void {
    }

    public static setupPercentCloserSoftShadows(): void {
        let shader = THREE.ShaderChunk.shadowmap_pars_fragment;
        shader = shader.replace(
            '#ifdef USE_SHADOWMAP',
            '#ifdef USE_SHADOWMAP' + PCSS
        );
        shader = shader.replace(
            '#if defined( SHADOWMAP_TYPE_PCF )',
            PCSSGetShadow + '#if defined( SHADOWMAP_TYPE_PCF )'
        );
        THREE.ShaderChunk.shadowmap_pars_fragment = shader;
    }
}


const PCSS = `
#define PCSS_LIGHT_WORLD_SIZE 0.001
#define PCSS_LIGHT_FRUSTUM_WIDTH 3.75
#define PCSS_LIGHT_SIZE_UV (PCSS_LIGHT_WORLD_SIZE / PCSS_LIGHT_FRUSTUM_WIDTH)
#define PCSS_NEAR_PLANE 9.5

#define PCSS_NUM_SAMPLES 17
#define PCSS_NUM_RINGS 11
#define PCSS_BLOCKER_SEARCH_NUM_SAMPLES PCSS_NUM_SAMPLES

vec2 pcssPoissonDisk[PCSS_NUM_SAMPLES];

void pcssInitPoissonSamples( const in vec2 randomSeed ) {
    float ANGLE_STEP = PI2 * float( PCSS_NUM_RINGS ) / float( PCSS_NUM_SAMPLES );
    float INV_NUM_SAMPLES = 1.0 / float( PCSS_NUM_SAMPLES );

    // jsfiddle that shows sample pattern: https://jsfiddle.net/a16ff1p7/
    float angle = rand( randomSeed ) * PI2;
    float radius = INV_NUM_SAMPLES;
    float radiusStep = radius;

    for( int i = 0; i < PCSS_NUM_SAMPLES; i ++ ) {
        pcssPoissonDisk[i] = vec2( cos( angle ), sin( angle ) ) * pow( radius, 0.75 );
        radius += radiusStep;
        angle += ANGLE_STEP;
    }
}

float pcssPenumbraSize( const in float zReceiver, const in float zBlocker ) { // Parallel plane estimation
    return (zReceiver - zBlocker) / zBlocker;
}

float pcssFindBlocker( sampler2D shadowMap, const in vec2 uv, const in float zReceiver ) {
    // This uses similar triangles to compute what
    // area of the shadow map we should search
    float searchRadius = PCSS_LIGHT_SIZE_UV * ( zReceiver - PCSS_NEAR_PLANE ) / zReceiver;
    float blockerDepthSum = 0.0;
    int numBlockers = 0;

    for( int i = 0; i < PCSS_BLOCKER_SEARCH_NUM_SAMPLES; i++ ) {
        float shadowMapDepth = unpackRGBAToDepth(texture2D(shadowMap, uv + pcssPoissonDisk[i] * searchRadius));
        if ( shadowMapDepth < zReceiver ) {
            blockerDepthSum += shadowMapDepth;
            numBlockers ++;
        }
    }

    if( numBlockers == 0 ) return -1.0;

    return blockerDepthSum / float(numBlockers);
}

float PCSS_PCF_Filter(sampler2D shadowMap, vec2 uv, float zReceiver, float filterRadius ) {
    float sum = 0.0;
    float depth;
    #pragma unroll_loop_start
    for( int i = 0; i < 17; i ++ ) {
        depth = unpackRGBAToDepth( texture2D( shadowMap, uv + pcssPoissonDisk[ i ] * filterRadius ) );
        if( zReceiver <= depth ) sum += 1.0;
    }
    #pragma unroll_loop_end
    #pragma unroll_loop_start
    for( int i = 0; i < 17; i ++ ) {
        depth = unpackRGBAToDepth( texture2D( shadowMap, uv + -pcssPoissonDisk[ i ].yx * filterRadius ) );
        if( zReceiver <= depth ) sum += 1.0;
    }
    #pragma unroll_loop_end
    return sum / ( 2.0 * float( 17 ) );
}

float PCSS ( sampler2D shadowMap, vec4 coords ) {
    vec2 uv = coords.xy;
    float zReceiver = coords.z-0.001; // Assumed to be eye-space z in this code

    pcssInitPoissonSamples( uv );
    // STEP 1: blocker search
    float avgBlockerDepth = pcssFindBlocker( shadowMap, uv, zReceiver );

    //There are no occluders so early out (this saves filtering)
    if( avgBlockerDepth == -1.0 ) return 1.0;

    // STEP 2: penumbra size
    float penumbraRatio = pcssPenumbraSize( zReceiver, avgBlockerDepth );
    float filterRadius = penumbraRatio * PCSS_LIGHT_SIZE_UV * PCSS_NEAR_PLANE / zReceiver;

    // STEP 3: filtering
    //return avgBlockerDepth;
    return PCSS_PCF_Filter( shadowMap, uv, zReceiver, filterRadius );
}
`
const PCSSGetShadow = 'if (shadowRadius >= 1000.0) { return PCSS( shadowMap, shadowCoord ); }\n'