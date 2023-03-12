import { OutlinePass } from 'three/examples/jsm/postprocessing/OutlinePass.js';
//import { OutlinePass } from '../experimental/outline_pass';
import { EffectComposer } from 'three/examples/jsm/postprocessing/EffectComposer';
import { Camera, Object3D, PerspectiveCamera, Scene, Vector2 } from 'three';

export interface OutlineParameters {
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
  private width: number = 0;
  private height: number = 0;
  private effectComposer: EffectComposer | null = null;
  public outlinePass: OutlinePass | null = null;
  public parameters: OutlineParameters;
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
    this.width = width;
    this.height = height;
    this.parameters = {
      enabled: parameters.enable ?? true,
      edgeStrength: parameters.edgeStrength ?? 2.0,
      edgeGlow: parameters.edgeGlow ?? 1.0,
      edgeThickness: parameters.edgeThickness ?? 2.0,
      pulsePeriod: parameters.pulsePeriod ?? 0,
      usePatternTexture: parameters.usePatternTexture ?? false,
      visibleEdgeColor: parameters.visibleEdgeColor ?? 0xffffff, // 0xdb0000,
      hiddenEdgeColor: parameters.hiddenEdgeColor ?? 0xffffff, // 0xdb0000,
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
    if (parameters.enabled !== undefined) {
      this.parameters.enabled = parameters.enabled;
    }
    if (parameters.edgeStrength !== undefined) {
      this.parameters.edgeStrength = parameters.edgeStrength;
    }
    if (parameters.edgeStrength !== undefined) {
      this.parameters.edgeStrength = parameters.edgeStrength;
    }
    if (parameters.edgeGlow !== undefined) {
      this.parameters.edgeGlow = parameters.edgeGlow;
    }
    if (parameters.edgeThickness !== undefined) {
      this.parameters.edgeThickness = parameters.edgeThickness;
    }
    if (parameters.pulsePeriod !== undefined) {
      this.parameters.pulsePeriod = parameters.pulsePeriod;
    }
    if (parameters.usePatternTexture !== undefined) {
      this.parameters.usePatternTexture = parameters.usePatternTexture;
    }
    if (parameters.visibleEdgeColor !== undefined) {
      this.parameters.visibleEdgeColor = parameters.visibleEdgeColor;
    }
    if (parameters.hiddenEdgeColor !== undefined) {
      this.parameters.hiddenEdgeColor = parameters.hiddenEdgeColor;
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
    if (this.outlinePassActivated) {
      return;
    }
    if (
      !this.outlinePass ||
      (camera as PerspectiveCamera).isPerspectiveCamera !==
        (this.outlinePass.renderCamera as PerspectiveCamera).isPerspectiveCamera
    ) {
      this.outlinePass = new OutlinePass(
        new Vector2(this.width, this.height),
        scene,
        camera,
        []
      );
    } else {
      this.outlinePass.renderScene = scene;
      this.outlinePass.renderCamera = camera;
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
