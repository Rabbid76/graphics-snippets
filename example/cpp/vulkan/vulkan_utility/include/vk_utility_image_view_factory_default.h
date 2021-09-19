#pragma once


#include "vk_utility_image_view_factory.h"
#include "vk_utility_swapchain.h"


namespace vk_utility
{

    namespace image
    {
        /// <summary>
        /// Interface for image view (`vk::ImageView`) factories
        /// </summary>
        class ImageViewFactoryDefault : public ImageViewFactory  
        {
        private:

            vk::Image _image;
            uint32_t _mipmap_levels = 1;
            vk::Format _format = vk::Format::eR8G8B8A8Srgb;
            vk::ImageAspectFlags _aspect_flags = vk::ImageAspectFlagBits::eColor;

        public:

            virtual ImageViewFactoryDefault& set_image(vk::Image image) override
            {
                _image = image;
                return *this;
            }

            ImageViewFactoryDefault& set_mipmap_levels(uint32_t mipmap_levels)
            {
                _mipmap_levels = mipmap_levels;
                return *this;
            }

            ImageViewFactoryDefault& set_format(vk::Format format)
            {
                _format = format;
                return *this;
            }

            ImageViewFactoryDefault& set_aspect_flags(vk::ImageAspectFlags aspect_flags)
            {
                _aspect_flags = aspect_flags;
                return *this;
            }

            virtual vk::ImageView New(vk::Device device) const override
            {
                // The `viewType` and `format` fields specify how the image data should be interpreted.
                // The `viewType` parameter allows you to treat images as 1D textures,+
                // 2D textures, 3D textures and cube maps.

                // The `components` field allows you to swizzle the color channels around.
                // For example, you can map all of the channels to the red channel for a monochrome texture.
                // You can also map constant values of 0 and 1 to a channel.
                // In our case we'll stick to the default mapping.

                // The subresourceRange field describes what the image's purpose is
                // and which part of the image should be accessed.
                // Our images will be used as color targets without any mipmapping levels or multiple layers.

                vk::ImageSubresourceRange subresource_range(_aspect_flags, 0, _mipmap_levels, 0, 1);
                vk::ComponentMapping components
                (
                    vk::ComponentSwizzle::eIdentity,
                    vk::ComponentSwizzle::eIdentity,
                    vk::ComponentSwizzle::eIdentity,
                    vk::ComponentSwizzle::eIdentity
                );

                vk::ImageViewCreateInfo image_view_infoirmation
                (
                    vk::ImageViewCreateFlags{},
                    _image,
                    vk::ImageViewType::e2D,
                    _format,
                    components,
                    subresource_range
                );

                auto image_view = device.createImageView(image_view_infoirmation);
                return image_view;
            };
        };
    }
}