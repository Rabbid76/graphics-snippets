#pragma once

#include "vk_utility_vulkan_include.h"

namespace vk_utility
{

    namespace image
    {
        class ImageViewFactory
        {
        public:

            virtual vk::ImageView New(vk::Device device) const = 0;
            virtual ImageViewFactory& set_image(vk::Image image) = 0;
        };
    }
}