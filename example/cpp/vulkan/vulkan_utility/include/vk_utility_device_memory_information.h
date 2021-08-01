#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_device.h>
#include <vk_utility_buffer.h>


namespace vk_utility
{
    namespace device
    {
        /// <summary>
        /// device memory allocation information (`vk::MemoryAllocateInfo`)
        /// </summary>
        class DeviceMemoryInformation
        {
        private:

            vk::MemoryAllocateInfo _allocate_information;

        public:

            static DeviceMemoryInformation New(vk::MemoryAllocateInfo allocate_information)
            {
                return DeviceMemoryInformation(allocate_information);
            }

            static DeviceMemoryInformation New(vk::DeviceSize size, uint32_t type)
            {
                vk::MemoryAllocateInfo allocate_information(size, type);
                return DeviceMemoryInformation(allocate_information);
            }

            static DeviceMemoryInformation New(vk_utility::device::DevicePtr device, vk::MemoryRequirements memory_requirements, vk::MemoryPropertyFlags properties)
            {
                vk::MemoryAllocateInfo allocate_information
                (
                    memory_requirements.size,
                    device->get().physical_device().find_memory_type(memory_requirements.memoryTypeBits, properties)
                );
                return DeviceMemoryInformation(allocate_information);
            }

            static DeviceMemoryInformation New(vk_utility::device::DevicePtr device, vk_utility::buffer::BufferPtr buffer, vk::MemoryPropertyFlags properties)
            {
                vk::MemoryRequirements memory_requirements = buffer->get().get_buffer_memory_requirements();
                vk::MemoryAllocateInfo allocate_information
                (
                    memory_requirements.size,
                    device->get().physical_device().find_memory_type(memory_requirements.memoryTypeBits, properties)
                );
                return DeviceMemoryInformation(allocate_information);
            }

            DeviceMemoryInformation(void) = default;
            DeviceMemoryInformation(const DeviceMemoryInformation &) = default;
            DeviceMemoryInformation &operator = (const DeviceMemoryInformation &) = default;

            DeviceMemoryInformation(const vk::MemoryAllocateInfo &allocation_information)
                : _allocate_information(allocation_information)
            {}

            DeviceMemoryInformation &operator = (const vk::MemoryAllocateInfo & allocation_information)
            {
                _allocate_information = allocation_information;
                return *this;
            }

            operator const vk::MemoryAllocateInfo &() const 
            {
                return _allocate_information;
            }

            vk::DeviceSize size(void) const
            {
                return _allocate_information.allocationSize;
            }
        };
    }
}

