#pragma once

#include "vk_utility_object.h"
#include "vk_utility_exception.h"
#include "vk_utility_vulkan_include.h"
#include "vk_utility_device.h"
#include "vk_utility_device_memory_factory.h"

namespace vk_utility
{

    namespace device
    {
        class DeviceMemory;
        using DeviceMemoryPtr = vk_utility::Ptr<DeviceMemory>;

        /// <summary>
        /// We now have a way to determine the right memory type,
        /// so we can actually allocate the memory by filling in the `vk::MemoryAllocateInfo` structure.
        /// Memory allocation is now as simple as specifying the size and type, 
        /// both of which are derived from the memory requirements of the vertex buffer and the desired property.
        /// </summary>
        class DeviceMemory
            : public GenericObject<vk::DeviceMemory>
        {
        private:

            vk::Device _device;
            vk::DeviceSize _memory_size;

        public:

            static DeviceMemory New(vk::Device& device, const DeviceMemoryFactory& device_memory_factory)
            {
                auto [device_memory, memory_size] = device_memory_factory.New(device);
                return DeviceMemory(device, device_memory, memory_size);
            }

            static DeviceMemoryPtr NewPtr(vk::Device& device, const DeviceMemoryFactory& device_memory_factory)
            {
                return vk_utility::make_shared(New(device, device_memory_factory));
            }

            DeviceMemory(void) = default;
            DeviceMemory(const DeviceMemory &) = default;
            DeviceMemory &operator = (const DeviceMemory &) = default;

            DeviceMemory(vk::Device device, vk::DeviceMemory device_memory, vk::DeviceSize memory_size)
                : _device(device)
                , _memory_size(memory_size)
                , GenericObject(device_memory)
            {}

            virtual void destroy() override
            {
                if (_device && _vk_object)
                {
                    _device.freeMemory(_vk_object);
                    _vk_object = vk::DeviceMemory();
                }
            }

            vk::DeviceSize size(void) const
            { 
                return _memory_size; 
            }

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
            /// <param name="offset"></param>
            /// <param name="size"></param>
            /// <param name="source_data"></param>
            /// <returns></returns>
            const DeviceMemory & copy(vk::DeviceSize offset, vk::DeviceSize size, const void * source_data) const
            {
                void * data = _device.mapMemory(_vk_object, 0, size);
                std::memcpy(data, source_data, (size_t)size);
                _device.unmapMemory(_vk_object);
                return *this;
            }

            const DeviceMemory & copy(const void * source_data) const
            {
                return copy(0, size(), source_data);
            }
        };
    }
}