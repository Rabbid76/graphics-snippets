#pragma once

#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_device.h>

namespace vk_utility
{

    namespace shader
    {
        class ShaderModule;
        using ShaderModulePtr = vk_utility::Ptr<ShaderModule>;

        /// <summary>
        /// ShaderrModule
        /// 
        /// Unlike earlier APIs, shader code in Vulkan has to be specified in a bytecode format
        /// as opposed to human-readable syntax like GLSL and HLSL.
        /// This bytecode format is called SPIR-V and is designed to be used with both Vulkan and OpenCL (both Khronos APIs).
        /// It is a format that can be used to write graphics and compute shaders. 
        /// </summary>
        class ShaderModule
            : public GenericObject<vk::ShaderModule>
        {
        private:

            vk_utility::device::DevicePtr _device;

        public:

            static ShaderModule Create(const vk_utility::device::DevicePtr& device, const std::vector<char> &source_code)
            {
                vk::ShaderModuleCreateInfo create_information
                (
                    vk::ShaderModuleCreateFlags{},
                    source_code.size(),
                    reinterpret_cast<const uint32_t*>(source_code.data())
                );
                auto shader_module = device->get().create_shader_module(create_information);
                return ShaderModule(device, shader_module);
            }

            static ShaderModulePtr New(const vk_utility::device::DevicePtr& device, const std::vector<char>& source_code)
            {

                return vk_utility::make_shared(Create(device, source_code));
            }

            static ShaderModule Create(const vk_utility::device::DevicePtr& device, const vk::ShaderModuleCreateInfo &create_information)
            {
                auto shader_module = device->get().create_shader_module(create_information);
                return ShaderModule(device, shader_module);
            }

            static ShaderModulePtr New(const vk_utility::device::DevicePtr& device, const vk::ShaderModuleCreateInfo& create_information)
            {
                
                return vk_utility::make_shared(Create(device, create_information));
            }

            static ShaderModule Create(const vk_utility::device::DevicePtr& device, vk::ShaderModule shader_module)
            {
                return ShaderModule(device, shader_module);
            }

            static ShaderModulePtr New(const vk_utility::device::DevicePtr& device, vk::ShaderModule shader_module)
            {
                return vk_utility::make_shared(Create(device, shader_module));
            }

            ShaderModule(void) = default;
            ShaderModule(const ShaderModule&) = default;
            ShaderModule& operator = (const ShaderModule&) = default;

            ShaderModule(const vk_utility::device::DevicePtr& device, vk::ShaderModule shader_module)
                : GenericObject(shader_module)
                , _device(device)
            {}

            virtual void destroy(void) override
            {
                if (_vk_object)
                {
                    if (_device)
                        _device->get().destroy_shader_module(_vk_object);
                    _vk_object = vk::ShaderModule();
                    _device = nullptr;
                }
            }
        };
    }
}