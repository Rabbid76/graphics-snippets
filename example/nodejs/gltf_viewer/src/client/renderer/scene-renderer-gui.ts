import { SceneRenderer } from './scene-renderer';
import {
  ACESFilmicToneMapping,
  CineonToneMapping,
  LinearEncoding,
  LinearToneMapping,
  NoToneMapping,
  ReinhardToneMapping,
  sRGBEncoding,
} from 'three';
import { GUI } from 'dat.gui';

export class SceneRendererGUI {
  private sceneRenderer: SceneRenderer;

  constructor(sceneRenderer: SceneRenderer) {
    this.sceneRenderer = sceneRenderer;
  }

  public addGUI(gui: GUI, updateCallback: () => void): void {
    this.addRepresentationalGUI(gui, updateCallback);
    this.addDebugGUI(gui, updateCallback);
    const contactShadowFolder = gui.addFolder('Ground Contact Shadow');
    this.addContactShadowGUI(contactShadowFolder, updateCallback);
    this.addShadowTypeGUI(gui, updateCallback);
    const shadowAndAoFolder = gui.addFolder('Shadow and Ambient Occlusion');
    this.addShadowAndAoGUI(shadowAndAoFolder, updateCallback);
    const outlineFolder = gui.addFolder('Outline');
    this.addOutlineGUI(outlineFolder, updateCallback);
  }

  private addRepresentationalGUI(gui: GUI, updateCallback: () => void): void {
    const outputEncodings = new Map([
      ['LinearEncoding', LinearEncoding],
      ['sRGBEncoding', sRGBEncoding],
    ]);
    const outputEncodingNames: string[] = [];
    outputEncodings.forEach((value, key) => {
      outputEncodingNames.push(key);
      if (this.sceneRenderer.renderer.outputEncoding === value) {
        this.sceneRenderer.outputEncoding = key;
      }
    });
    gui
      .add(this.sceneRenderer, 'outputEncoding', outputEncodingNames)
      .onChange((encoding: string) => {
        if (outputEncodings.has(encoding)) {
          this.sceneRenderer.renderer.outputEncoding =
            outputEncodings.get(encoding) ?? LinearEncoding;
          updateCallback();
        }
      });
    const toneMappings = new Map([
      ['NoToneMapping', NoToneMapping],
      ['LinearToneMapping', LinearToneMapping],
      ['ReinhardToneMapping', ReinhardToneMapping],
      ['CineonToneMapping', CineonToneMapping],
      ['ACESFilmicToneMapping', ACESFilmicToneMapping],
    ]);
    const toneMappingNames: string[] = [];
    toneMappings.forEach((value, key) => {
      toneMappingNames.push(key);
      if (this.sceneRenderer.renderer.toneMapping === value) {
        this.sceneRenderer.toneMapping = key;
      }
    });
    gui
      .add(this.sceneRenderer, 'toneMapping', toneMappingNames)
      .onChange((toneMapping: string) => {
        if (toneMappings.has(toneMapping)) {
          this.sceneRenderer.renderer.toneMapping =
            toneMappings.get(toneMapping) ?? NoToneMapping;
          updateCallback();
        }
      });
  }

  private addDebugGUI(gui: GUI, updateCallback: () => void): void {
    gui
      .add(this.sceneRenderer, 'debugOutput', {
        off: 'off',
        grayscale: 'grayscale',
        color: 'color',
        depth: 'depth',
        'normal vector': 'normal',
        SSAO: 'ssao',
        'blur SSAO': 'ssaoblur',
        shadow: 'shadow',
        'blur shadow': 'shadowblur',
      })
      .onChange(() => updateCallback());
  }

  private addContactShadowGUI(gui: GUI, updateCallback: () => void): void {
    const updateParameter = (): void => {
      this.sceneRenderer.groundContactShadow.updateParameters();
      updateCallback();
    };
    const parameters = this.sceneRenderer.groundContactShadow.parameters;
    gui.add(parameters, 'enabled');
    gui.add(parameters, 'cameraHelper');
    gui.add(parameters, 'alwaysUpdate');
    gui
      .add(parameters, 'blurMin', 0, 30, 0.1)
      .onChange(() => updateParameter());
    gui
      .add(parameters, 'blurMax', 0, 30, 0.1)
      .onChange(() => updateParameter());
    gui
      .add(parameters, 'darkness', 1, 5, 0.1)
      .onChange(() => updateParameter());
    gui
      .add(parameters, 'opacity', 0, 1, 0.01)
      .onChange(() => updateParameter());
    gui
      .add(parameters, 'cameraFar', 0.1, 10, 0.1)
      .onChange(() => updateParameter());
  }

  public addShadowTypeGUI(gui: GUI, updateCallback: () => void): void {
    const shadowConfiguration =
      this.sceneRenderer.screenSpaceShadow.shadowConfiguration;
    const shadowMapNames: string[] = [];
    shadowConfiguration.types.forEach((_, key) => {
      shadowMapNames.push(key);
    });
    gui
      .add(shadowConfiguration, 'shadowType', shadowMapNames)
      .onChange((type: string) => {
        if (shadowConfiguration.types.has(type)) {
          const currentType = shadowConfiguration.types.get(type);
          if (currentType) {
            shadowConfiguration.currentType = currentType;
          }
          this.sceneRenderer.screenSpaceShadow.needsUpdate = true;
          this.sceneRenderer.screenSpaceShadow.shadowTypeNeedsUpdate = true;
          updateCallback();
        }
      });
  }

  private addShadowAndAoGUI(gui: GUI, updateCallback: () => void): void {
    const parameters = this.sceneRenderer.shadowAndAoParameters;
    gui
      .add(parameters, 'aoAndSoftShadowEnabled')
      .onChange(() => updateCallback());
    gui.add(parameters, 'aoAlwaysUpdate').onChange(() => updateCallback());
    gui
      .add(parameters, 'aoIntensity', 0.01, 1)
      .onChange(() => updateCallback());
    gui.add(parameters, 'aoFadeout', 0, 1).onChange(() => updateCallback());
    gui
      .add(parameters, 'aoKernelRadius', 0.001, 0.2)
      .onChange(() => updateCallback());
    gui
      .add(parameters, 'aoDepthBias', 0.0001, 0.01)
      .onChange(() => updateCallback());
    gui
      .add(parameters, 'aoMaxDistance', 0.01, 1)
      .onChange(() => updateCallback());
    gui.add(parameters, 'aoMaxDepth', 0.9, 1).onChange(() => updateCallback());
    gui
      .add(parameters, 'shadowIntensity', 0, 1)
      .onChange(() => updateCallback());
    gui
      .add(parameters, 'shadowRadius', 0.001, 0.5)
      .onChange(() => updateCallback());
  }

  private addOutlineGUI(gui: GUI, updateCallback: () => void): void {
    const updateOutlineParameters = (): void => {
      this.sceneRenderer.outlineRenderer.updateOutlineParameters();
      updateCallback();
    };
    const parameters = this.sceneRenderer.outlineParameters;
    gui.add(parameters, 'enabled');
    gui
      .add(parameters, 'edgeStrength', 0.5, 20)
      .onChange(() => updateOutlineParameters());
    gui
      .add(parameters, 'edgeGlow', 0, 20)
      .onChange(() => updateOutlineParameters());
    gui
      .add(parameters, 'edgeThickness', 0.5, 20)
      .onChange(() => updateOutlineParameters());
    gui
      .add(parameters, 'pulsePeriod', 0, 5)
      .onChange(() => updateOutlineParameters());
    gui
      .addColor(parameters, 'visibleEdgeColor')
      .onChange(() => updateOutlineParameters());
    gui
      .addColor(parameters, 'hiddenEdgeColor')
      .onChange(() => updateOutlineParameters());
  }
}
