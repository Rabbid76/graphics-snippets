#pragma once

#include "vk_utility_vulkan_include.h"

namespace vk_utility
{
    namespace core
    {
        /// <summary>
        /// Creating a depth image is fairly straightforward. It should have the same resolution as the color attachment, 
        /// defined by the swap chain extent, an image usage appropriate for a depth attachment, optimal tiling and device local memory. 
        /// The only question is: what is the right format for a depth image?
        /// The format must contain a depth component, indicated by _D??_ in the VK_FORMAT_.
        ///
        /// Unlike the texture image, we don't necessarily need a specific format, 
        /// because we won't be directly accessing the texels from the program. 
        /// It just needs to have a reasonable accuracy, at least 24 bits is common in real-world applications. 
        /// There are several formats that fit this requirement:
        ///
        /// VK_FORMAT_D32_SFLOAT: 32-bit float for depth
        /// VK_FORMAT_D32_SFLOAT_S8_UINT: 32-bit signed float for depth and 8 bit stencil component
        /// VK_FORMAT_D24_UNORM_S8_UINT: 24-bit float for depth and 8 bit stencil component
        /// </summary>
        class FormatSelector
        {
        private:

            vk::ImageTiling _image_tiling = vk::ImageTiling::eOptimal;
            vk::FormatFeatureFlags _features{ 0 };

        public:

            FormatSelector& set_tiling(vk::ImageTiling image_tiling)
            {
                _image_tiling = image_tiling;
                return *this;
            }

            FormatSelector& set_feataures(vk::FormatFeatureFlags features)
            {
                _features = features;
                return *this;
            }

            vk::Format find_supported_format(vk::PhysicalDevice physical_device, const std::vector<vk::Format> &candidates)
            {
                for (vk::Format format : candidates) {
                    vk::FormatProperties props = physical_device.getFormatProperties(format);

                    if (_image_tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & _features) == _features) {
                        return format;
                    }
                    else if (_image_tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & _features) == _features) {
                        return format;
                    }
                }

                throw std::runtime_error("failed to find supported format!");
            }
        };
    }
}