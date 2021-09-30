#pragma once

#include "vk_utility_vulkan_include.h"

namespace vk_utility
{

    namespace image
    {
        class SwapchainImageViewsFactory
        {
        public:

            virtual std::vector<vk::ImageView> New(vk::Device device) const = 0;
        };
    }
}