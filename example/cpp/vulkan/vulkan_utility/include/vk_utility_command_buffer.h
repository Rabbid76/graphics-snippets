#pragma once

#include "vk_utility_vulkan_include.h"
#include "vk_utility_object.h"
#include "vk_utility_command_buffer_factory.h"

namespace vk_utility
{
    namespace command
    {
        class CommandBuffer;
        using CommandBufferPtr = vk_utility::Ptr<CommandBuffer>;

        class CommandBuffer
            : public GenericObject<vk::CommandBuffer>
        {
        private:

            vk::Device _device;
            vk::CommandPool _command_pool;

        public:

            static CommandBuffer New(vk::Device device, vk::CommandPool command_pool, const CommandBufferFactory& command_buffer_factory)
            {
                return CommandBuffer(device, command_pool, command_buffer_factory.Begin(device, command_pool));
            }

            static CommandBufferPtr NewPtr(vk::Device device, vk::CommandPool command_pool, const CommandBufferFactory& command_buffer_factory)
            {
                return vk_utility::make_shared(New(device, command_pool, command_buffer_factory));
            }

            CommandBuffer(void) = default;
            CommandBuffer(const CommandBuffer&) = default;
            CommandBuffer& operator =(const CommandBuffer&) = default;

            CommandBuffer(vk::Device device, vk::CommandPool command_pool, vk::CommandBuffer command_buffer)
                : GenericObject(command_buffer)
                , _device(device)
                , _command_pool(command_pool)
            {}

            virtual void destroy(void) override
            {
                if (_vk_object && _device && _command_pool)
                {
                    _device.freeCommandBuffers(_command_pool, _vk_object);
                    _vk_object = vk::CommandBuffer();
                }
            }
        };
    }
}