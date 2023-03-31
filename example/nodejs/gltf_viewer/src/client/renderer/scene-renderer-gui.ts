import { QualityLevel, SceneRenderer } from './scene-renderer';
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
  private qualityLevel = '';

  constructor(sceneRenderer: SceneRenderer) {
    this.sceneRenderer = sceneRenderer;
  }

  public addGUI(gui: GUI, updateCallback: () => void): void {
    this.addRepresentationalGUI(gui, updateCallback);
    this.addDebugGUI(gui, updateCallback);
    this.addShadowTypeGUI(gui, updateCallback);
    const shadowAndAoFolder = gui.addFolder('Shadow and Ambient Occlusion');
    this.addShadowAndAoGUI(shadowAndAoFolder, updateCallback);
    const groundReflectionFolder = gui.addFolder(
      'Ground Reflection and Shadow'
    );
    this.addGroundReflectionGUI(groundReflectionFolder, updateCallback);
    const bakedGroundContactShadowFolder = gui.addFolder(
      'Baked Ground Contact Shadow'
    );
    this.addBakedGroundContactShadowGUI(
      bakedGroundContactShadowFolder,
      updateCallback
    );
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
      .add<any>(this.sceneRenderer, 'outputEncoding', outputEncodingNames)
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
      .add<any>(this.sceneRenderer, 'toneMapping', toneMappingNames)
      .onChange((toneMapping: string) => {
        if (toneMappings.has(toneMapping)) {
          this.sceneRenderer.renderer.toneMapping =
            toneMappings.get(toneMapping) ?? NoToneMapping;
          updateCallback();
        }
      });
  }

  private addDebugGUI(gui: GUI, updateCallback: () => void): void {
    const qualityLevels = new Map([
      ['HIGH', QualityLevel.HIGH],
      ['MEDIUM', QualityLevel.MEDIUM],
      ['LOW', QualityLevel.LOW],
    ]);
    const outputQualityNames: string[] = [];
    qualityLevels.forEach((value, key) => outputQualityNames.push(key));
    gui
      .add<any>(this, 'qualityLevel', outputQualityNames)
      .onChange((qualityLevel: string) => {
        if (qualityLevels.has(qualityLevel)) {
          this.sceneRenderer.setQualityLevel(
            qualityLevels.get(qualityLevel) ?? QualityLevel.HIGH
          );
        }
      });
    gui
      .add<any>(this.sceneRenderer, 'debugOutput', {
        off: 'off',
        grayscale: 'grayscale',
        color: 'color',
        depth: 'depth',
        'normal vector': 'normal',
        SSAO: 'ssao',
        'blur SSAO': 'ssaoblur',
        shadow: 'shadow',
        'blur shadow': 'shadowblur',
        'ground shadow': 'groundshadow',
        'ground reflection': 'groundreflection',
        'baked ground shadow': 'bakedgroundshadow',
      })
      .onChange(() => updateCallback());
  }

  public addShadowTypeGUI(gui: GUI, updateCallback: () => void): void {
    const shadowConfiguration =
      this.sceneRenderer.screenSpaceShadow.shadowConfiguration;
    const shadowMapNames: string[] = [];
    shadowConfiguration.types.forEach((_, key) => {
      shadowMapNames.push(key);
    });
    gui
      .add<any>(shadowConfiguration, 'shadowType', shadowMapNames)
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
    const parameters = this.sceneRenderer.shadowAndAoPass.parameters;
    gui
      .add<any>(parameters, 'aoAndSoftShadowEnabled')
      .onChange(() => updateCallback());
    gui.add<any>(parameters, 'aoAlwaysUpdate').onChange(() => updateCallback());
    gui
      .add<any>(parameters, 'aoIntensity', 0.01, 1)
      .onChange(() => updateCallback());
    gui.add<any>(parameters, 'aoFadeout', 0, 1).onChange(() => updateCallback());
    gui
      .add<any>(parameters, 'aoKernelRadius', 0.001, 0.2)
      .onChange(() => updateCallback());
    gui
      .add<any>(parameters, 'aoDepthBias', 0.0001, 0.01)
      .onChange(() => updateCallback());
    gui
      .add<any>(parameters, 'aoMaxDistance', 0.01, 1)
      .onChange(() => updateCallback());
    gui.add<any>(parameters, 'aoMaxDepth', 0.9, 1).onChange(() => updateCallback());
    gui
      .add<any>(parameters, 'shadowIntensity', 0, 1)
      .onChange(() => updateCallback());
    gui
      .add<any>(parameters, 'shadowRadius', 0.001, 0.5)
      .onChange(() => updateCallback());
  }

  private addGroundReflectionGUI(gui: GUI, updateCallback: () => void): void {
    const parameters = this.sceneRenderer.parameters.groundReflectionParameters;
    gui.add<any>(parameters, 'enabled');
    gui.add<any>(parameters, 'intensity', 0.0, 1.0).onChange(() => updateCallback());
    gui.add<any>(parameters, 'fadeOut', 0.0, 2.0).onChange(() => updateCallback());
    gui
      .add<any>(parameters, 'brightness', 0.0, 2.0)
      .onChange(() => updateCallback());
    gui
      .add<any>(parameters, 'blurHorizontal', 0.0, 10.0)
      .onChange(() => updateCallback());
    gui
      .add<any>(parameters, 'blurVertical', 0.0, 10.0)
      .onChange(() => updateCallback());
    gui
      .add<any>(parameters, 'blurAscent', 0.0, 3.0)
      .onChange(() => updateCallback());
  }

  private addBakedGroundContactShadowGUI(
    gui: GUI,
    updateCallback: () => void
  ): void {
    const updateParameter = (): void => {
      this.sceneRenderer.bakedGroundContactShadow.applyParameters();
      updateCallback();
    };
    const parameters =
      this.sceneRenderer.parameters.bakedGroundContactShadowParameters;
    gui.add<any>(parameters, 'enabled');
    gui.add<any>(parameters, 'cameraHelper');
    gui.add<any>(parameters, 'alwaysUpdate');
    gui
      .add<any>(parameters, 'blurMin', 0, 0.2, 0.001)
      .onChange(() => updateParameter());
    gui
      .add<any>(parameters, 'blurMax', 0, 0.5, 0.01)
      .onChange(() => updateParameter());
    gui
      .add<any>(parameters, 'fadeoutFalloff', 0.0, 1.0, 0.01)
      .onChange(() => updateParameter());
    gui
      .add<any>(parameters, 'fadeoutBias', 0.0, 0.5)
      .onChange(() => updateParameter());
    gui
      .add<any>(parameters, 'opacity', 0, 1, 0.01)
      .onChange(() => updateParameter());
    gui
      .add<any>(parameters, 'cameraFar', 0.1, 10, 0.1)
      .onChange(() => updateParameter());
  }

  private addOutlineGUI(gui: GUI, updateCallback: () => void): void {
    const updateOutlineParameters = (): void => {
      this.sceneRenderer.outlineRenderer.applyParameters();
      updateCallback();
    };
    const parameters = this.sceneRenderer.outlineRenderer.parameters;
    gui.add<any>(parameters, 'enabled');
    gui
      .add<any>(parameters, 'edgeStrength', 0.5, 20)
      .onChange(() => updateOutlineParameters());
    gui
      .add<any>(parameters, 'edgeGlow', 0, 20)
      .onChange(() => updateOutlineParameters());
    gui
      .add<any>(parameters, 'edgeThickness', 0.5, 20)
      .onChange(() => updateOutlineParameters());
    gui
      .add<any>(parameters, 'pulsePeriod', 0, 5)
      .onChange(() => updateOutlineParameters());
    gui
      .addColor(parameters, 'visibleEdgeColor')
      .onChange(() => updateOutlineParameters());
    gui
      .addColor(parameters, 'hiddenEdgeColor')
      .onChange(() => updateOutlineParameters());
  }
}
