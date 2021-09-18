#pragma once

#include "vk_utility_vulkan_include.h"

#include <tuple>

namespace vk_utility
{
    namespace image
    {
        class ImageAndMemoryFactory
        {
        public:

            virtual std::tuple<vk::Image, vk::DeviceMemory, vk::DeviceSize> New(vk::Device device) const = 0;
        };
    }
}