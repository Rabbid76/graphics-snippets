# Cylindrical projection

[How to project top and bottom area of openGL control](https://stackoverflow.com/questions/53943867/how-to-project-top-and-bottom-area-of-opengl-control/53947821#comment94752319_53947821)

If you want to project a 2D texture on a 2d plane as it would be a 3D cylinder, then you have to transform the texture coordinate by an [arcus function](https://en.wikipedia.org/wiki/Inverse_trigonometric_functions) ([`asin`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/asin.xhtml) or [`acos`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/acos.xhtml)) in the fragment shader.

The texture coordinate in the range [0, 1] have to be associated to an angle on a circle in the range [-90°, 90°] by the `asin` function. This angle can be linear mapped to the new texture coordinate in the range [0, 1].

[![][1]][1]

The input to the function is an angle and the return value is a distance:

```glsl
float u = asin( vTexCoord.x*2.0-1.0 ) / 3.141593 + 0.5;
```

[![][2]][2]

*Vertex shader*:


```glsl
attribute vec3 a_position;
varying vec2 vTexCoord;
                                
void main()
{
    vTexCoord   = (a_position.xy + 1) / 2;
    gl_Position = vec4(a_position, 1);
}
```

*Fragment shader*:

```glsl
precision highp float;
uniform sampler2D sTexture;
varying vec2 vTexCoord;

void main()
{
    float u = asin( vTexCoord.x*2.0-1.0 ) / 3.141593 + 0.5;
    float v = vTexCoord.y; 

    vec4 color   = texture2D(sTexture, vec2(u, v));
    gl_FragColor = color;
}
```

See the difference between the result of the original code and the code which uses the `asin` mapping:

[![][3]][3]
[![][4]][4]

---

In the projection to a 2D plane, the top and bottom of the cylinder is an [ellipse](https://en.wikipedia.org/wiki/Ellipse), which can be expressed by:

[![][5]][5]

<!-- language: glsl -->

```glsl
float b = 0.3;
float y = b * sqrt(1.0 - x*x)
```

The projection of the texture has to be squeezed at the top and the bottom to form an elliptical shape:

[![][6]][6]


```glsl
float v_scale = (1.0 + b) / (1.0 + y);
float v = (pos.y * v_scale) * 0.5 + 0.5;
```

The area which is clipped has to be discarded by using the [`discard`](https://www.khronos.org/opengl/wiki/Fragment_Shader#Special_operations) keyword in the [fragment shader](https://www.khronos.org/opengl/wiki/Fragment_Shader):


```glsl
if ( v < 0.0 || v > 1.0 )
    discard;
```

See the difference between the result without the elliptical distortion and the code which uses the elliptical distortion:

[![][7]][7]
[![][8]][8]

---

The fragment shader which combines the `asin` texture coordinate mapping and the elliptical distortion:

*Fragment shader:*

```glsl
precision highp float;
uniform sampler2D sTexture;
varying vec2 vTexCoord;

void main()
{
    vec2  pos     = vTexCoord.xy * 2.0 - 1.0;
    float b       = 0.3;
    float v_scale = (1.0 + b) / (1.0 + b * sqrt(1.0 - pos.x*pos.x));

    float u = asin( pos.x ) / 3.1415 + 0.5;
    float v = (pos.y * v_scale) * 0.5 + 0.5;
    if ( v < 0.0 || v > 1.0 )
        discard;

    vec3 texColor = texture2D( u_texture, vec2(u, v) ).rgb;
    gl_FragColor  = vec4( texColor.rgb, 1.0 );
}
```

The combined result:

[![][9]][9]


  [1]: https://i.stack.imgur.com/4VE0t.png
  [2]: https://i.stack.imgur.com/QWpG5.png
  [3]: https://i.stack.imgur.com/C2srO.png
  [4]: https://i.stack.imgur.com/tS70o.png
  [5]: https://i.stack.imgur.com/zyKmY.png
  [6]: https://i.stack.imgur.com/sJaqz.png
  [7]: https://i.stack.imgur.com/HLQOM.png
  [8]: https://i.stack.imgur.com/uY0Cr.png
  [9]: https://i.stack.imgur.com/acSZH.png