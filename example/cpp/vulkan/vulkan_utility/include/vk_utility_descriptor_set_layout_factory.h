#pragma once

#include "vk_utility_vulkan_include.h"

namespace vk_utility
{

    namespace core
    {
        class DescriptorSetLayoutFactory
        {
        public:

            virtual vk::DescriptorSetLayout New(vk::Device device) const = 0;
        };
    }
}