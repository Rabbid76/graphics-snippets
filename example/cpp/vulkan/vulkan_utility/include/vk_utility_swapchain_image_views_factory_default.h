#pragma once


#include "vk_utility_swapchain_image_views_factory.h"
#include "vk_utility_swapchain.h"
#include "vk_utility_image_view_factory_default.h"


namespace vk_utility
{

    namespace image
    {
        /// <summary>
        /// Interface for image view (`vk::ImageView`) factories
        /// </summary>
        class SwapchainImageViewFactoryDefault : public SwapchainImageViewsFactory  
        {
        private:

            vk_utility::swap::SwapchainPtr _swapchain;

        public:

            SwapchainImageViewFactoryDefault& set_swapchain(vk_utility::swap::SwapchainPtr swapchain)
            {
                _swapchain = swapchain;
                return *this;
            }

            virtual std::vector<vk_utility::image::ImageViewPtr> New(vk::Device device) override
            {
                // To use any `vk::Image`, including those in the swap chain, in the render pipeline we have to create a `vk::ImageView` object.
                // An image view is quite literally a view into an image.
                // It describes how to access the image and which part of the image to access, for example if it should be treated as a 2D texture depth texture without any mipmapping levels.

                // If you were working on a stereographic 3D application, then you would create a swap chain with multiple layers.
                // You could then create multiple image views for each image representing the views for the left and right eyes by accessing different layers.

                auto _image_view_factory = ImageViewFactoryDefault()
                    .set_format(_swapchain->get().image_format())
                    .set_aspect_flags(vk::ImageAspectFlagBits::eColor)
                    .set_mipmap_levels(1);

                auto& swapchain_images = _swapchain->get().get_swapchain_images();
                std::vector<vk_utility::image::ImageViewPtr> image_views;
                std::transform(swapchain_images.begin(), swapchain_images.end(), std::back_inserter(image_views), [&](auto& swapchain_image) -> auto
                {
                    return vk_utility::image::ImageView::NewPtr(
                        device,
                        _image_view_factory
                            .set_image(*swapchain_image));
                });
                return image_views;
            };
        };
    }
}