#pragma once

#include "vk_utility_vulkan_include.h"

namespace vk_utility
{
    namespace command
    {
        class CommandBufferFactory
        {
        public:

            virtual vk::CommandBuffer Begin(vk::Device device, vk::CommandPool command_pool) const = 0;
            virtual void End(vk::CommandBuffer) const = 0;
        };
    }
}