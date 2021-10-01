#pragma once

#include "vk_utility_framebuffer_factory.h"

namespace vk_utility
{
    namespace buffer
    {
        class FramebufferFactoryDefault
            : public FramebufferFactory
        {
        private:

            vk::RenderPass _render_pass;
            uint32_t _width;
            uint32_t _height;
            std::vector<vk::ImageView> _attachments;

        public:

            FramebufferFactoryDefault& set_render_pass(vk::RenderPass render_pass)
            {
                _render_pass = render_pass;
                return *this;
            }

            FramebufferFactoryDefault& set_size(uint32_t width, uint32_t height)
            {
                _width = width;
                _height = height;
                return *this;
            }

            FramebufferFactoryDefault& add_attachment(vk::ImageView attachment)
            {
                _attachments.push_back(attachment);
                return *this;
            }

            virtual vk::Framebuffer New(vk::Device device) const override
            {
                vk::FramebufferCreateInfo framebuffer_information
                (
                    vk::FramebufferCreateFlags{},
                    _render_pass,
                    _attachments,
                    _width,
                    _height,
                    1
                );
                return device.createFramebuffer(framebuffer_information);
            }
        };
    }
}