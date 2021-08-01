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
        class Image;
        using ImagePtr = vk_utility::Ptr<Image>;

        /// <summary>
        /// Image (`vk::Image`)
        /// </summary>
        class Image
            : public GenericObject<vk::Image>
        {
        private:

            vk_utility::device::DevicePtr _device;

        public:

            static Image Create(
                vk_utility::device::DevicePtr device, 
                uint32_t width,
                uint32_t height,
                uint32_t mip_levels,
                vk::SampleCountFlagBits num_samples,
                vk::Format format,
                vk::ImageTiling tiling,
                vk::ImageUsageFlags usage)
            {
                // The image type, specified in the imageType field, tells Vulkan with what kind
                // of coordinate system the texels in the image are going to be addressed.
                // It is possible to create 1D, 2D and 3D images.
                // One dimensional images can be used to store an array of data or gradient,
                // two dimensional images are mainly used for textures, and three dimensional images
                // can be used to store voxel volumes, for example.
                // The extent field specifies the dimensions of the image, 
                // basically how many texels there are on each axis. That's why depth must be 1 instead of 0.
                // Our texture will not be an array and we won't be using mipmapping for now.
                // Vulkan supports many possible image formats,
                // but we should use the same format for the texels as the pixels in the buffer,
                // otherwise the copy operation will fail.
                // The tiling field can have one of two values:
                //     VK_IMAGE_TILING_LINEAR: Texels are laid out in row-major order like our pixels array
                //     VK_IMAGE_TILING_OPTIMAL: Texels are laid out in an implementation defined order for optimal access
                // Unlike the layout of an image, the tiling mode cannot be changed at a later time.
                // If you want to be able to directly access texels in the memory of the image, 
                // then you must use VK_IMAGE_TILING_LINEAR.
                // We will be using a staging buffer instead of a staging image, so this won't be necessary.
                // We will be using VK_IMAGE_TILING_OPTIMAL for efficient access from the shader.
                // There are only two possible values for the initialLayout of an image:
                //     VK_IMAGE_LAYOUT_UNDEFINED: Not usable by the GPU and the very first transition will discard the texels.
                //     VK_IMAGE_LAYOUT_PREINITIALIZED: Not usable by the GPU, but the first transition will preserve the texels.
                // There are few situations where it is necessary for the texels to be preserved during the first transition.
                // One example, however, would be if you wanted to use an image as a staging image
                // in combination with the VK_IMAGE_TILING_LINEAR layout.
                // In that case, you'd want to upload the texel data to it and then transition the image
                // to be a transfer source without losing the data.
                // In our case, however, we're first going to transition the image to be a transfer destination
                // and then copy texel data to it from a buffer object, so we don't need this property
                // and can safely use VK_IMAGE_LAYOUT_UNDEFINED.
                // The usage field has the same semantics as the one during buffer creation. 
                // The image is going to be used as destination for the buffer copy,
                // so it should be set up as a transfer destination.
                // We also want to be able to access the image from the shader to color our mesh,
                // so the usage should include VK_IMAGE_USAGE_SAMPLED_BIT.
                // The image will only be used by one queue family: the one that supports graphics
                // (and therefore also) transfer operations.
                // The samples flag is related to multisampling. 
                // This is only relevant for images that will be used as attachments, so stick to one sample.
                // There are some optional flags for images that are related to sparse images.
                // Sparse images are images where only certain regions are actually backed by memory.
                // If you were using a 3D texture for a voxel terrain, for example, 
                // then you could use this to avoid allocating memory to store large volumes of "air" values.
                // We won't be using it in this tutorial, so leave it to its default value of 0.

                // The image is created using vkCreateImage, which doesn't have any particularly noteworthy parameters.
                // It is possible that the VK_FORMAT_R8G8B8A8_UNORM format is not supported by the graphics hardware.
                // You should have a list of acceptable alternatives and go with the best one that is supported.
                // However, support for this particular format is so widespread that we'll skip this step.
                // Using different formats would also require annoying conversions.
                // We will get back to this in the depth buffer chapter, where we'll implement such a system.

                std::vector<uint32_t> queue_family_indices;
                vk::ImageCreateInfo image_information
                (
                    vk::ImageCreateFlags{},
                    vk::ImageType::e2D,
                    format,
                    vk::Extent3D(static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1),
                    mip_levels,
                    1,
                    num_samples,
                    tiling,
                    usage,
                    vk::SharingMode::eExclusive,
                    queue_family_indices,
                    vk::ImageLayout::eUndefined
                );
                return Create(device, image_information);
            }

            static ImagePtr New(
                vk_utility::device::DevicePtr device,
                uint32_t width,
                uint32_t height,
                uint32_t mip_levels,
                vk::SampleCountFlagBits num_samples,
                vk::Format format,
                vk::ImageTiling tiling,
                vk::ImageUsageFlags usage)
            {
                return vk_utility::make_shared(Create(device, width, height, mip_levels, num_samples, format, tiling, usage));
            }

            static Image Create(vk_utility::device::DevicePtr device, vk::ImageCreateInfo image_information)
            {
                auto image = device->get().create_image(image_information);
                return Image(device, image);
            }

            static ImagePtr New(vk_utility::device::DevicePtr device, vk::ImageCreateInfo image_information)
            {
                return vk_utility::make_shared(Create(device, image_information));
            }

            static Image Create(vk_utility::device::DevicePtr device, vk::Image image)
            {
                return Image(device, image);
            }

            static ImagePtr New(vk_utility::device::DevicePtr device, vk::Image image)
            {
                return vk_utility::make_shared(Create(device, image));
            }

            Image(void) = default;
            Image(const Image &) = default;
            Image &operator = (const Image &) = default;

            Image(vk::Image image)
                : GenericObject(image)
                , _device(nullptr)
            {}

            Image(vk_utility::device::DevicePtr device, vk::Image image)
                : GenericObject(image)
                , _device(device)
            {}

            virtual void destroy(void) override
            {
                if (_vk_object && _device)
                {
                    _device->get().destroy_image(_vk_object);
                    _vk_object = vk::Image();
                }
            }
        };
    }
}
