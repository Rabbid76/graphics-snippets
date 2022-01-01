#pragma once

#include "vk_utility_vulkan_include.h"
#include <vector>

namespace vk_utility
{

    namespace command
    {
        class CommandBuffersFactory
        {
        public:

            virtual std::vector<vk::CommandBuffer> New(vk::Device device, vk::CommandPool command_pool) const = 0;
        };
    }
}
