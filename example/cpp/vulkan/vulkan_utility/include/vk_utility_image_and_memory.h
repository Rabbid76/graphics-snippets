#pragma once

#include "vk_utility_image.h"
#include "vk_utility_device_memory.h"

#include <memory>

namespace vk_utility
{
    namespace image
    {
        class ImageAndMemory;
        using ImageAndMemoryPtr = std::shared_ptr<ImageAndMemory>;

        class ImageAndMemory
        {
        private:

            ImagePtr _image;
            device::DeviceMemoryPtr _device_memory;

        public:

            ImageAndMemory(void) = default;
            ImageAndMemory(const ImageAndMemory&) = default;
            ImageAndMemory& operator =(const ImageAndMemory&) = default;

            ImageAndMemory(vk::Device device, vk::Image image, vk::DeviceMemory device_memory, vk::DeviceSize memory_size)
                : _image(vk_utility::make_shared(Image(device, image)))
                , _device_memory(vk_utility::make_shared(vk_utility::device::DeviceMemory(device, device_memory, memory_size)))
            {}

            virtual ~ImageAndMemory()
            {
                _image.reset();
                _device_memory.reset();
            }

            auto image(void) { return _image; }
            const auto image(void) const { return _image; }
            auto device_memory(void) { return _device_memory; }
            const auto device_memory(void) const { return _device_memory; }
        };
    }
}