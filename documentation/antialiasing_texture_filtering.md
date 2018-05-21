
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
