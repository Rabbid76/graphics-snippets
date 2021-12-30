#pragma once


#include "vk_utility_object.h"
#include "vk_utility_descriptor_set_layout_factory.h"


namespace vk_utility
{
    namespace core
    {
        class DescriptorSetLayout;
        using DescriptorSetLayoutPtr = vk_utility::Ptr<DescriptorSetLayout>;

        class DescriptorSetLayout
            : public GenericObject<vk::DescriptorSetLayout>
        {
        private:

            vk::Device _device;

        public:

            static DescriptorSetLayout New(vk::Device device, const DescriptorSetLayoutFactory& descriptor_set_layout_factory)
            {
                return DescriptorSetLayout(device, descriptor_set_layout_factory.New(device));
            }

            static DescriptorSetLayoutPtr NewPtr(vk::Device device, const DescriptorSetLayoutFactory& descriptor_set_layout_factory)
            {
                return make_shared(New(device, descriptor_set_layout_factory));
            }

            DescriptorSetLayout(void) = default;
            DescriptorSetLayout(const DescriptorSetLayout &) = default;
            DescriptorSetLayout &operator = (const DescriptorSetLayout &) = default;

            DescriptorSetLayout(vk::Device device, vk::DescriptorSetLayout descriptor_set_layout)
                : GenericObject(descriptor_set_layout)
                , _device(device)
            {}

            virtual void destroy(void) override
            {
                if (_vk_object && _device)
                {
                    _device.destroyDescriptorSetLayout(_vk_object);
                    _vk_object = vk::DescriptorSetLayout();
                }
            }
        };
    }
}

