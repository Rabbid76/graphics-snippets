#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_device.h>
#include <vk_utility_descriptor_set_layout.h>

#include <memory>


namespace vk_utility
{

    namespace core
    {
        /// <summary>
        /// Interface for Vulkan redner pass (`vk::DescriptorSetLayout`) factories
        /// </summary>
        class IDescriptorSetLayoutFactory
        {
        public:

            virtual DescriptorSetLayout Create(const vk_utility::device::DevicePtr &device) = 0;
            virtual DescriptorSetLayoutPtr New(const vk_utility::device::DevicePtr &device) = 0;
        };
    }
}