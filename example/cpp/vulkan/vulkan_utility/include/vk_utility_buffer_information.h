#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>


namespace vk_utility
{
    namespace buffer
    {
        /// <summary>
        /// buffer create information (`vk::BufferCreateInfo`)
        /// </summary>
        class BufferInformation
        {
        private:

            vk::BufferCreateInfo _buffer_infomration;

        public:

            static BufferInformation New(vk::BufferCreateInfo buffer_infomration)
            {
                return BufferInformation(buffer_infomration);
            }

            static BufferInformation New(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::SharingMode sharing_mode = vk::SharingMode::eExclusive)
            {
                std::vector<uint32_t> queue_family_indices;
                vk::BufferCreateInfo buffer_infomration
                (
                    vk::BufferCreateFlags{}, 
                    size, 
                    usage, 
                    sharing_mode, 
                    queue_family_indices
                );
                return BufferInformation(buffer_infomration);
            }

            BufferInformation(void) = default;
            BufferInformation(const BufferInformation &) = default;
            BufferInformation &operator = (const BufferInformation &) = default;

            BufferInformation(const vk::BufferCreateInfo &buffer_infomration)
                : _buffer_infomration(buffer_infomration)
            {}

            BufferInformation &operator = (const vk::BufferCreateInfo & buffer_infomration)
            {
                _buffer_infomration = buffer_infomration;
                return *this;
            }

            operator const vk::BufferCreateInfo &() const 
            {
                return _buffer_infomration;
            }

            vk::DeviceSize size(void) const
            {
                return _buffer_infomration.size;
            }
        };
    }
}