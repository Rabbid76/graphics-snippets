#pragma once

#include "vk_utility_object.h"
#include "vk_utility_exception.h"
#include "vk_utility_vulkan_include.h"
#include "vk_utility_buffer_information.h" 

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

            vk::DeviceSize _size;
            vk::BufferUsageFlags _usage;
            vk::SharingMode _sharing_mode;

        public:

            static BufferAndMemoryInformation New(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::SharingMode sharing_mode = vk::SharingMode::eExclusive)
            {
                return BufferAndMemoryInformation(size, usage, sharing_mode);
            }

            static BufferAndMemoryInformation NewStaging(vk::DeviceSize size, vk::SharingMode sharing_mode = vk::SharingMode::eExclusive)
            {
                return BufferAndMemoryInformation(
                    size,
                    vk::BufferUsageFlagBits::eTransferSrc,
                    sharing_mode);
            }

            static BufferAndMemoryInformation NewCoherentUniform(vk::DeviceSize size, vk::SharingMode sharing_mode = vk::SharingMode::eExclusive)
            {
                return BufferAndMemoryInformation(
                    size,
                    vk::BufferUsageFlagBits::eUniformBuffer,
                    sharing_mode);
            }

            static BufferAndMemoryInformation NewVertex(vk::DeviceSize size, vk::SharingMode sharing_mode = vk::SharingMode::eExclusive)
            {
                return BufferAndMemoryInformation(
                    size,
                    vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                    sharing_mode);
            }

            static BufferAndMemoryInformation NewIndex(vk::DeviceSize size, vk::SharingMode sharing_mode = vk::SharingMode::eExclusive)
            {
                return BufferAndMemoryInformation(
                    size,
                    vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                    sharing_mode);
            }

            BufferAndMemoryInformation(void) = default;
            BufferAndMemoryInformation(const BufferAndMemoryInformation &) = default;
            BufferAndMemoryInformation &operator = (const BufferAndMemoryInformation &) = default;

            BufferAndMemoryInformation(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::SharingMode sharing_mode)
                : _size(size)
                , _usage(usage)
                , _sharing_mode(sharing_mode)
            {}

            vk::DeviceSize size(void) const { return _size; }
            vk::BufferUsageFlags usage(void) const { return _usage; }
            vk::SharingMode sharing_mode(void) const { return _sharing_mode; }

            BufferInformation buffer_Information(void) const
            {
                return BufferInformation::New(_size, _usage, _sharing_mode);
            }
        };
    }
}