#pragma once

#include "vk_utility_object.h"
#include "vk_utility_descriptor_pool_factory.h"

namespace vk_utility
{
    namespace core
    {
        class DescriptorPool;
        using DescriptorPoolPtr = Ptr<DescriptorPool>;

        class DescriptorPool
            : public GenericObject<vk::DescriptorPool>
        {
        private:

            vk::Device _device;

        public:

            static DescriptorPool New(vk::Device device, const DescriptorPoolFactory &descriptor_pool_factory)
            {
                return DescriptorPool(device, descriptor_pool_factory.New(device));
            }

            static DescriptorPoolPtr NewPtr(vk::Device device, const DescriptorPoolFactory &descriptor_pool_factory)
            {
                return make_shared(New(device, descriptor_pool_factory));
            }

            DescriptorPool(void) = default;
            DescriptorPool(const DescriptorPool&) = default;
            DescriptorPool& operator =(const DescriptorPool&) = default;

            DescriptorPool(vk::Device device, vk::DescriptorPool descriptor_pool)
                : _device(device)
                , GenericObject(descriptor_pool)
            {}

            virtual void destroy(void) override
            {
                if (_vk_object && _device)
                {
                    _device.destroyDescriptorPool(_vk_object);
                    _vk_object = vk::DescriptorPool();
                }
            }
        };
    }
}