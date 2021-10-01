#pragma once

#include <vk_utility_swapchain_framebuffer_factory.h>
#include "vk_utility_swapchain.h"
#include "vk_utility_image_view_and_image_memory.h"
#include "vk_utility_render_pass.h"
#include "vk_utility_framebuffer_factory_default.h"

namespace vk_utility
{
    namespace buffer
    {
        /// <summary>
        /// Interface for Vulkan framebuffer (`vk::Framebuffer`) factories
        /// </summary>
        class SwapchainFramebufferFactoryDefault : public SwapchainFramebufferFactory
        {
        private:

            vk_utility::swap::SwapchainPtr _swapchain;
            std::vector<vk_utility::image::ImageViewPtr> _swapchain_image_views;
            vk_utility::image::ImageViewAndImageMemoryPtr _color_image_view_memory;
            vk_utility::image::ImageViewAndImageMemoryPtr _depth_image_view_memory;
            vk_utility::core::RenderPassPtr _render_pass;

        public:

            SwapchainFramebufferFactoryDefault& set_swapchain(vk_utility::swap::SwapchainPtr swapchain)
            {
                _swapchain = swapchain;
                return *this;
            }

            SwapchainFramebufferFactoryDefault& set_swapchain_image_views(std::vector<vk_utility::image::ImageViewPtr> swapchain_image_views)
            {
                _swapchain_image_views = swapchain_image_views;
                return *this;
            }

            SwapchainFramebufferFactoryDefault& set_color_image_view(vk_utility::image::ImageViewAndImageMemoryPtr color_image_view_memory)
            {
                _color_image_view_memory = color_image_view_memory;
                return *this;
            }

            SwapchainFramebufferFactoryDefault& set_depth_image_view(vk_utility::image::ImageViewAndImageMemoryPtr depth_image_view_memory)
            {
                _depth_image_view_memory = depth_image_view_memory;
                return *this;
            }

            SwapchainFramebufferFactoryDefault& set_render_pass(vk_utility::core::RenderPassPtr render_pass)
            {
                _render_pass = render_pass;
                return *this;
            }

            virtual std::vector<vk::Framebuffer> New(vk::Device device) const override
            {
                std::vector<vk::Framebuffer> framebuffers;
                std::transform(_swapchain_image_views.begin(), _swapchain_image_views.end(), std::back_inserter(framebuffers), [&](auto& swapchain_image_view) -> auto
                {
                    return FramebufferFactoryDefault()
                        .set_render_pass(*_render_pass)
                        .set_size(_swapchain->get().image_width_2D(), _swapchain->get().image_height_2D())
                        .add_attachment(_color_image_view_memory->get().image_view())
                        .add_attachment(_depth_image_view_memory->get().image_view())
                        .add_attachment(*swapchain_image_view)
                        .New(device);
                });
                return framebuffers;
            }
        };
    }
}