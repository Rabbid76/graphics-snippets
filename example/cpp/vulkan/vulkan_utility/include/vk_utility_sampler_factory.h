#pragma once

#include "vk_utility_vulkan_include.h"

namespace vk_utility
{
    namespace image
    {
        class SamplerFactory
        {
        public:

            virtual vk::Sampler New(vk::Device device) const = 0;
        };
    }
}