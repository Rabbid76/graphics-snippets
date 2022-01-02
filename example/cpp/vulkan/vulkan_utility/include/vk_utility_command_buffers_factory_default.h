#pragma once

#include "vk_utility_command_buffers_factory.h"
#include "vk_utility_framebuffer.h"

namespace vk_utility
{
    namespace command
    {
        class CommandBuffersFactoryDefault
            : public CommandBuffersFactory
        {
        private:

            const std::vector<vk_utility::buffer::FramebufferPtr>* _swapchain_framebuffers;
            vk::RenderPass _render_pass;
            vk::Extent2D _image_extent;
            vk::Pipeline _graphics_pipeline;
            vk::PipelineLayout _pipeline_layout;
            const std::vector<vk::DescriptorSet>* _descriptor_sets;
            vk::Buffer _vertex_buffer;
            vk::Buffer _index_buffer;
            vk::IndexType _index_type;
            uint32_t _no_of_inideces;

        public:

            CommandBuffersFactoryDefault& set_swapchain_framebuffers(
                const std::vector<vk_utility::buffer::FramebufferPtr>* swapchain_framebuffers)
            {
                _swapchain_framebuffers = swapchain_framebuffers;
                return *this;
            }

            CommandBuffersFactoryDefault& set_render_pass(vk::RenderPass render_pass)
            {
                _render_pass = render_pass;
                return *this;
            }

            CommandBuffersFactoryDefault& set_image_size(vk::Extent2D image_extent)
            {
                _image_extent = image_extent;
                return *this;
            }

            CommandBuffersFactoryDefault& set_graphics_pipeline(vk::Pipeline graphics_pipeline)
            {
                _graphics_pipeline = graphics_pipeline;
                return *this;
            }

            CommandBuffersFactoryDefault& set_pipeline_layout(vk::PipelineLayout pipeline_layout)
            {
                _pipeline_layout = pipeline_layout;
                return *this;
            }

            CommandBuffersFactoryDefault& set_descriptor_sets(
                const std::vector<vk::DescriptorSet>* descriptor_sets)
            {
                _descriptor_sets = descriptor_sets;
                return *this;
            }

            CommandBuffersFactoryDefault& set_vertex_buffer(vk::Buffer vertex_buffer)
            {
                _vertex_buffer = vertex_buffer;
                return *this;
            }

            CommandBuffersFactoryDefault& set_index_buffer(vk::Buffer index_buffer, vk::IndexType index_type, uint32_t no_of_inideces)
            {
                _index_buffer = index_buffer;
                _index_type = index_type;
                _no_of_inideces = no_of_inideces;
                return *this;
            }

            virtual std::vector<vk::CommandBuffer> New(vk::Device device, vk::CommandPool command_pool) const override
            {
                // The level parameter specifies if the allocated command buffers are primary or secondary command buffers.
                // - `VK_COMMAND_BUFFER_LEVEL_PRIMARY`: Can be submitted to a queue for execution, but cannot be called from other command buffers.
                // - `VK_COMMAND_BUFFER_LEVEL_SECONDARY`: Cannot be submitted directly, but can be called from primary command buffers.

                vk::CommandBufferAllocateInfo alloc_info(
                    command_pool, vk::CommandBufferLevel::ePrimary, (uint32_t)_swapchain_framebuffers->size());
                auto command_buffers = device.allocateCommandBuffers(alloc_info);

                //-------------------------------------------
                // Starting command buffer recording
                //-------------------------------------------

                //! The flags parameter specifies how we're going to use the command buffer. The following values are available:
                //! - `VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT`: The command buffer will be rerecorded right after executing it once.
                //! - `VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT`: This is a secondary command buffer that will be entirely within a single render pass.
                //! - `VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT`: The command buffer can be resubmitted while it is also already pending execution.

                for (size_t i = 0; i < command_buffers.size(); i++)
                {
                    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse, nullptr);
                    command_buffers[i].begin(beginInfo);

                    //! We have used the last flag because we may already be scheduling the drawing commands for the next frame while the last frame is not finished yet.
                    //! The pInheritanceInfo parameter is only relevant for secondary command buffers. It specifies which state to inherit from the calling primary command buffers.

                    //! If the command buffer was already recorded once, then a call to vkBeginCommandBuffer will implicitly reset it.
                    //! It's not possible to append commands to a buffer at a later time.


                    //-------------------------------------------
                    // Starting a render pass
                    //-------------------------------------------

                    //! The render pass can now begin. All of the functions that record commands can be recognized by their vkCmd prefix.
                    //! They all return void, so there will be no error handling until we've finished recording.

                    //! The first parameters are the render pass itself and the attachments to bind. 
                    //! We created a framebuffer for each swap chain image that specifies it as color attachment.

                    //! The next two parameters define the size of the render area.
                    //! The render area defines where shader loads and stores will take place.
                    //! The pixels outside this region will have undefined values.
                    //! It should match the size of the attachments for best performance.

                    //! The last two parameters define the clear values to use for VK_ATTACHMENT_LOAD_OP_CLEAR, which we used as load operation for the color attachment.
                    //! I've defined the clear color to simply be black with 100% opacity.

                    std::vector<vk::ClearValue> clear_values
                    {
                        vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f})),
                        vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0))
                    };

                    vk::RenderPassBeginInfo render_Pass_info
                    (
                        _render_pass,
                        (*_swapchain_framebuffers)[i]->handle(),
                        vk::Rect2D(vk::Offset2D(0, 0), _image_extent),
                        clear_values
                    );


                    //! The first parameter for every command is always the command buffer to record the command to.
                    //! The second parameter specifies the details of the render pass we've just provided. The final parameter controls how the drawing commands within the render pass will be provided.
                    //! It can have one of two values:
                    //! - `VK_SUBPASS_CONTENTS_INLINE`: The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
                    //! - `VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS`: The render pass commands will be executed from secondary command buffers.

                    command_buffers[i].beginRenderPass(render_Pass_info, vk::SubpassContents::eInline);


                    //-------------------------------------------
                    // Basic drawing commands
                    //-------------------------------------------

                    //! The second parameter specifies if the pipeline object is a graphics or compute pipeline.

                    command_buffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, _graphics_pipeline);

                    //! The actual vkCmdDraw function is a bit anticlimactic, but it's so simple because of all the information we specified in advance.
                    //! It has the following parameters, aside from the command buffer:
                    //! - vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
                    //! - instanceCount: Used for instanced rendering, use 1 if you're not doing that.
                    //! - firstVertex: Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
                    //! - firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.

                    //! The `vkCmdBindVertexBuffers` function is used to bind vertex buffers to bindings, like the one we set up in the previous chapter.
                    //! The first two parameters, besides the command buffer, specify the offset and number of bindings we're going to specify vertex buffers for.
                    //! The last two parameters specify the array of vertex buffers to bind and the byte offsets to start reading vertex data from.
                    //! You should also change the call to vkCmdDraw to pass the number of vertices in the buffer as opposed to the hardcoded number 3.

                    std::vector<vk::Buffer> vertexBuffers = { _vertex_buffer };
                    std::vector<vk::DeviceSize> offsets = { 0 };
                    command_buffers[i].bindVertexBuffers(0, vertexBuffers, offsets);

                    // vkCmdDraw(_commandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);


                    //-------------------------------------------
                    // Using an index buffer
                    //-------------------------------------------

                    //! Using an index buffer for drawing involves two changes to createCommandBuffers.
                    //! We first need to bind the index buffer, just like we did for the vertex buffer.
                    //! The difference is that you can only have a single index buffer.
                    //! It's unfortunately not possible to use different indices for each vertex attribute,
                    //! so we do still have to completely duplicate vertex data even if just one attribute varies.

                    //! An index buffer is bound with vkCmdBindIndexBuffer which has the index buffer, a byte offset into it,
                    //! and the type of index data as parameters. As mentioned before, the possible types are VK_INDEX_TYPE_UINT16 and VK_INDEX_TYPE_UINT32.

                    command_buffers[i].bindIndexBuffer(_index_buffer, 0, _index_type);


                    //-------------------------------------------
                    // Using descriptor sets
                    //-------------------------------------------

                    //! Unlike vertex and index buffers, descriptor sets are not unique to graphics pipelines.
                    //! Therefore we need to specify if we want to bind descriptor sets to the graphics or compute pipeline.
                    //! The next parameter is the layout that the descriptors are based on.
                    //! The next three parameters specify the index of the first descriptor set, the number of sets to bind, and the array of sets to bind. We'll get back to this in a moment.
                    //! The last two parameters specify an array of offsets that are used for dynamic descriptors.

                    command_buffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipeline_layout, 0, 1, &(*_descriptor_sets)[i], 0, nullptr);


                    //! Just binding an index buffer doesn't change anything yet, we also need to change the drawing command to tell Vulkan to use the index buffer.
                    //! Remove the vkCmdDraw line and replace it with vkCmdDrawIndexed:

                    vkCmdDrawIndexed(command_buffers[i], _no_of_inideces, 1, 0, 0, 0);


                    //-------------------------------------------
                    // Finishing up
                    //-------------------------------------------

                    command_buffers[i].endRenderPass();
                    command_buffers[i].end();
                }

                return command_buffers;
            }
        };
    }
}

