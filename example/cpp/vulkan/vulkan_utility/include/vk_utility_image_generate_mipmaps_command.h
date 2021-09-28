#pragma once

#include "vk_utility_command_buffer_factory.h"

namespace vk_utility
{
    namespace image
    {
        /// <summary>
        /// We're going to make several transitions, so we'll reuse this `vk::ImageMemoryBarrier`. 
        /// The fields set above will remain the same for all barriers. 
        /// subresourceRange.miplevel, oldLayout, newLayout, srcAccessMask, and dstAccessMask will be changed for each transition.
        ///        
        /// First, we transition level i - 1 to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL.
        /// This transition will wait for level i - 1 to be filled, either from the previous blit command, or from vkCmdCopyBufferToImage.
        /// The current blit command will wait on this transition.
        /// 
        /// Next, we specify the regions that will be used in the blit operation. 
        /// The source mipmap level is i - 1 and the destination mipmap level is i. 
        /// The two elements of the srcOffsets array determine the 3D region that data will be blitted from.
        /// dstOffsets determines the region that data will be blitted to. 
        /// The X and Y dimensions of the dstOffsets[1] are divided by two since each mipmap level is half the size of the previous level.
        /// The Z dimension of srcOffsets[1] and dstOffsets[1] must be 1, since a 2D image has a depth of 1.
        ///
        /// Now, we record the blit command. Note that textureImage is used for both the srcImage and dstImage parameter. 
        /// This is because we're blitting between different levels of the same image.
        /// The source mipmap level was just transitioned to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL and the destination level
        ///  is still in VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL from createTextureImage.
        ///
        /// The last parameter allows us to specify a `vk::Filter` to use in the blit. 
        /// We have the same filtering options here that we had when making the `vk::Sampler`. 
        /// We use the VK_FILTER_LINEAR to enable interpolation.
        /// 
        /// At the end of the loop, we divide the current mipmap dimensions by two. 
        /// We check each dimension before the division to ensure that dimension never becomes 0. 
        /// This handles cases where the image is not square, since one of the mipmap dimensions would reach 1 before the other dimension. 
        /// When this happens, that dimension should remain 1 for all remaining levels.
        /// 
        /// Before we end the command buffer, we insert one more pipeline barrier. 
        /// This barrier transitions the last mipmap level from VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL. 
        /// This wasn't handled by the loop, since the last mipmap level is never blitted from.
        /// </summary>
        class GeneratrMipmapsCommand
        {
        private:

            const command::CommandBufferFactory* _command_buffer_factory;
            vk::Image _image;
            uint32_t _width;
            uint32_t _height;
            uint32_t _mipmap_levels;
            vk::Format _format = vk::Format::eR8G8B8A8Srgb;

        public:

            GeneratrMipmapsCommand& set_command_buffer_factory(const command::CommandBufferFactory* command_buffer_factory)
            {
                _command_buffer_factory = command_buffer_factory;
                return *this;
            }

            GeneratrMipmapsCommand& set_image(vk::Image image)
            {
                _image = image;
                return *this;
            }

            GeneratrMipmapsCommand& set_size(uint32_t width, uint32_t height)
            {
                _width = width;
                _height = height;
                return *this;
            }

            GeneratrMipmapsCommand& set_mipmap_levels(uint32_t mipmap_levels)
            {
                _mipmap_levels = mipmap_levels;
                return *this;
            }

            GeneratrMipmapsCommand& set_format(vk::Format format)
            {
                _format = format;
                return *this;
            }

            GeneratrMipmapsCommand& execute_command(vk::Device device, vk::CommandPool command_pool)
            {
                auto command_buffer = _command_buffer_factory->Begin(device, command_pool);

                vk::ImageSubresourceRange subresource_range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
                vk::ImageMemoryBarrier barrier
                (
                    vk::AccessFlags{},
                    vk::AccessFlags{},
                    vk::ImageLayout::eUndefined,
                    vk::ImageLayout::eUndefined,
                    VK_QUEUE_FAMILY_IGNORED,
                    VK_QUEUE_FAMILY_IGNORED,
                    _image,
                    subresource_range
                );

                int32_t mipmap_width = _width;
                int32_t mipmap_height = _height;
                for (uint32_t i = 1; i < _mipmap_levels; i++) {

                    barrier.subresourceRange.setBaseMipLevel(i - 1);
                    barrier
                        .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                        .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
                        .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                        .setDstAccessMask(vk::AccessFlagBits::eTransferRead);

                    command_buffer.pipelineBarrier(
                        vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(),
                        0, nullptr,
                        0, nullptr,
                        1, &barrier);

                    vk::ImageSubresourceLayers src_subresource(vk::ImageAspectFlagBits::eColor, i - 1, 0, 1);
                    std::array<vk::Offset3D, 2> src_offsets{ vk::Offset3D(0, 0, 0), vk::Offset3D(mipmap_width, mipmap_height, 1) };
                    vk::ImageSubresourceLayers dst_subresource(vk::ImageAspectFlagBits::eColor, i, 0, 1);
                    std::array<vk::Offset3D, 2> dst_offsets{ vk::Offset3D(0, 0, 0), vk::Offset3D(mipmap_width > 1 ? mipmap_width / 2 : 1, mipmap_height > 1 ? mipmap_height / 2 : 1, 1) };
                    vk::ImageBlit blit(src_subresource, src_offsets, dst_subresource, dst_offsets);

                    command_buffer.blitImage(
                        _image, vk::ImageLayout::eTransferSrcOptimal,
                        _image, vk::ImageLayout::eTransferDstOptimal,
                        1, &blit,
                        vk::Filter::eLinear);

                    barrier
                        .setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
                        .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                        .setSrcAccessMask(vk::AccessFlagBits::eTransferRead)
                        .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

                    command_buffer.pipelineBarrier(
                        vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, vk::DependencyFlags(),
                        0, nullptr,
                        0, nullptr,
                        1, &barrier);

                    if (mipmap_width > 1) 
                        mipmap_width /= 2;
                    if (mipmap_height > 1) 
                        mipmap_height /= 2;
                }

                barrier.subresourceRange.setBaseMipLevel(_mipmap_levels - 1);
                barrier
                    .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                    .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                    .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

                command_buffer.pipelineBarrier(
                    vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, vk::DependencyFlags(),
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);

                _command_buffer_factory->End(command_buffer);
                device.freeCommandBuffers(command_pool, command_buffer);
                return *this;
            }
        };
    }
}
