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
        /// Interface for Vulkan swap extent selection (`vk::Extent2D`)
        /// </summary>
        class ISwapExtentSelector
        {
        public:

            virtual vk::Extent2D Select(device::PhysicalDevicePtr physical_device) = 0;
        };

        using SwapExtentSelectorPtr = std::shared_ptr<ISwapExtentSelector>;
    }
}