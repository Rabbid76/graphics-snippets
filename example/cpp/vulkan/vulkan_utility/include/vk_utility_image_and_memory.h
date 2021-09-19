#pragma once

#include "vk_utility_object.h"
#include "vk_utility_image.h"
#include "vk_utility_device_memory.h"
#include "vk_utility_image_and_memory_factory.h"

namespace vk_utility
{
    namespace image
    {
        class ImageAndMemory;
        using ImageAndMemoryPtr = vk_utility::Ptr<ImageAndMemory>;

        class ImageAndMemory
            : public GenericObject<int>
        {
        private:

            ImagePtr _image;
            device::DeviceMemoryPtr _device_memory;

        public:

            static ImageAndMemory New(vk::Device device, ImageAndMemoryFactory &image_and_memory_factory)
            {
                auto [image, image_memory, memory_size] = image_and_memory_factory.New(device);
                return ImageAndMemory(device, image, image_memory, memory_size);
            }

            static ImageAndMemoryPtr NewPtr(vk::Device device, ImageAndMemoryFactory& image_and_memory_factory)
            {
                return vk_utility::make_shared<ImageAndMemory>(New(device, image_and_memory_factory));
            }

            ImageAndMemory(void) = default;
            ImageAndMemory(const ImageAndMemory&) = default;
            ImageAndMemory& operator =(const ImageAndMemory&) = default;

            ImageAndMemory(vk::Device device, vk::Image image, vk::DeviceMemory device_memory, vk::DeviceSize memory_size)
                : GenericObject(0)
                , _image(vk_utility::make_shared(Image(device, image)))
                , _device_memory(vk_utility::make_shared(vk_utility::device::DeviceMemory(device, device_memory, memory_size)))
            {}

            virtual void destroy(void) override
            {
                _image = nullptr;
                _device_memory = nullptr;
            }

            auto image(void) { return _image; }
            const auto image(void) const { return _image; }
            auto device_memory(void) { return _device_memory; }
            const auto device_memory(void) const { return _device_memory; }

            auto&& detach_image(void) { return std::move(_image); }
            auto&& detach_device_memory(void) { return std::move(_device_memory); }
        };
    }
}