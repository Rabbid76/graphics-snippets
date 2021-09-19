#pragma once


#include "vk_utility_object.h"
#include "vk_utility_exception.h"
#include "vk_utility_vulkan_include.h"
#include "vk_utility_image_view_factory.h"

#include <memory>


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

