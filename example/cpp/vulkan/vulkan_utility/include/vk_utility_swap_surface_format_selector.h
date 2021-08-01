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
        /// Interface for Vulkan swap surface format selection (`vk::SurfaceFormatKHR`)
        /// </summary>
        class ISwapSurfaceFormatSelector
        {
        public:

            virtual vk::SurfaceFormatKHR Select(device::PhysicalDevicePtr physical_device) = 0;
        };

        using SwapSurfaceFormatSelectorPtr = std::shared_ptr<ISwapSurfaceFormatSelector>;
    }
}