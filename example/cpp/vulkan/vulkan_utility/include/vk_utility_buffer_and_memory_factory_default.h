#pragma once

#include "vk_utility_buffer_and_memory_factory.h"
#include "vk_utility_buffer_factory.h"
#include "vk_utility_buffer_device_memory_factory.h"

namespace vk_utility
{
    namespace buffer
    {
        class BufferAndMemoryFactoryDefault
            : public BufferAndMemoryFactory
        {
        private:

            BufferFactory* _buffer_factory;
            BufferDeviceMemoryFactory* _buffer_device_memory_factory;

        public:

            auto& set_buffer_factory(BufferFactory* buffer_factory)
            {
                _buffer_factory = buffer_factory;
                return *this;
            }

            auto& set_buffer_memory_factory(BufferDeviceMemoryFactory* buffer_device_memory_factory)
            {
                _buffer_device_memory_factory = buffer_device_memory_factory;
                return *this;
            }

            std::tuple<vk::Buffer, vk::DeviceMemory, vk::DeviceSize> New(vk::Device device) const override
            {
                auto [buffer, buffer_size] = _buffer_factory->New(device);
                auto [buffer_memory, memory_size] = _buffer_device_memory_factory
                    ->set_buffer(buffer)
                    .New(device);
                return std::make_tuple(buffer, buffer_memory, memory_size);
            }
        };
    }
}
