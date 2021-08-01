#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_swapchain.h>
#include <vk_utility_device.h>
#include <vk_utility_surface.h>


namespace vk_utility
{

    namespace swap
    {
        /// <summary>
        /// Interface for Vulkan swapchain (`vk::SwapchainKHR`) factories
        /// </summary>
        class ISwapchainFactory
        {
        public:

            virtual Swapchain Create(vk_utility::device::DevicePtr device, vk_utility::device::SurfacePtr surface) = 0;
            virtual SwapchainPtr New(vk_utility::device::DevicePtr device, vk_utility::device::SurfacePtr surface) = 0;
        };
    }
}
