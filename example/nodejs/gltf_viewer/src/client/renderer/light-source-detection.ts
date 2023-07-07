import {
  DataTexture,
  Mesh,
  MeshBasicMaterial,
  OrthographicCamera,
  PlaneGeometry,
  ShaderMaterial,
  Texture,
  UniformsUtils,
  Vector2,
  Vector3,
  WebGLRenderer,
  WebGLRenderTarget,
} from 'three';

export const sphereToEquirectangular = (pointOnSphere: Vector3): Vector2 => {
  const u = Math.atan2(pointOnSphere.y, pointOnSphere.x) / (2 * Math.PI) + 0.5;
  const v = Math.asin(pointOnSphere.z) / Math.PI + 0.5;
  return new Vector2(u, v);
};

export const equirectangularToSphere = (uv: Vector2): Vector3 => {
  const theta = (uv.x - 0.5) * 2 * Math.PI;
  const phi = (uv.y - 0.5) * Math.PI;
  const length = Math.cos(phi);
  return new Vector3(
    Math.cos(theta) * length,
    Math.sin(theta) * length,
    Math.sin(phi)
  );
};

export interface TextureConverterResult {
  texture: Texture;
  pixels: Uint8Array;
}

export class TextureConverter {
  private _colorRenderTarget?: WebGLRenderTarget;
  private _grayscaleRenderTarget?: WebGLRenderTarget;
  private _grayscaleShaderMaterial?: GrayscaleShaderMaterial;
  private _camera?: OrthographicCamera;
  private planeMesh?: Mesh;

  get colorRenderTarget(): WebGLRenderTarget {
    this._colorRenderTarget =
      this._colorRenderTarget ?? new WebGLRenderTarget();
    return this._colorRenderTarget;
  }

  get grayscaleRenderTarget(): WebGLRenderTarget {
    this._grayscaleRenderTarget =
      this._grayscaleRenderTarget ?? new WebGLRenderTarget();
    //this._grayscaleRenderTarget = this._grayscaleRenderTarget ?? new WebGLRenderTarget(1, 1, { format: RedFormat });
    return this._grayscaleRenderTarget;
  }

  get grayscaleShaderMaterial(): GrayscaleShaderMaterial {
    this._grayscaleShaderMaterial =
      this._grayscaleShaderMaterial ?? new GrayscaleShaderMaterial();
    return this._grayscaleShaderMaterial;
  }

  get camera(): OrthographicCamera {
    this._camera = this._camera ?? new OrthographicCamera(-1, 1, 1, -1, -1, 1);
    return this._camera;
  }

  public scaleTexture(
    renderer: WebGLRenderer,
    texture: Texture,
    targetWidth: number,
    targetHeight: number
  ): TextureConverterResult {
    this.colorRenderTarget.setSize(targetWidth, targetHeight);
    this.planeMesh =
      this.planeMesh ??
      new Mesh(
        new PlaneGeometry(2, 2),
        new MeshBasicMaterial({ map: texture })
      );
    const renderTargetBackup = renderer.getRenderTarget();
    renderer.setRenderTarget(this.colorRenderTarget);
    renderer.render(this.planeMesh, this.camera);
    renderer.setRenderTarget(renderTargetBackup);
    const colorTexture = this.grayscaleRenderTarget.texture;
    const pixelBuffer = new Uint8Array(targetWidth * targetHeight * 4);
    renderer.readRenderTargetPixels(
      this.colorRenderTarget,
      0,
      0,
      targetWidth,
      targetHeight,
      pixelBuffer
    );
    return { texture: colorTexture, pixels: pixelBuffer };
  }

  public newGrayscaleTexture(
    renderer: WebGLRenderer,
    texture: Texture,
    targetWidth: number,
    targetHeight: number
  ): TextureConverterResult {
    this.grayscaleRenderTarget.setSize(targetWidth, targetHeight);
    this.grayscaleShaderMaterial.setSourceTexture(texture);
    this.planeMesh =
      this.planeMesh ??
      new Mesh(new PlaneGeometry(2, 2), this.grayscaleShaderMaterial);
    const renderTargetBackup = renderer.getRenderTarget();
    renderer.setRenderTarget(this.grayscaleRenderTarget);
    renderer.render(this.planeMesh, this.camera);
    renderer.setRenderTarget(renderTargetBackup);
    const grayscaleTexture = this.grayscaleRenderTarget.texture;
    const pixelBuffer = new Uint8Array(targetWidth * targetHeight * 4);
    renderer.readRenderTargetPixels(
      this.grayscaleRenderTarget,
      0,
      0,
      targetWidth,
      targetHeight,
      pixelBuffer
    );
    return { texture: grayscaleTexture, pixels: pixelBuffer };
  }
}

const GrayscaleShader = {
  uniforms: {
    tDiffuse: { value: null as Texture | null },
  },
  vertexShader: `
        varying vec2 vUv;
        void main() {
            vUv = uv;
            gl_Position = (projectionMatrix * modelViewMatrix * vec4(position, 1.0)).xyww;
        }`,
  fragmentShader: `
        uniform sampler2D tDiffuse;
        varying vec2 vUv;
        void main() {
            vec4 color = texture2D(tDiffuse, vUv);
            float grayscale = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
            //float grayscale = dot(color.rgb, vec3(1.0/3.0));
            gl_FragColor = vec4(vec3(grayscale), 1.0);
        }`,
};

export class GrayscaleShaderMaterial extends ShaderMaterial {
  constructor() {
    super({
      uniforms: UniformsUtils.clone(GrayscaleShader.uniforms),
      vertexShader: GrayscaleShader.vertexShader,
      fragmentShader: GrayscaleShader.fragmentShader,
    });
  }

  setSourceTexture(map: Texture) {
    this.uniforms.tDiffuse.value = map;
  }
}

export class LightSourceDetector {
  private numberOfSamples: number;
  private width: number;
  private height: number;
  private sampleThreshold: number;
  public readonly pointDistance: number;
  public readonly pixelDistance: number;
  public readonly samplePoints: Vector3[] = [];
  public readonly sampleUVs: Vector2[] = [];
  public textureData?: any;
  public grayscaleTexture: TextureConverterResult = {
    texture: new Texture(),
    pixels: new Uint8Array(0),
  };
  public detectorTexture: Texture = new Texture();
  public detectorArray: Float32Array = new Float32Array(0);
  private textureConverter?: TextureConverter;
  public lightSamples: LightSample[] = [];
  public lightGraph: LightGraph = new LightGraph(0);
  public lightSources: LightSource[] = [];

  constructor(parameters?: any) {
    this.numberOfSamples = parameters?.numberOfSamples ?? 1000;
    this.width = parameters?.width ?? 1024;
    this.height = parameters?.height ?? 512;
    this.sampleThreshold = parameters?.sampleThreshold ?? 0.707;
    this.pointDistance =
      Math.sqrt(4 * Math.PI) / Math.sqrt(this.numberOfSamples);
    this.pixelDistance = (Math.sqrt(2) * Math.PI * 2) / this.width;
    this.samplePoints = this.createEquirectangularSamplePoints(
      this.numberOfSamples
    );
    this.sampleUVs = this.samplePoints.map((point) =>
      sphereToEquirectangular(point)
    );
  }

  public detectLightSources(
    renderer: WebGLRenderer,
    equirectangularTexture: Texture,
    textureData?: any
  ) {
    this.textureData = textureData;
    this.textureConverter = this.textureConverter ?? new TextureConverter();
    this.grayscaleTexture = this.textureConverter.newGrayscaleTexture(
      renderer,
      equirectangularTexture,
      this.width,
      this.height
    );
    this.detectorArray = this.redFromRgbaToNormalizedFloatArray(
      this.grayscaleTexture.pixels
    );
    this.detectorTexture = this.grayscaleTextureFromFloatArray(
      this.detectorArray,
      this.width,
      this.height
    );
    this.lightSamples = this.filterLightSamples(this.sampleThreshold);
    this.lightGraph = this.findClusterSegments(
      this.lightSamples,
      this.sampleThreshold
    );
    this.lightGraph.findConnectedComponents();
    this.lightSources = this.createLightSourcesFromLightGraph(
      this.lightSamples,
      this.lightGraph
    );
    this.lightSources.sort((a, b) => b.maxIntensity - a.maxIntensity);
  }

  private createEquirectangularSamplePoints = (
    numberOfPoints: number
  ): Vector3[] => {
    const points: Vector3[] = [];
    for (let i = 0; i < numberOfPoints; i++) {
      const spiralAngle = i * Math.PI * (3 - Math.sqrt(5));
      const z = 1 - (i / (numberOfPoints - 1)) * 2;
      const radius = Math.sqrt(1 - z * z);
      const x = Math.cos(spiralAngle) * radius;
      const y = Math.sin(spiralAngle) * radius;
      points.push(new Vector3(x, y, z));
    }
    return points;
  };

  private redFromRgbaToNormalizedFloatArray(
    rgba: Uint8Array,
    exponent?: number
  ): Float32Array {
    const floatArray = new Float32Array(rgba.length / 4);
    let minimumValue = 1;
    let maximumValue = 0;
    for (let i = 0; i < rgba.length / 4; ++i) {
      const value = rgba[i * 4] / 255;
      minimumValue = Math.min(minimumValue, value);
      maximumValue = Math.max(maximumValue, value);
      floatArray[i] = value;
    }
    if (exponent) {
      for (let i = 0; i < floatArray.length; ++i) {
        const normalizedValue =
          (floatArray[i] - minimumValue) / (maximumValue - minimumValue);
        floatArray[i] = Math.pow(normalizedValue, exponent);
      }
    } else {
      for (let i = 0; i < floatArray.length; ++i) {
        floatArray[i] =
          (floatArray[i] - minimumValue) / (maximumValue - minimumValue);
      }
    }
    return floatArray;
  }

  private grayscaleTextureFromFloatArray(
    floatArray: Float32Array,
    width: number,
    height: number
  ): Texture {
    const noOfPixels = width * height;
    const uint8data = new Uint8Array(4 * noOfPixels);
    for (let i = 0; i < noOfPixels; i++) {
      const grayscale = floatArray[i] * 255;
      uint8data[i * 4 + 0] = grayscale;
      uint8data[i * 4 + 1] = grayscale;
      uint8data[i * 4 + 2] = grayscale;
      uint8data[i * 4 + 3] = 255;
    }
    const dataTexture = new DataTexture(uint8data, width, height);
    dataTexture.needsUpdate = true;
    return dataTexture;
  }

  private filterLightSamples(threshold: number): LightSample[] {
    const lightSamples: LightSample[] = [];
    for (let i = 0; i < this.sampleUVs.length; i++) {
      const uv = this.sampleUVs[i];
      const value = this.detectorTextureLuminanceValueFromUV(uv);
      if (value > threshold) {
        lightSamples.push(new LightSample(this.samplePoints[i], uv));
      }
    }
    return lightSamples;
  }

  private detectorTextureLuminanceValueFromUV(uv: Vector2): number {
    const column = Math.floor(uv.x * this.width);
    const row = Math.floor(uv.y * this.height);
    const index = row * this.width + column;
    return this.detectorArray[index];
  }

  private originalLuminanceValueFromUV(uv: Vector2): number {
    if (
      !this.textureData ||
      !this.textureData.data ||
      !this.textureData.width ||
      !this.textureData.height
    ) {
      return this.detectorTextureLuminanceValueFromUV(uv) * 256;
    }
    const column = Math.floor(uv.x * this.textureData.width);
    const row = Math.floor(uv.y * this.textureData.height);
    let luminance = 0;
    for (let x = Math.max(0, column - 2); x < Math.max(0, column + 2); ++x) {
      for (let y = Math.max(0, row - 2); y < Math.max(0, row + 2); ++y) {
        const index = y * this.textureData.width + x;
        const grayValue =
          (this.textureData.data[index * 4] +
            this.textureData.data[index * 4 + 1] +
            this.textureData.data[index * 4 + 2]) /
          3;
        luminance = Math.max(luminance, grayValue);
      }
    }
    return luminance;
  }

  private findClusterSegments(
    samples: LightSample[],
    threshold: number
  ): LightGraph {
    const stepDistance = this.pixelDistance * 2;
    const maxDistance = this.pointDistance * 1.5;
    const lightGraph = new LightGraph(samples.length);
    for (let i = 0; i < samples.length; i++) {
      for (let j = i + 1; j < samples.length; j++) {
        if (samples[i].position.angleTo(samples[j].position) < maxDistance) {
          const direction = samples[j].position
            .clone()
            .sub(samples[i].position);
          const steps = Math.floor(direction.length() / stepDistance);
          let inTreshold = true;
          let outOfTresholdCount = 0;
          for (let k = 1; k < steps; k++) {
            const step = direction.clone().multiplyScalar(k / steps);
            const uv = sphereToEquirectangular(
              samples[i].position.clone().add(step).normalize()
            );
            const value = this.detectorTextureLuminanceValueFromUV(uv);
            if (value < threshold) {
              outOfTresholdCount++;
              if (outOfTresholdCount > 1) {
                inTreshold = false;
                break;
              }
            } else {
              outOfTresholdCount = 0;
            }
          }
          if (inTreshold) {
            lightGraph.adjacent[i].push(j);
            lightGraph.adjacent[j].push(i);
            lightGraph.edges.push([i, j]);
          }
        }
      }
    }
    return lightGraph;
  }

  private createLightSourcesFromLightGraph(
    samples: LightSample[],
    lightGraph: LightGraph
  ): LightSource[] {
    const lightSources: LightSource[] = lightGraph.components
      .filter((component) => component.length > 1)
      .map(
        (component) => new LightSource(component.map((index) => samples[index]))
      );
    lightSources.forEach((lightSource) =>
      lightSource.calculateLightSourceProperties((uv) =>
        this.originalLuminanceValueFromUV(uv)
      )
    );
    return lightSources;
  }
}

export class LightSample {
  public readonly position: Vector3;
  public readonly uv: Vector2;

  constructor(position: Vector3, uv: Vector2) {
    this.position = position;
    this.uv = uv;
  }
}

export class LightGraph {
  public readonly noOfNodes: number;
  public edges: number[][] = [];
  public adjacent: number[][] = [];
  public components: number[][] = [];

  constructor(noOfNodes: number) {
    this.noOfNodes = noOfNodes;
    for (let i = 0; i < noOfNodes; i++) {
      this.adjacent.push([]);
    }
  }

  public findConnectedComponents() {
    const visited = new Array(this.noOfNodes).fill(false);
    this.components = [];
    for (let i = 0; i < this.noOfNodes; i++) {
      if (!visited[i]) {
        const component: number[] = [];
        this.dfs(i, visited, component);
        this.components.push(component);
      }
    }
    this.components.sort((a, b) => b.length - a.length);
  }

  private dfs(node: number, visited: boolean[], component: number[]) {
    visited[node] = true;
    component.push(node);
    for (const adjacentNode of this.adjacent[node]) {
      if (!visited[adjacentNode]) {
        this.dfs(adjacentNode, visited, component);
      }
    }
  }
}

export class LightSource {
  public readonly lightSamples: LightSample[];
  public position: Vector3 = new Vector3();
  public uv: Vector2 = new Vector2();
  public averageIntensity: number = 0;
  public maxIntensity: number = 0;
  public size: number = 0;

  constructor(lightSamples: LightSample[]) {
    this.lightSamples = lightSamples;
  }

  public calculateLightSourceProperties(
    luminanceFunction: (uv: Vector2) => number
  ) {
    this.position = new Vector3();
    this.averageIntensity = 0;
    this.maxIntensity = 0;
    for (const lightSample of this.lightSamples) {
      this.position.add(lightSample.position);
      const luminanceValue = luminanceFunction(lightSample.uv);
      this.averageIntensity += luminanceValue;
      this.maxIntensity = Math.max(this.maxIntensity, luminanceValue);
    }
    this.averageIntensity /= this.lightSamples.length;
    this.position.normalize();
    this.uv = sphereToEquirectangular(this.position);
    let averageDistance = 0;
    for (const lightSample of this.lightSamples) {
      averageDistance += lightSample.position.distanceTo(this.position);
    }
    averageDistance /= this.lightSamples.length;
    this.size = averageDistance / Math.PI;
  }
}
