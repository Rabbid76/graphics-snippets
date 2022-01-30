[![StackOverflow](https://stackexchange.com/users/flair/7322082.png)](https://stackoverflow.com/users/5577765/rabbid76?tab=profile)

---

# GLSL code snippets

## Shader stages

**[Is dynamically load shaders in opengl senseless?](https://stackoverflow.com/questions/50698600/is-dynamically-load-shaders-in-opengl-sensless/50706605#50706605)**

It is sufficient to link the shader program object once, after all attached shader objects have been compiled.

[OpenGL 4.6 API Core Profile Specification; 7.3. PROGRAM OBJECTS; page 94](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf):

>Shader objects may be attached to program objects before source code has been loaded into the shader object, or before the shader object has been compiled or specialized.

Note, it is sufficient that the shader object is successfully compiled, before the shader program object, where it is attached to, gets linked.

[OpenGL 4.6 API Core Profile Specification; 7.3. PROGRAM OBJECTS; page 94](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf):

>Multiple shader objects of the same type may be attached to a single program object, and a single shader object may be attached to more than one program object.

e.g.

One shader object contains a function (`FragColor`) 

```glsl
#version 460

uniform sampler2D u_texture;

vec4 FragColor(vec2 uv)
{
    return texture(u_texture, uv);
}
```

A second shader object of the same type contains the function signature (but not the implementation) and the usage of the function.

```glsl
#version 460

in vec2 vUV;
out vec4 fragColor;

vec4 FragColor(vec2 uv);

void main()
{
    fragColor = FragColor(vUV);
}
```

Both of the above code snippets can be placed to 2 separate shader objects of type `GL_FRAGMENT_SHADER`. Each of the 2 shader objects can be successfully compiled. And if they are attached to the same shader program object, then the shader program object can be successfully liked.

See also [Attaching multiple shaders of the same type in a single OpenGL program?](https://stackoverflow.com/questions/9168252/attaching-multiple-shaders-of-the-same-type-in-a-single-opengl-program)

**[Will GLSL compiler remove unnecessary variable initialization?](https://stackoverflow.com/questions/57534921/will-glsl-compiler-remove-unnecessary-variable-initialization/57535173#57535173)**

The glsl shader code is compiled by the graphics driver (except you generate [SPIR-V](https://www.khronos.org/opengl/wiki/SPIR-V)), so it depends on. But a modern graphics driver will do such optimizations.

By default optimizations are switched on, but they can be switched off by

```glsl
#pragma optimize(off)
```

See [OpenGL Shading Language 4.60 Specification - 3.3. Preprocessor](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.html#preprocessor)

**[Fragment shader ignores texture coordinates](https://stackoverflow.com/questions/58746783/fragment-shader-ignores-texture-coordinates/58747040#58747040)**  

The output of a shader stage is linked to the input of the next shader stage by its name (except when you use a layout qualifier). See [interface matching rules between shader stages.](https://www.khronos.org/opengl/wiki/Shader_Compilation#Interface_matching)

## Color space

[HSL and HSV](https://translate.google.at/?hl=de&tab=rT&sl=de&tl=en&text=farbraum&op=translate)

```glsl
const float Epsilon = 1e-10;
```

```glsl
vec3 RGBtoHCV( in vec3 RGB )
{
   vec4 P = (RGB.g < RGB.b) ? vec4(RGB.bg, -1.0, 2.0/3.0) : vec4(RGB.gb, 0.0, -1.0/3.0);
   vec4 Q = (RGB.r < P.x) ? vec4(P.xyw, RGB.r) : vec4(RGB.r, P.yzx);
   float C = Q.x - min(Q.w, Q.y);
   float H = abs((Q.w - Q.y) / (6.0 * C + Epsilon) + Q.z);
   return vec3(H, C, Q.x);
}
```

```glsl
vec3 RGBtoHSV(in vec3 RGB)
{
    vec3 HCV = RGBtoHCV(RGB);
    float S = HCV.y / (HCV.z + Epsilon);
    return vec3(HCV.x, S, HCV.z);
}
```

```glsl
vec3 HUEtoRGB(in float H)
{
    float R = abs(H * 6.0 - 3.0) - 1.0;
    float G = 2.0 - abs(H * 6.0 - 2.0);
    float B = 2.0 - abs(H * 6.0 - 4.0);
    return clamp( vec3(R,G,B), 0.0, 1.0 );
}
```

```glsl
vec3 HSVtoRGB(in vec3 HSV)
{
    vec3 RGB = HUEtoRGB(HSV.x);
    return ((RGB - 1.0) * HSV.y + 1.0) * HSV.z;
}
```

```glsl
vec3 RGBtoHSL(in vec3 RGB)
{
    vec3 HCV = RGBtoHCV(RGB);
    float L = HCV.z - HCV.y * 0.5;
    float S = HCV.y / (1.0 - abs(L * 2.0 - 1.0) + Epsilon);
    return vec3(HCV.x, S, L);
}
```

```glsl
vec3 HSLtoRGB( in vec3 HSL )
{
    vec3 RGB = HUEtoRGB(HSL.x);
    float C = (1.0 - abs(2.0 * HSL.z - 1.0)) * HSL.y;
    return (RGB - 0.5) * C + HSL.z;
}
```