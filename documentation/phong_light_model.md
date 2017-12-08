
# Phong light model


The Phong reflection model (also called Phong illumination or Phong lighting) is an empirical model of the local illumination of points on a surface. In 3D computer graphics, it is sometimes ambiguously referred to as "Phong shading", in particular if the model is used in combination with the interpolation method of the same name and in the context of pixel shaders or other places where a lighting calculation can be referred to as "shading". \n

Basically, it is an empirical model which obeys neither energy conservation nor reciprocity. It depends on an `alpha` angle, which is calculated after reflecting the incident direction. \n

![dot A, B](image/light_reflect.svg)

    alpha   = theta_o - theta_i
    f_phong = max(0, pow(cos(alpha), sh)

`sh` (shininess) in [0, infinite] characterizes the shape of the specular highlight (from 0 or dull to more glossy surface).

`cos(alpha)` can be caluclated by the dot product of the view vector and the reflection vector:

    R       = reflect(L, N)
    VdotR   = dot(V, R)
    f_phong = max(0, pow(VdotR, sh))

![reflect L, N](image/phong_reflect.svg)

![dot V, R](image/phong_specular.svg)


<br/>
GLSL coding:

    float Distribution_Phong( vec3 esVEye, vec3 esVLight, vec3 esPtNV, float shininess ) 
    {
        vec3 reflVector = normalize( reflect( -esVLight, esPtNV ) );
        return 4.0 * max( pow( dot(reflVector, esVEye), 0.3 * shininess ), 0.0 );
    }


<br/>
Furthermore, the performance of this model could be improved by an optimization of the exponential operator, like the approximation given by Schlick:

    VdotR_sh = VdotR / (sh - sh * VdotR - VdotR)


<br/>
See also:

- [wikipedia, Phong reflection model](https://en.wikipedia.org/wiki/Phong_reflection_model)
- [wikibooks, GLSL Programming/Unity/Smooth Specular Highlights](https://en.wikibooks.org/wiki/GLSL_Programming/Unity/Smooth_Specular_Highlights)
- [Bui Tuong Phong, University of Utah, Illumination for computer generated pictures, 1975](http://www.cs.northwestern.edu/~ago820/cs395/Papers/Phong_1975.pdf)
