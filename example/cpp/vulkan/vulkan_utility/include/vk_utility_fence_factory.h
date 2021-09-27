#pragma once

#include "vk_utility_vulkan_include.h"

namespace vk_utility
{
    namespace core
    {
        class FenceFactory
        {
        public:

            virtual vk::Fence New(vk::Device device) const = 0;
        };
    }
}
