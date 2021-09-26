#pragma once

#include "vk_utility_vulkan_include.h"
#include "vk_utility_object.h"
#include "vk_utility_sampler_factory.h"

namespace vk_utility
{
    namespace image
    {
        class Sampler;
        using SamplerPtr = Ptr<Sampler>;

        class Sampler
            : public GenericObject<vk::Sampler>
        {
        private:

            vk::Device _device;

        public:

            static Sampler New(vk::Device device, const SamplerFactory &sampler_factory)
            {
                return Sampler(device, sampler_factory.New(device));
            }

            static SamplerPtr NewPtr(vk::Device device, const SamplerFactory& sampler_factory)
            {
                return make_shared(New(device, sampler_factory));
            }

            Sampler(void) = default;
            Sampler(const Sampler&) = default;
            Sampler& operator = (const Sampler&) = default;

            Sampler(vk::Device device, vk::Sampler sampler)
                : _device(device)
                , GenericObject(sampler)
            {}

            virtual void destroy(void) override
            {
                if (_vk_object && _device)
                {
                    _device.destroySampler(_vk_object);
                    _vk_object = vk::Sampler();
                }
            }
        };
    }
}