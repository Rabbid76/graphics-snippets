#pragma once

#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_device.h>
#include <vk_utility_swapchain.h>
#include <vk_utility_pipeline.h>

#include <memory>


namespace vk_utility
{

    namespace pipeline
    {
        /// <summary>
        /// Interface for Vulkan pipeline (`vk::Pipeline`) factories
        /// </summary>
        class IPipelineFactory
        {
        public:

            virtual Pipeline Create(vk_utility::device::DevicePtr device, vk_utility::swap::SwapchainPtr swapchain) = 0;
            virtual PipelinePtr New(vk_utility::device::DevicePtr device, vk_utility::swap::SwapchainPtr swapchain) = 0;
        };
    }
}

