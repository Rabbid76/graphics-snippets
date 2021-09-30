#pragma once


#include "vk_utility_object.h"
#include "vk_utility_exception.h"
#include "vk_utility_vulkan_include.h"
#include "vk_utility_image_view_factory.h"
#include "vk_utility_swapchain_image_views_factory.h"
#include <vector>
#include <algorithm>


namespace vk_utility
{
    namespace image
    {
        class ImageView;
        using ImageViewPtr = vk_utility::Ptr<ImageView>;

        /// <summary>
        /// ImageView (`vk::ImageView`)
        /// </summary>
        class ImageView
            : public GenericObject<vk::ImageView>
        {
        private:

            vk::Device _device;

        public:

            static std::vector<ImageViewPtr> NewPtrVector(vk::Device device, const SwapchainImageViewsFactory& image_view_factory)
            {
                auto swapchain_image_views = image_view_factory.New(device);
                std::vector<ImageViewPtr> swapchain_image_view_ptrs;
                std::transform(swapchain_image_views.begin(), swapchain_image_views.end(),
                    std::back_inserter(swapchain_image_view_ptrs), [&device](auto image_view) -> auto
                {
                    return vk_utility::make_shared(ImageView(device, image_view));
                });
                return swapchain_image_view_ptrs;
            }

            static ImageView New(vk::Device device, ImageViewFactory & image_view_factory)
            {
                return ImageView(device, image_view_factory.New(device));
            }

            static ImageViewPtr NewPtr(vk::Device device, ImageViewFactory& image_view_factory)
            {
                return vk_utility::make_shared(New(device, image_view_factory));
            }

            ImageView(void) = default;
            ImageView(const ImageView&) = default;
            ImageView& operator = (const ImageView&) = default;

            ImageView(vk::Device device, vk::ImageView image)
                : GenericObject(image)
                , _device(device)
            {}

            virtual void destroy(void) override
            {
                if (_vk_object && _device)
                {
                    _device.destroyImageView(_vk_object);
                    _vk_object = vk::ImageView();
                }
            }
        };
    }
}

