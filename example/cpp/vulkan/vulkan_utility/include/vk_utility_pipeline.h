#pragma once

#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_device.h>

namespace vk_utility
{

    namespace pipeline
    {
        class Pipeline;
        using PipelinePtr = vk_utility::Ptr<Pipeline>;

        /// <summary>
        /// ShaderrModule
        /// 
        /// Unlike earlier APIs, shader code in Vulkan has to be specified in a bytecode format
        /// as opposed to human-readable syntax like GLSL and HLSL.
        /// This bytecode format is called SPIR-V and is designed to be used with both Vulkan and OpenCL (both Khronos APIs).
        /// It is a format that can be used to write graphics and compute shaders. 
        /// </summary>
        class Pipeline
            : public GenericObject<vk::Pipeline>
        {
        private:

            vk_utility::device::DevicePtr _device;

        public:

            static Pipeline Create(const vk_utility::device::DevicePtr& device, vk::GraphicsPipelineCreateInfo pipeline_information)
            {
                auto pipeline = device->get().create_graphics_pipeline(vk::PipelineCache(), pipeline_information);
                return Pipeline(device, pipeline.value);
            }

            static PipelinePtr New(const vk_utility::device::DevicePtr& device, vk::GraphicsPipelineCreateInfo pipeline_information)
            {
                return vk_utility::make_shared(Create(device, pipeline_information));
            }

            static Pipeline Create(const vk_utility::device::DevicePtr& device, vk::PipelineCache pipeline_cache, vk::GraphicsPipelineCreateInfo pipeline_information)
            {
                auto pipeline = device->get().create_graphics_pipeline(pipeline_cache, pipeline_information);
                return Pipeline(device, pipeline.value);
            }

            static PipelinePtr New(const vk_utility::device::DevicePtr& device, vk::PipelineCache pipeline_cache, vk::GraphicsPipelineCreateInfo pipeline_information)
            {
                return vk_utility::make_shared(Create(device, pipeline_cache, pipeline_information));
            }

            Pipeline(void) = default;
            Pipeline(const Pipeline&) = default;
            Pipeline& operator = (const Pipeline&) = default;

            Pipeline(const vk_utility::device::DevicePtr& device, vk::Pipeline pipeline)
                : GenericObject(pipeline)
                , _device(device)
            {}

            virtual void destroy(void) override
            {
                if (_vk_object)
                {
                    if (_device)
                        _device->get().destroy_pipeline(_vk_object);
                    _vk_object = vk::Pipeline();
                    _device = nullptr;
                }
            }
        };
    }
}
