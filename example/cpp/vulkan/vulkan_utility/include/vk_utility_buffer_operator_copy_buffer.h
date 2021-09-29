#pragma once


#include "vk_utility_object.h"
#include "vk_utility_exception.h"
#include "vk_utility_vulkan_include.h"

#include <memory>


namespace vk_utility
{
    namespace buffer
    {
        class BufferAndMemory;
        using BufferAndMemoryPtr = Ptr<BufferAndMemory>;
        class BufferOperatorCopyBuffer;
        using BufferOperatorCopyBufferPtr = std::shared_ptr<BufferOperatorCopyBuffer>;
        
        /// <summary>
        /// abstract class for copy operations from a buffer to a buffer
        /// </summary>
        class BufferOperatorCopyBuffer
        {
        public:
            virtual BufferOperatorCopyBuffer & copy(BufferAndMemoryPtr destination_buffer, BufferAndMemoryPtr source_buffer) = 0;
        };
    }
}
