#pragma once

#include "vk_utility_vulkan_include.h"

namespace vk_utility
{
    namespace image
    {
        class ImageFactory
        {
        public:

            virtual vk::Image New(vk::Device device) const = 0;
        };
    }
}