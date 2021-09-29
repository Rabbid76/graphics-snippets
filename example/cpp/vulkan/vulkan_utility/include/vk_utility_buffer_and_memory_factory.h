#pragma once

#include "vk_utility_vulkan_include.h"

#include <tuple>

namespace vk_utility
{
    namespace buffer
    {
        class BufferAndMemoryFactory
        {
        public:

            virtual std::tuple<vk::Buffer, vk::DeviceSize, vk::DeviceMemory, vk::DeviceSize> New(vk::Device device) const = 0;
        };
    }
}
