#pragma once

#include "vk_utility_vulkan_include.h"

namespace vk_utility
{
    namespace command
    {
        class CommandPoolFactory
        {
        public:

            virtual vk::CommandPool New(vk::Device device) const = 0;
        };
    }
}