#pragma once

#include "vk_utility_buffer_factory.h"

namespace vk_utility
{
    namespace buffer
    {
        class BufferFactoryDefault
            : public BufferFactory
        {
        private:

            vk::DeviceSize _buffer_size;
            vk::BufferUsageFlags _buffer_usage;
            vk::SharingMode _sharing_mode = vk::SharingMode::eExclusive;

        public:

            BufferFactoryDefault& set_buffer_size(vk::DeviceSize buffer_size)
            {
                _buffer_size = buffer_size;
                return *this;
            }

            BufferFactoryDefault& set_staging_buffer_usage()
            {
                _buffer_usage = vk::BufferUsageFlagBits::eTransferSrc;
                return *this;
            }

            BufferFactoryDefault& set_vertex_buffer_usage()
            {
                _buffer_usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
                return *this;
            }

            BufferFactoryDefault& set_index_buffer_usage()
            {
                _buffer_usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
                return *this;
            }

            BufferFactoryDefault& set_coherent_uniform_buffer_usage()
            {
                _buffer_usage = vk::BufferUsageFlagBits::eUniformBuffer;
                return *this;
            }

            BufferFactoryDefault& set_buffer_usage(vk::BufferUsageFlags buffer_usage)
            {
                _buffer_usage = buffer_usage;
                return *this;
            }

            BufferFactoryDefault& set_sharing_mode(vk::SharingMode sharing_mode)
            {
                _sharing_mode = sharing_mode;
                return *this;
            }

            virtual std::tuple<vk::Buffer, vk::DeviceSize> New(vk::Device device) const override
            {
                std::vector<uint32_t> queue_family_indices;
                vk::BufferCreateInfo buffer_information
                (
                    vk::BufferCreateFlags{},
                    _buffer_size,
                    _buffer_usage,
                    _sharing_mode,
                    queue_family_indices
                );
                auto buffer = device.createBuffer(buffer_information);
                return { buffer, _buffer_size };
            }
        };
    }
}