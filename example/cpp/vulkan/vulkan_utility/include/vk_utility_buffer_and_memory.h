#pragma once


#include "vk_utility_object.h"
#include "vk_utility_exception.h"
#include "vk_utility_vulkan_include.h"
#include "vk_utility_physical_device.h"
#include "vk_utility_device_memory.h"
#include "vk_utility_device.h"
#include "vk_utility_buffer.h"
#include "vk_utility_buffer_and_memory_information.h"
#include "vk_utility_buffer_operator_copy_data.h"

#include <memory>


namespace vk_utility
{
    namespace buffer
    {
        class BufferAndMemory;
        using BufferAndMemoryPtr = std::shared_ptr<BufferAndMemory>;

        /// <summary>
        /// Buffer and memory management. Association of `vk::DeviceMemory` and `vk::Buffer`.
        /// 
        /// If memory allocation was successful, then we can now associate this memory with the buffer using.
        /// The first three parameters are self-explanatory and the fourth parameter is the offset within the region of memory.
        /// Since this memory is allocated specifically for this the vertex buffer, the offset is simply 0.
        /// If the offset is non-zero, then it is required to be divisible by `memRequirements.alignment`.
        /// </summary>
        class BufferAndMemory
            : Object
        {
        private:

            vk_utility::device::DeviceMemoryPtr _memory;
            BufferPtr _buffer;

        public:

            static BufferAndMemoryPtr New(vk_utility::device::DeviceMemoryPtr memory, BufferPtr buffer)
            {
                return std::make_shared<BufferAndMemory>(memory, buffer);
            }

            /// <summary>
            /// Construction of a buffer and its device memory.
            /// </summary>
            /// <param name="device"></param>
            /// <param name="memory"></param>
            /// <param name="buffer"></param>
            /// <returns></returns>
            static BufferAndMemoryPtr New(vk_utility::device::DevicePtr device, vk_utility::device::DeviceMemoryPtr memory, BufferPtr buffer)
            {
                device->get()->bindBufferMemory(buffer->handle(), memory->handle(), 0);
                return New(memory, buffer);
            }

            /// <summary>
            /// Construction of a buffer and its device memory.
            /// </summary>
            /// <param name="device"></param>
            /// <param name="buffer_and_memory_information"></param>
            /// <returns></returns>
            static BufferAndMemoryPtr New(vk_utility::device::DevicePtr device, const BufferAndMemoryInformation & buffer_and_memory_information)
            {
                BufferPtr buffer = Buffer::New(device, buffer_and_memory_information.buffer_Information());
                vk_utility::device::DeviceMemoryPtr memory = vk_utility::device::DeviceMemory::New(
                    device,
                    buffer_and_memory_information.memory_Information(device, buffer)
                );
                return New(device, memory, buffer);
            }

            /// <summary>
            /// Construction of a buffer and its device memory.
            /// The buffer is initialized by a data array.
            /// The data copying process is carried out by a copy operator, with the data either copied directly or in stages.
            /// </summary>
            /// <param name="device"></param>
            /// <param name="buffer_and_memory_information"></param>
            /// <param name="source_data"></param>
            /// <param name="copy_operator"></param>
            /// <returns></returns>
            static BufferAndMemoryPtr New(
                vk_utility::device::DevicePtr device,
                const BufferAndMemoryInformation & buffer_and_memory_information,
                const void * source_data,
                BufferOperatorCopyDataPtr copy_operator)
            {
                BufferPtr buffer = Buffer::New(device, buffer_and_memory_information.buffer_Information());
                vk_utility::device::DeviceMemoryPtr memory = vk_utility::device::DeviceMemory::New(
                    device,
                    buffer_and_memory_information.memory_Information(device, buffer)
                );
                auto buffer_and_memory = New(device, memory, buffer);
                copy_operator->copy(buffer_and_memory, 0, buffer_and_memory_information.size(), source_data);
                return buffer_and_memory;
            }

            /// <summary>
            /// Construction of a buffer and its device memory.
            /// The buffer is initialized by a data array.
            /// The data copying process is carried out by a copy operator, with the data either copied directly or in stages.
            /// </summary>
            /// <typeparam name="T"></typeparam>
            /// <param name="device"></param>
            /// <param name="buffer_and_memory_information"></param>
            /// <param name="source_data"></param>
            /// <param name="copy_operator"></param>
            /// <returns></returns>
            template<typename T>
            static BufferAndMemoryPtr New(
                vk_utility::device::DevicePtr device,
                const BufferAndMemoryInformation & buffer_and_memory_information,
                const std::vector<T> &source_data,
                BufferOperatorCopyDataPtr copy_operator)
            {
                BufferPtr buffer = Buffer::New(device, buffer_and_memory_information.buffer_Information());
                vk_utility::device::DeviceMemoryPtr memory = vk_utility::device::DeviceMemory::New(
                    device,
                    buffer_and_memory_information.memory_Information(device, buffer)
                );
                auto buffer_and_memory = New(device, memory, buffer);
                copy_operator->copy(buffer_and_memory, 0, sizeof(T) * source_data.size(), source_data.data());
                return buffer_and_memory;
            }

            BufferAndMemory(void) = delete;
            BufferAndMemory(const BufferAndMemory &) = delete;
            BufferAndMemory &operator = (const BufferAndMemory &) = delete;

            BufferAndMemory(vk_utility::device::DeviceMemoryPtr memory, BufferPtr buffer)
                :_memory(memory)
                ,_buffer(buffer)
            {}

            virtual void destroy(void) override
            {
                if (_buffer)
                {
                    _buffer->get().destroy();
                    _buffer = BufferPtr();
                }
                if (_memory)
                {
                    _memory->get().destroy();
                    _memory = vk_utility::device::DeviceMemoryPtr();
                }
            }

            vk_utility::device::DeviceMemory & memory(void) { return _memory->get(); }
            const vk_utility::device::DeviceMemory & memory(void) const { return _memory->get(); }

            Buffer & buffer(void) { return _buffer->get(); }
            const Buffer & buffer(void) const { return _buffer->get(); }
        };
    }
}
