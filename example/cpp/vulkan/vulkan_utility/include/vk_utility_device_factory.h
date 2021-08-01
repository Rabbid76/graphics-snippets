#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_physical_device.h>
#include <vk_utility_device.h>

#include <memory>


namespace vk_utility
{

    namespace device
    {
        /// <summary>
        /// Interface for Vulkan device (`vk::Device`) factories
        /// </summary>
        class IDeviceFactory
        {
        public:

            virtual Device Create(vk_utility::device::PhysicalDevicePtr physical_device) = 0;
            virtual DevicePtr New(vk_utility::device::PhysicalDevicePtr physical_device) = 0;
        };
    }
}
