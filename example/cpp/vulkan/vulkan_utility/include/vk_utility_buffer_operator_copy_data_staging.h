#pragma once


#include "vk_utility_object.h"
#include "vk_utility_exception.h"
#include "vk_utility_vulkan_include.h"
#include "vk_utility_buffer_and_memory.h"
#include "vk_utility_buffer_operator_copy_data.h"
#include "vk_utility_buffer_operator_copy_data_to_memory.h"
#include "vk_utility_buffer_operator_copy_buffer.h"
#include "vk_utility_device.h"
#include "vk_utility_buffer_factory_default.h"
#include "vk_utility_device_memory_factory_default.h"
#include "vk_utility_buffer_and_memory_factory_default.h"

#include <memory>


namespace vk_utility
{
    namespace buffer
    {
        class BufferOperatorCopyDataStaging;
        using BufferOperatorCopyDataStagingPtr = std::shared_ptr<BufferOperatorCopyDataStaging>;


        /// <summary>
        /// Copy operations which uses a staging buffer to copy form an data array to a buffer
        /// 
        /// We're now using a new stagingBuffer with stagingBufferMemory for mapping and copying the data.
        /// In this chapter we're going to use two new buffer usage flags:
        /// - VK_BUFFER_USAGE_TRANSFER_SRC_BIT: Buffer can be used as source in a memory transfer operation.
        /// - VK_BUFFER_USAGE_TRANSFER_DST_BIT: Buffer can be used as destination in a memory transfer operation.
        /// The buffer is now allocated from a memory type that is device local, 
        /// which generally means that we're not able to use `vkMapMemory`.
        /// However, we can copy data from the stagingBuffer to the buffer.
        /// We have to indicate that we intend to do that by specifying the transfer source flag for the staging buffer
        /// and the transfer destination flag for the buffer.
        /// </summary>
        class BufferOperatorCopyDataStaging
            : public BufferOperatorCopyData
        {
        private:

            vk_utility::device::DevicePtr _device;
            BufferOperatorCopyBufferPtr _buffer_copy_operator;

        public:

            static BufferOperatorCopyDataPtr New(vk_utility::device::DevicePtr device, BufferOperatorCopyBufferPtr buffer_copy_operator)
            {
                return std::make_shared<BufferOperatorCopyDataStaging>(device, buffer_copy_operator);
            }

            BufferOperatorCopyDataStaging(vk_utility::device::DevicePtr device, BufferOperatorCopyBufferPtr buffer_copy_operator)
                : _device(device)
                , _buffer_copy_operator(buffer_copy_operator)
            {}

            virtual BufferOperatorCopyDataStaging &copy(
                BufferAndMemoryPtr buffer_and_memory, 
                vk::DeviceSize offset, 
                vk::DeviceSize size, 
                const void *source_data) override
            {
                auto staging_buffer = BufferAndMemory::NewPtr(
                    *_device,
                    BufferAndMemoryFactoryDefault()
                        .set_buffer_factory(
                            &BufferFactoryDefault()
                                .set_buffer_size(size)
                                .set_staging_buffer_usage())
                        .set_buffer_memory_factory(
                            &BufferDeviceMemoryFactory()
                                .set_staging_memory_properties()
                                .set_from_physical_device(*_device->get().physical_device())));
                BufferOperatorCopyDataToMemory::New()
                    ->copy(staging_buffer, 0, size, source_data);
                _buffer_copy_operator->copy(buffer_and_memory, staging_buffer);
                staging_buffer->get().destroy();
                return *this;
            }

        };
    }
}
