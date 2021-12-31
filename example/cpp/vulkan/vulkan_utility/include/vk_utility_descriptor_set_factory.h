#pragma once

#include "vk_utility_vulkan_include.h"
#include <vector>

namespace vk_utility
{

    namespace core
    {
        class DescriptorSetFactory
        {
        public:

            virtual std::vector<vk::DescriptorSet> New(vk::Device device) const = 0;
        };
    }
}
