#pragma once


#include "vk_utility_object.h"
#include "vk_utility_exception.h"
#include "vk_utility_vulkan_include.h"
#include "vk_utility_buffer_and_memory.h"
#include "vk_utility_buffer_operator_copy_data.h"

#include <memory>


namespace vk_utility
{
    namespace buffer
    {
        class BufferOperatorCopyDataToMemory;
        using BufferOperatorCopyDataToMemoryPtr = std::shared_ptr<BufferOperatorCopyDataToMemory>;

        /// <summary>
        /// Copy operations which directly copies data form a data array to a buffer
        /// </summary>
        class BufferOperatorCopyDataToMemory
            : public BufferOperatorCopyData
        {
        public:

            static BufferOperatorCopyDataPtr New(void)
            {
                return std::make_shared<BufferOperatorCopyDataToMemory>();
            }

            virtual BufferOperatorCopyDataToMemory &copy(BufferAndMemoryPtr buffer_and_memory, vk::DeviceSize offset, vk::DeviceSize size, const void *source_data) override
            {
                buffer_and_memory->memory().copy(offset, size, source_data);
                return *this;
            }

        };
    }
}
