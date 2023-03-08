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
  public outlineParameters: OutlineParameters;
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
    this.outlineParameters = {
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

  public updateOutlineParameters(): void {
    if (!this.outlinePass) {
      return;
    }
    this.outlinePass.edgeStrength = this.outlineParameters.edgeStrength;
    this.outlinePass.edgeGlow = this.outlineParameters.edgeGlow;
    this.outlinePass.edgeThickness = this.outlineParameters.edgeThickness;
    this.outlinePass.pulsePeriod = this.outlineParameters.pulsePeriod;
    this.outlinePass.usePatternTexture =
      this.outlineParameters.usePatternTexture;
    this.outlinePass.visibleEdgeColor.set(
      this.outlineParameters.visibleEdgeColor
    );
    this.outlinePass.hiddenEdgeColor.set(
      this.outlineParameters.hiddenEdgeColor
    );
  }

  public activateOutline(scene: Scene, camera: Camera): void {
    if (!this.outlineParameters.enabled) {
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
    this.updateOutlineParameters();
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
