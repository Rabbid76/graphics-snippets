#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_device.h>
#include <vk_utility_render_pass.h>

#include <memory>


namespace vk_utility
{

    namespace core
    {
        /// <summary>
        /// Interface for Vulkan redner pass (`vk::RenderPass`) factories
        /// </summary>
        class IRenderPassFactory
        {
        public:

            virtual RenderPass Create(const vk_utility::device::DevicePtr &device) = 0;
            virtual RenderPassPtr New(const vk_utility::device::DevicePtr &device) = 0;
        };
    }
}
