#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_device.h>

#include <memory>


namespace vk_utility
{
    namespace core
    {
        class DescriptorSetLayout;
        using DescriptorSetLayoutPtr = vk_utility::Ptr<DescriptorSetLayout>;

        /// <summary>
        /// Interface for Vulkan redner pass (`vk::RenderPass`)
        /// </summary>
        class DescriptorSetLayout
            : public GenericObject<vk::DescriptorSetLayout>
        {
        private:

            vk_utility::device::DevicePtr _device;

        public:

            static DescriptorSetLayout Create(vk_utility::device::DevicePtr device, vk::DescriptorSetLayout descriptor_set_layout)
            {
                return DescriptorSetLayout(device, descriptor_set_layout);
            }

            static DescriptorSetLayoutPtr New(vk_utility::device::DevicePtr device, vk::DescriptorSetLayout descriptor_set_layout)
            {
                return vk_utility::make_shared(Create(device, descriptor_set_layout));
            }

            static DescriptorSetLayout Create(vk_utility::device::DevicePtr device, vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_infomration)
            {
                auto descriptor_set_layout = device->get().create_descriptor_set_layout(descriptor_set_layout_infomration);
                return DescriptorSetLayout(device, descriptor_set_layout);
            }

            static DescriptorSetLayoutPtr New(vk_utility::device::DevicePtr device, vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_infomration)
            {
                return vk_utility::make_shared(Create(device, descriptor_set_layout_infomration));
            }

            DescriptorSetLayout(void) = default;
            DescriptorSetLayout(const DescriptorSetLayout &) = default;
            DescriptorSetLayout &operator = (const DescriptorSetLayout &) = default;

            DescriptorSetLayout(vk_utility::device::DevicePtr device, vk::DescriptorSetLayout descriptor_set_layout)
                : GenericObject(descriptor_set_layout)
                , _device(device)
            {}

            virtual void destroy(void) override
            {
                if (_vk_object && _device)
                {
                    _device->get().destroy_descriptor_set_layout(_vk_object);
                    _vk_object = vk::DescriptorSetLayout();
                }
            }
        };
    }
}

