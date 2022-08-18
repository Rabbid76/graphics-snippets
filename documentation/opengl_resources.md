[![StackOverflow](https://stackexchange.com/users/flair/7322082.png)](https://stackoverflow.com/users/5577765/rabbid76?tab=profile)

---

<!-- TOC -->

- [OpenGL resources and reference](#opengl-resources-and-reference)
    - [GLM](#glm)
    - [GLEW](#glew)
    - [GLFW](#glfw)
    - [GLUT](#glut)
    - [Assimp](#assimp)
- [Related and miscellaneous](#related-and-miscellaneous)
    - [Context](#context)
        - [Shared context](#shared-context)
    - [Debug context and Debug output](#debug-context-and-debug-output)
    - [Direct State Access](#direct-state-access)
    - [Errors](#errors)
    - [Execution Model](#execution-model)
    - [Extensions](#extensions)
    - [History](#history)
    - [Light](#light)
    - [Multiple GPUs](#multiple-gpus)
    - [Multiple monitors](#multiple-monitors)
    - [iOS](#ios)
    - [Off screen](#off-screen)
    - [Performance](#performance)
    - [Sampling](#sampling)
    - [Shader program](#shader-program)
    - [Support](#support)
    - [Transparent window background](#transparent-window-background)
        - [three.js](#threejs)
    - [Threading](#threading)
    - [WebGL](#webgl)
        - [WebGL - Geometry shader](#webgl---geometry-shader)
    - [Windows](#windows)
- [Drawing](#drawing)
    - [Buffer](#buffer)
        - [Depth buffer](#depth-buffer)
        - [Frame Buffer](#frame-buffer)
        - [Uniform Buffer Object UBO and Shader storage buffer SSBO](#uniform-buffer-object-ubo-and-shader-storage-buffer-ssbo)
        - [Vertex buffer](#vertex-buffer)
    - [Draw calls](#draw-calls)
        - [Error](#error)
    - [Fragment shader](#fragment-shader)
    - [Geometry shader](#geometry-shader)
    - [Gooey](#gooey)
    - [Glow](#glow)
    - [GLSL](#glsl)
    - [Height map and height map normals](#height-map-and-height-map-normals)
    - [Image format](#image-format)
    - [Invocations](#invocations)
    - [Lines](#lines)
        - [Line stipple](#line-stipple)
        - [Line thickness](#line-thickness)
    - [Matrix and matrix transformation](#matrix-and-matrix-transformation)
    - [Mip mapping](#mip-mapping)
    - [Multi Sampling](#multi-sampling)
    - [Morphing](#morphing)
    - [Multi Indexing](#multi-indexing)
    - [Noise](#noise)
    - [Normal matrix - transpose inverse](#normal-matrix---transpose-inverse)
    - [Outline](#outline)
    - [Projection](#projection)
    - [Quaternion](#quaternion)
    - [Ray tracing](#ray-tracing)
    - [Screen space ambient occlusion - SSAO](#screen-space-ambient-occlusion---ssao)
    - [Screen Space Local Reflections SSLR](#screen-space-local-reflections-sslr)
    - [Shader](#shader)
    - [Silhouette detection](#silhouette-detection)
    - [Stencil](#stencil)
    - [Tangent space](#tangent-space)
    - [Text rendering](#text-rendering)
    - [Texture](#texture)
    - [Transparency](#transparency)
    - [Triangle strips](#triangle-strips)
    - [Vertex specification](#vertex-specification)
    - [Zoom](#zoom)

<!-- /TOC -->

# OpenGL resources and reference

## GLM

- [GLM](https://glm.g-truc.net/0.9.8/index.html)
- [GLM GitHub](https://github.com/g-truc/glm)

## GLEW

- [GLEW - The OpenGL Extension Wrangler Library](http://glew.sourceforge.net/)

## GLFW

- [GLFW](http://www.glfw.org/)
  
## GLUT

- [GLUT](https://www.opengl.org/resources/libraries/glut/)

GLUT, FreeGlut, GLFW, etc...
Stackoverflow answers

## Assimp

[The Open-Asset-Importer-Lib](http://www.assimp.org/)  
[The Open-Asset-Importer-Lib - Viewer](http://cms.assimp.org/index.php/viewer)  
[irrXML 1.2 API documentation](http://sir-kimmi.de/assimp/lib_html/index.html)  

---

# Related and miscellaneous

## Context

[OpenGL Context](https://www.khronos.org/opengl/wiki/OpenGL_Context)  
[OpenGL and multithreading](https://www.khronos.org/opengl/wiki/OpenGL_and_multithreading)  
[Legacy OpenGL](https://www.khronos.org/opengl/wiki/Legacy_OpenGL)  
[Fixed Function Pipeline](https://www.khronos.org/opengl/wiki/Fixed_Function_Pipeline)  
[Getting Started](https://www.khronos.org/opengl/wiki/Getting_Started)  
[Creating an OpenGL Context (WGL)](https://www.khronos.org/opengl/wiki/Creating_an_OpenGL_Context_(WGL))  

### Shared context

[What is shareable between OpenGL contexts and how to enable sharing](https://stackoverflow.com/questions/55885139/what-is-shareable-between-opengl-contexts-and-how-to-enable-sharing)  

## Debug context and Debug output

[Debug Output](https://www.khronos.org/opengl/wiki/Debug_Output)

[GitHub - graphics-snippets - C++ code snippets - Debug context and Debug output](https://github.com/Rabbid76/graphics-snippets/blob/master/documentation/cpp_code_snippets.md#debug-context-and-debug-output)

## Direct State Access

[Direct State Access](https://www.khronos.org/opengl/wiki/Direct_State_Access)

## Errors

| error                  | hex    | number |
|------------------------|--------|--------|
| `GL_INVALID_ENUM`      | 0x0500 | 1280   |
| `GL_INVALID_VALUE`     | 0x0501 | 1281   |
| `GL_INVALID_OPERATION` | 0x0502 | 1282   |
| `GL_STACK_OVERFLOW`    | 0x0503 | 1283   |
| `GL_STACK_UNDERFLOW`   | 0x0504 | 1284   |
| `GL_OUT_OF_MEMORY`     | 0x0505 | 1285   |

## Execution Model

[https://stackoverflow.com/questions/59793047/what-is-proper-way-of-handling-data-in-compute-shader](https://stackoverflow.com/questions/59712670/opengl-uniform-name-pointer-as-string/59712917#59712917)  
[When does OpenGL get finished with pointers in functions?](https://stackoverflow.com/questions/15697861/when-does-opengl-get-finished-with-pointers-in-functions)  

## Extensions

[EXT_shader_framebuffer_fetch](https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_shader_framebuffer_fetch.txt)  
[KHR_blend_equation_advanced](https://www.khronos.org/registry/OpenGL/extensions/KHR/KHR_blend_equation_advanced.txt)  

[OpenGL extension availability on newer contexts](https://stackoverflow.com/questions/50673534/opengl-extension-availability-on-newer-contexts)  

## History

[Why OpenGL uses column-major matrix order?](https://stackoverflow.com/questions/49232185/why-opengl-uses-column-major-matrix-order)  

## Light

[Concentration of potentially active microfacets in a Normal Distribution Function](https://stackoverflow.com/questions/59236575/concentration-of-potentially-active-microfacets-in-a-normal-distribution-functio)  

## Multiple GPUs

[Can I use different multiGPU in OpenGL?](https://stackoverflow.com/questions/62372029/can-i-use-different-multigpu-in-opengl)  

## Multiple monitors

[OpenGL Context with Multiple Devices (Monitors)](https://stackoverflow.com/questions/45766394/opengl-context-with-multiple-devices-monitors)  
[What happens when rendering on monitors connected to different GPUs](https://stackoverflow.com/questions/49381248/what-happens-when-rendering-on-monitors-connected-to-different-gpus)  

## iOS

[OpenGL ES deprecated in iOS 12 and SKShader](https://stackoverflow.com/questions/53828497/opengl-es-deprecated-in-ios-12-and-skshader)  

## Off screen

[Minimal Windowless OpenGL Context Initialization](https://stackoverflow.com/questions/7062804/minimal-windowless-opengl-context-initialization)  
[Windowless OpenGL](https://stackoverflow.com/questions/2896879/windowless-opengl)  
[How to render offscreen on OpenGL?](https://stackoverflow.com/questions/12157646/how-to-render-offscreen-on-opengl)  
[How to use GLUT/OpenGL to render to a file?](https://stackoverflow.com/questions/3191978/how-to-use-glut-opengl-to-render-to-a-file)  
[PyOpenGL headless rendering](https://stackoverflow.com/questions/54483960/pyopengl-headless-rendering)  
[OpenGL render view without a visible window in python](https://stackoverflow.com/questions/51627603/opengl-render-view-without-a-visible-window-in-python/51672538#51672538)  
[Creating OpenGL context without window](https://stackoverflow.com/questions/12482166/creating-opengl-context-without-window)  

It is not possible to create an [OpenGL Context](https://www.khronos.org/opengl/wiki/OpenGL_Context) with an version above 1.0 without any window.
But it is possible to use a completely hidden window for "offscreen" rendering. 
It is possible to create a initially hidden window with the [GLFW](http://www.glfw.org/) library by setting the [window hint](http://www.glfw.org/docs/latest/window_guide.html#window_hints) `VISIBLE` to `False`.

Sadly it is not possible to create a initially hidden window with [Pygame](https://www.pygame.org/news).  
It is only possible to hide a window after it was created by [`pygame.display.iconify()`](https://www.pygame.org/docs/ref/display.html#pygame.display.iconify).
See also [Hiding pygame display](https://stackoverflow.com/questions/10466590/hiding-pygame-display).

## Performance

[Opengl reduce usage of uniforms](https://stackoverflow.com/questions/63645568/opengl-reduce-usage-of-uniforms)  

## Program pipelines

[Program pipelines](https://www.khronos.org/opengl/wiki/Shader_Compilation#Program_pipelines)

## Sampling

[Rendering Pipeline — Performance — Scaling with regard to amount of pixels](https://stackoverflow.com/questions/48254865/rendering-pipeline-performance-scaling-with-regard-to-amount-of-pixels)  

## Shader program

[LearnOpenGL.com - Shaders](https://learnopengl.com/#!Getting-started/Shaders)  
[The Book of Shaders](https://thebookofshaders.com/)

[Khronos wiki, Rendering Pipeline Overview](https://www.khronos.org/opengl/wiki/Rendering_Pipeline_Overview)  
[Khronos wiki, Shader Compilation](https://www.khronos.org/opengl/wiki/Shader_Compilation)  
[Khronos wiki, GLSL Object](https://www.khronos.org/opengl/wiki/GLSL_Object#Program_pipeline_objects)  
[Khronos wiki, Program Introspection](https://www.khronos.org/opengl/wiki/Program_Introspection)  
[Khronos wiki, Example/GLSL Separate Program Multi Stage](https://www.khronos.org/opengl/wiki/Example/GLSL_Separate_Program_Multi_Stage)  
[Khronos wiki, Example/GLSL Separate Program Basics](https://www.khronos.org/opengl/wiki/Example/GLSL_Separate_Program_Basics)  

## SPIR-V compilation

[SPIR-V compilation](https://www.khronos.org/opengl/wiki/Shader_Compilation#SPIR-V_compilation)

## Support

[Can the OpenGL 'deprecated' functions possibly be unsupported?](https://stackoverflow.com/questions/30962359/can-the-opengl-deprecated-functions-possibly-be-unsupported)  
[Will Vulkan effectively “replace” OpenGL or not?](https://community.khronos.org/t/will-vulkan-effectively-replace-opengl-or-not/4970)  
[Introducing Zink, an OpenGL implementation on top of Vulkan](https://www.khronos.org/news/permalink/introducing-zink-an-opengl-implementation-on-top-of-vulkan)  

## Transparent window background

[How to make an OpenGL rendering context with transparent background?](https://stackoverflow.com/questions/4052940/how-to-make-an-opengl-rendering-context-with-transparent-background)  

### three.js

[How to move shader scripts in external files?](https://stackoverflow.com/questions/45666947/how-to-move-shader-scripts-in-external-files)  

## Threading

[Multithreaded Rendering on OpenGL](https://stackoverflow.com/questions/11097170/multithreaded-rendering-on-opengl)  
[Im trying to use OpenGL with the windows API on different threads](https://stackoverflow.com/questions/59694432/im-trying-to-use-opengl-with-the-windows-api-on-different-threads)  

## WebGL

[WebGL - is there an alternative to embedding shaders in HTML?](https://stackoverflow.com/questions/5878703/webgl-is-there-an-alternative-to-embedding-shaders-in-html)  

### WebGL - Geometry shader

[Geometry shader in web](https://stackoverflow.com/questions/46337470/geometry-shader-in-web)  
[Does every variable have to be set for every vertex emitted in a geometry shader?](https://stackoverflow.com/questions/54590642/does-every-variable-have-to-be-set-for-every-vertex-emitted-in-a-geometry-shader)  

## Windows

[Copying pixel data directly from windows Device Context to an openGL rendering context](https://stackoverflow.com/questions/35004121/copying-pixel-data-directly-from-windows-device-context-to-an-opengl-rendering-c)  
[Switching from OpenGL to GDI](https://stackoverflow.com/questions/50587293/switching-from-opengl-to-gdi)  

# Drawing

## Alpha Compositing, Blending and Premultiplied Alpha

[Alpha Compositing, OpenGL Blending and Premultiplied Alpha](http://apoorvaj.io/alpha-compositing-opengl-blending-and-premultiplied-alpha.html)
[Alpha Blending: To Pre or Not To Pre](https://developer.nvidia.com/content/alpha-blending-pre-or-not-pre) 
[Alpha compositing](https://en.wikipedia.org/wiki/Alpha_compositing)  
[Blending](https://www.khronos.org/opengl/wiki/Blending)

### Premultiplied Alpha - WebGL

`premultipliedAlpha: false`, `UNPACK_PREMULTIPLIED_ALPHA_WEBGL`!

[generating text texture in webgl: Alpha is opaque](https://stackoverflow.com/questions/46215101/generating-text-texture-in-webgl-alpha-is-opaque/46225744#46225744)  
[WebGL: Beveled smooth anti-aliased circles using GL_POINT?](https://stackoverflow.com/questions/46519734/webgl-beveled-smooth-anti-aliased-circles-using-gl-point/46519965#46519965)  

## Buffer

[Buffer Object](https://www.khronos.org/opengl/wiki/Buffer_Object) (Immutable Storage / Mutable Storage)  

[Is it possible to in-place resize VBOs?](https://stackoverflow.com/questions/61972140/is-it-possible-to-resize-a-vbo/61972239#61972239)  

### Depth buffer

[How can I improve the performance of my custom OpenGL ES 2.0 depth texture generation?](https://stackoverflow.com/questions/6051237/how-can-i-improve-the-performance-of-my-custom-opengl-es-2-0-depth-texture-gener)  
[How to render depth linearly in modern OpenGL with gl_FragCoord.z in fragment shader?](https://stackoverflow.com/questions/7777913/how-to-render-depth-linearly-in-modern-opengl-with-gl-fragcoord-z-in-fragment-sh/45710371#45710371)  
[Pack depth information in a RGBA texture using mediump precison](https://stackoverflow.com/questions/48288154/pack-depth-information-in-a-rgba-texture-using-mediump-precison/48289986#48289986)  

[Is it possible to depth test against a depth texture I am also sampling, in the same draw call?](https://stackoverflow.com/questions/63542591/is-it-possible-to-depth-test-against-a-depth-texture-i-am-also-sampling-in-the)  

[OpenGL depth test doesn't work on some computers](https://stackoverflow.com/questions/63842472/opengl-depth-test-doesnt-work-on-some-computers/63843180#63843180)

In addition to activating the [Depth Test](https://www.khronos.org/opengl/wiki/Depth_Test) ([`glEnable(GL_DEPTH_TEST)`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glEnable.xhtml)), it is important that the current [framebuffer](https://www.khronos.org/opengl/wiki/Framebuffer_Object) has a [depth buffer](https://www.khronos.org/opengl/wiki/Depth_Test#Depth_buffer).  
The [default framebuffer](https://www.khronos.org/opengl/wiki/Default_Framebuffer) is created at the time the [OpenGL Context](https://www.khronos.org/opengl/wiki/OpenGL_Context) is constructed. Thecreation of the OpenGL context depends on the OS and windowing library (e.g. [GLFW](https://www.glfw.org/), [SDL](https://www.libsdl.org/), [SFML](https://www.sfml-dev.org/)). Whether a depth buffer is created by default often depends on the system. In general, window libraries provide additional options for explicitly specifying a depth buffer when generating the OpenGL window:

For instance:

- [GLFW - Framebuffer related hints](https://www.glfw.org/docs/latest/window_guide.html#window_hints_fb)

  ```cpp
  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  // [...]

  GLFWwindow *wnd = glfwCreateWindow(800, 600, "OpenGL window", nullptr, nullptr);
  ```

- [SDL - Using OpenGL With SDL](https://www.libsdl.org/release/SDL-1.2.15/docs/html/guidevideoopengl.html)

  ```cpp
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  // [...]

  SDL_SetVideoMode(800, 600, bpp, flags);  
  ```

- [SFML - Using OpenGL in a SFML window](https://www.sfml-dev.org/tutorials/2.5/window-opengl.php) 

  ```cpp
  sf::ContextSettings settings;
  settings.depthBits = 24;
  // [...]

  sf::Window window(sf::VideoMode(800, 600), "OpenGL window", sf::Style::Default, settings);
  ```

- [GLUT - `glutInitDisplayMode`](https://www.opengl.org/resources/libraries/glut/spec3/node12.html)

  ```cpp
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
  glutInitWindowSize(800, 600);
  ```

### Frame Buffer

[Framebuffer Object](https://www.khronos.org/opengl/wiki/Framebuffer_Object)  
[Framebuffer Object Extension Examples](https://www.khronos.org/opengl/wiki/Framebuffer_Object_Extension_Examples)  
[Multisampling](https://www.khronos.org/opengl/wiki/Multisampling)  

[Multiple output buffers with the default framebuffer](https://stackoverflow.com/questions/57097570/multiple-output-buffers-with-the-default-framebuffer)  

[OpenGL 4.6 API Compatibility Profile Specification - 17.4.1 Selecting Buffers for Writing](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.compatibility.pdf), page 631:

If the default framebuffer is affected, then each of the constants must be one of the values listed in table 17.6 or the special value BACK. When BACK is used, n must
be 1 and color values are written into the left buffer for single-buffered contexts, or into the back left buffer for double-buffered contexts.
If a framebuffer object is affected, then each of the constants must be one of the values listed in table 17.5.
In both cases, the draw buffers being defined correspond in order to the respective fragment colors. The draw buffer for fragment colors beyond n is set to NONE.

[Khronos reference page - `glCheckFramebufferStatus`][https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glCheckFramebufferStatus.xhtml]:
>`GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE` is returned if the value of `GL_RENDERBUFFER_SAMPLES` is not the same for all attached renderbuffers; if the value of `GL_TEXTURE_SAMPLES` is the not same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of `GL_RENDERBUFFER_SAMPLES` does not match the value of `GL_TEXTURE_SAMPLES`.

[Concept: what is the use of glDrawBuffer and glDrawBuffers?](https://stackoverflow.com/questions/51030120/concept-what-is-the-use-of-gldrawbuffer-and-gldrawbuffers)  
[opengl es 2.0 android c++ glGetTexImage alternative](https://stackoverflow.com/questions/53993820/opengl-es-2-0-android-c-glgetteximage-alternative/53993894#53993894)  

### Uniform Buffer Object UBO and Shader storage buffer SSBO

[Should I ever use a `vec3` inside of a uniform buffer or shader storage buffer object?](https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o)  
[Passing a list of values to fragment shader](https://stackoverflow.com/questions/7954927/passing-a-list-of-values-to-fragment-shader)  
[Can I put a R8G8B8A8 in a UBO, and use it as a vec4?](https://stackoverflow.com/questions/55947646/can-i-put-a-r8g8b8a8-in-a-ubo-and-use-it-as-a-vec4)  
[Can I make a GLSL struct have std140 layout?](https://stackoverflow.com/questions/57775940/can-i-make-a-glsl-struct-have-std140-layout)  
[GLSL: Are writes to Shader Storage Buffers discarded (or undone) by a later discard statement?](https://stackoverflow.com/questions/61294288/glsl-are-writes-to-shader-storage-buffers-discarded-or-undone-by-a-later-disc/61295231#61295231)  

### Vertex buffer

[What is the proper way to modify OpenGL vertex buffer?](https://stackoverflow.com/questions/15821969/what-is-the-proper-way-to-modify-opengl-vertex-buffer)  

## Draw calls

[Overhead of glDrawArrays vs. glMultiDrawArrays](https://stackoverflow.com/questions/54724263/overhead-of-gldrawarrays-vs-glmultidrawarrays)  

### Error

[Errors during creation of OpenGL objects](https://stackoverflow.com/questions/58053495/errors-during-creation-of-opengl-objects)  

## Fragment shader

[EXT_shader_framebuffer_fetch](https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_shader_framebuffer_fetch.txt) - `gl_LastFragData`  
[Is there a way in Opengl es 2.0 fragment shader, to get a previews fragment color](https://stackoverflow.com/questions/65642257/is-there-a-way-in-opengl-es-2-0-fragment-shader-to-get-a-previews-fragment-colo/65642346#65642346)  

```glsl
#extension GL_EXT_shader_framebuffer_fetch : require
```

## Geometry shader

[OpenGL Shadow Map Mobile Version Doesn't work](https://stackoverflow.com/questions/59090503/opengl-shadow-map-mobile-version-doesnt-work)  

## Gooey

[Implementing a gooey effect with a shader (Processing 3)](https://stackoverflow.com/questions/49806658/implementing-a-gooey-effect-with-a-shader-processing-3)  

## Glow

[How to add glowing effect to a line for OpenGL?](https://stackoverflow.com/questions/8293839/how-to-add-glowing-effect-to-a-line-for-opengl)  
[How to get a “Glow” shader effect in OpenGL ES 2.0?](https://stackoverflow.com/questions/8166384/how-to-get-a-glow-shader-effect-in-opengl-es-2-0/45043547#45043547)  

## GLSL

[Do conditional statements slow down shaders?](https://stackoverflow.com/questions/37827216/do-conditional-statements-slow-down-shaders)  
[Why won't my GLSL program link when one shader is optimized and the other is not?](https://stackoverflow.com/questions/52841269/why-wont-my-glsl-program-link-when-one-shader-is-optimized-and-the-other-is-not)  
[Is it ok to declare an oversized output array in fragment shader and leave some indices unused?](https://stackoverflow.com/questions/52173024/is-it-ok-to-declare-an-oversized-output-array-in-fragment-shader-and-leave-some)  

## Height map and height map normals

[OpenGL - How to calculate normals in a terrain height grid?](https://stackoverflow.com/questions/13983189/opengl-how-to-calculate-normals-in-a-terrain-height-grid)  

## Image format

[How can I use ARGB color in opengl/SDL?](https://stackoverflow.com/questions/18624895/how-can-i-use-argb-color-in-opengl-sdl)  

## Invocations

[Frequency of shader invocations in rendering commands](https://stackoverflow.com/questions/35243518/frequency-of-shader-invocations-in-rendering-commands)  

## Lines

[Is it really so difficult to draw smooth lines in Unity?](https://stackoverflow.com/questions/43547886/is-it-really-so-difficult-to-draw-smooth-lines-in-unity)  

### Line stipple

[glLineStipple deprecated in OpenGL 3.1](https://stackoverflow.com/questions/6017176/gllinestipple-deprecated-in-opengl-3-1/55088683#55088683)  
[Three.js uniform dashed line relative to camera](https://stackoverflow.com/questions/54516794/three-js-uniform-dashed-line-relative-to-camera/54524171#54524171)  
[Dashed line in OpenGL3?](https://stackoverflow.com/questions/52928678/dashed-line-in-opengl3/54543267#54543267)  
[OpenGL ES - Dashed Lines](https://stackoverflow.com/questions/37975618/opengl-es-dashed-lines)  
[Drawing rectangle with dashed boundary in openGL android](https://stackoverflow.com/questions/60188402/drawing-rectangle-with-dashed-boundary-in-opengl-android/60190604#60190604)  
[Draw a simple dotted line or dashed line in OpenGL GLES20 android using fragment shader and GL_LINE_STRIP](https://stackoverflow.com/questions/60293538/draw-a-simple-dotted-line-or-dashed-line-in-opengl-gles20-android-using-fragment/60315849#60315849)  

### Line thickness

[OpenGL Line Width](https://stackoverflow.com/questions/3484260/opengl-line-width/59688394#59688394)  
[GLSL Geometry shader to replace glLineWidth](https://stackoverflow.com/questions/54686818/glsl-geometry-shader-to-replace-gllinewidth)  
[OpenGL 3.2 Core Profile glLineWidth](https://stackoverflow.com/questions/8791531/opengl-3-2-core-profile-gllinewidth/59868362#59868362)  

## Matrix and matrix transformation

[Transformation order reversed in glMatrix?](https://stackoverflow.com/questions/38425426/transformation-order-reversed-in-glmatrix)  
[Algorithm for finding Eigenvectors given Eigenvalues of a 3x3 matrix in C#](https://stackoverflow.com/questions/53469969/algorithm-for-finding-eigenvectors-given-eigenvalues-of-a-3x3-matrix-in-c-sharp)  

## Mip mapping

[How to access automatic mipmap level in GLSL fragment shader texture?](https://stackoverflow.com/questions/24388346/how-to-access-automatic-mipmap-level-in-glsl-fragment-shader-texture)  
[How to perform Mipmapping in WebGL?](https://stackoverflow.com/questions/21540520/how-to-perform-mipmapping-in-webgl)  
[Does it make sense to use own mipmap creation algorithm for OpenGL textures?](https://stackoverflow.com/questions/831893/does-it-make-sense-to-use-own-mipmap-creation-algorithm-for-opengl-textures)  

## Multi Sampling

[How multi sampling works for Integer textures](https://stackoverflow.com/questions/64329477/how-multi-sampling-works-for-integer-textures)

## Morphing

[how should i handle (morphing) 4D objects in opengl?](https://stackoverflow.com/questions/44939879/how-should-i-handle-morphing-4d-objects-in-opengl)  

## Multi Indexing

[Rendering meshes with multiple indices](https://stackoverflow.com/questions/11148567/rendering-meshes-with-multiple-indices)  
[Why does OpenGL not support multiple index buffering?](https://stackoverflow.com/questions/44046585/why-does-opengl-not-support-multiple-index-buffering)  

## Noise

[Random / noise functions for GLSL](https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl)  

## Normal matrix - transpose inverse

[Why transforming normals with the transpose of the inverse of the modelview matrix?](https://stackoverflow.com/questions/13654401/why-transforming-normals-with-the-transpose-of-the-inverse-of-the-modelview-matr)  

## Outline

[Silhouette-Outlined shader](https://stackoverflow.com/questions/46603878/silhouette-outlined-shader)  
[OpenGL - object outline](https://stackoverflow.com/questions/53897949/opengl-object-outline)  
[Scalable outline for any shape with THREE.js](https://stackoverflow.com/questions/53098863/scalable-outline-for-any-shape-with-three-js)  

## Projection

[What exactly are eye space coordinates?](https://stackoverflow.com/questions/15588860/what-exactly-are-eye-space-coordinates/61537327#61537327)  
[Get Projection and View Matrix from Camera Matrix](https://stackoverflow.com/questions/47200505/get-projection-and-view-matrix-from-camera-matrix)  
[Projection matrices: What should depth map to?](https://stackoverflow.com/questions/52893875/projection-matrices-what-should-depth-map-to)  
[Transformation of 3D objects related to vanishing points and horizon line](https://stackoverflow.com/questions/53289330/transformation-of-3d-objects-related-to-vanishing-points-and-horizon-line)  

## Quaternion

[Using Quaternions for OpenGL Rotations](https://stackoverflow.com/questions/9715776/using-quaternions-for-opengl-rotations)  

## Ray tracing

[Reflection and refraction impossible without recursive ray tracing?](https://stackoverflow.com/questions/42876586/reflection-and-refraction-impossible-without-recursive-ray-tracing)  
[How to handle incorrect index calculation for discretized ray tracing?](https://stackoverflow.com/questions/49214524/how-to-handle-incorrect-index-calculation-for-discretized-ray-tracing)  
[Ray Tracer Shadow Problems](https://stackoverflow.com/questions/53841706/ray-tracer-shadow-problems)  

## Scissor

[Scissor Test](https://www.khronos.org/opengl/wiki/Scissor_Test)  

## Screen space ambient occlusion - SSAO

[Antialiasing during depth post-processing for SSAO](https://stackoverflow.com/questions/48523905/antialiasing-during-depth-post-processing-for-ssao)  

## Screen Space Local Reflections SSLR

[Incorrect tracing with SSLR (Screen Space Local Reflections)](https://stackoverflow.com/questions/53145377/incorrect-tracing-with-sslr-screen-space-local-reflections)  
[How to scale environment map perspective according to the position of the main camera](https://stackoverflow.com/questions/49738967/how-to-scale-environment-map-perspective-according-to-the-position-of-the-main-c)  

## Shader

[Passing a list of values to fragment shader](https://stackoverflow.com/questions/7954927/passing-a-list-of-values-to-fragment-shader)  

## Silhouette detection

[How to draw inside the black edges in iOS SDK with OpenGL ES?](https://stackoverflow.com/questions/50944736/how-to-draw-inside-the-black-edges-in-ios-sdk-with-opengl-es)  

## Stencil

[Drawing Filled, Concave Polygons Using the Stencil Buffer](http://glprogramming.com/red/chapter14.html#name13)

[opengl create a depth_stencil texture for reading](https://stackoverflow.com/questions/27535727/opengl-create-a-depth-stencil-texture-for-reading)  
[OpenGL 16 bit stencil buffers?](https://stackoverflow.com/questions/62741921/opengl-16-bit-stencil-buffers/62742277#62742277)  
[Strange bugs occur while using stencil testing](https://stackoverflow.com/questions/49786763/strange-bugs-occur-while-using-stencil-testing/49832387#49832387)  
[How to use stencil buffer to achieve hierarchical clipping](https://stackoverflow.com/questions/56636337/how-to-use-stencil-buffer-to-achieve-hierarchical-clipping/56637285#56637285)  

## Tangent space

[In a TBN Matrix are the normal, tangent, and bitangent vectors always perpendicular?](https://stackoverflow.com/questions/15433917/in-a-tbn-matrix-are-the-normal-tangent-and-bitangent-vectors-always-perpendicu)  

## Text rendering

[What is state-of-the-art for text rendering in OpenGL as of version 4.1?](https://stackoverflow.com/questions/5262951/what-is-state-of-the-art-for-text-rendering-in-opengl-as-of-version-4-1)  

## Texture

[Correspondance between texture units and sampler uniforms in opengl](https://stackoverflow.com/questions/54931941/correspondance-between-texture-units-and-sampler-uniforms-in-opengl)  
[How do opengl texture coordinates work?](https://stackoverflow.com/questions/5532595/how-do-opengl-texture-coordinates-work)  
[Difference between U V and S T texture coordinates](https://stackoverflow.com/questions/10568390/difference-between-u-v-and-s-t-texture-coordinates)  
[What is, in simple terms, textureGrad()?](https://stackoverflow.com/questions/52975878/what-is-in-simple-terms-texturegrad)  
[OpenGL vertex shader for pinhole camera model](https://stackoverflow.com/questions/52928147/opengl-vertex-shader-for-pinhole-camera-model)  
[Does it make sense to use own mipmap creation algorithm for OpenGL textures?](https://stackoverflow.com/questions/831893/does-it-make-sense-to-use-own-mipmap-creation-algorithm-for-opengl-textures)  
[Is glTexStorage2D imperative when auto generating mipmaps](https://stackoverflow.com/questions/15405869/is-gltexstorage2d-imperative-when-auto-generating-mipmaps)  
[How to get texture data using textureID's in openGL](https://stackoverflow.com/questions/5117653/how-to-get-texture-data-using-textureids-in-opengl/62965713#62965713)  
[Will any of the following texture lookups cause undefined behavior, non-uniform flow, or both?](https://stackoverflow.com/questions/53734640/will-any-of-the-following-texture-lookups-cause-undefined-behavior-non-uniform)

## Transparency

[Weighted Blended Order-Independent Transparency rendering issues](https://stackoverflow.com/questions/50817040/weighted-blended-order-independent-transparency-rendering-issues)  
[OpenGL: Transparent texture issue](https://stackoverflow.com/questions/45954668/opengl-transparent-texture-issue/45955236#45955236)  

## Triangle strips

[How are tripled sequence in IBO working?](https://stackoverflow.com/questions/46511972/how-are-tripled-sequence-in-ibo-working)  

## Vertex specification

[glVertexAttribPointer and glVertexAttribFormat: What's the difference?](https://stackoverflow.com/questions/37972229/glvertexattribpointer-and-glvertexattribformat-whats-the-difference)  
[OpenGL: Vertex attribute arrays per primitive?](https://stackoverflow.com/questions/11351537/opengl-vertex-attribute-arrays-per-primitive)  
[Drawing multiple triangles in OpenGL](https://stackoverflow.com/questions/47760298/drawing-multiple-triangles-in-opengl)  

## Zoom

[How to fix zoom towards mouse routine?](https://stackoverflow.com/questions/57638051/how-to-fix-zoom-towards-mouse-routine)  
