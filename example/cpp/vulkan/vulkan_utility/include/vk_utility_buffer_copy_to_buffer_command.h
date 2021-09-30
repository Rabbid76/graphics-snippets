#pragma once

#include "vk_utility_core_command.h"
#include "vk_utility_command_buffer_factory.h"

namespace vk_utility
{
    namespace buffer
    {
        class CopyBufferMemoryToBufferCommand
            : public core::CoreCommand
        {
        private:

            const command::CommandBufferFactory* _command_buffer_factory;
            vk::Buffer _source_buffer;
            vk::Buffer _destination_buffer;
            vk::DeviceSize _copy_size;
            vk::DeviceSize _source_offeset = 0;
            vk::DeviceSize _destination_offeset = 0;

        public:

            CopyBufferMemoryToBufferCommand& set_command_buffer_factory(const command::CommandBufferFactory* command_buffer_factory)
            {
                _command_buffer_factory = command_buffer_factory;
                return *this;
            }

            CopyBufferMemoryToBufferCommand& set_source_buffer(vk::Buffer source_buffer)
            {
                _source_buffer = source_buffer;
                return *this;
            }

            CopyBufferMemoryToBufferCommand& set_destination_buffer(vk::Buffer destination_buffer)
            {
                _destination_buffer = destination_buffer;
                return *this;
            }

            CopyBufferMemoryToBufferCommand& set_copy_size(vk::DeviceSize offset)
            {
                _copy_size = offset;
                return *this;
            }

            CopyBufferMemoryToBufferCommand& set_source_offset(vk::DeviceSize offset)
            {
                _source_offeset = offset;
                return *this;
            }

            CopyBufferMemoryToBufferCommand& set_destination_offset(vk::DeviceSize offset)
            {
                _destination_offeset = offset;
                return *this;
            }

            virtual void execute_command(vk::Device device, vk::CommandPool command_pool) const override
            {
                // TODO $$$
                // [Transfer queue](https://vulkan-tutorial.com/Vertex_buffers/Staging_buffer)
                // The buffer copy command requires a queue family that supports transfer operations, which is indicated using VK_QUEUE_TRANSFER_BIT.
                // The good news is that any queue family with VK_QUEUE_GRAPHICS_BIT or VK_QUEUE_COMPUTE_BIT capabilities already implicitly support VK_QUEUE_TRANSFER_BIT operations.
                // The implementation is not required to explicitly list it in queueFlags in those cases.
                //
                // If you like a challenge, then you can still try to use a different queue family specifically for transfer operations. 
                // It will require you to make the following modifications to your program:
                // - Modify QueueFamilyIndices and findQueueFamilies to explicitly look for a queue family with the VK_QUEUE_TRANSFER bit, but not the VK_QUEUE_GRAPHICS_BIT.
                // - Modify createLogicalDevice to request a handle to the transfer queue
                // - Create a second command pool for command buffers that are submitted on the transfer queue family
                // - Change the sharingMode of resources to be VK_SHARING_MODE_CONCURRENT and specify both the graphics and transfer queue families
                // - Submit any transfer commands like vkCmdCopyBuffer (which we'll be using in this chapter) to the transfer queue instead of the graphics queue
                //
                // Contents of buffers are transferred using the vkCmdCopyBuffer command.
                // It takes the source and destination buffers as arguments, and an array of regions to copy.
                // The regions are defined in `vk::BufferCopy` structures and consist of a source buffer offset, destination buffer offset and size.
                // It is not possible to specify VK_WHOLE_SIZE here, unlike the vkMapMemory command.

                auto command_buffer = _command_buffer_factory->Begin(device, command_pool);

                std::vector<vk::BufferCopy> copyRegions
                { 
                    vk::BufferCopy(_source_offeset, _destination_offeset, _copy_size) 
                };
                command_buffer.copyBuffer(_source_buffer, _destination_buffer, copyRegions);

                _command_buffer_factory->End(command_buffer);
                device.freeCommandBuffers(command_pool, command_buffer);
            }
        };
    }
}
