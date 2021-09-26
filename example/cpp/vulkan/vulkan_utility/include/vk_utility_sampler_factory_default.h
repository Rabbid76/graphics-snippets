#pragma once

#include "vk_utility_sampler_factory.h"

namespace vk_utility
{
    namespace image
    {
        /// <summary>
        /// Samplers are configured through a `vk::SamplerCreateInfo` structure, which specifies all filters and transformations that it should apply.
        ///
        /// The magFilter and minFilter fields specify how to interpolate texels that are magnified or minified.
        /// Magnification concerns the oversampling problem describes above, and minification concerns under-sampling.
        /// The choices are VK_FILTER_NEAREST and VK_FILTER_LINEAR, corresponding to the modes demonstrated in the images above.
        ///
        /// The addressing mode can be specified per axis using the addressMode fields.
        /// The available values are listed below. Most of these are demonstrated in the image above.
        /// Note that the axes are called U, V and W instead of X, Y and Z. This is a convention for texture space coordinates.
        /// 
        /// - VK_SAMPLER_ADDRESS_MODE_REPEAT: Repeat the texture when going beyond the image dimensions.
        /// - VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: Like repeat, but inverts the coordinates to mirror the image when going beyond the dimensions.
        /// - VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: Take the color of the edge closest to the coordinate beyond the image dimensions.
        /// - VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE: Like clamp to edge, but instead uses the edge opposite to the closest edge.
        /// - VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER: Return a solid color when sampling beyond the dimensions of the image.
        /// 
        /// It doesn't really matter which addressing mode we use here, because we're not going to sample outside of the image in this tutorial. 
        /// However, the repeat mode is probably the most common mode, because it can be used to tile textures like floors and walls.
        ///
        /// These two fields specify if anisotropic filtering should be used. There is no reason not to use this unless performance is a concern.
        /// The maxAnisotropy field limits the amount of texel samples that can be used to calculate the final color. 
        /// A lower value results in better performance, but lower quality results. There is no graphics hardware available today that will use more than 16 samples, because the difference is negligible beyond that point.
        ///
        /// The borderColor field specifies which color is returned when sampling beyond the image with clamp to border addressing mode. 
        /// It is possible to return black, white or transparent in either float or int formats. You cannot specify an arbitrary color.
        ///
        /// The unnormalizedCoordinates field specifies which coordinate system you want to use to address texels in an image. 
        /// If this field is VK_TRUE, then you can simply use coordinates within the [0, texWidth) and [0, texHeight) range.
        /// If it is VK_FALSE, then the texels are addressed using the [0, 1) range on all axes. 
        /// Real-world applications almost always use normalized coordinates, because then it's possible to use textures of varying resolutions with the exact same coordinates.
        ///
        /// If a comparison function is enabled, then texels will first be compared to a value, and the result of that comparison is used in filtering operations. This is mainly used for percentage-closer filtering on shadow maps.
        /// We'll look at this in a future chapter.
        /// </summary>
        class SamplerFactoryDefault
            : public SamplerFactory
        {
        private:

            uint32_t _mipmap_levels = 1;

        public:

            SamplerFactoryDefault& set_mipmap_levels(uint32_t mipmap_levels)
            {
                _mipmap_levels = mipmap_levels;
                return *this;
            }

            virtual vk::Sampler New(vk::Device device) const override
            {
                vk::SamplerCreateInfo sampler_information
                (
                    vk::SamplerCreateFlags{},
                    vk::Filter::eLinear,
                    vk::Filter::eLinear,
                    vk::SamplerMipmapMode::eLinear,
                    vk::SamplerAddressMode::eRepeat,
                    vk::SamplerAddressMode::eRepeat,
                    vk::SamplerAddressMode::eRepeat,
                    0.0f,
                    VK_TRUE,
                    16.0f,
                    VK_FALSE,
                    vk::CompareOp::eAlways,
                    0.0f, // static_cast<float>(_mipmap_levels / 2)
                    static_cast<float>(_mipmap_levels),
                    vk::BorderColor::eIntOpaqueBlack,
                    VK_FALSE
                );
                return device.createSampler(sampler_information);
            }
        };
    }
}