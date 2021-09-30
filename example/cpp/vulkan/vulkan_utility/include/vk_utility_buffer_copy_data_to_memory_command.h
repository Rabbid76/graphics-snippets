#pragma once

#include "vk_utility_core_command.h"

namespace vk_utility
{
    namespace buffer
    {
        class CopyDataToBufferMemoryCommand
            : public core::CoreCommand
        {
        private:
            
            vk::DeviceSize _size;
            const void* _source_data;
            vk::DeviceSize _offset = 0;
            vk::DeviceMemory _buffer_memory;

        public:

            CopyDataToBufferMemoryCommand& set_source_data(vk::DeviceSize size, const void* source_data)
            {
                _size = size;
                _source_data = source_data;
                return *this;
            }

            CopyDataToBufferMemoryCommand& set_destination_offset(vk::DeviceSize offset)
            {
                _offset = offset;
                return *this;
            }

            CopyDataToBufferMemoryCommand& set_destination_memory(vk::DeviceMemory buffer_memory)
            {
                _buffer_memory = buffer_memory;
                return *this;
            }

            virtual void execute_command(vk::Device device, vk::CommandPool command_pool) const override
            {
                void* data = device.mapMemory(_buffer_memory, _offset, _size);
                std::memcpy(data, _source_data, (size_t)_size);
                device.unmapMemory(_buffer_memory);
            }
        };
    }
}
