#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>

#include <memory>


namespace vk_utility
{
    namespace buffer
    {
        class BufferAndMemory;
        using BufferAndMemoryPtr = std::shared_ptr<BufferAndMemory>;
        class BufferOperatorCopyData;
        using BufferOperatorCopyDataPtr = std::shared_ptr<BufferOperatorCopyData>;
        
        /// <summary>
        /// abstract class for copy operations from a data array to a buffer
        /// </summary>
        class BufferOperatorCopyData
        {
        public:
            virtual BufferOperatorCopyData & copy(BufferAndMemoryPtr buffer_and_memory, vk::DeviceSize offset, vk::DeviceSize size, const void *source_data) = 0;
        };
    }
}
