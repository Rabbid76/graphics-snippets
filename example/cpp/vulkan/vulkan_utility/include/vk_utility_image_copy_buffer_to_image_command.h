#pragma once

#include "vk_utility_core_command.h"
#include "vk_utility_command_buffer_factory.h"

namespace vk_utility
{
    namespace image
    {
        /// <summary>
        /// The bufferOffset specifies the byte offset in the buffer at which the pixel values start.
        /// The bufferRowLength and bufferImageHeight fields specify how the pixels are laid out in memory.
        /// For example, you could have some padding bytes between rows of the image.
        /// Specifying 0 for both indicates that the pixels are simply tightly packed like they are in our case. 
        /// The imageSubresource, imageOffset and imageExtent fields indicate to which part of the image we want to copy the pixels.   7
        /// 
        /// The fourth parameter indicates which layout the image is currently using.
        /// I'm assuming here that the image has already been transitioned to the layout that is optimal for copying pixels to.
        /// Right now we're only copying one chunk of pixels to the whole image,
        /// but it's possible to specify an array of `vk::BufferImageCopy` to perform many different copies from this buffer to the image in one operation.
        /// </summary>
        class ImageCopyBufferToImageCommand
            : public core::CoreCommand
        {
        private:

            const command::CommandBufferFactory *_command_buffer_factory;
            vk::Buffer _buffer;
            vk::Image _image;
            uint32_t _width;
            uint32_t _height;

        public:

            ImageCopyBufferToImageCommand& set_command_buffer_factory(const command::CommandBufferFactory* command_buffer_factory)
            {
                _command_buffer_factory = command_buffer_factory;
                return *this;
            }

            ImageCopyBufferToImageCommand& set_buffer(vk::Buffer buffer)
            {
                _buffer = buffer;
                return *this;
            }

            ImageCopyBufferToImageCommand& set_image(vk::Image image)
            {
                _image = image;
                return *this;
            }

            ImageCopyBufferToImageCommand& set_size(uint32_t width, uint32_t height)
            {
                _width = width;
                _height = height;
                return *this;
            }

            virtual void execute_command(vk::Device device, vk::CommandPool command_pool) const override
            {
                auto command_buffer = _command_buffer_factory->Begin(device, command_pool);

                vk::ImageSubresourceLayers image_subresource(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
                vk::Offset3D image_offset(0, 0, 0);
                vk::Extent3D image_extent(_width, _height, 1);
                vk::BufferImageCopy region(0, 0, 0, image_subresource, image_offset, image_extent);
                command_buffer.copyBufferToImage(
                    _buffer,
                    _image,
                    vk::ImageLayout::eTransferDstOptimal,
                    1,
                    &region
                );

                _command_buffer_factory->End(command_buffer);
                device.freeCommandBuffers(command_pool, command_buffer);
            }
        };
    }
}