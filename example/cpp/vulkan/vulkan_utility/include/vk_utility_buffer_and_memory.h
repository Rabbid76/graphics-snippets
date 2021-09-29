#pragma once


#include "vk_utility_object.h"
#include "vk_utility_exception.h"
#include "vk_utility_physical_device.h"
#include "vk_utility_device_memory.h"
#include "vk_utility_device.h"
#include "vk_utility_buffer.h"
#include "vk_utility_buffer_operator_copy_data.h"
#include "vk_utility_buffer_factory.h"
#include "vk_utility_buffer_device_memory_factory.h"
#include "vk_utility_buffer_and_memory_factory.h"

#include <memory>


namespace vk_utility
{
    namespace buffer
    {
        class BufferAndMemory;
        using BufferAndMemoryPtr = Ptr<BufferAndMemory>;

        /// <summary>
        /// Buffer and memory management. Association of `vk::DeviceMemory` and `vk::Buffer`.
        /// 
        /// If memory allocation was successful, then we can now associate this memory with the buffer using.
        /// The first three parameters are self-explanatory and the fourth parameter is the offset within the region of memory.
        /// Since this memory is allocated specifically for this the vertex buffer, the offset is simply 0.
        /// If the offset is non-zero, then it is required to be divisible by `memRequirements.alignment`.
        /// </summary>
        class BufferAndMemory
            : public GenericObject<int>
        {
        private:

            BufferPtr _buffer;
            device::DeviceMemoryPtr _device_memory;

        public:

            static BufferAndMemory New(vk::Device device, const BufferAndMemoryFactory& buffer_and_memory_factory)
            {
                auto [buffer, buffer_size, buffer_memory, memory_size] = buffer_and_memory_factory.New(device);
                return BufferAndMemory(device, buffer, buffer_size, buffer_memory, memory_size);
            }

            static BufferAndMemoryPtr NewPtr(vk::Device device, const BufferAndMemoryFactory& buffer_and_memory_factory)
            {
                return make_shared<BufferAndMemory>(New(device, buffer_and_memory_factory));
            }

            BufferAndMemory(void) = default;
            BufferAndMemory(const BufferAndMemory &) = default;
            BufferAndMemory &operator = (const BufferAndMemory &) = default;

            BufferAndMemory(const BufferPtr &buffer, const vk_utility::device::DeviceMemoryPtr &device_memory)
                : _buffer(buffer)
                , _device_memory(device_memory)
            {}

            BufferAndMemory(vk::Device device, vk::Buffer buffer, vk::DeviceSize buffer_size, vk::DeviceMemory device_memory, vk::DeviceSize memory_size)
                : _buffer(make_shared(Buffer(device, buffer, buffer_size)))
                , _device_memory(make_shared(device::DeviceMemory(device, device_memory, memory_size)))
            {}

            virtual void destroy(void) override
            {
                _buffer = nullptr;
                _device_memory = nullptr;
            }

            device::DeviceMemory & memory(void) { return _device_memory->get(); }
            const device::DeviceMemory & memory(void) const { return _device_memory->get(); }

            Buffer & buffer(void) { return _buffer->get(); }
            const Buffer & buffer(void) const { return _buffer->get(); }
        };
    }
}
