#pragma once

#include "vk_utility_object.h"
#include "vk_utility_exception.h"
#include "vk_utility_vulkan_include.h"
#include "vk_utility_device.h"
#include "vk_utility_swapchain.h"
#include "vk_utility_image_view.h"

namespace vk_utility
{

    namespace image
    {
        /// <summary>
        /// Interface for image view (`vk::ImageView`) factories
        /// </summary>
        class IImageViewFactory
        {
        public:

            virtual std::vector<vk_utility::image::ImageViewPtr> New(vk_utility::device::DevicePtr device) = 0;
        };
    }
}