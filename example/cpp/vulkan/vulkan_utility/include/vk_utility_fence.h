#pragma once

#include "vk_utility_object.h"
#include "vk_utility_fence_factory.h"

namespace vk_utility
{
    namespace core
    {
        class Fence;
        using FencePtr = Ptr<Fence>;

        class Fence
            : public GenericObject<vk::Fence>
        {
        private:

            vk::Device _device;

        public:

            static Fence New(vk::Device device, const FenceFactory& fence_factory)
            {
                return Fence(device, fence_factory.New(device));
            }

            static FencePtr NewPtr(vk::Device device, const FenceFactory& fence_factory)
            {
                return make_shared(New(device, fence_factory));
            }

            Fence(void) = default;
            Fence(const Fence&) = default;
            Fence& operator =(const Fence&) = default;

            Fence(vk::Device device, vk::Fence fence)
                : _device(device)
                , GenericObject(fence)
            {}

            virtual void destroy(void) override
            {
                if (_vk_object && _device)
                {
                    _device.destroyFence(_vk_object);
                    _vk_object = vk::Fence();
                }
            }
        };
    }
}