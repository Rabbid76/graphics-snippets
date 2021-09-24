#pragma once

#include "vk_utility_command_buffer_factory.h"

namespace vk_utility
{
    namespace image
    {
        /// <summary>
        // Layout transitions:
        ///
        /// One of the most common ways to perform layout transitions is using an image memory barrier.
        /// A pipeline barrier like that is generally used to synchronize access to resources,
        /// like ensuring that a write to a buffer completes before reading from it,
        /// but it can also be used to transition image layouts and transfer queue family ownership when VK_SHARING_MODE_EXCLUSIVE is used.
        ///
        //! There is an equivalent buffer memory barrier to do this for buffers.
        /// The first two fields specify layout transition.
        /// It is possible to use VK_IMAGE_LAYOUT_UNDEFINED as oldLayout if you don't care about the existing contents of the image.
        /// If you are using the barrier to transfer queue family ownership, then these two fields should be the indices of the queue families.
        /// They must be set to VK_QUEUE_FAMILY_IGNORED if you don't want to do this (not the default value!).
        /// The image and subresourceRange specify the image that is affected and the specific part of the image.
        /// Our image is not an array and does not have mipmapping levels, so only one level and layer are specified.
        /// Barriers are primarily used for synchronization purposes, so you must specify which types of operations that involve the resource must happen before the barrier,
        /// and which operations that involve the resource must wait on the barrier.
        /// We need to do that despite already using vkQueueWaitIdle to manually synchronize.
        /// The right values depend on the old and new layout, so we'll get back to this once we've figured out which transitions we're going to use.
        ///
        /// All types of pipeline barriers are submitted using the same function.
        /// The first parameter after the command buffer specifies in which pipeline stage the operations occur that should happen before the barrier.
        /// The second parameter specifies the pipeline stage in which operations will wait on the barrier.
        /// The pipeline stages that you are allowed to specify before and after the barrier depend on how you use the resource before and after the barrier.
        /// The allowed values are listed in this table of the specification. 
        /// For example, if you're going to read from a uniform after the barrier,
        /// you would specify a usage of VK_ACCESS_UNIFORM_READ_BIT and the earliest shader that will read from the uniform as pipeline stage,
        /// for example VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT.
        /// It would not make sense to specify a non-shader pipeline stage for this type of usage and the validation layers will warn you when you specify a pipeline stage that does not match the type of usage.
        /// The third parameter is either 0 or VK_DEPENDENCY_BY_REGION_BIT.
        /// The latter turns the barrier into a per-region condition. 
        /// That means that the implementation is allowed to already begin reading from the parts of a resource that were written so far, for example.
        /// The last three pairs of parameters reference arrays of pipeline barriers of the three available types:
        /// memory barriers, buffer memory barriers, and image memory barriers like the one we're using here.
        /// Note that we're not using the `vk::Format` parameter yet, but we'll be using that one for special transitions in the depth buffer chapter.
        ///
        /// Transition barrier masks
        ///
        //// There are two transitions we need to handle:
        ///     Undefined transfer destination: transfer writes that don't need to wait on anything
        ///     transfer destination shader reading: shader reads should wait on transfer writes, specifically the shader reads in the fragment shader, because that's where we're going to use the texture
        ///
        /// As you can see in the aforementioned table, transfer writes must occur in the pipeline transfer stage.
        /// Since the writes don't have to wait on anything, you may specify an empty access mask and the earliest possible pipeline stage VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT for the pre-barrier operations.
        /// It should be noted that VK_PIPELINE_STAGE_TRANSFER_BIT is not a real stage within the graphics and compute pipelines.
        /// It is more of a pseudo-stage where transfers happen. See the documentation for more information and other examples of pseudo-stages.
        /// The image will be written in the same pipeline stage and subsequently read by the fragment shader, which is why we specify shader reading access in the fragment shader pipeline stage.
        /// If we need to do more transitions in the future, then we'll extend the function.
        /// The application should now run successfully, although there are of course no visual changes yet.
        /// One thing to note is that command buffer submission results in implicit VK_ACCESS_HOST_WRITE_BIT synchronization at the beginning.
        /// Since the transitionImageLayout function executes a command buffer with only a single command, you could use this implicit synchronization and set srcAccessMask to 0 if you ever needed a VK_ACCESS_HOST_WRITE_BIT dependency in a layout transition.
        /// It's up to you if you want to be explicit about it or not, but I'm personally not a fan of relying on these OpenGL-like "hidden" operations.
        /// There is actually a special type of image layout that supports all operations, VK_IMAGE_LAYOUT_GENERAL.
        /// The problem with it, of course, is that it doesn't necessarily offer the best performance for any operation.
        /// It is required for some special cases, like using an image as both input and output, or for reading an image after it has left the preinitialized layout.
        /// All of the helper functions that submit commands so far have been set up to execute synchronously by waiting for the queue to become idle.
        /// For practical applications it is recommended to combine these operations in a single command buffer and execute them asynchronously for higher throughput, especially the transitions and copy in the createTextureImage function.
        /// Try to experiment with this by creating a setupCommandBuffer that the helper functions record commands into, and add a flushSetupCommands to execute the commands that have been recorded so far.
        /// It's best to do this after the texture mapping works to check if the texture resources are still set up correctly.
        /// </summary>
        class ImageTransitionCommand
        {
        private:

            const command::CommandBufferFactory *_command_buffer_factory;
            vk::Image _image;
            uint32_t _mipmap_levels = 1;
            vk::ImageLayout _old_layout;
            vk::ImageLayout _new_layout;

        public:

            ImageTransitionCommand& set_command_buffer_factory(const command::CommandBufferFactory* command_buffer_factory)
            {
                _command_buffer_factory = command_buffer_factory;
                return *this;
            }

            ImageTransitionCommand& set_image(vk::Image image)
            {
                _image = image;
                return *this;
            }

            ImageTransitionCommand& set_mipmap_levels(uint32_t mipmap_levels)
            {
                _mipmap_levels = mipmap_levels;
                return *this;
            }

            ImageTransitionCommand& set_old_layout(vk::ImageLayout layout)
            {
                _old_layout = layout;
                return *this;
            }

            ImageTransitionCommand& set_new_layout(vk::ImageLayout layout)
            {
                _new_layout = layout;
                return *this;
            }

            ImageTransitionCommand& execute_command(vk::Device device, vk::CommandPool command_pool)
            {
                auto command_buffer = _command_buffer_factory->Begin(device, command_pool);

                vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor, 0, _mipmap_levels, 0, 1);
                vk::PipelineStageFlags sourceStage;
                vk::PipelineStageFlags destinationStage;
                vk::AccessFlags srcAccessMask{};
                vk::AccessFlags dstAccessMask{};

                if (_old_layout == vk::ImageLayout::eUndefined && _new_layout == vk::ImageLayout::eTransferDstOptimal)
                {
                    dstAccessMask = vk::AccessFlagBits::eTransferWrite;
                    sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
                    destinationStage = vk::PipelineStageFlagBits::eTransfer;
                }
                else if (_old_layout == vk::ImageLayout::eTransferDstOptimal && _new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
                {
                    srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                    dstAccessMask = vk::AccessFlagBits::eShaderRead;
                    sourceStage = vk::PipelineStageFlagBits::eTransfer;
                    destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
                }
                else
                {
                    throw std::invalid_argument("unsupported layout transition!");
                }

                vk::ImageMemoryBarrier barrier
                (
                    srcAccessMask,
                    dstAccessMask,
                    _old_layout,
                    _new_layout,
                    VK_QUEUE_FAMILY_IGNORED,
                    VK_QUEUE_FAMILY_IGNORED,
                    _image,
                    subresourceRange
                );

                command_buffer.pipelineBarrier(
                    sourceStage, destinationStage,
                    vk::DependencyFlags(),
                    0, nullptr,
                    0, nullptr,
                    1, &barrier
                );
                _command_buffer_factory->End(command_buffer);

                device.freeCommandBuffers(command_pool, command_buffer);
                return *this;
            }
        };
    }
}