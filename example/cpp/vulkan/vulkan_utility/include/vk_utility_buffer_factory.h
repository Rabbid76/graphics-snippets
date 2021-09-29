#pragma once

#include "vk_utility_vulkan_include.h"

namespace vk_utility
{
    namespace buffer
    {
        class BufferFactory
        {
        public:

            virtual std::tuple<vk::Buffer, vk::DeviceSize> New(vk::Device device) const = 0;
        };
    }
}