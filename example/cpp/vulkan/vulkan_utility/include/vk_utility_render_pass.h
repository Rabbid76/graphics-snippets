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
        class RenderPass;
        using RenderPassPtr = vk_utility::Ptr<RenderPass>;

        /// <summary>
        /// Interface for Vulkan redner pass (`vk::RenderPass`)
        /// </summary>
        class RenderPass
            : public GenericObject<vk::RenderPass>
        {
        private:

            vk_utility::device::DevicePtr _device;

        public:

            static RenderPass Create(vk_utility::device::DevicePtr device, vk::RenderPass render_pass)
            {
                return RenderPass(device, render_pass);
            }

            static RenderPassPtr New(vk_utility::device::DevicePtr device, vk::RenderPass render_pass)
            {
                return vk_utility::make_shared(Create(device, render_pass));
            }

            static RenderPass Create(vk_utility::device::DevicePtr device, vk::RenderPassCreateInfo render_pass_infomration)
            {
                auto render_pass = device->get().create_pender_pass(render_pass_infomration);
                return RenderPass(device, render_pass);
            }

            static RenderPassPtr New(vk_utility::device::DevicePtr device, vk::RenderPassCreateInfo render_pass_infomration)
            {
                return vk_utility::make_shared(Create(device, render_pass_infomration));
            }

            RenderPass(void) = default;
            RenderPass(const RenderPass &) = default;
            RenderPass &operator = (const RenderPass &) = default;

            RenderPass(vk_utility::device::DevicePtr device, vk::RenderPass render_pass)
                : GenericObject(render_pass)
                , _device(device)
            {}

            virtual void destroy(void) override
            {
                if (_vk_object && _device)
                {
                    _device->get().destroy_render_pass(_vk_object);
                    _vk_object = vk::RenderPass();
                }
            }
        };
    }
}
