[![StackOverflow](https://stackexchange.com/users/flair/7322082.png)](https://stackoverflow.com/users/5577765/rabbid76?tab=profile)

---

# Legacy OpenGL - immediate mode and fixed function pipeline

[OpenGL Context](https://www.khronos.org/opengl/wiki/OpenGL_Context)  
[Legacy OpenGL](https://www.khronos.org/opengl/wiki/Legacy_OpenGL)  
[Fixed Function Pipeline](https://www.khronos.org/opengl/wiki/Fixed_Function_Pipeline)  

[What is fixed function pipeline(PyOpenGL)?](https://stackoverflow.com/questions/53930214/what-is-fixed-function-pipelinepyopengl/53930912#53930912)  

The [Fixed Function Pipeline](https://www.khronos.org/opengl/wiki/Fixed_Function_Pipeline) is deprecated since decades and must not to be use any more. It is not flexible enough for a modern render algorithms and technologies.

See [Vertex Specification](https://www.khronos.org/opengl/wiki/Vertex_Specification) and [Shader](https://www.khronos.org/opengl/wiki/Shader) for a flexible state of the art way of rendering.

The key points of the fixed function pipeline are:

- At the fixed function pipeline, the geometry is drawn by [`glBegin`/`glEnd`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glBegin.xml) sequences or fixed function attributes (e.g. [`glVertexPointer`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertexPointer.xml)) and enabling client-side capability ([`glEnableClientState`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glEnableClientState.xml))

- The vertex coordinate transformation (projection matrix and model view matrix) are applied by the fixed function matrix stack ([`glMatrixMode`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glMatrixMode.xml), [`glPushMatrix`/`glPopMatrix`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glPushMatrix.xml), [`glTranslate`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glTranslate.xml), ...).

- The fixed function pipeline light model is shrinked to a per vertex [Gouraud shading](https://en.wikipedia.org/wiki/Gouraud_shading), [Blinn–Phong](https://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_shading_model) light model.  
See [Basic OpenGL Lighting](https://www.sjbaker.org/steve/omniv/opengl_lighting.html) and [GLSL fixed function fragment program replacement](https://stackoverflow.com/questions/8421778/glsl-fixed-function-fragment-program-replacement/45716107#45716107).

To the use the fixed function pipeline a compatibility profile [OpenGL Context](https://www.khronos.org/opengl/wiki/OpenGL_Context#OpenGL_3.1_and_ARB_compatibility) must be used.  
See [OpenGL 4.6 API Compatibility Profile Specification](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.compatibility.pdf), where the difference between a core profile and a compatibility profile a marked in red.

[Why I'm getting access violation errors with my Vertex Buffer Object?](https://stackoverflow.com/questions/54536582/why-im-getting-access-violation-errors-with-my-vertex-buffer-object/54537409#54537409)  

## Begin/End sequences

Note, that drawing by [`glBegin`/`glEnd`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glBegin.xml) sequences, the fixed function matrix stack and fixed function, per vertex light model, is deprecated since decades. See [Fixed Function Pipeline](https://www.khronos.org/opengl/wiki/Fixed_Function_Pipeline) and [Legacy OpenGL](https://www.khronos.org/opengl/wiki/Legacy_OpenGL).  
Read about [Vertex Specification](https://www.khronos.org/opengl/wiki/Vertex_Specification) and [Shader](https://www.khronos.org/opengl/wiki/Shader) for a state of the art way of rendering.

[OpenGL 3D model texture mapping](https://stackoverflow.com/questions/48598330/opengl-3d-model-texture-mapping/48598938#48598938)  
[Cube map/ frame buffer problems](https://stackoverflow.com/questions/50543316/cube-map-frame-buffer-problems/50543749#50543749)  
[Cannot impose texture to object](https://stackoverflow.com/questions/51918268/cannot-impose-texture-to-object/51919610#51919610)  

## Primitives

[Issue converting legacy pipeline to modern openGL](https://stackoverflow.com/questions/60128278/issue-converting-legacy-pipeline-to-modern-opengl/60128488#60128488)  

> [...] is there a way to combine all of the data and send it as a single call?

If you want to render multiple [`GL_LINE_LOOP`](https://www.khronos.org/opengl/wiki/Primitive#Line_primitives) primitives with on draw call, then I recommend to use [Primitive Restart](https://www.khronos.org/opengl/wiki/Vertex_Rendering#Primitive_Restart):

> Primitive restart functionality allows you to tell OpenGL that a particular index value means, not to source a vertex at that index, but to begin a new Primitive

Enable `GL_PRIMITIVE_RESTART` and define the restart index by [`glPrimitiveRestartIndex`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glPrimitiveRestartIndex.xhtml) (e.g. 0xFFFFFFFF):

```cpp
glEnable( GL_PRIMITIVE_RESTART );
glPrimitiveRestartIndex( 0xFFFFFFFF );
```

e.g. If you want to draw 2 line loops (with one draw call) with the indices (0, 1, 2) and (3, 4, 5, 6) then you have to define the following index array:

```cpp
std::vector<uint32_t> indices{ 0, 1, 2, 0xFFFFFFFF, 3, 4, 5, 6 };
```

## OpenGL Display List

[Unexpected gluLookAt behavior with a point draw on OpenGL](https://stackoverflow.com/questions/46374873/unexpected-glulookat-behavior-with-a-point-draw-on-opengl/46376883#46376883)

## Raster operation

[Issues with the color comand in OpenGL](https://stackoverflow.com/questions/53245206/issues-with-the-color-comand-in-opengl/53246925#53246925)  

## Fixed pipeline Shader

[How can I draw gluQuadric with color?](https://stackoverflow.com/questions/64632166/how-can-i-draw-gluquadric-with-color/64633229#64633229)  
[OpenGL gluLookat not working with shaders on](https://stackoverflow.com/questions/56090164/opengl-glulookat-not-working-with-shaders-on/56090352#56090352)  
[OpenGL: glColor3f() and glVertex3f() with shader](https://stackoverflow.com/questions/70408799/opengl-glcolor3f-and-glvertex3f-with-shader)  

## Fixed-Function Attributes

[OpenGL 4.6 API Compatibility Profile Specification; 10.3.3 Specifying Arrays for Fixed-Function Attributes; page 402](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.compatibility.pdf#page=430&zoom=100,168,658)

> The commands
>
> ```cpp
> void VertexPointer( int size, enum type, sizei stride, const void *pointer );
> void NormalPointer( enum type, sizei stride, const void *pointer );
> void ColorPointer( int size, enum type, sizei stride, const void *pointer );
> void SecondaryColorPointer( int size, enum type, sizei stride, c onst void *pointer );
> void IndexPointer( enum type, sizei stride, const void *pointer );
> void EdgeFlagPointer( sizei stride, const void *pointer );
> void FogCoordPointer( enum type, sizei stride, const void *pointer );
> void TexCoordPointer( int size, enum type, sizei stride, const void *pointer );
>```
>
> specify the location and organization of arrays to store vertex coordinates, normals, colors, secondary colors, color indices, edge flags, fog coordinates.
> ...
> An individual array is enabled or disabled by calling one of
>
> ```cpp
> void EnableClientState( enum array );
> void DisableClientState( enum array );
> ```
>
> with array set to `VERTEX_ARRAY`, `NORMAL_ARRAY`, `COLOR_ARRAY`, `SECONDARY_COLOR_ARRAY`, `INDEX_ARRAY`, `EDGE_FLAG_ARRAY`, `FOG_COORD_ARRAY`, or `TEXTURE_COORD_ARRAY`, for the vertex, normal, color, secondary color, color index, edge flag, fog coordinate, or texture coordinate array, respectively.

[Trying to draw a basic shape with LWJGL3](https://stackoverflow.com/questions/51390730/trying-to-draw-a-basic-shape-with-lwjgl3/51394034#51394034)  
[VBO and VAO not drawing opengl and LWJGL3](https://stackoverflow.com/questions/51548601/vbo-and-vao-not-drawing-opengl-and-lwjgl3/51550827#51550827)  
[LWJGL 3.1.6 crashes on Win 10](https://stackoverflow.com/questions/51598412/lwjgl-3-1-6-crashes-on-win-10/51598873#51598873)  

The modern way of rendering in OpenGL, would be to use a [Shader](https://www.khronos.org/opengl/wiki/Shader) program.

If you don't use a shader program, than you have to define the array of vertex data using the deprecated way by [`glVertexPointer`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertexPointer.xml) and you have to enable the client-side capability for vertex coordinates by [`glEnableClientState( GL_VERTEX_ARRAY )`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glEnableClientState.xml).

**[What are the Attribute locations for fixed function pipeline in OpenGL 4.0++ core profile?](https://stackoverflow.com/questions/20573235/what-are-the-attribute-locations-for-fixed-function-pipeline-in-opengl-4-0-cor/51949673#51949673)**  

If the OpenGL extension [`ARB_vertex_program;  Modify Section 2.7, Vertex Specification`](https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_vertex_program.txt) is valid,
then there is a mapping between Fixed function attributes and attribute indices:

> **Setting generic vertex attribute zero specifies a vertex; the four vertex coordinates are taken from the values of attribute zero.**
> A Vertex2, Vertex3, or Vertex4 command is completely equivalent to the corresponding VertexAttrib command with an index of zero.
> Setting any other generic vertex attribute updates the current values of the attribute.
> There are no current values for vertex attribute zero.
>
> **Implementations may, but do not necessarily, use the same storage for the current values of generic and certain conventional vertex attributes**.
> When any generic vertex attribute other than zero is specified, the current values for the corresponding conventional attribute in Table X.1 become undefined.
> Additionally, when a conventional vertex attribute is specified, the current values for the corresponding generic vertex  attribute in Table X.1 become undefined.
> For example, setting the current normal will leave generic vertex attribute 2 undefined, and vice versa.
>
> | Generic Attribute |  Conventional Attribute  | Conventional Attribute Command |
> |-------------------|--------------------------|--------------------------------|
> | 0                 | vertex position          | Vertex                         |
> | 1                 | vertex weights 0-3       | WeightARB, VertexWeightEXT     |
> | 2                 | normal                   | Normal                         |
> | 3                 | primary color            | Color                          |
> | 4                 | secondary color          | SecondaryColorEXT              |
> | 5                 | fog coordinate           | FogCoordEXT                    |
> | 6                 | -                        | -                              |
> | 7                 | -                        | -                              |
> | 8                 | texture coordinate set 0 | MultiTexCoord(TEXTURE0, ...    |
> | ...               |                          |                                |
>

This means there is a "mapping" between vertex attribute *0* and the fixed function attribute `GL_VERTEX_ARRAY`, but not necessarily a mapping for any other vertex attribute.  

Nvidia goes a step ahead as specified in [Release Notes for NVIDIA OpenGL Shading Language Support; November 9, 2006; - pp. 7-8](http://developer.download.nvidia.com/opengl/glsl/glsl_release_notes.pdf).  
There is an actual mapping between the fixed function attributes and vertex attribute indices, as specified in the table above.  
See also the answer to [What are the Attribute locations for fixed function pipeline in OpenGL 4.0++ core profile?](https://stackoverflow.com/questions/20573235/what-are-the-attribute-locations-for-fixed-function-pipeline-in-opengl-4-0-cor)

[drawing several triangles with different colours with one glDrawArrays command](https://stackoverflow.com/questions/54168401/drawing-several-triangles-with-different-colours-with-one-gldrawarrays-command/54168603#54168603)  

[Why doesn't OpenGL draw a polygon in this code?](https://stackoverflow.com/questions/57030291/why-doesnt-opengl-draw-a-polygon-in-this-code/57030480#57030480)  

## Translation, Rotation and Scale - Fixed function pipeline

[Drawing cubes with stacked matrix](https://stackoverflow.com/questions/46238282/drawing-cubes-with-stacked-matrix/46244181#46244181)**  
![Drawing cubes with stacked matrix](https://i.stack.imgur.com/6Ej8Q.png)

**[OpenGL translation before and after a rotation](https://stackoverflow.com/questions/49236745/opengl-translation-before-and-after-a-rotation/49262569#49262569)**  

**[OpenGL Arm that rotates shoulder and elbow](https://stackoverflow.com/questions/52652098/opengl-arm-that-rotates-shoulder-and-elbow/52652663#52652663):  
![arms fixed pipeline](image/arms_fixed_pipeline.png)**

**[OpenGL: Objects rotating strangely](https://stackoverflow.com/questions/63850009/opengl-objects-rotating-strangely)**
[Rotating a multipart object](https://stackoverflow.com/questions/46720166/rotating-a-multipart-object/46722875#46722875)  
[how to use glTranslatef,glScalef,glRotatef in openGL](https://stackoverflow.com/questions/48236120/how-to-use-gltranslatef-glscalef-glrotatef-in-opengl/48239102#48239102)  
[Move Object in Circle using OpenGL?](https://stackoverflow.com/questions/48360049/move-object-in-circle-using-opengl/48360166#48360166)  
[OpenGL movement object and rotate in same axis](https://stackoverflow.com/questions/49020426/opengl-movement-object-and-rotate-in-same-axis/49022935#49022935)  
![OpenGL movement object and rotate in same axis](https://i.stack.imgur.com/kuXZ0.png)
![OpenGL movement object and rotate in same axis](https://i.stack.imgur.com/hp8Cf.png)

[Solar System in OpenGL, Camera position](https://stackoverflow.com/questions/53141027/solar-system-in-opengl-camera-position/53141799#53141799)  
![solarsystem](image/solarsystem.gif)

[How to rotate 2 objects independently in pygame and pyopengl](https://stackoverflow.com/questions/53157171/how-to-rotate-2-objects-independently-in-pygame-and-pyopengl/53159418#53159418)  

[Animate cube inside an animation](https://stackoverflow.com/questions/59600532/animate-cube-inside-an-animation/59601017#59601017)  
![[Animate cube inside an animation](https://i.stack.imgur.com/PMIvw.gif)  

## Write to Framebuffer

[Using gl_FragColor vs. out vec4 color?](https://stackoverflow.com/questions/51459596/using-gl-fragcolor-vs-out-vec4-color/51459750#51459750)

[OpenGL Shading Language 4.60 Specification - 7.1.7. Compatibility Profile Built-In Language Variables; page 144](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf)
[OpenGL Shading Language 3.30 Specification - 7.2 Fragment Shader Special Variables; page 71](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.3.30.pdf)

> The following fragment output variables are available in a fragment shader when using the **compatibility profile**:
>
>```glsl
> out vec4 gl_FragColor;
> out vec4 gl_FragData[gl_MaxDrawBuffers];
>```

Writing to `gl_FragColor` specifies the fragment color that will be used by the subsequent fixed functionality pipeline. If subsequent fixed functionality consumes fragment color and an execution of the fragment shader executable does not write a value to `gl_FragColor` then the fragment color consumed is undefined.

This means you can't use `gl_FragColor`, in an [OpenGL Core profile Context](https://www.khronos.org/opengl/wiki/OpenGL_Context#Context_types), because it is deprecated, but it would still be available in a compatibility profile.

The modern way of writing to the output buffers from a fragment shader, is to declare [user-defined output variables](https://www.khronos.org/opengl/wiki/Fragment_Shader#Outputs) and to use [Layout Qualifiers](https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)#Fragment_shader_buffer_output).

## Fixed function pipeline (deprecated) Light model

```lang-none
glEnable(GL_LIGHTING)
```

```lang-none
glEnable(GL_LIGHTING)
```

[Basic OpenGL Lighting](https://www.sjbaker.org/steve/omniv/opengl_lighting.html)  
[Per Fragment Lighting](https://www.opengl.org/sdk/docs/tutorials/ClockworkCoders/lighting.php)  

![legacy lighting flowchart](https://www.khronos.org/opengl/wiki_opengl/images/Opengl_lighting_flowchart.png)

[OpenGL lighting position is fixed at undesired location](https://stackoverflow.com/questions/50516210/opengl-lighting-position-is-fixed-at-undesired-location/50516879#50516879)  

When the light position is set by [`glLightfv(GL_LIGHT0, GL_POSITION, pos)`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glLight.xml), then `pos` is multiplied by the current model view matrix.  
This means if the position is set before the view matrix is set (`gluLookAt`), then the light position is relative to the camera (view space position).  
If it is set after the view matrix was set, then the light position has to be in world coordinates, becaus it is transformed by the view matrix.

This means if the position is set before the model view matrix is set, then the light position is placed in world coordiantes. It is transformed by the view matrix but not by the model matrix.  
If it is set after the model view matrix was set, then the light position can be set in the coordinate system of the model.

When the spot light direction is set by [`glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir)`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glLight.xml), then
the direction is multiplied by the upper 3x3 of the modelview matrix. The parameter `GL_SPOT_DIRECTION` is a direction vector and not a position.  

The OpenGL fixed function pipeline calculates the light per vertex: [Gouraud shading](https://en.wikipedia.org/wiki/Gouraud_shading).  
This may cause, that you can't "see" the light, if the light vector hits the vertex coordinate by a acute angle relative to the plane of the surface or if the light cone of the spotlight does not hit any vertex position.
You are in danger of that behavior, because your geometry consists of "large" primitives and the cone opening angle of the spot light is very small (15°).

See **[OpenGL Lighting on texture plane is not working](https://stackoverflow.com/questions/49516161/opengl-lighting-on-texture-plane-is-not-working/49517742#49517742)**

[How to make opengl text visible/change color?](https://stackoverflow.com/questions/50618009/how-to-make-opengl-text-visible-change-color/50618767#50618767)  
[OpenGL object no lighting](https://stackoverflow.com/questions/51916224/opengl-object-no-lighting/51917284#51917284)  
[Inter dependency between gl_color and material](https://stackoverflow.com/questions/48389412/inter-dependency-between-gl-color-and-material/48391162#48391162)  
[Adding a Light Source to 3D objects in OpenGL](https://stackoverflow.com/questions/53585826/adding-a-light-source-to-3d-objects-in-opengl/53588245#53588245)  

When lighting ([`GL_LIGHTING`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glEnable.xml)) is enabled, then the color which is associated, is taken from the material parameters  ([`glMaterial`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glMaterial.xml)).

If you still want to use the current color attribute (which is set by [`glColor`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColor.xml)), then you have to enable [`GL_COLOR_MATERIAL`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glEnable.xml)
and to set the color material paramters ([`glColorMaterial`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColorMaterial.xml)):

```cpp
glEnable(GL_COLOR_MATERIAL);
glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
```

See also [Basic OpenGL Lighting](https://www.sjbaker.org/steve/omniv/opengl_lighting.html).

The standard OpenGL [Blinn-Phong](https://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_shading_model) light model is calcualted like this:

[OBJ file, after read mtl can not show the colors](https://stackoverflow.com/questions/49510133/obj-file-after-read-mtl-can-not-show-the-colors/49512028#49512028)  
[opengl light reduce reflection](https://stackoverflow.com/questions/50392510/opengl-light-reduce-reflection/50394377#50394377)  
[OpenGL import PNG darker](https://stackoverflow.com/questions/53634975/opengl-import-png-darker/53640908#53640908)  

```txt
Ka ... ambient material
Kd ... difusse material
Ks ... specular material

La ... ambient light
Ld ... diffuse light
Ls ... specular light
sh ... shininess

N  ... norlmal vector 
L  ... light vector (from the vertex position to the light) 
V  ... view vector (from the vertex position to the eye)

Id    = max(dot(N, L), 0.0);

H     = normalize(V + L);
NdotH = max(dot(N, H), 0.0);
Is    = pow(NdotH, sh);

fs    = Ka*La + Id*Kd*Ld + Is*Ks*Ls;
```

```txt
Is    = (sh + 2.0) * pow(NdotH, sh) / (2.0 * 3.14159265);
```

[Color and movement of OpenGL object does not alternate properly](https://stackoverflow.com/questions/45673338/color-and-movement-of-opengl-object-does-not-alternate-properly/45673652#45673652)  
[How can I make my light stay fixed relative to my scene (in the cube)?](https://stackoverflow.com/questions/53129373/how-can-i-make-my-light-stay-fixed-relative-to-my-scene-in-the-cube/53129654#53129654)  
[glColor3f in OpenGl have little brightness](https://stackoverflow.com/questions/54011452/glcolor3f-in-opengl-have-little-brightness/54011535#54011535)  
[OpenGL: Updating shading with several movable lights](https://stackoverflow.com/questions/55132406/opengl-updating-shading-with-several-movable-lights/55135600#55135600)  
[How do i make OpenGL Specular Light work?](https://stackoverflow.com/questions/55338066/how-do-i-make-opengl-specular-light-work/55343535#55343535)  
[Texture Mapping disappeared when added 3D models](https://stackoverflow.com/questions/55896162/texture-mapping-disappeared-when-added-3d-models/55897274#55897274)  
[How to stop positional light moving with camera](https://stackoverflow.com/questions/56152563/how-to-stop-positional-light-moving-with-camera/56152696#56152696)  
[OpenGL doesn't consider distance of GL_DIFFUSE positional light](https://stackoverflow.com/questions/57349994/opengl-doesnt-consider-distance-of-gl-diffuse-positional-light/57357716#57357716)  

[OpenGL: How do I render individual RGB values given a cross product that contains the total light?](https://stackoverflow.com/questions/55149946/opengl-how-do-i-render-individual-rgb-values-given-a-cross-product-that-contain/55151660#55151660)  
![OpenGL: How do I render individual RGB values given a cross product that contains the total light?](https://i.stack.imgur.com/PSV6n.png)  
![OpenGL: How do I render individual RGB values given a cross product that contains the total light?](https://i.stack.imgur.com/dgeFs.png)  

## Fixed function pipeline (deprecated) Gouraud versus Phong

[OpenGL Lighting on texture plane is not working](https://stackoverflow.com/questions/49516161/opengl-lighting-on-texture-plane-is-not-working/49517742#49517742)

## Gouraud versus Phong

**[what the difference between phong shading and gouraud shading?](https://stackoverflow.com/questions/63958531/what-is-the-difference-between-phong-shading-and-gouraud-shading/63958763#63958763)**  

[GLSL fixed function fragment program replacement](https://stackoverflow.com/questions/8421778/glsl-fixed-function-fragment-program-replacement/45716107#45716107)  
[How to add lighting to shader of instancing](https://stackoverflow.com/questions/46373271/how-to-add-lighting-to-shader-of-instancing/46482751#46482751)  
[How do you incorporate per pixel lighting in shaders with LIBGDX?](https://stackoverflow.com/questions/46511824/how-do-you-incorporate-per-pixel-lighting-in-shaders-with-libgdx/46512044#46512044)  
[Per Vertex Diffuse and Specular Shader](https://stackoverflow.com/questions/48523723/per-vertex-diffuse-and-specular-shader/48528327#48528327)  
[Phong and Gouraud Shading WebGL](https://stackoverflow.com/questions/49966240/phong-and-gouraud-shading-webgl/49966346#49966346)  

In the case of specular highlights, the light distribution is not linear and cannot be calculated with linear interpolation. The effect is distorted or is completely lost.

## Drawing geometry and mesh

### GLU

[“invalid value” when calling gluNewQuadric](https://stackoverflow.com/questions/54263677/invalid-value-when-calling-glunewquadric/54265534#54265534)  

[texturing a moving sphere in OpenGL C++](https://stackoverflow.com/questions/49464644/texturing-a-moving-sphere-in-opengl-c/49475387#49475387)  
![sphere texture](https://i.stack.imgur.com/Vf4A4.png)

```cpp
glColor3f(1.0f, 1.0f, 1.0f);
glEnable(GL_TEXTURE_2D);

glPushMatrix();
glRotatef(angle_degree, 1.0f, 0.0f, 0.0f);

GLUquadric *qobj = gluNewQuadric();
gluQuadricTexture( qobj, GL_TRUE );
gluSphere( qobj, 0.1, 10, 10 );
gluDeleteQuadric( qobj );

glPopMatrix();

glDisable(GL_TEXTURE_2D);
```

### Particles

[OpenGL - Stop spray and stay in its position](https://stackoverflow.com/questions/53833514/opengl-stop-spray-and-stay-in-its-position/53838943#53838943)  
![spray](https://i.stack.imgur.com/5VMco.gif)

### Map Buffer

[OpenGL NURBS surfaces](https://stackoverflow.com/questions/53934033/opengl-nurbs-surfaces/53934868#53934868)  
![nurbs](https://i.stack.imgur.com/IBUBX.gif)
