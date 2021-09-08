#pragma once

#include "vk_utility_command_buffer_factory.h"

#include <vector>

namespace vk_utility
{
    namespace command
    {
        /// <summary>
        /// Memory transfer operations are executed using command buffers, just like drawing commands. 
        /// Therefore we must first allocate a temporary command buffer. 
        /// You may wish to create a separate command pool for these kinds of short-lived buffers, 
        /// because the implementation may be able to apply memory allocation optimizations.
        /// You should use the VK_COMMAND_POOL_CREATE_TRANSIENT_BIT flag during command pool generation in that case.
        /// 
        /// The VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT flag that we used for the drawing command buffers is not necessary here,
        /// because we're only going to use the command buffer once and wait with returning from the function until the copy operation has finished executing.
        /// It's good practice to tell the driver about our intent using VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        /// 
        /// Contents of buffers are transferred using the vkCmdCopyBuffer command.
        /// It takes the source and destination buffers as arguments, and an array of regions to copy.
        /// The regions are defined in `vk::BufferCopy` structures and consist of a source buffer offset, destination buffer offset and size.
        /// It is not possible to specify VK_WHOLE_SIZE here, unlike the vkMapMemory command.
        /// 
        /// Unlike the draw commands, there are no events we need to wait on this time. We just want to execute the transfer on the buffers immediately.
        /// There are again two possible ways to wait on this transfer to complete.
        /// We could use a fence and wait with vkWaitForFences, or simply wait for the transfer queue to become idle with vkQueueWaitIdle.
        /// A fence would allow you to schedule multiple transfers simultaneously and wait for all of them complete, instead of executing one at a time.
        /// That may give the driver more opportunities to optimize.
        /// </summary>
        class CommandBufferFactorySingleTimeCommand
            : public CommandBufferFactory
        {
        private: 

            vk::Queue _graphics_queue;

        public:

            CommandBufferFactorySingleTimeCommand set_graphics_queue(vk::Queue graphics_queue)
            {
                _graphics_queue = graphics_queue;
                return *this;
            }

            virtual vk::CommandBuffer Begin(vk::Device device, vk::CommandPool command_pool) const override
            {
                vk::CommandBufferAllocateInfo allocate_information(command_pool, vk::CommandBufferLevel::ePrimary, 1);
                auto command_buffer = device
                    .allocateCommandBuffers(allocate_information)
                    .front();
                vk::CommandBufferBeginInfo begin_information(vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr);
                command_buffer.begin(begin_information);
                return command_buffer;
            }

            virtual void End(vk::CommandBuffer command_buffer) const override
            {
                command_buffer.end();

                std::vector<vk::Semaphore> wait_semaphore;
                std::vector<vk::PipelineStageFlags> wait_dest_stage_mask;
                std::vector<vk::CommandBuffer> command_buffers{ command_buffer };
                std::vector<vk::Semaphore> signal_semaphores;
                vk::SubmitInfo submit_information
                (
                    wait_semaphore,
                    wait_dest_stage_mask,
                    command_buffers,
                    signal_semaphores
                );
                vk::Fence fence;
                _graphics_queue.submit(submit_information, fence);
                _graphics_queue.waitIdle();
            }
        };
    }
}