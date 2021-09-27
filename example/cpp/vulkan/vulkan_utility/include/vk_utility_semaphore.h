#pragma once

#include "vk_utility_object.h"
#include "vk_utility_Semaphore_factory.h"

namespace vk_utility
{
    namespace core
    {
        class Semaphore;
        using SemaphorePtr = Ptr<Semaphore>;

        class Semaphore
            : public GenericObject<vk::Semaphore>
        {
        private:

            vk::Device _device;

        public:

            static Semaphore New(vk::Device device, const SemaphoreFactory& semaphore_factory)
            {
                return Semaphore(device, semaphore_factory.New(device));
            }

            static SemaphorePtr NewPtr(vk::Device device, const SemaphoreFactory& semaphore_factory)
            {
                return make_shared(New(device, semaphore_factory));
            }

            Semaphore(void) = default;
            Semaphore(const Semaphore&) = default;
            Semaphore& operator =(const Semaphore&) = default;

            Semaphore(vk::Device device, vk::Semaphore semaphort)
                : _device(device)
                , GenericObject(semaphort)
            {}

            virtual void destroy(void) override
            {
                if (_vk_object && _device)
                {
                    _device.destroySemaphore(_vk_object);
                    _vk_object = vk::Semaphore();
                }
            }
        };
    }
}
