
# Line thickness questions

[Drawing a line in modern OpenGL](https://stackoverflow.com/questions/60440682/drawing-a-line-in-modern-opengl/60440937#60440937)  
[Drawing a variable width line in openGL (No glLineWidth)](https://stackoverflow.com/questions/101718/drawing-a-variable-width-line-in-opengl-no-gllinewidth)  
[opengl glLineWidth() doesn't change size of lines](https://stackoverflow.com/questions/34866964/opengl-gllinewidth-doesnt-change-size-of-lines)  
[Thickness of lines using THREE.LineBasicMaterial](https://stackoverflow.com/questions/11638883/thickness-of-lines-using-three-linebasicmaterial)  
[Workaround for lack of line width on Windows when using Three.js](https://stackoverflow.com/questions/21067461/workaround-for-lack-of-line-width-on-windows-when-using-three-js)  
[Detecting support for WebGL thick lines in browser](https://stackoverflow.com/questions/38333977/detecting-support-for-webgl-thick-lines-in-browser)  
[OpenGL : thick and smooth/non-broken lines *in 3D*](https://stackoverflow.com/questions/36655888/opengl-thick-and-smooth-non-broken-lines-in-3d/36663605)  

# Answered

[OpenGL Line Width](https://stackoverflow.com/questions/3484260/opengl-line-width)  
[GLSL Geometry shader to replace glLineWidth](https://stackoverflow.com/questions/54686818/glsl-geometry-shader-to-replace-gllinewidth)  

[OpenGL 3.2 Core Profile glLineWidth](https://stackoverflow.com/questions/8791531/opengl-3-2-core-profile-gllinewidth) 

Line width > 1.0 is deprecated and not further supported in a core profile [OpenGL Context](https://www.khronos.org/opengl/wiki/OpenGL_Context).  
However, it is still maintained in a compatibility profile context.

See [OpenGL 4.6 API Core Profile Specificatio - E.2.1 Deprecated But Still Supported Features](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf#page=700&zoom=100,168,758):

>The following features are deprecated, but still present in the core profile. They
may be removed from a future version of OpenGL, and are removed in a forwardcompatible context implementing the core profile.
>
> -  Wide lines - LineWidth values greater than 1.0 will generate an INVALID_VALUE error

---

Draw a single triangle for arcs. The triangle has to cover the entire arc. `discard` fragments in the fragment shader. 

---

<sub>
For a core profile context possible solutions are presented in the answers to:  
[OpenGL Line Width](https://stackoverflow.com/questions/3484260/opengl-line-width)  
[GLSL Geometry shader to replace glLineWidth](https://stackoverflow.com/questions/54686818/glsl-geometry-shader-to-replace-gllinewidth)   
[Drawing a variable width line in openGL (No glLineWidth)](https://stackoverflow.com/questions/101718/drawing-a-variable-width-line-in-opengl-no-gllinewidth)  
</sub>