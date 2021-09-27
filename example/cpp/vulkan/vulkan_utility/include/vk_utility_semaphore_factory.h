#pragma once

#include "vk_utility_vulkan_include.h"

namespace vk_utility
{
    namespace core
    {
        class SemaphoreFactory
        {
        public:

            virtual vk::Semaphore New(vk::Device device) const = 0;
        };
    }
}