import { QualityLevel, SceneRenderer } from './scene-renderer';
import { AmbientOcclusionType } from './shadow-and-ao-pass';
import {
  ACESFilmicToneMapping,
  CineonToneMapping,
  LinearSRGBColorSpace,
  LinearToneMapping,
  NoToneMapping,
  ReinhardToneMapping,
  SRGBColorSpace,
} from 'three';
import { GUI } from 'dat.gui';

export class SceneRendererGUI {
  private sceneRenderer: SceneRenderer;
  private qualityLevel = '';
  private ambientOcclusionType = '';

  constructor(sceneRenderer: SceneRenderer) {
    this.sceneRenderer = sceneRenderer;
  }

  public addGUI(gui: GUI, updateCallback: () => void): void {
    this.addRepresentationalGUI(gui, updateCallback);
    this.addDebugGUI(gui, updateCallback);
    const shadowMapFolder = gui.addFolder('Shadow map');
    this.addShadowTypeGUI(shadowMapFolder, updateCallback);
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
    const outputColorSpaces = new Map([
      ['LinearSRGBColorSpace', LinearSRGBColorSpace],
      ['SRGBColorSpace', SRGBColorSpace],
    ]);
    const outputColorSpaceNames: string[] = [];
    outputColorSpaces.forEach((value, key) => {
      outputColorSpaceNames.push(key);
      if (this.sceneRenderer.renderer.outputColorSpace === value) {
        this.sceneRenderer.outputColorSpace = key;
      }
    });
    gui
      .add<any>(this.sceneRenderer, 'outputColorSpace', outputColorSpaceNames)
      .onChange((colorSpace: string) => {
        if (outputColorSpaces.has(colorSpace)) {
          this.sceneRenderer.renderer.outputColorSpace =
            outputColorSpaces.get(colorSpace) ?? SRGBColorSpace;
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
      ['HIGHEST', QualityLevel.HIGHEST],
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
            qualityLevels.get(qualityLevel) ?? QualityLevel.HIGHEST
          );
        }
      });
    gui
      .add<any>(this.sceneRenderer, 'debugOutput', {
        'off ': 'off',
        'grayscale (no textures)': 'grayscale',
        'color buffer': 'color',
        'linear depth': 'lineardepth',
        'g-buffer normal vector': 'g-normal',
        'g-buffer depth': 'g-depth',
        'AO pure': 'ssao',
        'AO denoised': 'ssaodenoise',
        'shadow map': 'shadowmap',
        'shadow Monte Carlo': 'shadow',
        'blur shadow': 'shadowblur',
        'ground shadow': 'groundshadow',
        'ground reflection': 'groundreflection',
        'baked ground shadow': 'bakedgroundshadow',
        'selection outline': 'outline',
      })
      .onChange(() => updateCallback());
  }

  public addShadowTypeGUI(gui: GUI, updateCallback: () => void): void {
    const shadowConfiguration =
      this.sceneRenderer.screenSpaceShadow.shadowConfiguration;
    const shadowMapNames: any[] = [];
    shadowConfiguration.types.forEach((_, key) => {
      shadowMapNames.push(key);
    });
    const updateShadow = () => {
      this.sceneRenderer.screenSpaceShadow.needsUpdate = true;
      this.sceneRenderer.screenSpaceShadow.shadowTypeNeedsUpdate = true;
      this.sceneRenderer.shadowAndAoPass.needsUpdate = true;
      updateCallback();
    };
    gui
      .add<any>(shadowConfiguration, 'shadowType', shadowMapNames)
      .onChange((type: string) => {
        if (this.sceneRenderer.screenSpaceShadow.switchType(type)) {
          shadowBiasController.object =
            shadowConfiguration.currentConfiguration;
          shadowNormalBiasController.object =
            shadowConfiguration.currentConfiguration;
          shadowBiasController.updateDisplay();
          shadowNormalBiasController.updateDisplay();
          updateShadow();
        }
      });
    const shadowBiasController = gui
      .add<any>(
        shadowConfiguration.currentConfiguration,
        'bias',
        -0.001,
        0.001,
        0.00001
      )
      .onChange(() => updateShadow());
    const shadowNormalBiasController = gui
      .add<any>(
        shadowConfiguration.currentConfiguration,
        'normalBias',
        -0.05,
        0.05
      )
      .onChange(() => updateShadow());
  }

  private addShadowAndAoGUI(gui: GUI, updateCallback: () => void): void {
    const updateParameters = (): void => {
      this.sceneRenderer.gBufferRenderTarget.needsUpdate = true;
      this.sceneRenderer.shadowAndAoPass.needsUpdate = true;
      this.sceneRenderer.shadowAndAoPass.shadowAndAoRenderTargets.parametersNeedsUpdate =
        true;
      updateCallback();
    };
    const parameters = this.sceneRenderer.shadowAndAoPass.parameters;
    const ahAndAoParameters = parameters.shAndAo;
    const shadowMapParameters = this.sceneRenderer.screenSpaceShadow.parameters;
    const ssaoParameters = parameters.ssao;
    const hbaoParameters = parameters.hbao;
    const denoiseParameters = parameters.poissionDenoise;
    gui.add<any>(parameters, 'enabled').onChange(() => updateParameters());
    const aoTypes = new Map([
      ['none', AmbientOcclusionType.NONE],
      ['SSAO', AmbientOcclusionType.SSAO],
      ['effects SSAO', AmbientOcclusionType.EffectsSSAO],
      ['effects HBAO', AmbientOcclusionType.EffectsHBAO],
    ]);
    const aoNames: string[] = Array.from(aoTypes.keys());
    aoTypes.forEach((value, key) => {
      if (value === parameters.aoType) {
        this.ambientOcclusionType = key;
      }
    });
    gui
      .add<any>(this, 'ambientOcclusionType', aoNames)
      .onChange((aoType: string) => {
        if (aoTypes.has(aoType)) {
          parameters.aoType = aoTypes.get(aoType) ?? AmbientOcclusionType.SSAO;
          updateParameters();
        }
      });
    gui.add<any>(parameters, 'aoIntensity', 0, 1).onChange(() => {
      updateParameters();
    });
    gui.add<any>(parameters, 'aoOnGround').onChange(() => {
      updateParameters();
    });
    gui
      .add<any>(parameters, 'enablePoissionDenoise')
      .onChange(() => updateParameters());
    gui.add<any>(parameters, 'alwaysUpdate').onChange(() => updateParameters());
    gui
      .add<any>(shadowMapParameters, 'maximumNumberOfLightSources')
      .onChange(() => updateParameters());
    gui
      .add<any>(ahAndAoParameters, 'aoAndSoftShadowFxaa')
      .onChange(() => updateParameters());

    const aoFolder = gui.addFolder('Roomle SSAO');
    aoFolder
      .add<any>(ahAndAoParameters, 'aoFadeout', 0, 1)
      .onChange(() => updateParameters());
    aoFolder
      .add<any>(ahAndAoParameters, 'aoKernelRadius', 0.001, 1)
      .onChange(() => updateParameters());
    aoFolder
      .add<any>(ahAndAoParameters, 'aoDepthBias', 0.0001, 0.01)
      .onChange(() => updateParameters());
    aoFolder
      .add<any>(ahAndAoParameters, 'aoMaxDistance', 0.01, 1)
      .onChange(() => updateParameters());
    aoFolder
      .add<any>(ahAndAoParameters, 'aoMaxDepth', 0.9, 1)
      .onChange(() => updateParameters());
    aoFolder
      .add<any>(ahAndAoParameters, 'shadowIntensity', 0, 1)
      .onChange(() => updateParameters());
    aoFolder
      .add<any>(ahAndAoParameters, 'shadowRadius', 0.001, 0.5)
      .onChange(() => updateParameters());

    const ssaoFolder = gui.addFolder('effects SSAO');
    ssaoFolder
      .add<any>(ssaoParameters, 'resolutionScale', 0.25, 1, 0.25)
      .onChange(() => updateParameters());
    ssaoFolder
      .add<any>(ssaoParameters, 'spp', 1, 64, 1)
      .onChange(() => updateParameters());
    ssaoFolder
      .add<any>(ssaoParameters, 'distance', 0.1, 10, 0.1)
      .onChange(() => updateParameters());
    ssaoFolder
      .add<any>(ssaoParameters, 'distancePower', 0, 2, 0.125)
      .onChange(() => updateParameters());
    ssaoFolder
      .add<any>(ssaoParameters, 'bias', 0, 500, 1)
      .onChange(() => updateParameters());
    ssaoFolder
      .add<any>(ssaoParameters, 'power', 0.5, 32, 0.5)
      .onChange(() => updateParameters());
    ssaoFolder
      .add<any>(ssaoParameters, 'thickness', 0, 0.1, 0.001)
      .onChange(() => updateParameters());

    const hbaoFolder = gui.addFolder('effects HBAO');
    hbaoFolder
      .add<any>(hbaoParameters, 'resolutionScale', 0.25, 1, 0.2)
      .onChange(() => updateParameters());
    hbaoFolder
      .add<any>(hbaoParameters, 'spp', 1, 64, 1)
      .onChange(() => updateParameters());
    hbaoFolder
      .add<any>(hbaoParameters, 'distance', 0.1, 10, 0.01)
      .onChange(() => updateParameters());
    hbaoFolder
      .add<any>(hbaoParameters, 'distancePower', 0.1, 10, 0.1)
      .onChange(() => updateParameters());
    hbaoFolder
      .add<any>(hbaoParameters, 'bias', 0, 100, 1)
      .onChange(() => updateParameters());
    hbaoFolder
      .add<any>(hbaoParameters, 'power', 0.5, 8, 0.5)
      .onChange(() => updateParameters());
    hbaoFolder
      .add<any>(hbaoParameters, 'thickness', 0, 0.1, 0.001)
      .onChange(() => updateParameters());

    const denoiseFolder = gui.addFolder('Possion Denoise');
    denoiseFolder
      .add<any>(denoiseParameters, 'iterations', 0, 3, 1)
      .onChange(() => updateParameters());
    denoiseFolder
      .add<any>(denoiseParameters, 'radius', 0, 32, 1)
      .onChange(() => updateParameters());
    denoiseFolder
      .add<any>(denoiseParameters, 'rings', 0, 16, 0.125)
      .onChange(() => updateParameters());
    denoiseFolder
      .add<any>(denoiseParameters, 'samples', 0, 32, 1)
      .onChange(() => updateParameters());
    denoiseFolder
      .add<any>(denoiseParameters, 'lumaPhi', 0, 20, 0.001)
      .onChange(() => updateParameters());
    denoiseFolder
      .add<any>(denoiseParameters, 'depthPhi', 0, 20, 0.001)
      .onChange(() => updateParameters());
    denoiseFolder
      .add<any>(denoiseParameters, 'normalPhi', 0, 20, 0.001)
      .onChange(() => updateParameters());
  }

  private addGroundReflectionGUI(gui: GUI, updateCallback: () => void): void {
    const parameters = this.sceneRenderer.parameters.groundReflectionParameters;
    gui.add<any>(parameters, 'enabled');
    gui
      .add<any>(parameters, 'intensity', 0.0, 1.0)
      .onChange(() => updateCallback());
    gui
      .add<any>(parameters, 'fadeOut', 0.0, 2.0)
      .onChange(() => updateCallback());
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
    const updateParameters = (): void => {
      this.sceneRenderer.bakedGroundContactShadow.applyParameters();
      updateCallback();
    };
    const parameters =
      this.sceneRenderer.parameters.bakedGroundContactShadowParameters;
    gui.add<any>(parameters, 'enabled');
    gui.add<any>(parameters, 'cameraHelper').onChange(() => updateParameters());
    gui.add<any>(parameters, 'alwaysUpdate');
    gui.add<any>(parameters, 'fadeIn');
    gui
      .add<any>(parameters, 'blurMin', 0, 0.2, 0.001)
      .onChange(() => updateParameters());
    gui
      .add<any>(parameters, 'blurMax', 0, 0.5, 0.01)
      .onChange(() => updateParameters());
    gui
      .add<any>(parameters, 'fadeoutFalloff', 0.0, 1.0, 0.01)
      .onChange(() => updateParameters());
    gui
      .add<any>(parameters, 'fadeoutBias', 0.0, 0.5)
      .onChange(() => updateParameters());
    gui
      .add<any>(parameters, 'opacity', 0, 1, 0.01)
      .onChange(() => updateParameters());
    gui
      .add<any>(parameters, 'cameraFar', 0.1, 10, 0.1)
      .onChange(() => updateParameters());
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
