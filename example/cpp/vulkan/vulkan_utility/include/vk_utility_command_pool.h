#pragma once

#include "vk_utility_vulkan_include.h"
#include "vk_utility_object.h"
#include "vk_utility_command_pool_factory.h"

namespace vk_utility
{
    namespace command
    {
        class CommandPool;
        using CommandPoolPtr = Ptr<CommandPool>;

        class CommandPool
            : public GenericObject<vk::CommandPool>
        {
        private:

            vk::Device _device;

        public:

            static CommandPool New(vk::Device device, const CommandPoolFactory& command_pool_factory)
            {
                return CommandPool(device, command_pool_factory.New(device));
            }

            static CommandPoolPtr NewPtr(vk::Device device, const CommandPoolFactory& command_pool_factory)
            {
                return vk_utility::make_shared(New(device, command_pool_factory));
            }

            CommandPool(void) = default;
            CommandPool(const CommandPool&) = default;
            CommandPool& operator =(const CommandPool&) = default;

            CommandPool(vk::Device &device, vk::CommandPool command_pool)
                : GenericObject(command_pool),
                _device(device)
            {}

            virtual void destroy()
            {
                if (_vk_object && _device)
                {
                    _device.destroyCommandPool(_vk_object);
                    _vk_object = vk::CommandPool();
                }
            }
        };
    }
}