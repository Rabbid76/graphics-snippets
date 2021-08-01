#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_physical_device.h>

#include <memory>


namespace vk_utility
{

    namespace swap
    {
        /// <summary>
        /// Interface for Vulkan swap present mode selection (vk::PresentModeKHR`)
        /// </summary>
        class ISwapPresentModeSelector
        {
        public:

            virtual vk::PresentModeKHR Select(device::PhysicalDevicePtr physical_device) = 0;
        };

        using SwapPresentModeSelectorPtr = std::shared_ptr<ISwapPresentModeSelector>;
    }
}