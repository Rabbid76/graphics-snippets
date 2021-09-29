#pragma once

#include "vk_utility_device_memory_factory.h"

namespace vk_utility
{
    namespace device
    {
        class DeviceMemoryFactoryDefault
            : public DeviceMemoryFactory
        {
        private:

            vk::DeviceSize _memory_size;
            uint32_t _memory_type;

        private:

            DeviceMemoryFactoryDefault& set_memory_size(vk::DeviceSize memory_size)
            {
                _memory_size = memory_size;
                return *this;
            }

            DeviceMemoryFactoryDefault& set_memory_type(uint32_t memory_type)
            {
                _memory_type = memory_type;
                *this;
            }

            virtual std::tuple<vk::DeviceMemory, vk::DeviceSize> New(vk::Device device) const override
            {
                const vk::MemoryAllocateInfo allocate_information
                (
                    _memory_size,
                    _memory_type
                );
                vk::DeviceMemory memory = device.allocateMemory(allocate_information);
                return { memory, _memory_size };
            }
        };
    }
}