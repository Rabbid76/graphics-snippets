#pragma once


#include "vk_utility_object.h"
#include "vk_utility_exception.h"
#include "vk_utility_vulkan_include.h"
#include "vk_utility_device.h"

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

            vk_utility::device::DevicePtr _device;

        public:

            static ImageView Create(vk_utility::device::DevicePtr device, vk::Image image, vk::Format format, vk::ImageAspectFlags aspect_flags, uint32_t mip_levels)
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

                vk::ImageSubresourceRange subresource_range(aspect_flags, 0, mip_levels, 0, 1);
                vk::ComponentMapping components
                (
                    vk::ComponentSwizzle::eIdentity,
                    vk::ComponentSwizzle::eIdentity,
                    vk::ComponentSwizzle::eIdentity,
                    vk::ComponentSwizzle::eIdentity
                );

                vk::ImageViewCreateInfo view_information
                (
                    vk::ImageViewCreateFlags{},
                    image,
                    vk::ImageViewType::e2D,
                    format,
                    components,
                    subresource_range
                );

                return Create(device, view_information);
            }

            static ImageViewPtr New(vk_utility::device::DevicePtr device, vk::Image image, vk::Format format, vk::ImageAspectFlags aspect_flags, uint32_t mip_levels)
            {
                return vk_utility::make_shared(Create(device, image, format, aspect_flags, mip_levels));
            }

            static ImageView Create(vk_utility::device::DevicePtr device, const vk::ImageViewCreateInfo &image_view_infoirmation)
            {
                auto iamge_view = device->get().create_image_view(image_view_infoirmation);
                return ImageView(device, iamge_view);
            }

            static ImageViewPtr New(vk_utility::device::DevicePtr device, const vk::ImageViewCreateInfo &image_view_infoirmation)
            {
                return vk_utility::make_shared(Create(device, image_view_infoirmation));
            }

            static ImageView Create(vk_utility::device::DevicePtr device, vk::ImageView image)
            {
                return ImageView(device, image);
            }

            static ImageViewPtr New(vk_utility::device::DevicePtr device, vk::ImageView image_view)
            {
                return vk_utility::make_shared(Create(device, image_view));
            }

            ImageView(void) = default;
            ImageView(const ImageView&) = default;
            ImageView& operator = (const ImageView&) = default;

            ImageView(vk_utility::device::DevicePtr device, vk::ImageView image)
                : GenericObject(image)
                , _device(device)
            {}

            virtual void destroy(void) override
            {
                if (_vk_object && _device)
                {
                    _device->get().destroy_image_view(_vk_object);
                    _vk_object = vk::ImageView();
                }
            }
        };
    }
}

