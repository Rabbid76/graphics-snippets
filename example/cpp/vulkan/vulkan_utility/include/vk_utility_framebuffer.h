#pragma once

#include "vk_utility_object.h"
#include "vk_utility_framebuffer_factory.h"
#include "vk_utility_swapchain_framebuffer_factory.h"
#include <memory>

namespace vk_utility
{
    namespace buffer
    {
        class Framebuffer;
        using FramebufferPtr = vk_utility::Ptr<Framebuffer>;

        /// <summary>
        /// FrameBuffer
        /// </summary>
        class Framebuffer
            : public GenericObject<vk::Framebuffer>
        {
        private:

            vk::Device _device;

        public:

            static std::vector<FramebufferPtr> NewPtrVector(vk::Device device, const SwapchainFramebufferFactory& framebuffer_factory)
            {
                auto framebuffers = framebuffer_factory.New(device);
                std::vector<FramebufferPtr> framebuffer_ptrs;
                std::transform(framebuffers.begin(), framebuffers.end(),
                    std::back_inserter(framebuffer_ptrs), [&device](auto framebuffer) -> auto
                {
                    return vk_utility::make_shared(Framebuffer(device, framebuffer));
                });
                return framebuffer_ptrs;
            }

            static Framebuffer New(vk::Device device, const FramebufferFactory& framebuffer_factory)
            {
                return Framebuffer(device, framebuffer_factory.New(device));
            }

            static FramebufferPtr NewPtr(vk::Device device, const FramebufferFactory& framebuffer_factory)
            {
                return make_shared(New(device, framebuffer_factory));
            }

            Framebuffer(void) = default;
            Framebuffer(const Framebuffer&) = default;
            Framebuffer& operator = (const Framebuffer&) = default;

            Framebuffer(vk::Device device, vk::Framebuffer framebuffer)
                : GenericObject(framebuffer)
                , _device(device)
            {}

            virtual void destroy(void) override
            {
                if (_vk_object && _device)
                {
                    _device.destroyFramebuffer(_vk_object);
                    _vk_object = vk::Framebuffer();
                }
            }
        };
    }
}