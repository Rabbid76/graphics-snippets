import {
  BufferGeometry,
  Camera,
  DoubleSide,
  Float32BufferAttribute,
  Matrix4,
  Mesh,
  Object3D,
  ShaderMaterial,
  Vector2,
} from 'three';
import { GUI, GUIController } from 'dat.gui';

export enum BackgroundType {
  None,
  GridPaper,
  Concrete,
  Marble,
  Granite,
  VorocracksMarble,
  Kraft,
  Line,
  Test,
}

export interface BackgroundParameter {
  [key: string]: any;
  backgroundType: BackgroundType;
}

export class BackgroundEnvironment {
  public parameters: BackgroundParameter;
  public backgroundMaterial?: ShaderMaterial;
  public backgroundMesh: Mesh;
  private materialMap = new Map<BackgroundType, ShaderMaterial>();

  public get isSet(): boolean {
    return this.parameters.backgroundType !== BackgroundType.None;
  }

  constructor(parameters?: any) {
    const geometry = new BufferGeometry();
    geometry.setAttribute(
      'position',
      new Float32BufferAttribute([-1, 3, 0, -1, -1, 0, 3, -1, 0], 3)
    );
    geometry.setAttribute(
      'uv',
      new Float32BufferAttribute([0, 2, 0, 0, 2, 0], 2)
    );
    this.backgroundMesh = new Mesh(geometry);
    this.backgroundMesh.name = 'Background';
    this.parameters = {
      visible: false,
      backgroundType: BackgroundType.None,
      ...parameters,
    };
    this.updateBackground();
  }

  private updateParameters(parameters: any) {
    for (let propertyName in parameters) {
      if (this.parameters.hasOwnProperty(propertyName)) {
        this.parameters[propertyName] = parameters[propertyName];
      }
    }
  }

  public updateBackground() {
    this.backgroundMesh.visible =
      this.parameters.backgroundType !== BackgroundType.None;
    this.backgroundMaterial = this.getMaterial();
    this.backgroundMesh.material = this.backgroundMaterial;
  }

  public update(width: number, height: number, camera: Camera) {
    if (!this.backgroundMesh.visible) {
      return;
    }
    if (this.backgroundMaterial) {
      this.backgroundMaterial.uniforms.viewMatrixInverse.value.copy(
        camera.matrixWorld
      );
      this.backgroundMaterial.uniforms.projectionMatrixInverse.value.copy(
        camera.projectionMatrixInverse
      );
      this.backgroundMaterial.uniforms.iResolution.value.set(width, height);
    }
  }

  public addToScene(scene: Object3D) {
    if (this.backgroundMesh !== scene) {
      scene.add(this.backgroundMesh);
    }
  }

  public hideBackground() {
    if (this.parameters.backgroundType !== BackgroundType.None) {
      this.parameters.backgroundType = BackgroundType.None;
      this.updateBackground();
    }
  }

  private getMaterial(): ShaderMaterial {
    let material = this.materialMap.get(this.parameters.backgroundType);
    if (material) {
      return material;
    }

    let fragmentShader: string = glslTestBackgroundFragmentShader;
    let defines: any = {};
    switch (this.parameters.backgroundType) {
      default:
        break;
      case BackgroundType.Test:
        fragmentShader = glslTestBackgroundFragmentShader;
        break;
      case BackgroundType.GridPaper:
        fragmentShader = glslGridPaperBackgroundFragmentShader;
        break;
      case BackgroundType.Concrete:
        fragmentShader = glslGraprogBackgroundFragmentShader;
        defines = { patternChoice: 1 };
        break;
      case BackgroundType.Marble:
        fragmentShader = glslGraprogBackgroundFragmentShader;
        defines = { patternChoice: 3 };
        break;
      case BackgroundType.Granite:
        fragmentShader = glslGraprogBackgroundFragmentShader;
        defines = { patternChoice: 6 };
        break;
      case BackgroundType.VorocracksMarble:
        fragmentShader = glslMarbleBackgroundFragmentShader;
        break;
      case BackgroundType.Kraft:
        fragmentShader = glslKraftBackgroundFragmentShader;
        break;
      case BackgroundType.Line:
        fragmentShader = glslLineBackgroundFragmentShader;
        break;
    }
    material = new ShaderMaterial({
      name: 'BackgroundShader',
      vertexShader: glslBackgroundVertexShader,
      fragmentShader,
      defines,
      uniforms: {
        viewMatrixInverse: { value: new Matrix4() },
        projectionMatrixInverse: { value: new Matrix4() },
        scale: { value: 8.0 },
        iResolution: { value: new Vector2(1024.0, 1024.0) },
      },
      side: DoubleSide,
      depthWrite: false,
    });
    this.materialMap.set(this.parameters.backgroundType, material);
    return material;
  }
}

export class BackgroundEnvironmentGUI {
  private backgroundEnvironment: BackgroundEnvironment;
  private backgroundTypeController?: GUIController;

  constructor(backgroundEnvironment: BackgroundEnvironment) {
    this.backgroundEnvironment = backgroundEnvironment;
  }

  public hideBackground(): void {
    this.backgroundTypeController?.setValue('None');
    this.backgroundTypeController?.updateDisplay();
  }

  public addGUI(
    gui: GUI,
    updateCallback?: (backgroundEnvironment: BackgroundEnvironment) => void
  ): void {
    const properties: any = {
      backgroundType: '',
    };
    const backgroundTypes = new Map([
      ['None', BackgroundType.None],
      ['Grid Paper', BackgroundType.GridPaper],
      ['Concrete', BackgroundType.Concrete],
      ['Marble', BackgroundType.Marble],
      ['Granite', BackgroundType.Granite],
      ['Vorocracks Marble', BackgroundType.VorocracksMarble],
      ['Kraft', BackgroundType.Kraft],
      ['Line (slow)', BackgroundType.Line],
      ['Test', BackgroundType.Test],
    ]);
    const backgroundNames: string[] = [];
    backgroundTypes.forEach((value, key) => {
      backgroundNames.push(key);
      if (this.backgroundEnvironment.parameters.backgroundType === value) {
        properties.backgroundType = key;
      }
    });
    gui
      .add<any>(properties, 'backgroundType', backgroundNames)
      .onChange((backgroundType: string) => {
        if (backgroundTypes.has(backgroundType)) {
          this.backgroundEnvironment.parameters.backgroundType =
            backgroundTypes.get(backgroundType) ?? BackgroundType.None;
          this.backgroundEnvironment.updateBackground();
          if (updateCallback) {
            updateCallback(this.backgroundEnvironment);
          }
        }
      });
  }
}

const glslBackgroundVertexShader = `varying vec2 vertexUv;
uniform mat4 viewMatrixInverse;
uniform mat4 projectionMatrixInverse;
uniform float scale;

void main() {
    vec4 p = viewMatrixInverse * projectionMatrixInverse * vec4(position.xy, 0.0, 1.0);
    vertexUv = (p.xz / p.w * 0.5 + 0.5) / scale;  
    gl_Position = vec4(position.xy, 1.0, 1.0);
}`;

const glslTestBackgroundFragmentShader = `varying vec2 vertexUv;
void main() {
    vec2 uv = fract(vertexUv);
    gl_FragColor = vec4(uv.x, uv.y, (1.0-uv.x) * (1.0 - uv.y), 1.0);
}`;

const glslGridPaperBackgroundFragmentShader = `varying vec2 vertexUv;
uniform vec2 iResolution;
float nsin(float a)
{
    return (sin(a)+1.)/2.;
}
float rand(float n)
{
 	return fract(cos(n*89.42)*343.42);
}
vec2 rand(vec2 n)
{
 	return vec2(rand(n.x*23.62-300.0+n.y*34.35),rand(n.x*45.13+256.0+n.y*38.89)); 
}

// returns (dx, dy, distance)
vec3 worley(vec2 n,float s)
{
    vec3 ret = vec3(s * 10.);
    // look in 9 cells (n, plus 8 surrounding)
    for(int x = -1;x<2;x++)
    {
        for(int y = -1;y<2;y++)
        {
            vec2 xy = vec2(x,y);// xy can be thought of as both # of cells distance to n, and 
            vec2 cellIndex = floor(n/s) + xy;
            vec2 worleyPoint = rand(cellIndex);// random point in this cell (0-1)
            worleyPoint += xy - fract(n/s);// turn it into distance to n. ;
            float d = length(worleyPoint) * s;
            if(d < ret.z)
                ret = vec3(worleyPoint, d);
        }
    }
    return ret;
}

vec2 mouse = vec2(1.);// how do i initialize this??

vec4 applyLighting(vec4 inpColor, vec2 uv, vec3 normal, vec3 LightPos, vec4 LightColor, vec4 AmbientColor)
{
   // if(distance(uv.xy, LightPos.xy) < 0.01) return vec4(1.,0.,0.,1.);
    vec3 LightDir = vec3(LightPos.xy - uv, LightPos.z);
    vec3 N = normalize(normal);
    vec3 L = normalize(LightDir);
    vec3 Diffuse = (LightColor.rgb * LightColor.a) * max(dot(N, L), 0.0);
    vec3 Ambient = AmbientColor.rgb * AmbientColor.a;
    vec3 Intensity = Ambient + Diffuse;
    vec3 FinalColor = inpColor.rgb * Intensity;
    return vec4(FinalColor, inpColor.a);
}

// convert distance to alpha value (see https://www.shadertoy.com/view/ltBGzt)
float dtoa(float d)
{
    const float amount = 800.0;
    return clamp(1.0 / (clamp(d, 1.0/amount, 1.0)*amount), 0.,1.);
}

// distance to edge of grid line. real distance, and centered over its position.
float grid_d(vec2 uv, vec2 gridSize, float gridLineWidth)
{
    uv += gridLineWidth / 2.0;
    uv = mod(uv, gridSize);
    vec2 halfRemainingSpace = (gridSize - gridLineWidth) / 2.0;
    uv -= halfRemainingSpace + gridLineWidth;
    uv = abs(uv);
    uv = -(uv - halfRemainingSpace);
    return min(uv.x, uv.y);
}
// centered over lineposy
float hline_d(vec2 uv, float lineposy, float lineWidth)
{
	return distance(uv.y, lineposy) - (lineWidth / 2.0);
}
// centered over lineposx
float vline_d(vec2 uv, float lineposx, float lineWidth)
{
	return distance(uv.x, lineposx) - (lineWidth / 2.0);
}
float circle_d(vec2 uv, vec2 center, float radius)
{
	return length(uv - center) - radius;
}

// not exactly perfectly perfect, but darn close
float pointRectDist(vec2 p, vec2 rectTL, vec2 rectBR)
{
  float dx = max(max(rectTL.x - p.x, 0.), p.x - rectBR.x);
  float dy = max(max(rectTL.y - p.y, 0.), p.y - rectBR.y);
  return max(dx, dy);
}


vec2 getuv(vec2 fragCoord, vec2 newTL, vec2 newSize, out float distanceToVisibleArea, out float vignetteAmt)
{
    vec2 ret = vec2(fragCoord.x / iResolution.x, (iResolution.y - fragCoord.y) / iResolution.y);// ret is now 0-1 in both dimensions
    
    // warp
    //ret = tvWarp(ret / 2.) * 2.;// scale it by 2.
    distanceToVisibleArea = pointRectDist(ret, vec2(0.0), vec2(1.));

    // vignette
    vec2 vignetteCenter = vec2(0.5, 0.5);
	vignetteAmt = 1.0 - distance(ret, vignetteCenter);
    vignetteAmt = 0.03 + pow(vignetteAmt, .25);// strength
    vignetteAmt = clamp(vignetteAmt, 0.,1.);
    
    
    ret *= newSize;// scale up to new dimensions
    float aspect = iResolution.x / iResolution.y;
    ret.x *= aspect;// orig aspect ratio
    float newWidth = newSize.x * aspect;
    return ret + vec2(newTL.x - (newWidth - newSize.x) / 2.0, newTL.y);
}

vec4 drawHole(vec4 inpColor, vec2 uv, vec2 pos)
{
    vec4 circleWhiteColor = vec4(vec3(0.95), 1.);
	float d = circle_d(uv, pos, 0.055);
    return vec4(mix(inpColor.rgb, circleWhiteColor.rgb, circleWhiteColor.a * dtoa(d)), 1.);
}

void main()
{
    vec4 fragColor;
    vec2 fragCoord = vertexUv.xy * iResolution.yy;
    float distanceToVisibleArea;
    float vignetteAmt;
	vec2 uv = getuv(fragCoord, vec2(-1.,1.), vec2(2., -2.), distanceToVisibleArea, vignetteAmt);
    float throwaway;
    //mouse = getuv(iMouse.xy, vec2(-1.,1.), vec2(2., -2.), throwaway, throwaway);

    fragColor = vec4(0.94, 0.96, 0.78, 1.0);// background
    float d;
    
    // grid
    vec4 gridColor = vec4(0.2,0.4,.9, 0.35);
	d = grid_d(uv, vec2(0.10), 0.001);
	fragColor = vec4(mix(fragColor.rgb, gridColor.rgb, gridColor.a * dtoa(d)), 1.);
    
    // red h line
    //vec4 hlineColor = vec4(0.8,0.,.2, 0.55);
	//d = hline_d(uv, 0.60, 0.003);
	//fragColor = vec4(mix(fragColor.rgb, hlineColor.rgb, hlineColor.a * dtoa(d)), 1.);
    
    // red v line
    //vec4 vlineColor = vec4(0.8,0.,.2, 0.55);
	//d = vline_d(uv, -1.40, 0.003);
	//fragColor = vec4(mix(fragColor.rgb, vlineColor.rgb, vlineColor.a * dtoa(d)), 1.);

    
    // fractal worley crumpled paper effect
    float wsize = 0.8;
    const int iterationCount = 6;
    vec2 normal = vec2(0.);
    float influenceFactor = 1.0;
    for(int i = 0; i < iterationCount; ++ i)
    {
        vec3 w = worley(uv, wsize);
		normal.xy += influenceFactor * w.xy;
        wsize *= 0.5;
        influenceFactor *= 0.9;
    }
    
    // lighting
    //vec3 lightPos = vec3(mouse, 8.);
    //vec4 lightColor = vec4(vec3(0.99),0.6);
    //vec4 ambientColor = vec4(vec3(0.99),0.5);
	//fragColor = applyLighting(fragColor, uv, vec3(normal, 4.0), lightPos, lightColor, ambientColor);

    // white circles
    //fragColor = drawHole(fragColor, uv, vec2(-1.6, 0.2));
	//fragColor = drawHole(fragColor, uv, vec2(-1.6, -.7));
    
    // post effects
	//fragColor.rgb *= vignetteAmt;
    gl_FragColor = fragColor;
}`;

const glslGraprogBackgroundFragmentShader = `varying vec2 vertexUv;
uniform vec2 iResolution;
uniform float iTime;

// Solid Colors
vec3 red = vec3(1.0,0.0,0.0);
vec3 green = vec3(0.0,1.0,0.0);
vec3 blue = vec3(0.0,0.0,1.0);
vec3 black = vec3(0.0,0.0,0.0);
vec3 white = vec3(1.0,1.0,1.0);
// Concrete
vec3 concreteLite = vec3(0.909, 0.905, 0.917);
vec3 concreteDark = vec3(0.760, 0.729, 0.647);
// Lava
vec3 lavaLite = vec3(0.686, 0.203, 0.160);
vec3 lavaDark = vec3(0.580, 0.176, 0.117);
// Marble
vec3 marbleLite = vec3(0.988, 0.988, 0.988);
vec3 marbleStainBlue1 = vec3(0.690, 0.760, 0.811);
vec3 marbleStainBlue2 = vec3(0.647, 0.745, 0.803);
vec3 marbleStainBlue3 = vec3(0.654, 0.756, 0.772);
// Lava Lamp
vec3 lavaLampBG = vec3(0.462, 0.266, 0.6);
vec3 lavaLampLava = vec3(0.929, 0.203, 0.572);
// Clouds
vec3 sky = vec3(0.541, 0.729, 0.827);
vec3 cloud = vec3(0.941, 0.945, 0.941);
// Granite
vec3 graniteBG = vec3(0.956, 0.952, 0.976);
vec3 graniteGray = vec3(0.478, 0.474, 0.494);
vec3 graniteBrown = vec3(0.4, 0.356, 0.341);
vec3 graniteBlack = vec3(0.105, 0.121, 0.160);

// Tartan
vec3 tar1Blue = vec3(0.074, 0.349, 0.505);
vec3 tar1Green = vec3(0.286, 0.541, 0.341);
vec3 tar1White = vec3(1.0,1.0,1.0);
vec3 tar1Black = vec3(0.0,0.0,0.0);
vec3 tar1BG = vec3(0.062, 0.274, 0.109);

// Tartan 2
vec3 tar2BG = vec3(0.152, 0.160, 0.156);
vec3 tar2Blue = vec3(0.176, 0.501, 0.674);
vec3 tar2Orange = vec3(0.603, 0.407, 0.309);

// Art Installation
vec3 violet = vec3(0.662, 0.407, 0.870);
vec3 yellow = vec3(0.968, 0.752, 0.556);


float random (in vec2 uv) {
    return fract(sin(dot(uv.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 uv) {
    vec2 i = floor(uv);
    vec2 f = fract(uv);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

#define OCTAVES 6
float fbm (in vec2 uv) {
    // Initial values
    float value = 0.0;
    float amplitud = .5;
    float frequency = 0.;
    //
    // Loop of octaves
    for (int i = 0; i < OCTAVES; i++) {
        value += amplitud * noise(uv);
        uv *= 2.;
        amplitud *= .5;
    }
    return value;
}

// Simplex noise
vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x*34.0)+1.0)*x); }

float snoise(vec2 v) {
    const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                        0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                        -0.577350269189626,  // -1.0 + 2.0 * C.x
                        0.024390243902439); // 1.0 / 41.0
    vec2 i  = floor(v + dot(v, C.yy) );
    vec2 x0 = v -   i + dot(i, C.xx);
    vec2 i1;
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    i = mod289(i); // Avoid truncation effects in permutation
    vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
        + i.x + vec3(0.0, i1.x, 1.0 ));

    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
    m = m*m ;
    m = m*m ;
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

float lavaLamp(vec2 uv,vec2 shapePos, float times){
    shapePos = vec2(shapePos.x*1.5,shapePos.y*0.3);
    uv -= shapePos;
    
    float angle = atan(uv.y,uv.x);
    float radius = cos(times*angle*0.5);
    return radius;
}

vec4 rectangle(vec2 uv, vec2 pos, float width, float height, vec3 color) {
	float t = 0.0;
	if ((uv.x > pos.x - width / 2.0) && (uv.x < pos.x + width / 2.0)
		&& (uv.y > pos.y - height / 2.0) && (uv.y < pos.y + height / 2.0)) {
		t = 1.0;
	}
	return vec4(color, t);
}

void main()
{
    vec4 fragColor;
    vec2 fragCoord = vertexUv.xy * iResolution.yy;
	vec2 uv = fragCoord.xy / iResolution.xy;
	float ratio = iResolution.x/iResolution.y;
    uv.x *= ratio;
    
    if (patternChoice == 1) // Concrete
    {
        vec3 value = concreteLite;
        
        value = mix(value, concreteDark,random(uv)*0.6);
        value = mix(value, black, random(uv)*0.35);
        
        vec3 stains = vec3(fbm((uv*5.)*2.))*.12;
        
        value = mix(value, concreteLite,(smoothstep(0.03,.12,stains) - smoothstep(.2,.3,stains))*0.3);
    
	    fragColor = vec4(value, 1.0);
    }
    
    else if (patternChoice == 2) // Lava
    {
		vec3 value = black;
        
        vec3 stains = vec3(fbm((uv*1.7)*25.))*.75;
        
        float fade = sin(iTime * 5.)+2.5;
        
        value = mix(value, lavaLite, (smoothstep(0.05,0.1,stains) - smoothstep(.17, .22,stains) * fade));
        
        fragColor = vec4(value,1.0);
    }
    
    else if (patternChoice == 3) // Marble
    {
        vec3 value = marbleLite;
        
        vec3 stains = vec3(fbm((uv*1.2)*18.))*.113;
        vec3 stains2 = vec3(fbm((uv*5.)*1.5))*.12;
        
        vec3 stains3 = vec3(fbm((uv)*5.))*.12;
        vec3 stains4 = vec3(fbm((uv*2.)*2.5))*.12;
        
        value = mix(value, marbleStainBlue1,(smoothstep(0.065,0.1,stains) - smoothstep(0.1, 0.8,stains)));
        //value = mix(value, marbleStainBlue2,(smoothstep(0.065,0.1,stains2) - smoothstep(0.1, 0.8,stains2)));
        //value = mix(value, marbleStainBlue3,(smoothstep(0.09,0.1,stains2) - smoothstep(0.2, 0.5,stains2)));
        //value = mix(value, marbleStainBlue2,(smoothstep(0.07,0.1,stains3) - smoothstep(0.1, 0.8,stains3)));
        
        fragColor = vec4(value, 1.0);
    }
    
    else if (patternChoice == 4) // Lava Lamp
    {
        //float value = lavaLamp(uv,vec2(0.,0.5), 3.);
        //vec2 value = vec2(snoise(uv*3.-iTime));
      	//vec3 color = red*vec3(value, 1.0);
        
        vec3 value = lavaLampBG;
        float lava = lavaLamp(uv*0.5,vec2(snoise(uv*4.-iTime)), 1.);
        
        vec3 color = lavaLampLava*lava;
        value += color;
        
        
        fragColor = vec4(value, 1.0);
    }
  	
    else if (patternChoice == 5) // Clouds
    {
        uv *= 0.5;
        vec3 value = sky;
        
        vec3 clouds = vec3(fbm((uv*sin(iTime*0.25))*20.))*.12;
        
        value = mix(value, cloud,(smoothstep(0.05,0.1,clouds) - smoothstep(0.1, 0.2,clouds)));
        
        fragColor = vec4(value, 1.0);
    }
    
    else if (patternChoice == 6) // Granite
    {
    	vec3 value = graniteBG;
        
        uv *= 2.;
        uv += 2.3;
        vec3 layer1 = vec3(fbm((uv*0.6)*18.))*.113;
        uv *= 0.2;
        uv += 0.5;
        vec3 layer2 = vec3(fbm((uv*0.8)*30.))*.117;
        uv *= 2.2;
        uv += 2.5;
        vec3 layer3 = vec3(fbm((uv*0.4)*15.))*.12;
        
        value = mix(value, graniteBlack,(smoothstep(0.04,0.1,layer1) - smoothstep(0.1, 0.8,layer1)));
        value = mix(value, graniteBrown,(smoothstep(0.04,0.1,layer2) - smoothstep(0.1, 0.8,layer2)));
        value = mix(value, graniteGray,(smoothstep(0.072,0.1,layer3) - smoothstep(0.1, 0.8,layer3)));
        
        fragColor = vec4(value, 1.0);
    }
    
    else if (patternChoice == 7) // Scottish Tartan 1
    {
        vec3 value = tar1BG; 
            
        uv *= 2.;
    	uv = fract(uv);
        
        float blackTile = 0.;
        float greenTile = 0.;
        float blueTile = 0.;
        float whiteTile = 0.;
        
        // Blue blocks
        // Horizontal
    	blueTile += step(0.15, uv.x) - step(0.35, uv.x);
    	blueTile += step(0.65, uv.x) - step(0.85, uv.x);
        // Vertical
        blueTile += step(0.15, uv.y) - step(0.35, uv.y);
        blueTile += step(0.68, uv.y) - step(0.85, uv.y);
        
        // White lines
        // Vertical
        whiteTile += step(0.0001, uv.x) - step(0.02, uv.x);
        whiteTile += step(0.49, uv.x) - step(0.51, uv.x);
        // Horizontal
        whiteTile += step(0.220, uv.y) - step(0.245, uv.y);
    	whiteTile += step(0.755, uv.y) - step(0.775, uv.y);
        
        // Black lines
        // Vertical
        blackTile += step(0.10, uv.x) - step(0.11, uv.x);
        blackTile += step(0.12, uv.x) - step(0.13, uv.x);
        blackTile += step(0.14, uv.x) - step(0.15, uv.x);
        blackTile += step(0.18, uv.x) - step(0.19, uv.x);
        blackTile += step(0.2, uv.x) - step(0.21, uv.x);
        blackTile += step(0.29, uv.x) - step(0.3, uv.x);
        blackTile += step(0.31, uv.x) - step(0.32, uv.x);
        blackTile += step(0.35, uv.x) - step(0.36, uv.x);
        blackTile += step(0.37, uv.x) - step(0.38, uv.x);
        blackTile += step(0.39, uv.x) - step(0.4, uv.x);
        blackTile += step(0.59, uv.x) - step(0.6, uv.x);
        blackTile += step(0.61, uv.x) - step(0.62, uv.x);
        blackTile += step(0.63, uv.x) - step(0.64, uv.x);
        blackTile += step(0.68, uv.x) - step(0.69, uv.x);
        blackTile += step(0.7, uv.x) - step(0.71, uv.x);
        blackTile += step(0.77, uv.x) - step(0.78, uv.x);
        blackTile += step(0.8, uv.x) - step(0.81, uv.x);
        blackTile += step(0.85, uv.x) - step(0.86, uv.x);
        blackTile += step(0.87, uv.x) - step(0.88, uv.x);
        blackTile += step(0.89, uv.x) - step(0.9, uv.x);
        // Horizontal
        blackTile += step(0.07, uv.y) - step(0.08, uv.y);
        blackTile += step(0.05, uv.y) - step(0.06, uv.y);
        blackTile += step(0.39, uv.y) - step(0.4, uv.y);
        blackTile += step(0.41, uv.y) - step(0.42, uv.y);
        blackTile += step(0.59, uv.y) - step(0.6, uv.y);
        blackTile += step(0.61, uv.y) - step(0.62, uv.y);
        blackTile += step(0.90, uv.y) - step(0.91, uv.y);
        blackTile += step(0.92, uv.y) - step(0.93, uv.y);
        
        // Apply
        value = mix(value, tar1Blue, vec3(blueTile * noise(uv* 1000.)));
        value = mix(value, tar1White, vec3(whiteTile * noise(uv * 200.)));
        value = mix(value, tar1Black, vec3(blackTile * noise(uv * 200.)));
        
        fragColor = vec4(value, 1.0);
    }
    
    else if (patternChoice == 8) // Scottish Tartan 2
    {
        vec3 value = tar2BG; 
            
        uv *= 2.;
    	uv = fract(uv);
        
        float blackTile = 0.;
        float greenTile = 0.;
        float blueTile = 0.;
        float orangeTile = 0.;
        
        // Blue blocks
        // Horizontal
    	blueTile += step(0.15, uv.x) - step(0.35, uv.x);
    	blueTile += step(0.65, uv.x) - step(0.85, uv.x);
        // Vertical
        blueTile += step(0.15, uv.y) - step(0.35, uv.y);
        blueTile += step(0.68, uv.y) - step(0.85, uv.y);
        
        // Orange lines
        // Vertical
        orangeTile += step(0.0001, uv.x) - step(0.02, uv.x);
        orangeTile += step(0.49, uv.x) - step(0.51, uv.x);
        orangeTile += step(0.04, uv.x) - step(0.05, uv.x);
        orangeTile += step(0.97, uv.x) - step(0.98, uv.x);
        orangeTile += step(0.46, uv.x) - step(0.47, uv.x);
        orangeTile += step(0.53, uv.x) - step(0.54, uv.x);
        // Horizontal
        orangeTile += step(0.220, uv.y) - step(0.245, uv.y);
    	orangeTile += step(0.755, uv.y) - step(0.775, uv.y);
        orangeTile += step(0.200, uv.y) - step(0.210, uv.y);
        orangeTile += step(0.255, uv.y) - step(0.265, uv.y);
        orangeTile += step(0.730, uv.y) - step(0.740, uv.y);
        orangeTile += step(0.79, uv.y) - step(0.8, uv.y);
        
        // Apply
        value = mix(value, tar2Blue, vec3(blueTile * noise(uv* 1200.)));
        value = mix(value, tar2Orange, vec3(orangeTile * noise(uv * 200.)));
        
        fragColor = vec4(value, 1.0);
    }
    
    else if (patternChoice == 9) // Art Installation
    {
        vec2 uv2 = fragCoord.xy/iResolution.xy;
        uv2 -= 0.5;
        uv2.x *= 1.6;
        uv2.y *= 1.9;
        
        float dist = length(uv2);
        
        vec2 fragCo = fragCoord.xy;
        vec2 center = iResolution.xy * 0.5;
        float width = iResolution.x * 0.4;
        float height = iResolution.x * 0.2;
        
        vec3 value = mix(violet, yellow, dist);
        
        vec4 finalVal = vec4(value, 1.0);
        
        vec4 rect = rectangle(fragCo, center, width, height, violet);
        
        fragColor = mix(finalVal,rect,rect.a);
        fragColor = finalVal;
    }
    gl_FragColor = fragColor;
}`;

const glslMarbleBackgroundFragmentShader = `varying vec2 vertexUv;
uniform vec2 iResolution;
uniform float iTime;
// variant of Vorocracks: https://shadertoy.com/view/lsVyRy
// integrated with cracks here: https://www.shadertoy.com/view/Xd3fRN

#define MM 0

#define VARIANT 1              // 1: amplifies Voronoi cell jittering
#if VARIANT
      float ofs = .5;          // jitter Voronoi centers in -ofs ... 1.+ofs
#else
      float ofs = 0.;
#endif
    
//int FAULT = 1;                 // 0: crest 1: fault

float RATIO = 1.,              // stone length/width ratio
 /*   STONE_slope = .3,        // 0.  .3  .3  -.3
      STONE_height = 1.,       // 1.  1.  .6   .7
      profile = 1.,            // z = height + slope * dist ^ prof
 */   
      CRACK_depth = 3.,
      CRACK_zebra_scale = 1.,  // fractal shape of the fault zebra
      CRACK_zebra_amp = .67,
      CRACK_profile = 1.,      // fault vertical shape  1.  .2 
      CRACK_slope = 50.,       //                      10.  1.4
      CRACK_width = .0;
    

// std int hash, inspired from https://www.shadertoy.com/view/XlXcW4
vec3 hash3( uvec3 x ) 
{
#   define scramble  x = ( (x>>8U) ^ x.yzx ) * 1103515245U // GLIB-C const
    scramble; scramble; scramble; 
    return vec3(x) / float(0xffffffffU) + 1e-30; // <- eps to fix a windows/angle bug
}

// === Voronoi =====================================================
// --- Base Voronoi. inspired by https://www.shadertoy.com/view/MslGD8

#define hash22(p)  fract( 18.5453 * sin( p * mat2(127.1,311.7,269.5,183.3)) )
#define disp(p) ( -ofs + (1.+2.*ofs) * hash22(p) )

vec3 voronoi( vec2 u )  // returns len + id
{
    vec2 iu = floor(u), v;
	float m = 1e9,d;
#if VARIANT
    for( int k=0; k < 25; k++ ) {
        vec2  p = iu + vec2(k%5-2,k/5-2),
#else
    for( int k=0; k < 9; k++ ) {
        vec2  p = iu + vec2(k%3-1,k/3-1),
#endif
            o = disp(p),
      	      r = p - u + o;
		d = dot(r,r);
        if( d < m ) m = d, v = r;
    }

    return vec3( sqrt(m), v+u );
}

// --- Voronoi distance to borders. inspired by https://www.shadertoy.com/view/ldl3W8
vec3 voronoiB( vec2 u )  // returns len + id
{
    vec2 iu = floor(u), C, P;
	float m = 1e9,d;
#if VARIANT
    for( int k=0; k < 25; k++ ) {
        vec2  p = iu + vec2(k%5-2,k/5-2),
#else
    for( int k=0; k < 9; k++ ) {
        vec2  p = iu + vec2(k%3-1,k/3-1),
#endif
              o = disp(p),
      	      r = p - u + o;
		d = dot(r,r);
        if( d < m ) m = d, C = p-iu, P = r;
    }

    m = 1e9;
    
    for( int k=0; k < 25; k++ ) {
        vec2 p = iu+C + vec2(k%5-2,k/5-2),
		     o = disp(p),
             r = p-u + o;

        if( dot(P-r,P-r)>1e-5 )
        m = min( m, .5*dot( (P+r), normalize(r-P) ) );
    }

    return vec3( m, P+u );
}

// === pseudo Perlin noise =============================================
#define rot(a) mat2(cos(a),-sin(a),sin(a),cos(a))
int MOD = 1;  // type of Perlin noise
    
// --- 2D
#define hash21(p) fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453123)
float noise2(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p); f = f*f*(3.-2.*f); // smoothstep

    float v= mix( mix(hash21(i+vec2(0,0)),hash21(i+vec2(1,0)),f.x),
                  mix(hash21(i+vec2(0,1)),hash21(i+vec2(1,1)),f.x), f.y);
	return   MOD==0 ? v
	       : MOD==1 ? 2.*v-1.
           : MOD==2 ? abs(2.*v-1.)
                    : 1.-abs(2.*v-1.);
}

float fbm2(vec2 p) {
    float v = 0.,  a = .5;
    mat2 R = rot(.37);

    for (int i = 0; i < 9; i++, p*=2.,a/=2.) 
        p *= R,
        v += a * noise2(p);

    return v;
}
#define noise22(p) vec2(noise2(p),noise2(p+17.7))
vec2 fbm22(vec2 p) {
    vec2 v = vec2(0);
    float a = .5;
    mat2 R = rot(.37);

    for (int i = 0; i < 6; i++, p*=2.,a/=2.) 
        p *= R,
        v += a * noise22(p);

    return v;
}
vec2 mfbm22(vec2 p) {  // multifractal fbm 
    vec2 v = vec2(1);
    float a = .5;
    mat2 R = rot(.37);

    for (int i = 0; i < 6; i++, p*=2.,a/=2.) 
        p *= R,
        //v *= 1.+noise22(p);
          v += v * a * noise22(p);

    return v-1.;
}

/*
// --- 3D 
#define hash31(p) fract(sin(dot(p,vec3(127.1,311.7, 74.7)))*43758.5453123)
float noise3(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p); f = f*f*(3.-2.*f); // smoothstep

    float v= mix( mix( mix(hash31(i+vec3(0,0,0)),hash31(i+vec3(1,0,0)),f.x),
                       mix(hash31(i+vec3(0,1,0)),hash31(i+vec3(1,1,0)),f.x), f.y), 
                  mix( mix(hash31(i+vec3(0,0,1)),hash31(i+vec3(1,0,1)),f.x),
                       mix(hash31(i+vec3(0,1,1)),hash31(i+vec3(1,1,1)),f.x), f.y), f.z);
	return   MOD==0 ? v
	       : MOD==1 ? 2.*v-1.
           : MOD==2 ? abs(2.*v-1.)
                    : 1.-abs(2.*v-1.);
}

float fbm3(vec3 p) {
    float v = 0.,  a = .5;
    mat2 R = rot(.37);

    for (int i = 0; i < 9; i++, p*=2.,a/=2.) 
        p.xy *= R, p.yz *= R,
        v += a * noise3(p);

    return v;
}
*/
    
// ======================================================

void main()
{
    vec4 O;
    vec2 U = vertexUv.xy * iResolution.yy;
    U *= 4./iResolution.y;
    U.x += iTime;                                     // for demo
 // O = vec4( 1.-voronoiB(U).x,voronoi(U).x, 0,0 );   // for tests
    vec2 I = floor(U/2.); 
    bool vert = mod(I.x+I.y,2.)==0.; //if (vert) U = U.yx;
    vec3 H0;
    O-=O;

    for(float i=0.; i<CRACK_depth ; i++) {
        vec2 V =  U / vec2(RATIO,1),                  // voronoi cell shape
             D = CRACK_zebra_amp * fbm22(U/CRACK_zebra_scale) * CRACK_zebra_scale;
        vec3  H = voronoiB( V + D ); if (i==0.) H0=H;
        float d = H.x;                                // distance to cracks
   /*         r = voronoi(V).x,                       // distance to center
              s = STONE_height-STONE_slope*pow(r,profile);// stone interior
    */                                                // cracks
        d = min( 1., CRACK_slope * pow(max(0.,d-CRACK_width),CRACK_profile) );
  
        O += vec4(1.-d) / exp2(i);
        U *= 1.5 * rot(.37);
    }
    /*
    O = vec4( 
        FAULT==1 ? d * s                              // fault * stone
                 : mix(1.,s, d)                       // crest or stone
            ); */
    //if (vert) O = 1.-O; O *= vec4(.9,.85,.85,1);      // for demo
    
#if MM
    O.g = hash3(uvec3(H0.yz,1)).x;
#endif
    gl_FragColor = mix(vec4(0.9, 0.9, 0.7, 1.0), vec4(1.0, 0.8, 0.7, 1.0), O.r);
}`;

const glslKraftBackgroundFragmentShader = `varying vec2 vertexUv;
uniform vec2 iResolution;
vec3 mod289(vec3 x) {return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec2 mod289(vec2 x) {return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec3 permute(vec3 x) {return mod289(((x*34.0)+1.0)*x);}
float snoise(vec2 v)
{
    const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                        0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                        -0.577350269189626,  // -1.0 + 2.0 * C.x
                        0.024390243902439); // 1.0 / 41.0
    // First corner
    vec2 i  = floor(v + dot(v, C.yy) );
    vec2 x0 = v -   i + dot(i, C.xx);

    // Other corners
    vec2 i1;
    //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
    //i1.y = 1.0 - i1.x;
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    // x0 = x0 - 0.0 + 0.0 * C.xx ;
    // x1 = x0 - i1 + 1.0 * C.xx ;
    // x2 = x0 - 1.0 + 2.0 * C.xx ;
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;

    // Permutations
    i = mod289(i); // Avoid truncation effects in permutation
    vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
    + i.x + vec3(0.0, i1.x, 1.0 ));

    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
    m = m*m ;
    m = m*m ;

    // Gradients: 41 points uniformly over a line, mapped onto a diamond.
    // The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;

    // Normalise gradients implicitly by scaling m
    // Approximation of: m *= inversesqrt( a0*a0 + h*h );
    m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

    // Compute final noise value at P
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}
float sdLine( vec2 p, vec2 a, vec2 b, float r )
{
    vec2 pa = p - a, ba = b - a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h ) - r;
}
vec2 opRep( vec2 p, vec2 c )
{ 
    return mod(p,c)-0.5*c;
}
vec2 opRepFlip( vec2 p, vec2 c )
{ 
    vec2 fpc = floor(p/c);
    float flip = mod((fpc.x + fpc.y), 2.);
    vec2 ret = p - c*(fpc + 0.5);
    if(flip >= 1.)return ret.yx;
    return ret;
}
vec2 opWarp( vec2 p )
{ 
    // return vec2(sin(p.x+2.*p.y),sin(p.y+p.x));
    return vec2(p.x+0.1*sin(20.*p.y),p.y);
}

float nnoise( in vec2 uv ){return 0.5 + 0.5*snoise(uv);} //norm [0,1]
float rnoise( in vec2 uv ){return 1. - abs(snoise(uv));} //ridge
float fbm( vec2 x , int oct ) 
{
    float f = 1.98;  // could be 2.0
    float s = 0.49;  // could be 0.5
    float a = 0.0;
    float b = .9;
    for( int i=0; i < 10; i++ )
    {
        if(i >= oct) break;
        float n = nnoise(x);
        a += b * n;          // accumulate values		
        b *= s;
        x *= f;
    }
    return a;
}
float fbmr( vec2 x, int oct ) 
{
    float f = 1.98;  // could be 2.0
    float s = 0.9;  // could be 0.5
    float a = 0.0;
    float b = .4; //0.5
    for( int i=0; i < 10; i++ )
    {
        if(i >= oct) break;
        float n = rnoise(x);
        a += b * n;          // accumulate values		
        b *= s;
        x *= f;
    }
    return a;
}
float fbm2( in vec2 p )
{
    vec2 q = vec2( fbm( p + vec2(0.0,0.0) ,2 ),
                    fbm( p + vec2(5.2,1.3) ,2) );

    return fbmr( p + 2.9*q ,3); //4.0
}
float fbm3( in vec2 p )
{
    vec2 q = vec2( fbm( p + vec2(0.0,0.0) ,2),
                    fbm( p + vec2(5.2,1.3) ,2) );

    vec2 r = vec2( fbm( p + 4.0*q + vec2(1.7,9.2) ,2), //4q
                    fbm( p + 4.0*q + vec2(8.3,2.8) ,2) );
    // r = normalize(r);
    return fbmr( p*2.0 + .4*r , 3); //4.0
}

void main() {
    float aspect = 1.0;//iResolution.x/iResolution.y;
    float nVal, n, len;
    vec2 uv = vertexUv.xy;
	vec2 vUv = uv;
    vUv *= iResolution.y/666.;
    vUv.x *= aspect;
    //vUv.x += iTime/50.;
    vec2 c,luv,nuv = vUv*7.;
    //nuv += rnd;

    //background
    n = fbm3(nuv);
    nVal = 2.9*(n-0.06);
    // nVal = 2.5*(n-0.2);
    //nVal *= 1.-0.35*clamp(abs(vUv.y-0.5)-0.1,0.,1.);
    nVal *= 1.-0.65*smoothstep(0.27,0.45,(clamp(snoise(3.9*nuv)*nnoise(12.2*nuv)*rnoise(9.2*nuv),0.,1.)));
    n = nVal = clamp(nVal,0.,1.);
    //nVal *= 1.14-1.5*clamp(abs(uv.y-0.5)-0.25,0.,1.);
    // nVal *= (0.85+.1*rnoise(21.*nuv)*rnoise(17.*nuv));
    nVal *= (0.9+.1*abs(snoise(14.*nuv)));
    nVal *= (0.9+.1*abs(snoise(24.*nuv)));
    nVal *= (0.86+.14*nnoise(41.*nuv));
    // nVal *= (0.87+0.04*nnoise(0.6*nuv));
    nVal *= (0.95+0.05*fbm(1.2*nuv,2));
    
    //white spots
    // nVal += (1.-n)*2.7*nnoise(3.*nuv)*clamp(snoise(1.3*nuv),0.,1.); 
    // nVal += (1.-n)*1.72*nnoise(4.*nuv)*nnoise(6.3*nuv); 
    nVal += (1.-n)* smoothstep(0.5,1.,1.72*nnoise(3.*nuv)*nnoise(4.3*nuv)); 

    vec2 d1,d2;
    d1 = vec2(rnoise(1.45*nuv),rnoise(1.45*nuv+vec2(-7.2,6.9)));
    
    nuv+=22.;
    float lm = 0.6;
    //lines 1
    luv = nuv;
    luv += 0.08*d1;
    luv += 0.85 * vec2(snoise(.22*luv), snoise(.22*luv + vec2(4.2,-9.1)));
    c = vec2(1.585);
    len = 0.04;
    n = sdLine( 
        opRepFlip(luv,c)
    ,vec2(-c.x*len,-len),vec2(c.x*len,len),0.0001);
    nVal *= lm+(1.-lm)*smoothstep(0.,0.01,n);
    
    //lines 2
    luv = nuv + vec2(-13.2,15.1);
    luv += 0.09*d1;
    luv += 0.85 * vec2(snoise(.22*luv), snoise(.22*luv + vec2(11.2,-9.1)));
    c = vec2(1.79);
    len = 0.04; 
    n = sdLine( 
        opRepFlip(luv,c)
    ,vec2(-c.x*len,len),vec2(c.x*len,-len),0.0001);
    nVal *= lm+(1.-lm)*smoothstep(0.,0.01,n);
    
    //lines 3
    luv = nuv + vec2(27.2,-21.5);
    luv += 0.09*d1;
    luv += 0.85 * vec2(snoise(.22*luv), snoise(.22*luv + vec2(-17.2,8.7)));
    c = vec2(2.07);
    len = 0.085; 
    n = sdLine( 
        opRepFlip(luv,c)
    ,vec2(-c.x*len,0),vec2(c.x*len,0),0.0001);
    nVal *= lm+(1.-lm)*smoothstep(0.,0.01,n);
    
    //lines 3.2
    luv = nuv + vec2(-17.8,-28.5);
    luv += 0.09*d1;
    luv += 0.85 * vec2(snoise(.22*luv), snoise(.22*luv + vec2(-17.2,8.7)));
    c = vec2(1.87);
    len = 0.095; 
    n = sdLine( 
        opRepFlip(luv,c)
    ,vec2(-c.x*len,0),vec2(c.x*len,0),0.0001);
    nVal *= lm+(1.-lm)*smoothstep(0.,0.01,n);
    
    vec3 tint = vec3(1.,0.9,0.7);
    gl_FragColor = vec4(vec3(nVal)*tint,1.);//vec4(vUv, 0.0, 1.0);
}`;

const glslLineBackgroundFragmentShader = `varying vec2 vertexUv;
// Line SDF from iq.
float udSegment( in vec2 p, in vec2 a, in vec2 b )
{
    vec2 ba = b-a;
    vec2 pa = p-a;
    float h =clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length(pa-h*ba);
}

// Hashes from Dave Hoskins
float hash11(float p)
{
    p += 1.5;
    p = fract(p * .1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}
float hash12(vec2 p)
{
    p += 0.2512;
	vec3 p3  = fract(vec3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}
vec4 hash42(vec2 p)
{
    p += 1.;
	vec4 p4 = fract(vec4(p.xyxy) * vec4(.1031, .1030, .0973, .1099));
    p4 += dot(p4, p4.wzxy+33.33);
    return fract((p4.xxyz+p4.yzzw)*p4.zywx);

}

vec4 valueNoise(vec2 t, float w){
    vec2 fr = fract(t);
    vec2 fl = floor(t);
	return 
        mix(
            mix( 
                hash42(vec2(fl.x, fl.y)),
                hash42(vec2(fl.x, fl.y + 1.)),
            	smoothstep(0.,1.,fr.y)
            ),
            mix( 
                hash42(vec2(fl.x + 1.,fl.y)),
                hash42(vec2(fl.x + 1.,fl.y + 1.)),
            	smoothstep(0.,1.,fr.y)
            ),
        smoothstep(0.,1.,pow(fr.x, abs(w)))
    );
}

// space repetition
#define pmod(p,a) mod(p,a) - 0.5*a

// rotation
#define rot(a) mat2(cos(a),-sin(a),sin(a),cos(a))

void main() {
    vec2 uv = vertexUv.xy;

    vec3 col = vec3(1);
    uv.y += /*iTime*0.1*/ + 10.;
    
    for(float i = 0.; i < 471.; i++){
        // Bunch of random dicerolls.
        float ra = hash11(i);
        float rb = hash11(i + 1.5);
        float rc = hash11(i+0.61);
        float rd = hash11(i+0.21);
        
        
        // Get random repetition distance.
        float repd = mix(0.1,0.2,ra);
        vec2 p = uv + rb * 100.;
        
        // Do vertical strokes, based on chance.
        // The repetition distance is reduced.
        if(hash11(i + 0.15) < 0.1){
            p *= rot(0.5*3.14);
            repd *= 0.5;
        }
        // Just offset uv a bit.
        p.x -= valueNoise(vec2(p.y)*3. + i, 1.).x*0.01;
            
        // Repeat space.
        // Keep id in vec2 id.
        vec2 id = floor(p/repd);
        p = pmod(p,repd);
        
        // Bail out, based on chance from the id.
        if(hash12(id + rc + 0.15) < 0.36){
            continue;
        }
        
        // Random line height.
        float h = hash12(id + rd)*repd*0.4;
        // Line SDF
        float sd = udSegment( p, vec2(0,h), vec2(0,-h));
        // Line SDF offset by noise.
        sd -= valueNoise(vec2(p.y)*40. + i, 1.).x*0.001;
        
        // Line opacity.
        float lineWeight = hash12(id + ra);
        lineWeight = pow(lineWeight,5.)*0.15;
        
        // Fade out line.
        sd += smoothstep(h/2.,h,abs(p.y))*0.003;
        
        // Draw.
        col = mix(
            col,
            vec3(0),
            smoothstep(0.0018,0.,sd)*lineWeight
        );
    }

    col *= 0.9;
    col = pow(col,vec3(0.4545));
    gl_FragColor = vec4(col,1.0);
}`;
