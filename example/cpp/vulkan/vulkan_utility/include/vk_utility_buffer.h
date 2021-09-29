#pragma once


#include "vk_utility_object.h"
#include "vk_utility_exception.h"
#include "vk_utility_device.h"
#include "vk_utility_buffer_factory.h"
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

            vk::Device _device;
            vk::DeviceSize _buffer_size;

        public:

            static Buffer New(vk::Device device, const BufferFactory &buffer_factory)
            {
                auto [buffer, buffer_size] = buffer_factory.New(device);
                return Buffer(device, buffer, buffer_size);
            }

            static BufferPtr NewPtr(vk::Device device, const BufferFactory& buffer_factory)
            {
                return make_shared(New(device, buffer_factory));
            }

            Buffer(void) = default;
            Buffer(const Buffer &) = default;
            Buffer &operator = (const Buffer &) = default;

            Buffer(vk::Device device, vk::Buffer buffer, vk::DeviceSize buffer_size)
                : GenericObject(buffer)
                , _device(device)
                , _buffer_size(buffer_size)
            {}

            virtual void destroy(void) override
            {                 
                if (_vk_object && _device)
                {
                    _device.destroyBuffer(_vk_object);
                    _vk_object = vk::Buffer();
                }
            }

            vk::DeviceSize size(void) const { return _buffer_size; }
        };
    }
}