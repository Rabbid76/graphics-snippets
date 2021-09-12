#pragma once

#include "vk_utility_command_pool_factory.h"
#include "vk_utility_physical_device.h"

#include <memory>

namespace vk_utility
{
    namespace command
    {
        /// <summary>
        /// Commands in Vulkan, like drawing operations and memory transfers, are not executed directly using function calls.
        /// You have to record all of the operations you want to perform in command buffer objects.
        /// The advantage of this is that all of the hard work of setting up the drawing commands can be done in advance and in multiple threads.
        /// After that, you just have to tell Vulkan to execute the commands in the main loop.
        ///
        /// We have to create a command pool before we can create command buffers.
        /// Command pools manage the memory that is used to store the buffers and command buffers are allocated from them.
        ///
        /// Command buffers are executed by submitting them on one of the device queues, like the graphics and presentation queues we retrieved.
        /// Each command pool can only allocate command buffers that are submitted on a single type of queue.
        /// We're going to record commands for drawing, which is why we've chosen the graphics queue family.
        ///
        /// There are two possible flags for command pools:
        /// - `VK_COMMAND_POOL_CREATE_TRANSIENT_BIT`: Hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
        /// - `VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT`: Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
        /// </summary>
        class CommandPoolFactoryDefault
            : public CommandPoolFactory
        {
        private:

            std::shared_ptr<device::DeviceQueueInformation> _device_queue_information;

        public:

            CommandPoolFactoryDefault& set_device_queue_information(std::shared_ptr<device::DeviceQueueInformation> device_queue_information)
            {
                _device_queue_information = device_queue_information;
                return *this;
            }

            virtual vk::CommandPool New(vk::Device device) const override
            {
                const uint32_t queue_famili_index = _device_queue_information->_graphics[0];
                const vk::CommandPoolCreateInfo command_pool_create_information
                {
                    vk::CommandPoolCreateFlags{},
                    queue_famili_index,
                };
                return device.createCommandPool(command_pool_create_information);
            }
        };
    }
}