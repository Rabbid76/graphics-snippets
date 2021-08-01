#pragma once


#include "vk_utility_image_view_factory.h"


namespace vk_utility
{

    namespace image
    {
        /// <summary>
        /// Interface for image view (`vk::ImageView`) factories
        /// </summary>
        class ImageViewFactoryDefault : IImageViewFactory  
        {
        private:

            vk_utility::swap::SwapchainPtr _swapchain;

        public:

            ImageViewFactoryDefault& set_swapchain(vk_utility::swap::SwapchainPtr swapchain)
            {
                _swapchain = swapchain;
                return *this;
            }

            virtual std::vector<vk_utility::image::ImageViewPtr> New(vk_utility::device::DevicePtr device) override
            {
                // To use any `vk::Image`, including those in the swap chain, in the render pipeline we have to create a `vk::ImageView` object.
                // An image view is quite literally a view into an image.
                // It describes how to access the image and which part of the image to access, for example if it should be treated as a 2D texture depth texture without any mipmapping levels.

                // If you were working on a stereographic 3D application, then you would create a swap chain with multiple layers.
                // You could then create multiple image views for each image representing the views for the left and right eyes by accessing different layers.

                auto& swapchain_images = _swapchain->get().get_swapchain_images();
                std::vector<vk_utility::image::ImageViewPtr> swapchain_image_views;
                std::transform(swapchain_images.begin(), swapchain_images.end(), std::back_inserter(swapchain_image_views), [&](auto& swapchain_image) -> auto
                {
                    return vk_utility::image::ImageView::New(device, swapchain_image, _swapchain->get().image_format(), vk::ImageAspectFlagBits::eColor, 1);
                });
                return swapchain_image_views;
            };
        };
    }
}