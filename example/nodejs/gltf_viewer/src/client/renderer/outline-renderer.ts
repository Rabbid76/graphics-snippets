import { DepthNormalRenderTarget } from './depth-normal-render-targets';
//import { OutlinePass } from 'three/examples/jsm/postprocessing/OutlinePass.js';
import { OutlinePass } from './outline-pass';
import { EffectComposer } from 'three/examples/jsm/postprocessing/EffectComposer';
import { Camera, Object3D, PerspectiveCamera, Scene, Vector2 } from 'three';

export interface OutlineParameters {
  [key: string]: any;
  enabled: boolean;
  edgeStrength: number;
  edgeGlow: number;
  edgeThickness: number;
  pulsePeriod: number;
  usePatternTexture: false;
  visibleEdgeColor: any;
  hiddenEdgeColor: any;
}

export class OutLineRenderer {
  public parameters: OutlineParameters;
  private width: number = 0;
  private height: number = 0;
  private effectComposer: EffectComposer | null = null;
  private depthNormalRenderTarget?: DepthNormalRenderTarget;
  public outlinePass: OutlinePass | null = null;
  public outlinePassActivated = false;

  get isOutlinePassActivated(): boolean {
    return this.outlinePassActivated;
  }

  constructor(
    effectComposer: EffectComposer | null,
    width: number,
    height: number,
    parameters: any
  ) {
    this.effectComposer = effectComposer;
    this.depthNormalRenderTarget = parameters?.depthNormalRenderTarget;
    this.width = width;
    this.height = height;
    this.parameters = {
      enabled: true,
      edgeStrength: 2.0,
      edgeGlow: 1.0,
      edgeThickness: 2.0,
      pulsePeriod: 0,
      usePatternTexture: false,
      visibleEdgeColor: 0xffffff, // 0xdb0000,
      hiddenEdgeColor: 0xffffff, // 0xdb0000,
      ...parameters,
    };
  }

  public dispose(): void {
    this.outlinePass?.dispose();
  }

  public setSize(width: number, height: number): void {
    this.width = width;
    this.height = height;
    this.outlinePass?.setSize(width, height);
  }

  public updateParameters(parameters: any): void {
    for (let propertyName in parameters) {
      if (this.parameters.hasOwnProperty(propertyName)) {
        this.parameters[propertyName] = parameters[propertyName];
      }
    }
  }

  public applyParameters(): void {
    if (!this.outlinePass) {
      return;
    }
    this.outlinePass.edgeStrength = this.parameters.edgeStrength;
    this.outlinePass.edgeGlow = this.parameters.edgeGlow;
    this.outlinePass.edgeThickness = this.parameters.edgeThickness;
    this.outlinePass.pulsePeriod = this.parameters.pulsePeriod;
    this.outlinePass.usePatternTexture = this.parameters.usePatternTexture;
    this.outlinePass.visibleEdgeColor.set(this.parameters.visibleEdgeColor);
    this.outlinePass.hiddenEdgeColor.set(this.parameters.hiddenEdgeColor);
  }

  public activateOutline(scene: Scene, camera: Camera): void {
    if (!this.parameters.enabled) {
      this.deactivateOutline();
      return;
    }
    const needsUpdate =
      this.outlinePass?.renderCamera &&
      (camera as PerspectiveCamera).isPerspectiveCamera !==
        (this.outlinePass.renderCamera as PerspectiveCamera)
          .isPerspectiveCamera;
    if (this.outlinePass) {
      this.outlinePass.renderScene = scene;
      this.outlinePass.renderCamera = camera;
    }
    if (!needsUpdate && this.outlinePassActivated) {
      return;
    }
    if (needsUpdate || !this.outlinePass) {
      this.outlinePass = new OutlinePass(
        new Vector2(this.width, this.height),
        scene,
        camera,
        [],
        {
          downSampleRatio: 2,
          edgeDetectionFxaa: true,
          depthNormalRenderTarget: this.depthNormalRenderTarget,
        }
      );
    }
    this.applyParameters();
    if (this.effectComposer) {
      this.effectComposer.addPass(this.outlinePass);
    }
    this.outlinePassActivated = true;
  }

  public deactivateOutline(): void {
    if (!this.outlinePassActivated) {
      return;
    }
    if (this.outlinePass && this.effectComposer) {
      this.effectComposer.removePass(this.outlinePass);
    }
    this.outlinePassActivated = false;
  }

  public updateOutline(
    scene: Scene,
    camera: Camera,
    selectedObjects: Object3D[]
  ) {
    if (selectedObjects.length > 0) {
      this.activateOutline(scene, camera);
      if (this.outlinePass) {
        this.outlinePass.selectedObjects = selectedObjects;
      }
    } else {
      this.deactivateOutline();
    }
  }
}
