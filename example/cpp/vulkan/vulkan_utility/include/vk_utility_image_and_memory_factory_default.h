#pragma once

#include "vk_utility_image_and_memory_factory.h"
#include "vk_utility_image_factory.h"
#include "vk_utility_image_device_memory_factory.h"

namespace vk_utility
{
    namespace image
    {
        class ImageAndMemoryFactoryDefault
            : public ImageAndMemoryFactory
        {
        private:

            ImageFactory *_image_factory;
            ImageDeviceMemoryFactory *_image_device_memory_factory;

        public:

            auto& set_image_factory(ImageFactory *image_factory)
            {
                _image_factory = image_factory;
                return *this;
            }

            auto& set_device_memory_factory(ImageDeviceMemoryFactory *image_device_memory_factory)
            {
                _image_device_memory_factory = image_device_memory_factory;
                return *this;
            }

            std::tuple<vk::Image, vk::DeviceMemory, vk::DeviceSize> New(vk::Device device) const override
            {
                auto image = _image_factory->New(device);
                auto [image_memory, memory_size] = _image_device_memory_factory
                    ->set_image(image)
                    .New(device);
                return std::make_tuple(image, image_memory, memory_size);
            }
        };
    }
}
