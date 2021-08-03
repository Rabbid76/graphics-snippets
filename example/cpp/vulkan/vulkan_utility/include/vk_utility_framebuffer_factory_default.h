#pragma once

#include <vk_utility_framebuffer_factory.h>
#include "vk_utility_swapchain.h"
#include "vk_utility_image_view_memory.h"
#include "vk_utility_render_pass.h"


namespace vk_utility
{
    namespace buffer
    {
        /// <summary>
        /// Interface for Vulkan framebuffer (`vk::Framebuffer`) factories
        /// </summary>
        class FramebufferFactoryDefault : public IFramebufferFactory
        {
        private:

            vk_utility::swap::SwapchainPtr _swapchain;
            std::vector<vk_utility::image::ImageViewPtr> _swapchain_image_views;
            vk_utility::image::ImageViewMemoryPtr _color_image_view_memory;
            vk_utility::image::ImageViewMemoryPtr _depth_image_view_memory;
            vk_utility::core::RenderPassPtr _render_pass;

        public:

            FramebufferFactoryDefault& set_swapchain(vk_utility::swap::SwapchainPtr swapchain)
            {
                _swapchain = swapchain;
                return *this;
            }

            FramebufferFactoryDefault& set_swapchain_image_views(std::vector<vk_utility::image::ImageViewPtr> swapchain_image_views)
            {
                _swapchain_image_views = swapchain_image_views;
                return *this;
            }

            FramebufferFactoryDefault& set_color_image_view(vk_utility::image::ImageViewMemoryPtr color_image_view_memory)
            {
                _color_image_view_memory = color_image_view_memory;
                return *this;
            }

            FramebufferFactoryDefault& set_depth_image_view(vk_utility::image::ImageViewMemoryPtr depth_image_view_memory)
            {
                _depth_image_view_memory = depth_image_view_memory;
                return *this;
            }

            FramebufferFactoryDefault& set_render_pass(vk_utility::core::RenderPassPtr render_pass)
            {
                _render_pass = render_pass;
                return *this;
            }

            virtual std::vector<FramebufferPtr> New(vk_utility::device::DevicePtr device) override
            {
                std::vector<vk_utility::buffer::FramebufferPtr> framebuffers;
                std::transform(_swapchain_image_views.begin(), _swapchain_image_views.end(), std::back_inserter(framebuffers), [&](auto& swapchain_image_view) -> auto
                {
                    std::vector<vk::ImageView> attachments = {
                        _color_image_view_memory->get().image_view(),
                        _depth_image_view_memory->get().image_view(),
                        swapchain_image_view->handle(),
                    };
                    
                    vk::FramebufferCreateInfo framebuffer_information
                    (
                        vk::FramebufferCreateFlags{},
                        _render_pass->handle(),
                        attachments,
                        _swapchain->get().image_width_2D(),
                        _swapchain->get().image_height_2D(),
                        1
                    );

                    return vk_utility::buffer::Framebuffer::New(device, framebuffer_information);
                });
                return framebuffers;
            }
        };
    }
}