#pragma once

#include "vk_utility_vulkan_include.h"

#include <tuple>

namespace vk_utility
{
    namespace device
    {
        class DeviceMemoryFactory
        {
        public:

            virtual std::tuple<vk::DeviceMemory, vk::DeviceSize> New(vk::Device device) const = 0;
        };
    }
}