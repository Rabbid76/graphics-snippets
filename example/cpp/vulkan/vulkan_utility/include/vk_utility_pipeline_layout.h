#pragma once

#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_device.h>
#include <vk_utility_descriptor_set_layout.h>


namespace vk_utility
{

    namespace pipeline
    {
        class PipelineLayout;
        using PipelineLayoutPtr = vk_utility::Ptr<PipelineLayout>;

        /// <summary>
        /// PipelineLayout
        /// </summary>
        class PipelineLayout
            : public GenericObject<vk::PipelineLayout>
        {
        private:

            vk_utility::device::DevicePtr _device;

        public:

            static PipelineLayout Create(vk_utility::device::DevicePtr device, vk_utility::core::DescriptorSetLayoutPtr descriptor_set_layout)
            {
                std::vector<vk::DescriptorSetLayout> layouts{ descriptor_set_layout->handle() };
                std::vector<vk::PushConstantRange> push_constant_ranges;
                vk::PipelineLayoutCreateInfo pipeline_layout_information
                (
                    vk::PipelineLayoutCreateFlags{},
                    layouts,
                    push_constant_ranges
                );
                return Create(device, pipeline_layout_information);
            }

            static PipelineLayoutPtr New(vk_utility::device::DevicePtr device, vk_utility::core::DescriptorSetLayoutPtr descriptor_set_layout)
            {
                return vk_utility::make_shared(Create(device, descriptor_set_layout));
            }

            static PipelineLayout Create(vk_utility::device::DevicePtr device, vk::PipelineLayoutCreateInfo pipeline_layout_information)
            {
                auto pipeline_layout = device->get().create_graphics_pipeline_layout(pipeline_layout_information);
                return PipelineLayout(device, pipeline_layout);
            }

            static PipelineLayoutPtr New(vk_utility::device::DevicePtr device, vk::PipelineLayoutCreateInfo pipeline_layout_information)
            {
                return vk_utility::make_shared(Create(device, pipeline_layout_information));
            }

            PipelineLayout(void) = default;
            PipelineLayout(const PipelineLayout&) = default;
            PipelineLayout& operator = (const PipelineLayout&) = default;

            PipelineLayout(vk_utility::device::DevicePtr device, vk::PipelineLayout pipeline_layout)
                : GenericObject(pipeline_layout)
                , _device(device)
            {}

            virtual void destroy(void) override
            {
                if (_vk_object)
                {    
                    if (_device)
                        _device->get().destroy_pipeline_layout(_vk_object);
                    _vk_object = vk::PipelineLayout();
                    _device = nullptr;
                }
            }
        };
    }
}
