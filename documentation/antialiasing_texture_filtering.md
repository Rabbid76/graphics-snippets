
# TODO

[GridMaterial.](https://github.com/AnalyticalGraphicsInc/cesium/blob/1.16/Source/Shaders/Materials/GridMaterial.glsl#L17-L34)

## Removing moire patterns produced by GLSL shaders

[Removing moire patterns produced by GLSL shaders](https://stackoverflow.com/questions/34523682/removing-moire-patterns-produced-by-glsl-shaders)

Issue: [file:///C:/source/rabbid76workbench/dox/ogl/antialiasing/moire_patterns_question.html]
Solution: [file:///C:/source/rabbid76workbench/dox/ogl/antialiasing/moire_patterns_answer.html]


<br/><hr/>

# Anti-aliasing and Texture filtering

## Texture Filtering

### Bilinear filtering

[Bilinear filtering](https://en.wikipedia.org/wiki/Bilinear_filtering)

`GL_LINEAR` and no mipmap or `GL_LINEAR_MIPMAP_NEAREST` and mipmap


### Trilinear filtering

[Trilinear filtering](https://en.wikipedia.org/wiki/Trilinear_filtering)
`GL_LINEAR_MIPMAP_LINEAR` and mipmap 


### Anisotropic filtering

[Anisotropic filtering](https://en.wikipedia.org/wiki/Anisotropic_filtering)<br/>
[Anisotropic Filtering](https://www.geforce.com/whats-new/guides/aa-af-guide#1)


<br/><hr/>

## Anti-aliasing

[Anti-aliasing](https://www.geforce.com/whats-new/guides/aa-af-guide#2)

### Supersampling

Supersampling is a brute-force AA method that calculates a pixel's color by forcing the GPU to oversample the frame, or render it at dimensions equal to the resolution times the sampling rate (ex.: native resolution of 1680x1050 * 4 samples = 3360x2100) and obtaining color data from samples around a target pixel before downsampling (reducing) the frame to its original size. After the frame has been reduced a negative level of detail bias is applied to sharpen a sampled object's textures to counteract the blurring that downsampling and pixel merging produces. Supersampling is a type of full-scene anti-aliasing, meaning that every pixel in the frame is sampled and corrected rather than just those lying on the object's outer boundary, lending it to offer exceptional image quality but at an enormous cost to performance due to the GPU being required to compute so much additional information. The supersampling anti-aliasing modes are not available through the NVIDIA Control Panel.

### Multisampling

Multisampling still oversamples the frame (renders at a larger resolution) in the same fashion as supersampling, but to conserve processing time each sub-pixel inherits the color value from the sample pixel and only assigns unique depth values to the sub-pixels (whereas supersampling calculates individual color/depth values.) With the GPU being aware of which pixel will be displayed when the frame is downsampled, the final color is calculated by evaluating the depth values: if the sub-pixel depths are inconsistent, the pixel is lying on the edge and will be colored by altering the opacity of the pixel by accounting the number of samples taken and the number of sub-pixels that had different depth values (which for 4x would be 100%, 75%, 50%, 25%, and 0%.) This method comes with a substantially lower performance requirement than supersampling while still providing acceptable frame rates, leading it to typically being available in most 3D games.


<br/><hr/>

## Anti aliasing overview


- Alexander Grahn, An Image and Processing Comparison Study of Antialiasing Methods, [http://www.diva-portal.org/smash/get/diva2:972774/FULLTEXT02]
- Conservative Morphological Anti-Aliasing (CMAA), https://software.intel.com/en-us/articles/conservative-morphological-anti-aliasing-cmaa-update


<br/><hr/>
## Super-Sampling Anti-Aliasing (SSAA)

*Super-Sampling Anti-Aliasing* (SSAA) is a concept of producing antialiasing by calculating the scene in a higher resolution than normal, and then down-sampling it to the correct resolution. This will allow you to take more samples for each pixel. The method is, however, very inecient as it perform these extra samples for all pixels in an image, even in those that do not need it.


- Hobbes Productions, SSAA, [https://www.cise.ufl.edu/~mdwyer/ssaa.html]


<br/><hr/>
## Multi-Sampling Anti-Aliasing (MSAA)

*Multi-Sample Anti-Aliasing* (MSAA) follows in the footsteps of SSAA but introduced new techniques and optimizations. MSAA uses a two to eight times higher resolution than normal to find a limited amount of subsamples for the final pixel. By knowing the exposure of the subpixels it will perform supersampling of the edge of the polygon and perform colour calculations only once.


- Hobbes Productions, MSAA, [https://www.cise.ufl.edu/~mdwyer/msaa.html]

<br/><hr/>
## Decoupled Coverage Anti-Aliasing (DCAA)

State-of-the-art methods for geometric anti-aliasing in real-time rendering are based on Multi-Sample Anti-Aliasing (MSAA), which samples visibility more than shading to reduce the number of expensive shading calculations. However, for high-quality results the number of visibility samples needs to be large (e.g., 64 samples/pixel), which requires significant memory because visibility samples are usually 24-bit depth values. In this paper, we present Decoupled Coverage Anti-Aliasing (DCAA), which improves upon MSAA by further decoupling coverage from visibility for high-quality geometric anti-aliasing. Our work is based on the previously-explored idea that all fragments at a pixel can be consolidated into a small set of visible surfaces. Although in the past this was only used to reduce the memory footprint of the G-Buffer for deferred shading with MSAA, we leverage this idea to represent each consolidated surface with a 64-bit binary mask for coverage and a single decoupled depth value, thus significantly reducing the overhead for high-quality anti-aliasing. To do this, we introduce new surface merging heuristics and resolve mechanisms to manage the decoupled depth and coverage samples. Our prototype implementation runs in real-time on current graphics hardware, and results in a significant reduction in geometric aliasing with less memory overhead than 8� MSAA for several complex scenes.


- ACM / EG Symposium on High Performance Graphics, Decoupled Coverage Anti-Aliasing, [http://research.nvidia.com/publication/decoupled-coverage-anti-aliasing]
- decoupled sampling for graphics pipelines, [http://people.csail.mit.edu/jrk/decoupledsampling/]


<br/><hr/>
## Post-Processing Antialiasing (PPAA)

*Post-processing Anti-Aaliasing* (PPAA) takes the final image of a scene and essentially blurs it with various techniques to hide the aliasing. This method is signifficantly cheaper on performance, but may yield worse results in the final image. It is, however, important to note that supersampling can be combined with PPAA.


<br/><hr/>
### Morpho-Logical Anti-Aliasing (MLAA)
*Morpho-Logical Anti-Aliasing* (MLAA) introduced new concepts and yielded good results which inspired new ideas and further development of methods such as *Fast Approximate Anti-Aliasing* (FXAA) and *Subpixel Morphological Antialiasing* SMAA


- Geeks3D, What is the Morphological Anti-Aliasing (MLAA), [http://www.geeks3d.com/20101023/tips-what-is-the-morphological-anti-aliasing-mlaa/]
- Real-Time Rendering, Morphological Antialiasing, [http://www.realtimerendering.com/blog/morphological-antialiasing/]
- Practical morphological antialiasing on the GPU, [http://igm.univ-mlv.fr/~biri/mlaa-gpu/MLAAGPU.pdf]
- Alexander Reshetov , Morphological Antialiasing, [http://www.cs.cmu.edu/afs/cs/academic/class/15869-f11/www/readings/reshetov09_mlaa.pdf]
- Alexander Reshetov , Morphological Antialiasing, [http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.152.2744&rep=rep1&type=pdf]
- Morphological Antialiasing and topological reconstruction [http://igm.univ-mlv.fr/~biri/mlaa-gpu/TMLAA.pdf]


<br/><hr/>
### Fast Approximate Anti-Aliasing (FXAA)

Fast Approximate Anti-Aliasing (FXAA) is an anti-aliasing algorithm created by Timothy Lottes under NVIDIA. The main advantage of this technique over conventional anti-aliasing is that it does not require large amounts of computing power. It achieves this by smoothing jagged edges ("jaggies") according to how they appear on screen as pixels, rather than analyzing the 3D model itself as in conventional anti-aliasing. Since it is not based on the actual geometry, it will smooth not only edges between triangles, but also edges inside alpha-blended textures or resulting from pixel shader effects, which are immune to the effects of multisample anti-aliasing (MSAA).


- NVIDIA - FXAA - White paper (pdf), [http://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf]
- FXAA 3.11 in 15 Slides, [http://iryoku.com/aacourse/downloads/09-FXAA-3.11-in-15-Slides.pdf]
- Hobbes Productions, FXAA, [https://www.cise.ufl.edu/~mdwyer/fxaa.html]
- CODING HORROR.com - Fast Approximate Anti-Aliasing (FXAA), [https://blog.codinghorror.com/fast-approximate-anti-aliasing-fxaa/]
- Geeks3D - (Tested) Fast Approximate Anti-Aliasing (FXAA) Demo (GLSL), [http://www.geeks3d.com/20110405/fxaa-fast-approximate-anti-aliasing-demo-glsl-opengl-test-radeon-geforce/]
- Implementing FXAA, Simon Rodriguez, [http://blog.simonrodriguez.fr/articles/30-07-2016_implementing_fxaa.html]
- Jeff Atwood - Coding Horror, What Is FXAA, And Why Has It Made Anti-Aliasing As We Know It Obsolete?, [https://www.kotaku.com.au/2011/12/what-is-fxaa/]
- Stackoverflow - How does this simple FxAA work?, [https://stackoverflow.com/questions/12105330/how-does-this-simple-fxaa-work]
- GitHub, NVIDIA FXAA 3.11 by TIMOTHY LOTTES, [https://gist.github.com/kosua20/0c506b81b3812ac900048059d2383126]
- GitHub, GraphicsSamples/samples/es3-kepler/FXAA/, [https://github.com/NVIDIAGameWorks/GraphicsSamples/tree/master/samples/es3-kepler/FXAA]


<br/><hr/>
### Enhanced Subpixel Morphological Antialiasing SMAA

*Enhanced Subpixel Morphological Antialiasing* (SMAA) is a newer form of anti-aliasing that is gaining popularity in video games. Its strengths include performance, edge detection (using an algorithm which is based on MLAA's edge detection), and enhanced pattern detection (which enables it to render sharp geometries, such as sharp diagonal edges, that would otherwise be rounded off). It is particularly good at handling diagonal lines. However, because it is a post-processing approach, SMAA shares many of FXAA's defects (such as loss of clarity).	



- Hobbes Productions, SMAA, [https://www.cise.ufl.edu/~mdwyer/smaa.html]
- SMAA: Enhanced Subpixel Morphological Antialiasing, [http://www.iryoku.com/smaa/]


<br/><hr/>
### Conservative Morphological AntiAliasing (CMAA)

Conservative Morphological AntiAliasing (CMAA) is an interesting method which is inspired by MLAA, FXAA and SMAA but with its own variation of, amongst other things, nding local dominant edges and handling of symmetrical long edge shapes.


- Intel, Conservative Morphological Anti-Aliasing (CMAA), [https://software.intel.com/en-us/articles/conservative-morphological-anti-aliasing-cmaa-update]


<br/><hr/>
## Topological reconstruction AntiAliasing (TMLAA)



<br/><hr/>
## Temporal Anti-Aliasing TXAA

TXAA is a new film?style anti?aliasing technique designed specifically to reduce temporal aliasing (crawling and flickering seen in motion when playing games). This technology is a mix of a temporal filter, hardware anti?aliasing, and custom CG film?style anti?aliasing resolves. To filter any given pixel on the screen, TXAA uses a contribution of samples, both inside and outside of the pixel, in conjunction with samples from prior frames, to offer the highest quality filtering possible. TXAA has improved spatial filtering over standard 2xMSAA and 4xMSAA. For example, on fences or foliage and in motion, TXAA starts to approach and sometimes exceeds the quality of other high?end professional anti?aliasing algorithms.


- Hobbes Productions, Temporal, [https://www.cise.ufl.edu/~mdwyer/temporal.html
- Nvidia, Temporal Anti-Aliasing TXAA, [https://www.geforce.com/hardware/technology/txaa/technology]

