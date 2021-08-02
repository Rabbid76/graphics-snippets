#pragma once

#include "vk_utility_object.h"
#include "vk_utility_exception.h"
#include "vk_utility_vulkan_include.h"
#include "vk_utility_buffer_information.h" 
#include "vk_utility_device_memory_information.h"


namespace vk_utility
{
    namespace buffer
    {
        /// <summary>
        /// Information for the generation of a buffer and the corresponding device memory.
        /// </summary>
        class BufferAndMemoryInformation
        {
        private:

            vk::MemoryPropertyFlags _properties;
            vk::DeviceSize _size;
            vk::BufferUsageFlags _usage;
            vk::SharingMode _sharing_mode;

        public:

            static BufferAndMemoryInformation New(vk::MemoryPropertyFlags properties, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::SharingMode sharing_mode = vk::SharingMode::eExclusive)
            {
                return BufferAndMemoryInformation(properties, size, usage, sharing_mode);
            }

            static BufferAndMemoryInformation NewStaging(vk::DeviceSize size, vk::SharingMode sharing_mode = vk::SharingMode::eExclusive)
            {
                return BufferAndMemoryInformation(
                    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                    size,
                    vk::BufferUsageFlagBits::eTransferSrc,
                    sharing_mode);
            }

            static BufferAndMemoryInformation NewCoherentUniform(vk::DeviceSize size, vk::SharingMode sharing_mode = vk::SharingMode::eExclusive)
            {
                return BufferAndMemoryInformation(
                    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                    size,
                    vk::BufferUsageFlagBits::eUniformBuffer,
                    sharing_mode);
            }

            static BufferAndMemoryInformation NewVertex(vk::DeviceSize size, vk::SharingMode sharing_mode = vk::SharingMode::eExclusive)
            {
                return BufferAndMemoryInformation(
                    vk::MemoryPropertyFlagBits::eDeviceLocal,
                    size,
                    vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                    sharing_mode);
            }

            static BufferAndMemoryInformation NewIndex(vk::DeviceSize size, vk::SharingMode sharing_mode = vk::SharingMode::eExclusive)
            {
                return BufferAndMemoryInformation(
                    vk::MemoryPropertyFlagBits::eDeviceLocal,
                    size,
                    vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                    sharing_mode);
            }

            BufferAndMemoryInformation(void) = default;
            BufferAndMemoryInformation(const BufferAndMemoryInformation &) = default;
            BufferAndMemoryInformation &operator = (const BufferAndMemoryInformation &) = default;

            BufferAndMemoryInformation(vk::MemoryPropertyFlags properties, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::SharingMode sharing_mode)
                : _properties(properties)
                , _size(size)
                , _usage(usage)
                , _sharing_mode(sharing_mode)
            {}

            vk::MemoryPropertyFlags properties(void) const { return _properties; }
            vk::DeviceSize size(void) const { return _size; }
            vk::BufferUsageFlags usage(void) const { return _usage; }
            vk::SharingMode sharing_mode(void) const { return _sharing_mode; }

            BufferInformation buffer_Information(void) const
            {
                return BufferInformation::New(_size, _usage, _sharing_mode);
            }

            vk_utility::device::DeviceMemoryInformation memory_Information(vk_utility::device::DevicePtr device, vk::MemoryRequirements memory_requirements) const
            {
                return vk_utility::device::DeviceMemoryInformation::New(device, memory_requirements, _properties);
            }

            vk_utility::device::DeviceMemoryInformation memory_Information(vk_utility::device::DevicePtr device, vk_utility::buffer::BufferPtr buffer) const
            {
                return vk_utility::device::DeviceMemoryInformation::New(device, buffer, _properties);
            }
        };
    }
}