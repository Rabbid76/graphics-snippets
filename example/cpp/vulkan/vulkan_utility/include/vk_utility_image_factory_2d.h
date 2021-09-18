#pragma once

#include "vk_utility_image_factory.h"

namespace vk_utility
{
    namespace image
    {
        /// <summary>
        ///The image type, specified in the imageType field, tells Vulkan with what kind
        /// of coordinate system the texels in the image are going to be addressed.
        /// It is possible to create 1D, 2D and 3D images.
        /// One dimensional images can be used to store an array of data or gradient,
        /// two dimensional images are mainly used for textures, and three dimensional images
        /// can be used to store voxel volumes, for example.
        /// The extent field specifies the dimensions of the image, 
        /// basically how many texels there are on each axis. That's why depth must be 1 instead of 0.
        /// Our texture will not be an array and we won't be using mipmapping for now.
        /// Vulkan supports many possible image formats,
        /// but we should use the same format for the texels as the pixels in the buffer,
        /// otherwise the copy operation will fail.
        /// The tiling field can have one of two values:
        ///     VK_IMAGE_TILING_LINEAR: Texels are laid out in row-major order like our pixels array
        ///     VK_IMAGE_TILING_OPTIMAL: Texels are laid out in an implementation defined order for optimal access
        /// Unlike the layout of an image, the tiling mode cannot be changed at a later time.
        /// If you want to be able to directly access texels in the memory of the image, 
        /// then you must use VK_IMAGE_TILING_LINEAR.
        /// We will be using a staging buffer instead of a staging image, so this won't be necessary.
        /// We will be using VK_IMAGE_TILING_OPTIMAL for efficient access from the shader.
        /// There are only two possible values for the initialLayout of an image:
        ///     VK_IMAGE_LAYOUT_UNDEFINED: Not usable by the GPU and the very first transition will discard the texels.
        ///     VK_IMAGE_LAYOUT_PREINITIALIZED: Not usable by the GPU, but the first transition will preserve the texels.
        /// There are few situations where it is necessary for the texels to be preserved during the first transition.
        /// One example, however, would be if you wanted to use an image as a staging image
        /// in combination with the VK_IMAGE_TILING_LINEAR layout.
        /// In that case, you'd want to upload the texel data to it and then transition the image
        /// to be a transfer source without losing the data.
        /// In our case, however, we're first going to transition the image to be a transfer destination
        /// and then copy texel data to it from a buffer object, so we don't need this property
        /// and can safely use VK_IMAGE_LAYOUT_UNDEFINED.
        /// The usage field has the same semantics as the one during buffer creation. 
        /// The image is going to be used as destination for the buffer copy,
        /// so it should be set up as a transfer destination.
        /// We also want to be able to access the image from the shader to color our mesh,
        /// so the usage should include VK_IMAGE_USAGE_SAMPLED_BIT.
        /// The image will only be used by one queue family: the one that supports graphics
        /// (and therefore also) transfer operations.
        /// The samples flag is related to multisampling. 
        /// This is only relevant for images that will be used as attachments, so stick to one sample.
        /// There are some optional flags for images that are related to sparse images.
        /// Sparse images are images where only certain regions are actually backed by memory.
        /// If you were using a 3D texture for a voxel terrain, for example, 
        /// then you could use this to avoid allocating memory to store large volumes of "air" values.
        /// We won't be using it in this tutorial, so leave it to its default value of 0.
        ///
        /// The image is created using vkCreateImage, which doesn't have any particularly noteworthy parameters.
        /// It is possible that the VK_FORMAT_R8G8B8A8_UNORM format is not supported by the graphics hardware.
        /// You should have a list of acceptable alternatives and go with the best one that is supported.
        /// However, support for this particular format is so widespread that we'll skip this step.
        /// Using different formats would also require annoying conversions.
        /// We will get back to this in the depth buffer chapter, where we'll implement such a system.
        /// </summary>
        class ImageFactory2D
            : public ImageFactory
        {
        private:

            uint32_t _width;
            uint32_t _height;
            uint32_t _mipmap_levels = 1;
            vk::Format _format = vk::Format::eR8G8B8A8Srgb;
            vk::SampleCountFlagBits _sample_count = vk::SampleCountFlagBits::e1;
            vk::ImageTiling _tiling = vk::ImageTiling::eOptimal;
            vk::ImageUsageFlags _usage_flags;

        public:

            ImageFactory2D& set_size(uint32_t width, uint32_t height)
            {
                _width = width;
                _height = height;
                return *this;
            }

            ImageFactory2D& set_mipmap_levels(uint32_t mipmap_levels)
            {
                _mipmap_levels = mipmap_levels;
                return *this;
            }

            ImageFactory2D& set_format(vk::Format format)
            {
                _format = format;
                return *this;
            }

            ImageFactory2D& set_samples(vk::SampleCountFlagBits smaple_count)
            {
                _sample_count = smaple_count;
                return *this;
            }

            ImageFactory2D& set_tiling(vk::ImageTiling tiling)
            {
                _tiling = tiling;
                return *this;
            }

            ImageFactory2D& set_usage(vk::ImageUsageFlags usage_flags)
            {
                _usage_flags = usage_flags;
                return *this;
            }

            virtual vk::Image New(vk::Device device) const override
            {
                const std::vector<uint32_t> queue_family_indices;
                const vk::ImageCreateInfo image_create_information
                (
                    vk::ImageCreateFlags(),
                    vk::ImageType::e2D,
                    _format,
                    vk::Extent3D(_width, _height, 1),
                    _mipmap_levels,
                    1,
                    _sample_count,
                    _tiling,
                    _usage_flags,
                    vk::SharingMode::eExclusive,
                    queue_family_indices,
                    vk::ImageLayout::eUndefined
                );
                return device.createImage(image_create_information);
            }
        };
    }
}