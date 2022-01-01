#pragma once

#include "vk_utility_vulkan_include.h"
#include "vk_utility_object.h"
#include "vk_utility_command_buffers_factory.h"

namespace vk_utility
{
    namespace command
    {
        class CommandBuffers;
        using CommandBuffersPtr = vk_utility::Ptr<CommandBuffers>;

        class CommandBuffers
            : public GenericObject<std::vector<vk::CommandBuffer>>
        {
        private:

            vk::Device _device;
            vk::CommandPool _command_pool;

        public:

            static CommandBuffers New(vk::Device device, vk::CommandPool command_pool, const CommandBuffersFactory& command_buffers_factory)
            {
                return CommandBuffers(device, command_pool, command_buffers_factory.New(device, command_pool));
            }

            static CommandBuffersPtr NewPtr(vk::Device device, vk::CommandPool command_pool, const CommandBuffersFactory& command_buffers_factory)
            {
                return vk_utility::make_shared(New(device, command_pool, command_buffers_factory));
            }

            CommandBuffers(void) = default;
            CommandBuffers(const CommandBuffers&) = default;
            CommandBuffers& operator =(const CommandBuffers&) = default;

            CommandBuffers(vk::Device device, vk::CommandPool command_pool, std::vector<vk::CommandBuffer> command_buffers)
                : GenericObject(command_buffers)
                , _device(device)
                , _command_pool(command_pool)
            {}

            virtual void destroy(void) override
            {
                if (!_vk_object.empty() && _device && _command_pool)
                {
                    _device.freeCommandBuffers(_command_pool, _vk_object);
                    _vk_object.clear();
                }
            }
        };
    }
}
