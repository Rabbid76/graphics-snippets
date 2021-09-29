#pragma once

#include "vk_utility_device_memory_factory.h"
#include "vk_utility_physical_device.h"

#include <functional>

namespace vk_utility
{
    namespace buffer
    {
        class BufferDeviceMemoryFactory
            : public device::DeviceMemoryFactory
        {
        public:

            using FindMemoryTypeFunction = std::function<uint32_t(uint32_t, vk::MemoryPropertyFlags)>;

        private:

            vk::Buffer _buffer;
            vk::MemoryPropertyFlags _properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
            FindMemoryTypeFunction _find_memory_type;

        public:

            BufferDeviceMemoryFactory& set_buffer(vk::Buffer buffer)
            {
                _buffer = buffer;
                return *this;
            }

            BufferDeviceMemoryFactory& set_memory_properties(vk::MemoryPropertyFlags properties)
            {
                _properties = properties;
                return *this;
            }


            BufferDeviceMemoryFactory& set_staging_memory_properties(void)
            {
                _properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
                return *this;
            }

            BufferDeviceMemoryFactory& set_coherent_memory_properties(void)
            {
                _properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
                return *this;
            }

            BufferDeviceMemoryFactory& set_from_physical_device(const vk_utility::device::PhysicalDevice& physical_device)
            {
                _find_memory_type = [physical_device](uint32_t mempry_type_btis, vk::MemoryPropertyFlags properties) -> uint32_t
                {
                    return physical_device.find_memory_type(mempry_type_btis, properties);
                };
                return *this;
            }

            virtual std::tuple<vk::DeviceMemory, vk::DeviceSize> New(vk::Device device) const override
            {
                const vk::MemoryRequirements memory_requirements = device.getBufferMemoryRequirements(_buffer);
                const vk::MemoryAllocateInfo allocate_information
                (
                    memory_requirements.size,
                    _find_memory_type(memory_requirements.memoryTypeBits, _properties)
                );
                vk::DeviceMemory buffer_memory = device.allocateMemory(allocate_information);
                device.bindBufferMemory(_buffer, buffer_memory, 0);
                return { buffer_memory, memory_requirements.size };
            }
        };
    }
}