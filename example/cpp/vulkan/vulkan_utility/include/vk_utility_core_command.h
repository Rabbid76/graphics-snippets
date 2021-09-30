#pragma once

#include "vk_utility_vulkan_include.h"

namespace vk_utility
{
    namespace core
    {
        class CoreCommand
        {
        public:

            virtual void execute_command(vk::Device device, vk::CommandPool command_pool) const = 0;
        };
    }
}
