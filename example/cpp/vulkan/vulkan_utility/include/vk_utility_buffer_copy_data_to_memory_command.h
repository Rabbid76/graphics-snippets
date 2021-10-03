#pragma once

#include "vk_utility_core_command.h"

namespace vk_utility
{
    namespace buffer
    {
        /// <summary>
        /// It is now time to copy the data to the buffer. 
        /// This is done by mapping the buffer memory into CPU accessible memory with `vkMapMemory`.
        /// This function allows us to access a region of the specified memory resource defined by an offset and size.
        /// The offset and size here are 0 and `bufferInfo.size`, respectively.
        /// It is also possible to specify the special value `VK_WHOLE_SIZE` to map all of the memory.
        /// The second to last parameter can be used to specify flags, but there aren't any available yet in the current API.
        /// It must be set to the value 0. The last parameter specifies the output for the pointer to the mapped memory.
        /// You can now simply `memcpy` the vertex data to the mapped memory and unmap it again using `vkUnmapMemory`.
        /// Unfortunately the driver may not immediately copy the data into the buffer memory, for example because of caching.
        /// It is also possible that writes to the buffer are not visible in the mapped memory yet.
        /// There are two ways to deal with that problem:
        /// - Use a memory heap that is host coherent, indicated with `VK_MEMORY_PROPERTY_HOST_COHERENT_BIT`.
        /// - Call `vkFlushMappedMemoryRanges` to after writing to the mapped memory, 
        ///   and call `vkInvalidateMappedMemoryRanges` before reading from the mapped memory.
        /// We went for the first approach, which ensures that the mapped memory always matches the contents of the allocated memory.
        /// </summary>
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
