#pragma once

#include "vk_utility_texture_factory.h"
#include "vk_utility_buffer_factory_default.h"
#include "vk_utility_buffer_device_memory_factory.h"
#include "vk_utility_buffer_and_memory_factory_default.h"
#include "vk_utility_buffer_copy_data_to_memory_command.h"
#include "vk_utility_image_factory_2d.h"
#include "vk_utility_image_device_memory_factory.h"
#include "vk_utility_image_and_memory_factory_default.h"
#include "vk_utility_command_buffer_factory_single_time_command.h"
#include "vk_utility_image_transition_command.h"
#include "vk_utility_image_copy_buffer_to_image_command.h"
#include "vk_utility_image_generate_mipmaps_command.h"

namespace vk_utility
{
    namespace image
    {
        /// <summary>
        /// We're now going to create a buffer in host visible memory so that we can use vkMapMemory and copy the pixels to it.
        /// Add variables for this temporary buffer to the createTextureImage function:
        /// The buffer should be in host visible memory so that we can map it
        /// and it should be usable as a transfer source so that we can copy it to an image later on:
        /// We can then directly copy the pixel values that we got from the image loading library to the buffer:
        ///
        /// Copy the staging buffer to the texture image. This involves two steps:
        /// Transition the texture image to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        /// Execute the buffer to image copy operation
        /// The image was created with the VK_IMAGE_LAYOUT_UNDEFINED layout,
        /// so that one should be specified as old layout when transitioning textureImage.
        /// Remember that we can do this because we don't care about its contents before performing the copy operation.
        /// 
        /// To be able to start sampling from the texture image in the shader,
        /// we need one last transition to prepare it for shader access:
        /// transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
        /// </summary>
        class TextureFactoryDefault
            : public TextureFactory
        {
        private:

            const command::CommandBufferFactory *_command_buffer_factory;
            uint32_t _width;
            uint32_t _height;
            const void* _source_data;
            device::PhysicalDevicePtr _physical_device;
            vk::Queue _graphics_queue;

        public:

            TextureFactoryDefault& set_command_buffer_factory(const command::CommandBufferFactory * command_buffer_factory)
            {
                _command_buffer_factory = command_buffer_factory;
                return *this;
            }

            TextureFactoryDefault& set_source_data(uint32_t width, uint32_t height, const void* source_data)
            {
                _width = width;
                _height = height;
                _source_data = source_data;
                return *this;
            }

            TextureFactoryDefault& set_physical_device(device::PhysicalDevicePtr physical_device)
            {
                _physical_device = physical_device;
                return *this;
            }

            TextureFactoryDefault set_graphics_queue(vk::Queue graphics_queue)
            {
                _graphics_queue = graphics_queue;
                return *this;
            }

            virtual std::tuple<vk::Sampler, vk::ImageView, vk::Image, vk::DeviceMemory, vk::DeviceSize> New(
                vk::Device device, vk::CommandPool command_pool) const override
            {
                vk::DeviceSize image_size = (size_t)(_width * _height * 4);

                auto [staging_buffer, buffer_size, buffer_memory, buffer_memory_size] = vk_utility::buffer::BufferAndMemoryFactoryDefault()
                    .set_buffer_factory(
                        &buffer::BufferFactoryDefault()
                        .set_buffer_size(image_size)
                        .set_staging_buffer_usage())
                    .set_buffer_memory_factory(
                        &buffer::BufferDeviceMemoryFactory()
                        .set_staging_memory_properties()
                        .set_from_physical_device(*_physical_device))
                    .New(device);

                buffer::CopyDataToBufferMemoryCommand()
                    .set_source_data(buffer_size, _source_data)
                    .set_destination_offset(0)
                    .set_destination_memory(buffer_memory)
                    .execute_command(device, command_pool);

                uint32_t mipmap_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(_width, _height)))) + 1;

                vk::ImageUsageFlags usage_flags = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
                if (mipmap_levels > 1)
                    usage_flags |= vk::ImageUsageFlagBits::eTransferSrc;

                auto [image, image_memory, image_memory_size] = ImageAndMemoryFactoryDefault()
                    .set_image_factory(&ImageFactory2D()
                        .set_size(_width, _height)
                        .set_format(vk::Format::eR8G8B8A8Srgb)
                        .set_mipmap_levels(mipmap_levels)
                        .set_samples(vk::SampleCountFlagBits::e1)
                        .set_usage(usage_flags))
                    .set_device_memory_factory(&ImageDeviceMemoryFactory()
                        .set_memory_properties(vk::MemoryPropertyFlagBits::eDeviceLocal)
                        .set_from_physical_device(*_physical_device))
                    .New(device);

                ImageTransitionCommand()
                    .set_command_buffer_factory(_command_buffer_factory)
                    .set_image(image)
                    .set_mipmap_levels(mipmap_levels)
                    .set_old_layout(vk::ImageLayout::eUndefined)
                    .set_new_layout(vk::ImageLayout::eTransferDstOptimal)
                    .execute_command(device, command_pool);

                ImageCopyBufferToImageCommand()
                    .set_command_buffer_factory(_command_buffer_factory)
                    .set_buffer(staging_buffer)
                    .set_image(image)
                    .set_size(_width, _height)
                    .execute_command(device, command_pool);

                if (mipmap_levels == 1)
                    ImageTransitionCommand()
                        .set_command_buffer_factory(_command_buffer_factory)
                        .set_image(image)
                        .set_mipmap_levels(1)
                        .set_old_layout(vk::ImageLayout::eTransferDstOptimal)
                        .set_new_layout(vk::ImageLayout::eShaderReadOnlyOptimal)
                        .execute_command(device, command_pool);

                device.destroyBuffer(staging_buffer);
                device.freeMemory(buffer_memory);

                if (mipmap_levels > 1)
                    GeneratrMipmapsCommand()
                        .set_command_buffer_factory(_command_buffer_factory)
                        .set_image(image)
                        .set_size(_width, _height)
                        .set_mipmap_levels(mipmap_levels)
                        .set_format(vk::Format::eR8G8B8A8Srgb)
                        .execute_command(device, command_pool);

                auto image_view = ImageViewFactoryDefault()
                    .set_image(image)
                    .set_format(vk::Format::eR8G8B8A8Srgb)
                    .set_aspect_flags(vk::ImageAspectFlagBits::eColor)
                    .set_mipmap_levels(mipmap_levels)
                    .New(device);

                auto sampler = vk_utility::image::SamplerFactoryDefault()
                    .set_mipmap_levels(mipmap_levels)
                    .New(device);

                return { sampler, image_view, image, image_memory, image_size };
            }
        };
    }
}