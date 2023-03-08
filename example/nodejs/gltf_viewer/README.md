# Proof of Concept - Rubens Three.js renderer

The purpose of this project is to investigate the potential of a native Three.js renderer compared to the Pixotronics renderer.
Not everything is fully implemented in the sense of being ready for release (performance, stability), and some functions are still at an experimental stage.
However, this project gives a good impression of what is possible with Three.js.

[Open PoC Three.js renderer](https://roomle-dev.github.io/poc-rubens-threejs-renderer/beta/1/)

Load any configuration, component or item:

- `https://roomle-dev.github.io/poc-rubens-threejs-renderer/beta/1/?id=<your configuration id>`
- `https://roomle-dev.github.io/poc-rubens-threejs-renderer/beta/1/?id=component@<your component id>`
- `https://roomle-dev.github.io/poc-rubens-threejs-renderer/beta/1/?id=<your item id>`

If you do not specify an ID ([https://roomle-dev.github.io/poc-rubens-threejs-renderer/beta/1/](https://roomle-dev.github.io/poc-rubens-threejs-renderer/beta/1/)), a small menu appears at the top right where you can select one of several predefined IDs from a dropdown box.

As this is a PoC, neither UX design nor optimisations with regard to loading speed were made. So be patient, there is no loading spinner.
The renderer may also be unstable with intensive use. If there are glitches or it doesn't respond etc., don't bother, just refresh it.

Note that the result of 2 renderers is never the same. It always looks different. The overall impression a scene creates depends strongly on the light settings.
The light setting, ambient occlusion settings, encoding and tone mapping may not be the same as when loading a configuration in Rubens.
Advanced users can tweak the settings in the menu on the right.  

There is also the possibility to change the material settings for each material (not permanently, of course).
New material settings that are not yet implemented in Rubens but could be implemented with this renderer in the future can also be tried out.  
For example, change one of the following properties:

- "clearCoat" (The more metallic a material is, the less noticeable the effect)
- Select a non-black "sheenColor" and set the "sheen" (The more metallic a material is, the less noticeable the effect)
- Select a non-black "emissive"-color and set the "emissiveIntensity"

Also implemented is an optional outline for selections. This features is not yet implemented with Anti-Aliasing.  
In addition, there is optional Bloom (Unreal Bloom) and Screen Space Reflection Pass (SSR). These features are at the experimental stage.
Nice SSR/Bloom example (note the performance is low and the GPU is sweating):

- select configurationId "ktm 616"
- enable "ssr" and "bloom"

Three.js is widely used and improving rapidly, and in the future will allow us to use technologies such as WebGPU and Real-Time Ray-Tracing.
([Real-Time Ray-Tracing in WebGPU](https://maierfelix.github.io/2020-01-13-webgpu-ray-tracing/))

## Install and build

```lang-none
npm run install:webpack
npm i  
npm run build
npm run dev
```

## Three.js examples

- [https://threejs.org/examples/](https://threejs.org/examples/)

## Three.js Shadow

- [https://sbcode.net/threejs/directional-light-shadow/](https://sbcode.net/threejs/directional-light-shadow/)
- [https://sbcode.net/threejs/soft-shadows/](https://sbcode.net/threejs/soft-shadows/)
- [https://cs.wellesley.edu/~cs307/lectures/20new.html](https://cs.wellesley.edu/~cs307/lectures/20new.html)
- [https://discourse.threejs.org/t/performant-soft-shadows-three-js/27777](https://discourse.threejs.org/t/performant-soft-shadows-three-js/27777)
- [https://codesandbox.io/s/soft-shadows-dh2jc?file=/src/index.js:143-154](https://codesandbox.io/s/soft-shadows-dh2jc?file=/src/index.js:143-154)

### Three.js basic shadow map

- [https://threejs.org/examples/?q=shadow#webgl_shadowmap_pointlight](https://threejs.org/examples/?q=shadow#webgl_shadowmap_pointlight)
- [https://threejs.org/examples/?q=shadow#webgl_shadowmap_viewer](https://threejs.org/examples/?q=shadow#webgl_shadowmap_viewer)
- [https://threejs.org/examples/webgl_shadowmesh.html](https://threejs.org/examples/webgl_shadowmesh.html)

### Three.js PCF (soft) shadow map

- [https://threejs.org/examples/webgl_shadowmap.html](https://threejs.org/examples/webgl_shadowmap.html)
- [https://threejs.org/examples/webgl_shadowmap_performance.html](https://threejs.org/examples/webgl_shadowmap_performance.html)

### Three.js Cascaded shadow mapping (CSM)

- [https://threejs.org/examples/webgl_shadowmap_csm.html](https://threejs.org/examples/webgl_shadowmap_csm.html)

### Three.js VSM shadow map

- [https://threejs.org/examples/webgl_shadowmap_vsm.html](https://threejs.org/examples/webgl_shadowmap_vsm.html)

### Three.js PCSS shadow

- [https://threejs.org/examples/?q=shado#webgl_shadowmap_pcss](https://threejs.org/examples/?q=shado#webgl_shadowmap_pcss)

## Three-js postprocessing

[How to use post-processing](https://threejs.org/docs/#manual/en/introduction/How-to-use-post-processing)

### Progressive Light map

- [https://threejs.org/examples/webgl_shadowmap_progressive.html](https://threejs.org/examples/webgl_shadowmap_progressive.html)
- [https://codesandbox.io/s/adaptive-lightmaps-wsg13?file=/src/Lightmap.js:251-270](https://codesandbox.io/s/adaptive-lightmaps-wsg13?file=/src/Lightmap.js:251-270)

#### Three.js contact shadow

- [https://threejs.org/examples/webgl_shadow_contact.html](https://threejs.org/examples/webgl_shadow_contact.html)
- [https://codesandbox.io/s/shoe-configurator-qxjoj?file=/src/App.js](https://codesandbox.io/s/shoe-configurator-qxjoj?file=/src/App.js)

### Static light map

- [https://threejs.org/examples/webgl_materials_lightmap.html](https://threejs.org/examples/webgl_materials_lightmap.html)

### Ambient Occlusion

- [https://gkjohnson.github.io/threejs-sandbox/gtaoPass/](https://gkjohnson.github.io/threejs-sandbox/gtaoPass/)

#### Three.js SSAOPass

- [https://threejs.org/examples/webgl_postprocessing_ssao.html](https://threejs.org/examples/webgl_postprocessing_ssao.html)
- [https://alteredqualia.com/three/examples/webgl_postprocessing_ssao.html](https://alteredqualia.com/three/examples/webgl_postprocessing_ssao.html)

### Three.js SAOPass

- [https://threejs.org/examples/webgl_postprocessing_sao.html](https://threejs.org/examples/webgl_postprocessing_sao.html)

### Environment

- [https://threejs.org/docs/#api/en/extras/PMREMGenerator](https://threejs.org/docs/#api/en/extras/PMREMGenerator)

### Global illumination

- [Adopting a Progressive Photorealistic Global Illumination in Three.JS](https://github.com/mrdoob/three.js/issues/14051)  
- [https://threejs.org/examples/webgl_simple_gi.html](https://threejs.org/examples/webgl_simple_gi.html)

### Screen space reflection

- [https://threejs.org/examples/webgl_postprocessing_ssr.html](https://threejs.org/examples/webgl_postprocessing_ssr.html)
- [https://github.com/0beqz/screen-space-reflections](https://github.com/0beqz/screen-space-reflections)
- [npm - three.js Screen Space Reflections](https://www.npmjs.com/package/screen-space-reflections)  
- [https://screen-space-reflections.vercel.app/](https://screen-space-reflections.vercel.app/)
- [Three.js Screen Space Reflections](https://reactjsexample.com/three-js-screen-space-reflections/)  

### Outline

- [https://threejs.org/examples/webgl_postprocessing_outline.html](https://threejs.org/examples/webgl_postprocessing_outline.html)

### Bloom

- [https://threejs.org/examples/webgl_postprocessing_unreal_bloom.html](https://threejs.org/examples/webgl_postprocessing_unreal_bloom.html)
- [https://threejs.org/examples/webgl_postprocessing_unreal_bloom_selective.html](https://threejs.org/examples/webgl_postprocessing_unreal_bloom_selective.html)
- [https://github.com/mattatz/THREE.BloomBlendPass](https://github.com/mattatz/THREE.BloomBlendPass)
