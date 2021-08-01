#pragma once

#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_device.h>
#include <vk_utility_shader_module.h>

#include <memory>


namespace vk_utility
{

    namespace shader
    {
        /// <summary>
        /// Interface for Vulkan pipeline (`vk::ShaderModule`) factories
        /// </summary>
        class IShaderModuleFactory
        {
        public:

            virtual ShaderModule Create(vk_utility::device::DevicePtr device) = 0;
            virtual ShaderModulePtr New(vk_utility::device::DevicePtr device) = 0;
        };
    }
}
