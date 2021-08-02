#pragma once

#include "vk_utility_object.h"
#include "vk_utility_exception.h"
#include "vk_utility_vulkan_include.h"
#include "vk_utility_device.h"
#include "vk_utility_buffer_information.h"

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

            vk_utility::device::DevicePtr _device;

        public:

            static Framebuffer Create(vk_utility::device::DevicePtr device, const vk::FramebufferCreateInfo& framebuffer_infomration)
            {
                return Framebuffer(device, device->get().create_framebuffer(framebuffer_infomration));
            }

            static FramebufferPtr New(vk_utility::device::DevicePtr device, const vk::FramebufferCreateInfo& framebuffer_infomration)
            {
                return vk_utility::make_shared(Create(device, framebuffer_infomration));
            }

            Framebuffer(void) = default;
            Framebuffer(const Framebuffer&) = default;
            Framebuffer& operator = (const Framebuffer&) = default;

            Framebuffer(vk_utility::device::DevicePtr device, vk::Framebuffer framebuffer)
                : GenericObject(framebuffer)
                , _device(device)
            {}

            virtual void destroy(void) override
            {
                if (_vk_object && _device)
                {
                    _device->get().destroy_framebuffer(_vk_object);
                    _vk_object = vk::Framebuffer();
                }
            }
        };
    }
}