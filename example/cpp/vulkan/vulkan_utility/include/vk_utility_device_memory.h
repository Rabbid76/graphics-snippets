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