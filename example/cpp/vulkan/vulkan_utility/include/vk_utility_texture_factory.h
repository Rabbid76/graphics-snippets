#pragma once

#include "vk_utility_vulkan_include.h"
#include <tuple>

namespace vk_utility
{
    namespace image
    {
        class TextureFactory
        {
        public: 
            
            virtual std::tuple<vk::Sampler, vk::ImageView, vk::Image, vk::DeviceMemory, vk::DeviceSize> New(
                vk::Device device, vk::CommandPool command_pool) const = 0;
        };
    }
}