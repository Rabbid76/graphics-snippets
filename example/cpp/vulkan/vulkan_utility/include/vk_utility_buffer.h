#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_device.h>
#include <vk_utility_buffer_information.h>

#include <memory>


namespace vk_utility
{
    namespace buffer
    {
        class Buffer;
        using BufferPtr = vk_utility::Ptr<Buffer>;

        /// <summary>
        /// Buffer
        ///
        /// Buffers in Vulkan are regions of memory used for storing arbitrary data that can be read by the graphics card.
        /// They can be used to store vertex data, but they can also be used for many other purposes.
        /// Unlike the Vulkan objects we've been dealing with so far, buffers do not automatically allocate memory for themselves.
        /// The Vulkan API puts the programmer in control of almost everything and memory management is one of those things.
        /// </summary>
        class Buffer
            : public GenericObject<vk::Buffer>
        {
        private:

            BufferInformation _buffer_information;
            vk_utility::device::DevicePtr _device;

        public:

            static Buffer Create(vk_utility::device::DevicePtr device, const vk::BufferCreateInfo &buffer_infomration)
            {
                return Buffer(device, device->get().create_buffer(buffer_infomration), buffer_infomration);
            }

            static BufferPtr New(vk_utility::device::DevicePtr device, const vk::BufferCreateInfo &buffer_infomration)
            {
                return vk_utility::make_shared(Create(device, buffer_infomration));
            }

            Buffer(void) = default;
            Buffer(const Buffer &) = default;
            Buffer &operator = (const Buffer &) = default;

            Buffer(vk_utility::device::DevicePtr device, vk::Buffer buffer, const BufferInformation &buffer_information)
                : GenericObject(buffer)
                , _device(device)
                , _buffer_information(buffer_information)
            {}

            virtual void destroy(void) override
            {                 
                if (_vk_object && _device)
                {
                    _device->get().destroy_buffer(_vk_object);
                    _vk_object = vk::Buffer();
                }
            }

            vk::DeviceSize size(void) const { return _buffer_information.size(); }

            /// <summary>
            /// The buffer has been created, but it doesn't actually have any memory assigned to it yet.
            /// The first step of allocating memory for the buffer is to query its memory requirements
            /// using the named `vkGetBufferMemoryRequirements` function.
            /// The `vk::MemoryRequirements` `struct` has three fields:
            /// `size`: The size of the required amount of memory in bytes, may differ from bufferInfo.size.
            /// `alignment`: The offset in bytes where the buffer begins in the allocated region of memory, 
            /// depends on bufferInfo.usage and bufferInfo.flags.
            /// `memoryTypeBits`: Bit field of the memory types that are suitable for the buffer.
            /// </summary>
            /// <returns></returns>
            auto get_buffer_memory_requirements() const
            {
                return _device->get().get_buffer_memory_requirements(_vk_object);
            }
        };
    }
}